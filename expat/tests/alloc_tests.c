/* Tests in the "allocation" test case for the Expat test suite
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 2001-2006 Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
   Copyright (c) 2003      Greg Stein <gstein@users.sourceforge.net>
   Copyright (c) 2005-2007 Steven Solie <steven@solie.ca>
   Copyright (c) 2005-2012 Karl Waclawek <karl@waclawek.net>
   Copyright (c) 2016-2022 Sebastian Pipping <sebastian@pipping.org>
   Copyright (c) 2017-2022 Rhodri James <rhodri@wildebeest.org.uk>
   Copyright (c) 2017      Joe Orton <jorton@redhat.com>
   Copyright (c) 2017      José Gutiérrez de la Concha <jose@zeroc.com>
   Copyright (c) 2018      Marco Maggi <marco.maggi-ipsu@poste.it>
   Copyright (c) 2019      David Loffredo <loffredo@steptools.com>
   Copyright (c) 2020      Tim Gates <tim.gates@iress.com>
   Copyright (c) 2021      Dong-hee Na <donghee.na@python.org>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
   "Software"),  to  deal in  the  Software  without restriction,  including
   without  limitation the  rights  to use,  copy,  modify, merge,  publish,
   distribute, sublicense, and/or sell copies of the Software, and to permit
   persons  to whom  the Software  is  furnished to  do so,  subject to  the
   following conditions:

   The above copyright  notice and this permission notice  shall be included
   in all copies or substantial portions of the Software.

   THE  SOFTWARE  IS  PROVIDED  "AS  IS",  WITHOUT  WARRANTY  OF  ANY  KIND,
   EXPRESS  OR IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO  THE WARRANTIES  OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
   NO EVENT SHALL THE AUTHORS OR  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
   DAMAGES OR  OTHER LIABILITY, WHETHER  IN AN  ACTION OF CONTRACT,  TORT OR
   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
   USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <string.h>

#include "expat.h"
#include "common.h"
#include "minicheck.h"
#include "dummy.h"
#include "alloc_tests.h"

void
alloc_setup(void) {
  XML_Memory_Handling_Suite memsuite = {duff_allocator, duff_reallocator, free};

  /* Ensure the parser creation will go through */
  g_allocation_count = ALLOC_ALWAYS_SUCCEED;
  g_reallocation_count = REALLOC_ALWAYS_SUCCEED;
  g_parser = XML_ParserCreate_MM(NULL, &memsuite, NULL);
  if (g_parser == NULL)
    fail("Parser not created");
}

void
alloc_teardown(void) {
  basic_teardown();
}

/* Test the effects of allocation failures on xml declaration processing */
START_TEST(test_alloc_parse_xdecl) {
  const char *text = "<?xml version='1.0' encoding='utf-8'?>\n"
                     "<doc>Hello, world</doc>";
  int i;
  const int max_alloc_count = 15;

  for (i = 0; i < max_alloc_count; i++) {
    g_allocation_count = i;
    XML_SetXmlDeclHandler(g_parser, dummy_xdecl_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* Resetting the parser is insufficient, because some memory
     * allocations are cached within the parser.  Instead we use
     * the teardown and setup routines to ensure that we have the
     * right sort of parser back in our hands.
     */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed with max allocations");
}
END_TEST

TCase *
make_alloc_test_case(Suite *s) {
  TCase *tc_alloc = tcase_create("allocation tests");

  suite_add_tcase(s, tc_alloc);
  tcase_add_checked_fixture(tc_alloc, alloc_setup, alloc_teardown);

  tcase_add_test(tc_alloc, test_alloc_parse_xdecl);

  return tc_alloc; /* TEMPORARY: this will become a void function */
}
