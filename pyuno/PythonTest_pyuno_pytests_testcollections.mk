# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_PythonTest_PythonTest,pyuno_pytests_testcollections))

$(eval $(call gb_PythonTest_add_modules,pyuno_pytests_testcollections,$(SRCDIR)/pyuno/qa/pytests,\
    testcollections_XIndexAccess \
    testcollections_XIndexReplace \
    testcollections_XIndexContainer \
    testcollections_XNameAccess \
    testcollections_XNameReplace \
    testcollections_XNameContainer \
    testcollections_XEnumerationAccess \
    testcollections_XEnumeration \
    testcollections_XCellRange \
    testcollections_mixednameindex \
    testcollections_misc \
    testcollections_misc2 \
))

# vim: set noet sw=4 ts=4:
