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

from com.sun.star.lang import XMultiServiceFactory
from com.sun.star.text import XTextDocument
from com.sun.star.uno import UnoRuntime
from com.sun.star.text import XTextFramesSupplier
from com.sun.star.text import XTextGraphicObjectsSupplier
from com.sun.star.text import XTextEmbeddedObjectsSupplier
from com.sun.star.container import XNameAccess 
from selenium.common.exceptions import NoSuchElementException
from com.sun.star.container import XIndexAccess 
import unittest
import unohelper
import os
from org.libreoffice.unotest import UnoInProcess
#from org.openoffice.test.OfficeConnection;

class CheckFlies(unittest.TestCase):
    document = None
    #connection = OfficeConnection()

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        #cls._xDoc = cls._uno.openWriterTemplateDoc("CheckFlies.odt")

    def setUpDocuments():
        doc = UnoRuntime.queryInterface(type(XMultiServiceFactory).__name__, connection.getComponentContext().getServiceManager())
        document = util.WriterTools.loadTextDoc(doc, TestDocument.getUrl("CheckFlies.odt"))
            

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()
 
    def checkFlies():
        xTFS = UnoRuntime.queryInterface(type(XTextFramesSupplier).__name__, document)
        checkTextFrames(xTFS)
        xTGOS = UnoRuntime.queryInterface(type(XTextGraphicObjectsSupplier).__name__,document)
        checkGraphicFrames(xTGOS)
        xTEOS = UnoRuntime.queryInterface(type(XTextEmbeddedObjectsSupplier).__name__, document)
        checkEmbeddedFrames(xTEOS)


    def checkEmbeddedFrames(xTGOS):
        vExpectedEmbeddedFrames = ["Object1"]
        nEmbeddedFrames = len(vExpectedEmbeddedFrames)
        xEmbeddedFrames = xTGOS.getEmbeddedObjects()
        nCurrentFrameIdx=0
 
        for sFrameName in xEmbeddedFrames.getElementNames():
            self.assertTrue("Unexpected frame name", vExpectedEmbeddedFrames.remove(sFrameName))
            xEmbeddedFrames.getByName(sFrameName)
            self.assertTrue("Could not find embedded frame by name.", xEmbeddedFrames.hasByName(sFrameName))
 
        self.assertTrue("Missing expected embedded frames.",  vExpectedEmbeddedFrames.isEmpty())

        try:
            xEmbeddedFrames.getByName("Nonexisting embedded frame")
            self.fail("Got nonexisting embedded frame")
        except NoSuchElementException:
            raise
      
        self.assertFalse("Has nonexisting embedded frame", xEmbeddedFrames.hasByName("Nonexisting embedded frame"))

        xEmbeddedFramesIdx = UnoRuntime.queryInterface(type(XIndexAccess).__name__,xEmbeddedFrames)
        
        self.assertEqual( "Unexpected number of embedded frames reported.", nEmbeddedFrames, xEmbeddedFramesIdx.getCount())

        for nCurrentFrameIdx in range(xEmbeddedFramesIdx.getCount()):
            xEmbeddedFramesIdx.getByIndex(nCurrentFrameIdx)
   
    def checkGraphicFrames(xTGOS):
        vExpectedGraphicFrames = ["graphics1"]
        nGraphicFrames = len(vExpectedGraphicFrames)
        xGraphicFrames = xTGOS.getGraphicObjects()
        nCurrentFrameIdx = 0
        for sFrameName in xGraphicFrames.getElementNames():
            self.assertTrue(
                "Unexpected frame name",
                vExpectedGraphicFrames.remove(sFrameName))
            xGraphicFrames.getByName(sFrameName)
            self.assertTrue(
                "Could not find graphics frame by name.",
                xGraphicFrames.hasByName(sFrameName))
       
        self.assertTrue(
            "Missing expected graphics frames.",
            vExpectedGraphicFrames.isEmpty())
        
        try:
            xGraphicFrames.getByName("Nonexisting graphics frame")
            self.fail("Got nonexisting graphics frame")
        except NoSuchElementException:
            raise

        self.assertFalse(
            "Has nonexisting graphics frame",
            xGraphicFrames.hasByName("Nonexisting graphics frame"))

        xGraphicFramesIdx = UnoRuntime.queryInterface(
            type(XIndexAccess).__name__, xGraphicFrames)

        self.assertEqual(
            "Unexpected number of graphics frames reported.", nGraphicFrames,xGraphicFramesIdx.getCount())

        for nCurrentFrameIdx in range(xGraphicFramesIdx.getCount()):
            xGraphicFramesIdx.getByIndex(nCurrentFrameIdx);
    
    def checkTextFrames(xTFS):
        vExpectedTextFrames= ["Frame1" , "Frame2"]
        nTextFrames = len(vExpectedTextFrames)
        xTextFrames = xTFS.getTextFrames()
        nCurrentFrameIdx=0

        for( sFrameName in xTextFrames.getElementNames()):
            self.assertTrue(
                "Unexpected frame name",
                vExpectedTextFrames.remove(sFrameName))
            xTextFrames.getByName(sFrameName)
            self.assertTrue(
                "Could not find text frame by name.",
                xTextFrames.hasByName(sFrameName))
       
        self.assertTrue(
            "Missing expected text frames.", vExpectedTextFrames.isEmpty())
        try:
            xTextFrames.getByName("Nonexisting Textframe")
            self.fail("Got nonexisting text frame.")
        except NoSuchElementException:
            raise

        self.assertFalse(
            "Has nonexisting text frame.",
            xTextFrames.hasByName("Nonexisting text frame"))

        xTextFramesIdx = UnoRuntime.queryInterface(
           type(XIndexAccess).__name__, xTextFrames)

        self.assertEqual(
            "Unexpected number of text frames reported.", nTextFrames,
            xTextFramesIdx.getCount());
        
        for nCurrentFrameIdx in range(xTextFramesIdx.getCount()):
            xTextFramesIdx.getByIndex(nCurrentFrameIdx)
   

if __name__ == "__main__":
    unittest.main()
