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
from com.sun.star.beans import PropertyValue


# Tests behaviour of objects implementing XIndexContainer using the new-style
# collection accessors
# The objects chosen have no special meaning, they just happen to implement the
# tested interfaces

class TestXIndexContainer(CollectionsTestBase):

    def generateTestPropertyValues(self, count):
        sm = self.context.ServiceManager
        values = sm.createInstanceWithContext("com.sun.star.document.IndexedPropertyValues", self.context)
        for i in range(count):
            properties = (PropertyValue(Name='n'+str(i), Value='v'+str(i)),)
            uno.invoke(values, "insertByIndex", (i, uno.Any("[]com.sun.star.beans.PropertyValue", properties)))
        return values

    def generateTestTuple(self, values):
        properties = []
        for i in values:
            properties.append((PropertyValue(Name='n'+str(i), Value='v'+str(i)),))
        return tuple(properties)

    def assignValuesTestFixture(self, count, key, values, expected):
        # Given
        propertyValues = self.generateTestPropertyValues(count)
        if type(values) is list:
            toAssign = self.generateTestTuple(values)
        else:
            toAssign = values
        if not (isinstance(expected, Exception)):
            toCompare = self.generateTestTuple(expected)

        # When
        captured = None
        try:
            propertyValues[key] = toAssign
        except Exception as e:
            captured = e

        # Then
        if isinstance(expected, Exception):
            # expected is exception
            self.assertNotEqual(None, captured)
            self.assertEqual(type(expected).__name__, type(captured).__name__)
        else:
            # expected is list
            self.assertEqual(None, captured)
            self.assertEqual(len(expected), propertyValues.getCount())
            for i in range(propertyValues.getCount()):
                self.assertEqual(toCompare[i][0].Name, propertyValues.getByIndex(i)[0].Name)

    def deleteValuesTestFixture(self, count, key, expected):
        # Given
        propertyValues = self.generateTestPropertyValues(count)
        if not (isinstance(expected, Exception)):
            toCompare = self.generateTestTuple(expected)

        # When
        captured = None
        try:
            del propertyValues[key]
        except Exception as e:
            captured = e

        # Then
        if isinstance(expected, Exception):
            # expected is exception
            self.assertNotEqual(None, captured)
            self.assertEqual(type(expected).__name__, type(captured).__name__)
        else:
            # expected is list
            self.assertEqual(None, captured)
            self.assertEqual(len(expected), propertyValues.getCount())
            for i in range(propertyValues.getCount()):
                self.assertEqual(toCompare[i][0].Name, propertyValues.getByIndex(i)[0].Name)

    # Tests syntax:
    #    obj[2:4] = val1,val2        # Replace by slice
    #    obj[2:3] = val1,val2        # Insert/replace by slice
    #    obj[2:2] = (val,)           # Insert by slice
    #    obj[2:4] = (val,)           # Replace/delete by slice
    #    obj[2:3] = ()               # Delete by slice (implicit)
    # For:
    #    Cases requiring sequence type coercion
    def test_XIndexContainer_AssignSlice(self):
        baseMax = 5
        assignMax = 5
        for i in range(baseMax):
            for j in [x for x in range(-baseMax-2, baseMax+3)] + [None]:
                for k in [x for x in range(-baseMax-2, baseMax+3)] + [None]:
                    key = slice(j, k)
                    for l in range(assignMax):
                        assign = [y+100 for y in range(l)]
                        expected = list(range(i))
                        expected[key] = assign
                        self.assignValuesTestFixture(i, key, assign, expected)

    # Tests syntax:
    #    obj[2:4] = val1,val2        # Replace by slice
    #    obj[2:3] = val1,val2        # Insert/replace by slice
    #    obj[2:2] = (val,)           # Insert by slice
    # For:
    #    Cases not requiring sequence type coercion
    #    Invalid values
    def test_XIndexContainer_AssignSlice_Invalid(self):
        self.assignValuesTestFixture(2, slice(0, 2), None, TypeError())
        self.assignValuesTestFixture(2, slice(0, 2), 'foo', TypeError())
        self.assignValuesTestFixture(2, slice(0, 2), 12.34, TypeError())
        self.assignValuesTestFixture(2, slice(0, 2), {'a': 'b'}, TypeError())
        self.assignValuesTestFixture(2, slice(0, 2), ('foo',), TypeError())
        self.assignValuesTestFixture(2, slice(0, 2), ('foo', 'foo'), TypeError())

    # Tests syntax:
    #    obj[2:2] = (val,)           # Insert by slice
    # For:
    #    Cases not requiring sequence type coercion
    def test_XIndexContainer_AssignSlice_NoCoercion(self):
        # Given
        doc = self.createBlankTextDocument()
        form = doc.createInstance("com.sun.star.form.component.DataForm")
        form.Name = 'foo'

        # When
        doc.DrawPage.Forms[0:0] = (form,)

        # Then
        self.assertEqual('foo', doc.DrawPage.Forms[0].Name)

    # Tests syntax:
    #    obj[0:3:2] = val1,val2      # Replace by extended slice
    # For:
    #    Cases requiring sequence type coercion
    def test_XIndexContainer_AssignExtendedSlice(self):
        baseMax = 5
        assignMax = 5
        for i in range(baseMax):
            for j in [x for x in range(-baseMax-2, baseMax+3)] + [None]:
                for k in [x for x in range(-baseMax-2, baseMax+3)] + [None]:
                    for l in [-2, -1, 1, 2]:
                        key = slice(j, k, l)
                        for m in range(assignMax):
                            assign = [y+100 for y in range(m)]
                            expected = list(range(i))
                            try:
                                expected[key] = assign
                            except Exception as e:
                                expected = e

                            self.assignValuesTestFixture(i, key, assign, expected)

    # Tests syntax:
    #    del obj[0]                  # Delete by index
    def test_XIndexContainer_DelIndex(self):
        baseMax = 5
        for i in range(baseMax):
            for j in [x for x in range(-baseMax-2, baseMax+3)]:
                expected = list(range(i))
                try:
                    del expected[j]
                except Exception as e:
                    expected = e
                self.deleteValuesTestFixture(i, j, expected)

    # Tests syntax:
    #    del obj[2:4]                # Delete by slice
    def test_XIndexContainer_DelSlice(self):
        baseMax = 5
        for i in range(baseMax):
            for j in [x for x in range(-baseMax-2, baseMax+3)] + [None]:
                for k in [x for x in range(-baseMax-2, baseMax+3)] + [None]:
                    key = slice(j, k)
                    expected = list(range(i))
                    try:
                        del expected[key]
                    except Exception as e:
                        expected = e
                    self.deleteValuesTestFixture(i, key, expected)


if __name__ == '__main__':
    unittest.main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
