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
import uno

from os import sep as separator
from ...common.ConfigGroup import ConfigGroup
from ...document.OfficeDocument import OfficeDocument
from ...common.Properties import Properties
from ...common.PropertyNames import PropertyNames
from ...common.FileAccess import FileAccess
from ..TypeDetection import *
from ...common.Desktop import Desktop

from com.sun.star.document.MacroExecMode import NEVER_EXECUTE
from com.sun.star.document.UpdateDocMode import NO_UPDATE

'''
About the member fields Title, Decription and Author:
There is an ambiguicy in handling these fields.
On the one hand, the user can set those fields to a custom value,
relevant only for the web-wizard.
On the other-hand, the web-wizard can read those fields automatically
from SO documents.
In order to avoid conflict by saving, I use two fields: "cp_title", which
should be read from the configuratin (saved session), and "title" which is
the value read from the document.
The *cp_title* "overrides" *title*, if exists. if not, *title* is used.
The same is valid for *description* and *author*.
@author  rpiterman
'''

class CGDocument(ConfigGroup):
    cp_Exporter = None
    cp_Index = -1
    PAGE_TYPE_PAGE = 1
    PAGE_TYPE_SLIDE = 2
    cp_Title = ""
    cp_Description = ""
    cp_URL = ""
    cp_Author = ""
    localFilename = ""
    urlFilename = ""
    title = ""
    description = ""
    author = ""
    sizeBytes = -1
    pages = -1
    valid = False
    appType = None

    def __init__(self, url = "", xmsf = None, Task = None):
        if (xmsf is None):
            return
        cp_URL = self.getSettings().getFileAccess(xmsf).getURL(url);
        if (task is None):
            task = Task("", "", 5)
        self.validate(xmsf, task);

    def getSettings(self):
        return ConfigGroup.root

    '''
    the task will advance 5 times during validate.
    @param xmsf
    @param task
    @throws Exception
    '''

    def validate(self, xmsf, task):
        print ("WARNING !!! VALIDATING DOCUMENT ....")
        if not self.root.getFileAccess(xmsf).exists(self.cp_URL, False):
            raise FileNotFoundException (
                "The given URL does not point to a file");

        if self.root.getFileAccess(xmsf).isDirectory(self.cp_URL):
            raise IllegalArgumentException (
                "The given URL points to a directory");
            #create a TypeDetection service

        self.mediaDescriptor = OfficeDocument.getFileMediaDecriptor(
            xmsf, self.cp_URL)
        task.advance(True)
        #1
        self.analyzeFileType(self.mediaDescriptor)
        task.advance(True)
        #2
        path = self.root.getFileAccess(xmsf).getPath(self.cp_URL, "")
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
            print ("DEBUG !!! validate -- URL: ", self.cp_URL)
            component = desktop.loadComponentFromURL(
                self.cp_URL, "_default", 0, tuple(props))
            xProps = component.DocumentProperties

        task.advance(True)
        #4
        #now use the object to read some document properties.
        if xProps is not None:
            title = xProps.Title
            description = xProps.Description
            author = xProps.Author
            createDate = xProps.CreationDate
            updateDate = xProps.ModificationDate
        else:

            #get some information from OS.
            title = self.localFilename
            updateDate = \
                self.getSettings().getFileAccess(xmsf).getLastModified(self.cp_URL)

        task.advance(True)
        #5
        valid = True
        if self.cp_Title == "":
            cp_Title = self.title

        if self.cp_Title == "":
            cp_Title = self.localFilename

        if self.cp_Description == "":
            cp_Description = self.description

        if self.cp_Author == "":
            cp_Author = self.author

        if self.cp_Exporter is None or self.cp_Exporter == "":
            print ("WARNING !!! settign exporter for key:", CGDocument.appType)
            exp = self.root.getExporters(CGDocument.appType)
            print ("WARNING !!! got N exporters:", len(exp))
            print ("WARNING !!! got exporter:", exp[0])
            self.cp_Exporter = \
                self.root.cp_Exporters.getKey(exp[0])
            print ("WARNING !!! exporter: ", self.cp_Exporter)

    '''
    Analyzes a type-detection string, returned from the TypeDetection service,
    and sets the appType, isSOOpenable and isSODocument members.
    '''

    def analyzeFileType(self, mediaDesc):
        print ("DEBUG !!! analyzeFileType -- mediaDesc : ", mediaDesc)
        if mediaDesc is None:
            media = ""
        else:
            media = Properties.getPropertyValue(
                self.mediaDescriptor, PropertyNames.PROPERTY_NAME)
        CGDocument.appType = self.getDocType(media)
        print ("DEBUG !!! analyzeFileType --  appType: ", CGDocument.appType)
        self.isSOOpenable = (CGDocument.appType == WRITER_DOC or CGDocument.appType == CALC_DOC or CGDocument.appType == IMPRESS_DOC or CGDocument.appType == DRAW_DOC) or CGDocument.appType == HTML_DOC
        if (self.isSOOpenable):
            print ("DEBUG !!! analyzeFileType -- isSOOpenable .")
        else:
            print ("DEBUG !!! analyzeFileType -- NOT isSOOpenable .")
        parts = media.split("_")
        if len(parts) < 2:
            self.isSODocument = False
        else:
            self.isSODocument = self.isSOOpenable and (parts[1].startswith("Star"))
        if (self.isSODocument):
            print ("DEBUG !!! analyzeFileType -- isSODocument .")
        else:
            print ("DEBUG !!! analyzeFileType -- NOT isSODocument .")

    '''
    @param media is the media description string returned by an UNO TypeDetection object.
    @return one of the constants in the interface

    '''

    def getDocType(self, media):
        print ("DEBUG !!! getDocType -- media: ", media)
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
        '''return XMLHelper.addElement(parent, "document", ["title", "description", "author", "format", "filename", "create-date", "update-date", "pages", "size", "icon", "dir", "fn"], [d.cp_DisplayTitle ? self.cp_Title : "", d.cp_DisplayDescription ? self.cp_Description : "", d.cp_DisplayAuthor ? self.cp_Author : "", d.cp_DisplayFileFormat ? getTargetTypeName(exp) : "", d.cp_DisplayFilename ? self.localFilename : "", d.cp_DisplayCreateDate ? self.createDate() : "", d.cp_DisplayUpdateDate ? self.updateDate() : "", d.cp_DisplayPages and (self.pages > -1) ? "" + self.pages() : "", #TODO when do i calculate pages?
        d.cp_DisplaySize ? sizeKB() : "", #TODO when do i calculate size?
        d.cp_DisplayFormatIcon ? getIcon(exp) : "", self.dirName, self.urlFilename])'''

    def updateDate(self):
        if self.updateDate is None:
            return ""

        return self.getSettings().formatter.formatCreated(self.updateDate)

    def createDate(self):
        if self.createDate is None:
            return ""

        return self.getSettings().formatter.formatCreated(self.createDate)

    def sizeKB(self):
        if self.sizeBytes == -1:
            return ""
        else:
            return self.getSettings().formatter.formatFileSize(self.sizeBytes)

    def pages(self):
        if self.pages == -1:
            return ""
        else:
            return pagesTemplate().replace("%NUMBER", "" + self.pages)

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
            return getIcon(CGDocument.appType)
        else:
            return exporter.cp_Icon

    def getIcon(self, appType):
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
        exp = self.getSettings().getExporters(CGDocument.appType)[exporter_[0]]
        self.cp_Exporter = self.getSettings().cp_Exporters.getKey(exp)

    '''
    @see getExporter()
    @return
    '''

    def getExporterIndex(self):
        if self.cp_Exporter is None:
            return 0

        exporter = self.getSettings().cp_Exporters.getElement(self.cp_Exporter)
        exporters = self.getSettings().getExporters(CGDocument.appType)
        i = 0
        while i < len(exporters):
            if exporters[i] == exporter:
                return i

            i += 1
        return -1
