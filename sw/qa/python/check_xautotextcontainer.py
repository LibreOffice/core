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

    def test_removeByName(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        xAutoTextContainer.insertNewByName("testname1")
        xAutoTextContainer.removeByName("testname1")

    def test_removeByName_Unknown(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        try:
            xAutoTextContainer.removeByName("Some Unknown Name!!!")
        except Exception:
            pass

    def test_insertNewByName(self):
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

    def test_insertNewByName_Failed(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        # group name must contain a-z, A-z, '_', ' ' only
        xNames = ['', 'Name!!!', 'Red & White', 'Name.With.Dot', 'Name-2']
        for xName in xNames:
            try:
                xGroup = xAutoTextContainer.insertNewByName(xName)
                self.assertIsNone(xGroup)
            except Exception:
                pass


if __name__ == '__main__':
    unittest.main()
