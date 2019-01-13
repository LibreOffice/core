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
import os
import os.path
from urllib.parse import quote

import uno
from com.sun.star.beans import PropertyValue
from org.libreoffice.unotest import UnoInProcess

class LoadSaveTest(unittest.TestCase):

    def make_property_value(self, name, value):
        return uno.createUnoStruct("com.sun.star.beans.PropertyValue", name, 0, value, 0)

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

        cls.m_fileURL = "file:/"
        cls.m_SourceDir = "FIXME"
        cls.m_TargetDir = "/tmp/out/"
        cls.dirs = []
        cls.files = []
        cls.file_name = ""

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def setUp(self):
        self.xDoc = self.__class__._uno.openEmptyWriterDoc()
        self.xContext = self.__class__._uno.getContext()

    def tearDown(self):
        self.xDoc.close(True)

    def test_load_store(self):
        #Set up instance variables.
        self.dirs, self.files = [], []

        #self.get_dir_and_file_names(self.m_SourceDir)

        #self.make_directories(self.m_TargetDir)

        #Get a list of files from the testdocuments.
        self.files = os.listdir("../../../../sw/qa/python/testdocuments")

        print(self.files)

        for file_name in self.files:
            self.tst_document(file_name)

    def tst_document(self, file_name):
        try:
            props = [("ReadOnly", True)]

            load_props = tuple([self.make_property_value(name, value) for (name, value) in props])

            m_xMSF = self.xContext.ServiceManager
            desktop = m_xMSF.createInstanceWithContext('com.sun.star.frame.Desktop', self.xContext)

            filepath = os.path.abspath(file_name)

            if os.name == "nt":
                source_file = "file:///"+quote(filepath)

            else:
                source_file = "file://"+quote(filepath)

            print("test", source_file)

            #FIXME: segfault here for some reason.
            temp_xDoc = desktop.loadComponentFromURL(source_file, "_blank", 0,
                                                     [])#load_props)

            assert temp_xDoc

            print("test2")

            if os.name == "nt":
                target_file = ''.join(("file:///", self.m_TargetDir,
                                       quote(self.m_SourceDir), "/",
                                       quote(file_name)))

            else:
                target_file = ''.join(("file://", quote(self.m_TargetDir),
                                       quote(self.m_SourceDir), "/",
                                       quote(file_name)))

            p1 = PropertyValue()
            property_value = uno.Any("[]com.sun.star.beans.PropertyValue", (p1,))
            uno.invoke(self.xDoc, "storeToURL", (target_file, property_value))

        except Exception:
            raise

    def get_dir_and_file_names(self, _dir):
        """
        Make a big list of files in this directory, and add
        to self.files. Calls itself recursively when directories
        are found in th given directory.
        """

        if os.path.isdir(_dir):
            self.dirs.append(_dir)
            subfiles = os.listdir(_dir)

            #Add a '/' to the end of the directory, so we can
            #assemble the full path to any files inside.
            if not _dir[-1] == "/":
                _dir += "/"

            for subfile in subfiles:
                subfile_name = _dir+subfile
                self.get_dir_and_file_names(subfile_name)

        elif os.path.isfile(_dir):
            self.files.append(_dir.split('/')[-1])

    def make_directories(self, target):
        """
        Recreates the folder structure at the source, under the
        destination folder.
        """

        if not os.path.exists(target):
            os.mkdir(target)
            self.assertTrue(os.path.exists(target))

        for _dir in self.dirs:
            if not os.path.exists(target + _dir):
                os.mkdir(target + _dir)
                self.assertTrue(os.path.exists(target + _dir))

        #What are these 2 lines of code for?
        #target_path = target+_dir+"/"+self.m_SourceDir+".odt"
        #root = open(target_path, 'a')
