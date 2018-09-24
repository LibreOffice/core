#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
import unittest
from org.libreoffice.unotest import UnoInProcess


class XAutoTextContainer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls.xDoc = cls._uno.openEmptyWriterDoc()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_insertNewByName(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        # group name must contain a-z, A-z, '_', ' ' only
        xNames = ['Name', 'TEST', 'Name2', '_With_underscore_', 'with space', '123456']
        for xName in xNames:
            xAutoTextContainer.insertNewByName(xName)
            xAutoTextContainer.removeByName(xName)

    def test_insertNewByName_Spaces(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        # add
        xName = '  spaces  '
        xAutoTextContainer.insertNewByName(xName)

        # try to remove
        try:
            xAutoTextContainer.removeByName(xName)
        except Exception:
            pass

        # remove trimmed
        xAutoTextContainer.removeByName('spaces')

    def test_insertNewByName_Several(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        xAutoTextGroup1 = xAutoTextContainer.insertNewByName("atc_name1")
        xAutoTextGroup2 = xAutoTextContainer.insertNewByName("atc_name2")
        xAutoTextGroup3 = xAutoTextContainer.insertNewByName("atc_name3")

        self.assertEqual("atc_name1*0", xAutoTextGroup1.getName())
        self.assertEqual("atc_name2*0", xAutoTextGroup2.getName())
        self.assertEqual("atc_name3*0", xAutoTextGroup3.getName())

        xAutoTextContainer.removeByName("atc_name1")
        xAutoTextContainer.removeByName("atc_name2")
        xAutoTextContainer.removeByName("atc_name3")

    def test_insertNewByName_DifferentCase(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        xAutoTextGroup1 = xAutoTextContainer.insertNewByName("myname")
        xAutoTextGroup2 = xAutoTextContainer.insertNewByName("MYNAME")
        xAutoTextGroup3 = xAutoTextContainer.insertNewByName("MyName")

        self.assertEqual("myname*0", xAutoTextGroup1.getName())
        self.assertNotEqual("MYNAME*0", xAutoTextGroup2.getName())
        self.assertNotEqual("MyName*0", xAutoTextGroup3.getName())
        self.assertEqual("group", xAutoTextGroup2.getName()[:5])
        self.assertEqual("group", xAutoTextGroup3.getName()[:5])

        xAutoTextContainer.removeByName("myname")

        xName = xAutoTextGroup2.getName()
        xName = xName[:xName.find('*')]
        xAutoTextContainer.removeByName(xName)

        xName = xAutoTextGroup3.getName()
        xName = xName[:xName.find('*')]
        xAutoTextContainer.removeByName(xName)

    def test_insertNewByName_Failed(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        # group name must contain a-z, A-z, '_', ' ' only
        xNames = ['', 'Name!!!', 'Red & White', 'Name.With.Dot', 'Name-2', 'A1:B1']
        for xName in xNames:
            try:
                xGroup = xAutoTextContainer.insertNewByName(xName)
                self.assertIsNone(xGroup)
            except Exception:
                pass

    def test_removeByName_Unknown(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        try:
            xAutoTextContainer.removeByName("Some Unknown Name")
        except Exception:
            pass

    def test_removeByName_DifferentCases(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        xAutoTextContainer.insertNewByName('GroupName')

        try:
            xAutoTextContainer.removeByName('groupname')
        except Exception:
            pass

        try:
            xAutoTextContainer.removeByName('GROUPNAME')
        except Exception:
            pass

        xAutoTextContainer.removeByName('GroupName')


if __name__ == '__main__':
    unittest.main()
