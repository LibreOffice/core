# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from contextlib import contextmanager

# Calls UITest.close_doc at exit
@contextmanager
def create_doc_in_start_center(testCase, app):
    testCase.ui_test.create_doc_in_start_center(app)
    component = testCase.ui_test.get_component()
    try:
        yield component
    finally:
        testCase.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
