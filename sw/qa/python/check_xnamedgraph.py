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


class XNamedGraph(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_getStatements(self):
        xDMA = self._uno.openTemplateFromTDOC("XNamedGraph.ott")
        xRepo = xDMA.getRDFRepository()
        xGraphs = xRepo.getGraphNames()

        all = self.getStatementsCount(xGraphs, xDMA, None, None)
        self.assertTrue(all > 0)

        # check that we have some node from the second RDF graph
        # (but not any from the list of LO standard nodes)
        DATE_URI_STR = "http://www.wollmux.org/WollMuxMetadata#WollMuxVersion"
        xUri = self.nameToUri(DATE_URI_STR)

        onlyWollMux = self.getStatementsCount(xGraphs, xDMA, xUri, None)
        self.assertTrue(onlyWollMux > 0)
        self.assertTrue(onlyWollMux < all)

    def test_Statements_AddRemove(self):
        # take any first graph
        xDMA = self._uno.openTemplateFromTDOC("XNamedGraph.ott")
        xGraph = self.getAnyGraph(xDMA)

        DATE_URI_STR = "http://www.example.com/Metadata#Version"
        OBJECT_STR = "foo"

        # not exist => add => remove
        self.assertFalse(self.hasStatement(xDMA, xGraph, DATE_URI_STR))
        self.addStatement(xDMA, xGraph, DATE_URI_STR, OBJECT_STR)
        self.assertTrue(self.hasStatement(xDMA, xGraph, DATE_URI_STR))
        self.removeStatement(xDMA, xGraph, DATE_URI_STR, None)
        self.assertFalse(self.hasStatement(xDMA, xGraph, DATE_URI_STR))

    def test_Statements_RemoveByObject(self):
        # take any first graph
        xDMA = self._uno.openTemplateFromTDOC("XNamedGraph.ott")
        xGraph = self.getAnyGraph(xDMA)

        DATE_URI_STR = "http://www.example.com/Metadata#Version"
        OBJECT_STR = "foo"

        # remove by object
        self.assertFalse(self.hasStatement(xDMA, xGraph, DATE_URI_STR))
        self.addStatement(xDMA, xGraph, DATE_URI_STR, OBJECT_STR)
        self.assertTrue(self.hasStatement(xDMA, xGraph, DATE_URI_STR))
        self.removeStatement(xDMA, xGraph, None, OBJECT_STR)
        self.assertFalse(self.hasStatement(xDMA, xGraph, DATE_URI_STR))

    def test_Statements_RemoveByObject(self):
        # take any first graph
        xDMA = self._uno.openTemplateFromTDOC("XNamedGraph.ott")
        xGraph = self.getAnyGraph(xDMA)

        DATE_URI_STR_1 = "http://www.example.com/Metadata#Version"
        DATE_URI_STR_2 = "http://www.example.com/Metadata#Second"
        OBJECT_STR = "foo"

        # remove several with one call
        self.assertFalse(self.hasStatement(xDMA, xGraph, DATE_URI_STR_1))
        self.assertFalse(self.hasStatement(xDMA, xGraph, DATE_URI_STR_2))
        self.addStatement(xDMA, xGraph, DATE_URI_STR_1, OBJECT_STR)
        self.addStatement(xDMA, xGraph, DATE_URI_STR_2, OBJECT_STR)
        self.assertTrue(self.hasStatement(xDMA, xGraph, DATE_URI_STR_1))
        self.assertTrue(self.hasStatement(xDMA, xGraph, DATE_URI_STR_2))
        self.removeStatement(xDMA, xGraph, None, OBJECT_STR)
        self.assertFalse(self.hasStatement(xDMA, xGraph, DATE_URI_STR_1))
        self.assertFalse(self.hasStatement(xDMA, xGraph, DATE_URI_STR_2))

    def getAnyGraph(self, xDMA):
        xRepo = xDMA.getRDFRepository()
        xGraphs = xRepo.getGraphNames()
        self.assertTrue(len(xGraphs) > 0)
        xGraph = xRepo.getGraph(xGraphs[0])
        self.assertIsNotNone(xGraph)
        return xGraph

    def getStatementsCount(self, xGraphs, xDMA, xPredicate, xObject):
        count = 0
        xRepo = xDMA.getRDFRepository()

        for xGraphUri in xGraphs:
            xGraph = xRepo.getGraph(xGraphUri)

            xStatements = xGraph.getStatements(xDMA, xPredicate, xObject)
            self.assertIsNotNone(xStatements)

            if xPredicate is None:
                self.assertTrue(xStatements.hasMoreElements())

            for xStatement in xStatements:
                count += 1

        return count

    def hasStatement(self, xDMA, xGraph, xPredicateName):
        xPredicate = self.nameToUri(xPredicateName)
        self.assertIsNotNone(xPredicate)

        xStatements = xGraph.getStatements(xDMA, xPredicate, None)
        self.assertIsNotNone(xStatements)
        return xStatements.hasMoreElements()

    def nameToUri(self, xPredicateName):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xUri = xServiceManager.createInstance("com.sun.star.rdf.URI")
        xUri.initialize((xPredicateName,))
        return xUri

    def stringToLiteral(self, xString):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xLiteral = xServiceManager.createInstance("com.sun.star.rdf.Literal")
        xLiteral.initialize((xString,))
        return xLiteral

    def addStatement(self, xDMA, xGraph, xPredicateName, xObjectStr):
        xPredicate = self.nameToUri(xPredicateName)
        self.assertIsNotNone(xPredicate)

        xObject = self.stringToLiteral(xObjectStr)
        self.assertIsNotNone(xObject)

        xGraph.addStatement(xDMA, xPredicate, xObject)

    def removeStatement(self, xDMA, xGraph, xPredicateName, xObjectStr):
        xPredicate = None
        if xPredicateName is not None:
            xPredicate = self.nameToUri(xPredicateName)

        xObject = None
        if xObjectStr is not None:
            xObject = self.stringToLiteral(xObjectStr)

        xGraph.removeStatements(xDMA, xPredicate, xObject)


if __name__ == '__main__':
    unittest.main()
