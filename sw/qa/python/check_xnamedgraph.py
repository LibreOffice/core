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
        xDoc = self._uno.openTemplateFromTDOC("XNamedGraph.ott")
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xDMA = xDoc
        xRepo = xDMA.getRDFRepository()
        xGraphs = xRepo.getGraphNames()

        all = self.getStatementsCount(xGraphs, xDMA, None, None)
        self.assertTrue(all > 0)

        WOLLMUX_DATEN_URI_STR = "http://www.wollmux.org/WollMuxMetadata#WollMuxVersion"
        xWollMuxUri = xServiceManager.createInstance("com.sun.star.rdf.URI")
        xWollMuxUri.initialize((WOLLMUX_DATEN_URI_STR,))

        onlyWollMux = self.getStatementsCount(xGraphs, xDMA, xWollMuxUri, None)
        self.assertTrue(onlyWollMux > 0)
        self.assertTrue(onlyWollMux < all)

    def getStatementsCount(self, xGraphs, xDMA, xPredicate, xObject):
        count = 0
        xRepo = xDMA.getRDFRepository()
        for xGraphUri in xGraphs:
            xGraph = xRepo.getGraph(xGraphUri)
            print("Check graph:")
            print("\t" + xGraph.getName().Namespace)
            print("\t" + xGraph.getName().LocalName)

            xStatements = xGraph.getStatements(xDMA, xPredicate, xObject)
            self.assertIsNotNone(xStatements)

            if xPredicate is None:
                self.assertTrue(xStatements.hasMoreElements())

            for xStatement in xStatements:
                print("\tCheck statement:")
                print("\t\t" + xStatement.Subject.StringValue)
                print("\t\tPredicate.Namespace = " + xStatement.Predicate.Namespace)
                print("\t\tPredicate.LocalName = " + xStatement.Predicate.LocalName)

                count = count + 1
        return count


if __name__ == '__main__':
    unittest.main()
