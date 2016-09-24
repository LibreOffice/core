#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
import traceback

from .Exporter import Exporter
from ..TypeDetection import *
from ...common.Desktop import Desktop
from ...common.Properties import Properties
from ...common.FileAccess import FileAccess
from ...document.OfficeDocument import OfficeDocument
from ...text.TextDocument import TextDocument

from com.sun.star.lang import IllegalArgumentException
from com.sun.star.document.MacroExecMode import NEVER_EXECUTE
from com.sun.star.document.UpdateDocMode import NO_UPDATE

class AbstractExporter(Exporter):

    exporter = None
    fileAccess = None

    def __init__(self, exporter_):
        self.exporter = exporter_

    def storeToURL(self, officeDocument, props, targetUrl, filterName, filterData):
        props = Properties()
        props["FilterName"] = filterName
        if (len(filterData) > 0):
            props["FilterData"] = filterData
        o = props.getProperties1()
        officeDocument.storeToURL(targetUrl, tuple(o))

    def storeToURL1(self, officeDocument, targetUrl, filterName, filterData):
        self.storeToURL(officeDocument, Properties(), targetUrl, filterName, filterData)

    def storeToURL2(self, officeDocument, targetUrl, filterName):
        self.storeToURL(officeDocument, Properties(), targetUrl, filterName, [])

    def getArgument(self, name, p):
        return p.cp_Arguments.getElement(name).cp_Value

    def openDocument(self, doc, xmsf):
        document = None
        # open the document.
        try:
            desktop = Desktop.getDesktop(xmsf)
            props = list(range(3))
            props[0] = Properties.createProperty("Hidden", True)
            props[1] = Properties.createProperty("MacroExecutionMode", NEVER_EXECUTE)
            props[2] = Properties.createProperty("UpdateDocMode", NO_UPDATE)
            document = desktop.loadComponentFromURL(doc.cp_URL, "_blank", 0, tuple(props))
        except IllegalArgumentException:
            traceback.print_exc()
        # try to get the number of pages in the document
        try:
            self.pageCount(doc, document)
        except Exception:
            traceback.print_exc()
            # Here i do nothing since pages is not *so* important.
        return document

    def closeDocument(self, doc, xmsf):
        try:
            doc.close(False)
        except Exception:
            traceback.print_exc()

    def pageCount(self, doc, document):
        if (doc.appType == WRITER_DOC):
            doc.pages = TextDocument.getPageCount(document)
        elif (doc.appType == IMPRESS_DOC):
            doc.pages = OfficeDocument.getSlideCount(document)
        elif (doc.appType == DRAW_DOC):
            doc.pages = OfficeDocument.getSlideCount(document)

    def getFileAccess(self, xmsf):
        if (self.fileAccess == None):
            try:
                self.fileAccess = FileAccess(xmsf)
            except Exception:
                traceback.print_exc()
        return self.fileAccess

    def calcFileSize(self, doc, url, xmsf):
        # if the exporter exports to a
        # binary format, get the size of the destination.
        if (self.exporter.cp_Binary):
            doc.sizeBytes = self.getFileAccess(xmsf).getSize(url)
