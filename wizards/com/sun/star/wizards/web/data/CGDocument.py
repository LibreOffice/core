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
from os import sep as separator
from ...common.ConfigGroup import ConfigGroup
from ...document.OfficeDocument import OfficeDocument
from ...common.Properties import Properties
from ...common.PropertyNames import PropertyNames
from ...common.FileAccess import FileAccess
from ...common.XMLHelper import XMLHelper
from ..TypeDetection import *
from ...common.Desktop import Desktop

from com.sun.star.document.MacroExecMode import NEVER_EXECUTE
from com.sun.star.document.UpdateDocMode import NO_UPDATE

'''
About the member fields Title, Description and Author:
There is an ambiguity in handling these fields.
On the one hand, the user can set those fields to a custom value,
relevant only for the web-wizard.
On the other-hand, the web-wizard can read those fields automatically
from SO documents.
In order to avoid conflict by saving, I use two fields: "cp_title", which
should be read from the configuration (saved session), and "title" which is
the value read from the document.
The *cp_title* "overrides" *title*, if exists. if not, *title* is used.
The same is valid for *description* and *author*.
@author  rpiterman
'''

class CGDocument(ConfigGroup):

    PAGE_TYPE_PAGE = 1
    PAGE_TYPE_SLIDE = 2

    def __init__(self, url = "", xmsf = None, task = None):
        self.cp_URL = ""
        self.cp_Exporter = None
        self.cp_Index = -1
        self.cp_Title = ""
        self.cp_Description = ""
        self.cp_Author = ""
        self.localFilename = ""
        self.urlFilename = ""
        self.title = ""
        self.description = ""
        self.author = ""
        self.dirName = ""
        self.createdDate = None
        self.updatedDate = None
        self.sizeBytes = -1
        self.pages = -1
        self.valid = False
        self.appType = ""
        if (xmsf is None):
            return
        self.cp_URL = self.getSettings().getFileAccess(xmsf).getURL(url)
        if (task is None):
            task = Task("", "", 5)
        self.validate(xmsf, task)

    def getSettings(self):
        return self.root

    '''
    the task will advance 5 times during validate.
    @param xmsf
    @param task
    @throws Exception
    '''

    def validate(self, xmsf, task):
        if not self.getSettings().getFileAccess(xmsf).exists(self.cp_URL, False):
            raise FileNotFoundException (
                "The given URL does not point to a file")

        if self.getSettings().getFileAccess(xmsf).isDirectory(self.cp_URL):
            raise IllegalArgumentException ("The given URL points to a directory") #create a TypeDetection service

        self.mediaDescriptor = OfficeDocument.getFileMediaDecriptor(
            xmsf, self.cp_URL)
        task.advance(True)
        #1
        self.analyzeFileType(self.mediaDescriptor)
        task.advance(True)
        #2
        path = self.getSettings().getFileAccess(xmsf).getPath(self.cp_URL, "")
        self.localFilename = FileAccess.getFilename(path, separator)
        '''
        if the type is a star office convertible document
        We try to open the document to get some properties
        '''
        xProps = None
        task.advance(True)
        #3
        if self.isSOOpenable:
            # for documents which are openable through SO,
            # use DocumentProperties service.
            desktop = Desktop.getDesktop(xmsf)
            props = []
            props.append(Properties.createProperty("Hidden", True))
            props.append(Properties.createProperty(
                "MacroExecutionMode", NEVER_EXECUTE))
            props.append(Properties.createProperty(
                "UpdateDocMode", NO_UPDATE))
            component = desktop.loadComponentFromURL(
                self.cp_URL, "_default", 0, tuple(props))
            xProps = component.DocumentProperties

        task.advance(True)
        #4
        #now use the object to read some document properties.
        if xProps is not None:
            self.title = xProps.Title
            self.description = xProps.Description
            self.author = xProps.Author
            self.createdDate = xProps.CreationDate
            self.updatedDate = xProps.ModificationDate
        else:

            #get some information from OS.
            self.title = self.localFilename
            self.updatedDate = \
                self.getSettings().getFileAccess(xmsf).getLastModified(self.cp_URL)

        task.advance(True)
        #5
        if self.cp_Title == "":
            self.cp_Title = self.title

        if self.cp_Title == "":
            self.cp_Title = self.localFilename

        if self.cp_Description == "":
            self.cp_Description = self.description

        if self.cp_Author == "":
            self.cp_Author = self.author

        if self.cp_Exporter is None or self.cp_Exporter == "":
            exp = self.getSettings().getExporters(self.appType)
            self.cp_Exporter = \
                self.getSettings().cp_Exporters.getKey(exp[0])

    '''
    Analyzes a type-detection string, returned from the TypeDetection service,
    and sets the appType, isSOOpenable and isSODocument members.
    '''

    def analyzeFileType(self, mediaDesc):
        if mediaDesc is None:
            media = ""
        else:
            media = Properties.getPropertyValue(
                self.mediaDescriptor, PropertyNames.PROPERTY_NAME)
        self.appType = self.getDocType(media)
        self.isSOOpenable = (self.appType == WRITER_DOC or self.appType == CALC_DOC or self.appType == IMPRESS_DOC or self.appType == DRAW_DOC) or self.appType == HTML_DOC
        parts = media.split("_")
        if len(parts) < 2:
            self.isSODocument = False
        else:
            self.isSODocument = self.isSOOpenable and (parts[1].startswith("Star"))

    '''
    @param media is the media description string returned by an UNO TypeDetection object.
    @return one of the constants in the interface

    '''

    def getDocType(self, media):
        if media == "":
            return NO_TYPE
        elif media.startswith("generic_HTML"):
            return HTML_DOC
        elif media.startswith("writer"):
            return WRITER_DOC
        elif media.startswith("calc"):
            return CALC_DOC
        elif media.startswith("draw"):
            return DRAW_DOC
        elif media.startswith("impress"):
            return IMPRESS_DOC
        elif media.startswith("pdf"):
            return PDF_DOC
        elif media.startswith("gif") or media.startswith("jpg"):
            return WEB_GRAPHICS
        elif media.startswith("wav"):
            return SOUND_FILE
        else:
            return NO_TYPE

    def createDOM(self, parent):
        d = self.getSettings().cp_DefaultSession.cp_Design
        exp = self.getSettings().cp_Exporters.getElement(self.cp_Exporter)
        return XMLHelper.addElement(parent, "document",
                                    ["title", "description", "author", "format", "filename",
                                     "create-date", "update-date", "pages", "size", "icon",
                                     "dir", "fn"],
                                    [self.cp_Title if (d.cp_DisplayTitle) else "",
                                     self.cp_Description if (d.cp_DisplayDescription) else "",
                                     self.cp_Author if (d.cp_DisplayAuthor) else "",
                                     self.getTargetTypeName(exp) if (d.cp_DisplayFileFormat) else "",
                                     self.localFilename if (d.cp_DisplayFilename) else "",
                                     self.createDate() if (d.cp_DisplayCreateDate) else "",
                                     self.updateDate() if (d.cp_DisplayUpdateDate) else  "",
                                     "" + self.getPages() if (d.cp_DisplayPages and (self.pages > -1)) else "", #TODO when do i calculate pages?
                                     self.sizeKB() if (d.cp_DisplaySize) else "", #TODO when do i calculate size?
                                     self.getIcon(exp) if (d.cp_DisplayFormatIcon) else "",
                                     self.dirName, self.urlFilename])

    def updateDate(self):
        if self.updatedDate is None:
            return ""

        return self.getSettings().formatter.formatCreated(self.updatedDate)

    def createDate(self):
        if self.createdDate is None:
            return ""

        return self.getSettings().formatter.formatCreated(self.createdDate)

    def sizeKB(self):
        if self.sizeBytes == -1:
            return ""
        else:
            return self.getSettings().formatter.formatFileSize(self.sizeBytes)

    def getPages(self):
        if self.pages == -1:
            return ""
        else:
            return self.pagesTemplate().replace("%NUMBER", "" + self.pages)

    def pagesTemplate(self):
        pagesType = \
            self.getSettings().cp_Exporters.getElement(self.cp_Exporter).cp_PageType
        if pagesType == PAGE_TYPE_PAGE:
            return \
                self.getSettings().resources[CGSettings.RESOURCE_PAGES_TEMPLATE]
        elif pagesType == PAGE_TYPE_SLIDE:
            return \
                self.getSettings().resources[CGSettings.RESOURCE_SLIDES_TEMPLATE]
        else:
            return ""

    def getTargetTypeName(self, exp):
        if exp.targetTypeName == "":
            return Properties.getPropertyValue(self.mediaDescriptor, "UIName")
        else:
            return exp.targetTypeName

    def getIcon(self, exporter):
        if exporter.cp_Icon == "":
            return self.getIcon1(self.appType)
        else:
            return exporter.cp_Icon

    def getIcon1(self, appType):
        return appType + ".gif"

    '''
    This method is used by the DataAware model
    and returns the index of the exporter of
    this document, <b>in the list of the
    exporters available for this type of document</b>
    @return
    '''

    def getExporter(self):
        return [self.getExporterIndex()]

    '''
    @see getExporter()
    '''

    def setExporter(self, exporter_):
        exp = self.getSettings().getExporters(self.appType)[exporter_[0]]
        self.cp_Exporter = self.getSettings().cp_Exporters.getKey(exp)

    '''
    @see getExporter()
    @return
    '''

    def getExporterIndex(self):
        if self.cp_Exporter is None:
            return 0

        exporter = self.getSettings().cp_Exporters.getElement(self.cp_Exporter)
        exporters = self.getSettings().getExporters(self.appType)
        i = 0
        while i < len(exporters):
            if exporters[i] == exporter:
                return i

            i += 1
        return -1

    def toString(self):
        return self.localFilename
