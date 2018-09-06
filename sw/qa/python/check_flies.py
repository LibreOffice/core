'''
  This is file is part of the LibreOffice project.

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

from org.libreoffice.unotest import UnoInProcess
import unittest


class CheckFlies(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_checkFlies(self):
        document = self.__class__._uno.openDocFromTDOC("CheckFlies.odt")
        xTFS = document
        self.checkTextFrames(xTFS)
        xTGOS = document
        self.checkGraphicFrames(xTGOS)
        xTEOS = document
        self.checkEmbeddedFrames(xTEOS)

    def checkEmbeddedFrames(self, xTGOS):
        vExpectedEmbeddedFrames = ["Object1"]
        nEmbeddedFrames = len(vExpectedEmbeddedFrames)
        xEmbeddedFrames = xTGOS.getEmbeddedObjects()
        nCurrentFrameIdx = 0

        print (xEmbeddedFrames)
        for sFrameName in xEmbeddedFrames.getElementNames():
            vExpectedEmbeddedFrames.remove(sFrameName)
            # raises ValueError if not found
            print (sFrameName)
            xEmbeddedFrames[sFrameName]
            self.assertTrue(xEmbeddedFrames.hasByName(sFrameName),
                            "Could not find embedded frame by name.")

        self.assertTrue(not(vExpectedEmbeddedFrames),
                        "Missing expected embedded frames.")

        xEmbeddedFramesIdx = xEmbeddedFrames

        self.assertEqual(nEmbeddedFrames, len(xEmbeddedFramesIdx),
                         "Unexpected number of embedded frames reported")

        for nCurrentFrameIdx in range(len(xEmbeddedFramesIdx)):
            xEmbeddedFramesIdx[nCurrentFrameIdx]

    def checkGraphicFrames(self, xTGOS):
        vExpectedGraphicFrames = ["graphics1"]
        nGraphicFrames = len(vExpectedGraphicFrames)
        xGraphicFrames = xTGOS.getGraphicObjects()
        nCurrentFrameIdx = 0
        for sFrameName in xGraphicFrames.getElementNames():
            vExpectedGraphicFrames.remove(sFrameName)
            # raises ValueError if not found
            xGraphicFrames[sFrameName]
            self.assertTrue(
                sFrameName in xGraphicFrames,
                "Could not find graphics frame by name.")
        self.assertTrue(
            not(vExpectedGraphicFrames),
            "Missing expected graphics frames.")

        xGraphicFramesIdx = xGraphicFrames
        self.assertEqual(nGraphicFrames, len(xGraphicFramesIdx),
                         "Unexpected number of graphics frames reported")

        for nCurrentFrameIdx in range(len(xGraphicFramesIdx)):
            xGraphicFramesIdx[nCurrentFrameIdx]

    def checkTextFrames(self, xTFS):
        vExpectedTextFrames = ["Frame1", "Frame2"]
        nTextFrames = len(vExpectedTextFrames)
        xTextFrames = xTFS.getTextFrames()
        nCurrentFrameIdx = 0

        for sFrameName in xTextFrames.getElementNames():
            vExpectedTextFrames.remove(sFrameName)
            # raises ValueError if not found
            xTextFrames[sFrameName]
            self.assertTrue(
                sFrameName in xTextFrames,
                "Could not find text frame by name.")

        self.assertTrue(
            not(vExpectedTextFrames), "Missing expected text frames.")

        xTextFramesIdx = xTextFrames

        self.assertEqual(nTextFrames, len(xTextFrames),
                         "Unexpected number of text frames reported")

        for nCurrentFrameIdx in range(len(xTextFramesIdx)):
            xTextFramesIdx[nCurrentFrameIdx]


if __name__ == "__main__":
    unittest.main()
