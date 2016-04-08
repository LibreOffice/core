'''
  This file is part of the LibreOffice project.

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This file incorporates work covered by the following license notice:

    Licensed to the Apache Software Foundation (ASF) under one or more
    contributor license agreements. See the NOTICE file distributed
    with this work for additional information regarding copyright
    ownership. The ASF licenses this file to you under the Apache
    License, Version 2.0 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.apache.org/licenses/LICENSE-2.0 .
'''

import unittest
import unohelper
import os
import os.path
import time
import uno
from com.sun.star.lang import EventObject
from com.sun.star.lang import XMultiServiceFactory
from com.sun.star.lang import XComponent
from com.sun.star.beans import PropertyValue
from com.sun.star.frame import XGlobalEventBroadcaster
from com.sun.star.frame import XStorable
from com.sun.star.document import DocumentEvent
from com.sun.star.document import XDocumentEventListener
from org.libreoffice.unotest import UnoInProcess
from urllib.parse import quote


class LoadSaveTest(unittest.TestCase):

    def mkPropertyValue(self, name, value):
        return uno.createUnoStruct("com.sun.star.beans.PropertyValue", name, 0, value, 0)

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls.xDoc = cls._uno.openEmptyWriterDoc()
        cls.xContext = cls._uno.getContext()

        cls.m_fileURL = "file:/"
        cls.m_SourceDir = "FIXME"
        cls.m_TargetDir = "/tmp/out/"
        cls.dirs = []
        cls.files = []
        cls.fileName = ""

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def testLoadStore(self):
        self.dirs, self.files = self.getDirAndFile(self.m_SourceDir)
        self.makeDirs(self.m_TargetDir)
        for self.fileName in self.files:
            self.tstDoc()

    def tstDoc(self):
        try:
            props = [("ReadOnly", True)]
            loadProps = tuple([self.mkPropertyValue(name, value) for (name, value) in props])

            m_xMSF = self.xContext.ServiceManager
            desktop = m_xMSF.createInstanceWithContext('com.sun.star.frame.Desktop', self.xContext)

            filepath = os.path.abspath("FIXME")
            if os.name == "nt":
                sourceFile = "file:///" + filepath + "/" + quote(self.fileName)
            else:
                sourceFile = "file://" + quote(filepath) + "/" + quote(self.fileName)
            self.xDoc = desktop.loadComponentFromURL(sourceFile, "_blank", 0, loadProps)
            assert(self.xDoc)

            if os.name == "nt":
                targetFile = "file:///" + self.m_TargetDir + quote(self.m_SourceDir) + "/" + quote(self.fileName)
            else:
                targetFile = "file://" +
                            quote(self.m_TargetDir) +
                            quote(self.m_SourceDir) +
                            "/" +
                            quote(self.fileName)

            p1 = PropertyValue()
            PropValue = uno.Any("[]com.sun.star.beans.PropertyValue", (p1,))
            uno.invoke(self.xDoc, "storeToURL", (targetFile, PropValue))

        except Exception:
            raise

    def getDirAndFile(self, dir):

        root2 = os.mkdir(dir)
        root = open(dir + "/" + dir + ".odt", 'a')

        self.getDirAndFileNames(dir)
        return self.dirs, self.files

    def getDirAndFileNames(self, fdName):

        if os.path.isdir(fdName):
            self.dirs.append(fdName)
            subfiles = os.listdir(fdName)

            if not fdName[-1] == "/":
                fdName += "/"

            for subfile in subfiles:
                subfileName = fdName + subfile
                self.getDirAndFileNames(subfileName)

        if os.path.isfile(fdName):
            self.files.append(fdName.split('/')[-1])

    def makeDirs(self, target):
        if not os.path.exists(target):
            os.mkdir(target)
            self.assertTrue(os.path.exists(target))

        for dir in self.dirs:
            if not os.path.exists(target + dir):
                f = os.mkdir(target + dir)
                self.assertTrue(os.path.exists(target + dir))

        root = open(target + dir + "/" + self.m_SourceDir + ".odt", 'a')
        filepath = os.path.abspath(target + dir + "/" + self.m_SourceDir + ".odt")
