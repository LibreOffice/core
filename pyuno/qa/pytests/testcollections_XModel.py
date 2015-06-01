#!/usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest
import uno

from testcollections_base import CollectionsTestBase


# Tests behaviour of objects implementing XModel

class TestXModel(CollectionsTestBase):

    # Tests syntax:
    #    with obj: ...
    # For:
    #    Normal operation
    def test_XModel_with(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        before = doc.hasControllersLocked()
        with doc:
            during = doc.hasControllersLocked()
        after = doc.hasControllersLocked()

        # Then
        self.assertFalse(before)
        self.assertTrue(during)
        self.assertFalse(after)

    # Tests syntax:
    #    with obj: ...
    # For:
    #    Exception thrown
    def test_XModel_with_exception(self):
        # Given
        doc = self.createBlankTextDocument()

        # When
        before = doc.hasControllersLocked()
        try:
            with doc:
                during = doc.hasControllersLocked()
                raise RuntimeError
        except:
            pass
        after = doc.hasControllersLocked()

        # Then
        self.assertFalse(before)
        self.assertTrue(during)
        self.assertFalse(after)


if __name__ == '__main__':
    unittest.main()

# vim:set shiftwidth=4 softtabstop=4 expandtab: