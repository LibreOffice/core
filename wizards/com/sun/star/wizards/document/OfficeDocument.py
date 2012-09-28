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
import traceback
import unohelper
from ..ui.event.CommonListener import TerminateListenerProcAdapter
from ..common.Desktop import Desktop

from com.sun.star.awt import WindowDescriptor
from com.sun.star.awt import Rectangle
from com.sun.star.awt.WindowClass import TOP
from com.sun.star.task import ErrorCodeIOException

#Window Constants
com_sun_star_awt_WindowAttribute_BORDER \
    = uno.getConstantByName( "com.sun.star.awt.WindowAttribute.BORDER" )
com_sun_star_awt_WindowAttribute_SIZEABLE \
    = uno.getConstantByName( "com.sun.star.awt.WindowAttribute.SIZEABLE" )
com_sun_star_awt_WindowAttribute_MOVEABLE \
    = uno.getConstantByName( "com.sun.star.awt.WindowAttribute.MOVEABLE" )
com_sun_star_awt_VclWindowPeerAttribute_CLIPCHILDREN \
    = uno.getConstantByName(
        "com.sun.star.awt.VclWindowPeerAttribute.CLIPCHILDREN" )

class OfficeDocument(object):
    '''Creates a new instance of OfficeDocument '''

    def __init__(self, _xMSF):
        self.xMSF = _xMSF

    @classmethod
    def attachEventCall(self, xComponent, EventName, EventType, EventURL):
        try:
            oEventProperties = range(2)
            oEventProperties[0] = uno.createUnoStruct(
                'com.sun.star.beans.PropertyValue')
            oEventProperties[0].Name = "EventType"
            oEventProperties[0].Value = EventType
            # "Service", "StarBasic"
            oEventProperties[1] = uno.createUnoStruct(
                'com.sun.star.beans.PropertyValue')
            oEventProperties[1].Name = "Script" #"URL";
            oEventProperties[1].Value = EventURL
            uno.invoke(xComponent.Events, "replaceByName",
                (EventName, uno.Any("[]com.sun.star.beans.PropertyValue",
                    tuple(oEventProperties))))
        except Exception, exception:
            traceback.print_exc()

    def dispose(self, xMSF, xComponent):
        try:
            if xComponent != None:
                xFrame = xComponent.CurrentController.Frame
                if xComponent.isModified():
                    xComponent.setModified(False)

                Desktop.dispatchURL(xMSF, ".uno:CloseDoc", xFrame)

        except PropertyVetoException, exception:
            traceback.print_exc()

    '''
    Create a new office document, attached to the given frame.
    @param desktop
    @param frame
    @param sDocumentType e.g. swriter, scalc, ( simpress, scalc : not tested)
    @return the document Component
    (implements XComponent) object ( XTextDocument, or XSpreadsheedDocument )
    '''

    @classmethod
    def createNewDocument(self, frame, sDocumentType, preview, readonly):
        loadValues = range(2)
        loadValues[0] = uno.createUnoStruct(
            'com.sun.star.beans.PropertyValue')
        loadValues[0].Name = "ReadOnly"
        if readonly:
            loadValues[0].Value = True
        else:
            loadValues[0].Value = False

        loadValues[1] = uno.createUnoStruct(
            'com.sun.star.beans.PropertyValue')
        loadValues[1].Name = "Preview"
        if preview:
            loadValues[1].Value = True
        else:
            loadValues[1].Value = False
        sURL = "private:factory/" + sDocumentType
        xComponent = None
        try:
            xComponent = frame.loadComponentFromURL(
                sURL, "_self", 0, tuple(loadValues))

        except Exception:
            traceback.print_exc()

        return xComponent

    @classmethod
    def createNewFrame(self, xMSF, listener, FrameName="_blank"):
        xFrame = None
        if FrameName.lower() == "WIZARD_LIVE_PREVIEW".lower():
            xFrame = self.createNewPreviewFrame(xMSF, listener)
        else:
            xF = Desktop.getDesktop(xMSF)
            xFrame = xF.findFrame(FrameName, 0)
            if listener is not None:
                xFF = xF.getFrames()
                xFF.remove(xFrame)
                xF.addTerminateListener(TerminateListenerProcAdapter(listener))

        return xFrame

    @classmethod
    def createNewPreviewFrame(self, xMSF, listener):
        xToolkit = None
        try:
            xToolkit = xMSF.createInstance("com.sun.star.awt.Toolkit")
        except Exception, e:
            # TODO Auto-generated catch block
            traceback.print_exc()

        #describe the window and its properties
        aDescriptor = WindowDescriptor()
        aDescriptor.Type = TOP
        aDescriptor.WindowServiceName = "window"
        aDescriptor.ParentIndex = -1
        aDescriptor.Parent = None
        aDescriptor.Bounds = Rectangle(10, 10, 640, 480)

        #Set Window Attributes
        gnDefaultWindowAttributes = \
            com_sun_star_awt_WindowAttribute_BORDER + \
            com_sun_star_awt_WindowAttribute_MOVEABLE + \
            com_sun_star_awt_WindowAttribute_SIZEABLE + \
            com_sun_star_awt_VclWindowPeerAttribute_CLIPCHILDREN

        aDescriptor.WindowAttributes = gnDefaultWindowAttributes
        #create a new blank container window
        xPeer = None
        try:
            xPeer = xToolkit.createWindow(aDescriptor)
        except IllegalArgumentException, e:
            # TODO Auto-generated catch block
            traceback.print_exc()

        #define some further properties of the frame window
        #if it's needed .-)
        #xPeer->setBackground(...);
        #create new empty frame and set window on it
        xFrame = None
        try:
            xFrame = xMSF.createInstance("com.sun.star.frame.Frame")
        except Exception, e:
            # TODO Auto-generated catch block
            traceback.print_exc()

        xFrame.initialize(xPeer)
        #from now this frame is useable ...
        #and not part of the desktop tree.
        #You are alone with him .-)
        if listener is not None:
            Desktop.getDesktop(xMSF).addTerminateListener(
                TerminateListenerProcAdapter(listener))

        return xFrame

    @classmethod
    def load(self, xInterface, sURL, sFrame, xValues):
        xComponent = None
        try:
            xComponent = xInterface.loadComponentFromURL(
                sURL, sFrame, 0, tuple(xValues))
        except Exception, exception:
            traceback.print_exc()

        return xComponent

    @classmethod
    def store(self, xMSF, xComponent, StorePath, FilterName):
        try:
            if len(FilterName):
                oStoreProperties = range(2)
                oStoreProperties[0] = uno.createUnoStruct(
                    'com.sun.star.beans.PropertyValue')
                oStoreProperties[0].Name = "FilterName"
                oStoreProperties[0].Value = FilterName
                oStoreProperties[1] = uno.createUnoStruct(
                    'com.sun.star.beans.PropertyValue')
                oStoreProperties[1].Name = "InteractionHandler"
                oStoreProperties[1].Value = xMSF.createInstance(
                    "com.sun.star.comp.uui.UUIInteractionHandler")
            else:
                oStoreProperties = range(0)

            if StorePath.startswith("file://"):
                #Unix
                StorePath = StorePath[7:]

            sPath = StorePath[:(StorePath.rfind("/") + 1)]
            sFile = StorePath[(StorePath.rfind("/") + 1):]
            xComponent.storeToURL(
                unohelper.absolutize(
                    unohelper.systemPathToFileUrl(sPath),
                    unohelper.systemPathToFileUrl(sFile)),
                    tuple(oStoreProperties))
            return True
        except ErrorCodeIOException:
            return True
            #There's a bug here, fix later
            pass
        except Exception, exception:
            traceback.print_exc()
            return False

    def close(self, xComponent):
        bState = False
        if xComponent != None:
            try:
                xComponent.close(True)
                bState = True
            except com.sun.star.util.CloseVetoException, exCloseVeto:
                print "could not close doc"
                bState = False

        else:
            xComponent.dispose()
            bState = True

        return bState

    def ArraytoCellRange(self, datalist, oTable, xpos, ypos):
        try:
            rowcount = datalist.length
            if rowcount > 0:
                colcount = datalist[0].length
                if colcount > 0:
                    xNewRange = oTable.getCellRangeByPosition(
                        xpos, ypos, (colcount + xpos) - 1,
                            (rowcount + ypos) - 1)
                    xNewRange.setDataArray(datalist)

        except Exception, e:
            traceback.print_exc()

    @classmethod
    def getFileMediaDecriptor(self, xmsf, url):
        typeDetect = xmsf.createInstance(
            "com.sun.star.document.TypeDetection")
        mediaDescr = range(1)
        mediaDescr[0] = uno.createUnoStruct(
            'com.sun.star.beans.PropertyValue')
        mediaDescr[0].Name = "URL"
        mediaDescr[0].Value = url
        Type = typeDetect.queryTypeByDescriptor(tuple(mediaDescr), True)[0]
        if Type == "":
            return None
        else:
            return typeDetect.getByName(Type)

    @classmethod
    def getTypeMediaDescriptor(self, xmsf, type):
        typeDetect = xmsf.createInstance(
            "com.sun.star.document.TypeDetection")
        return typeDetect.getByName(type)

    '''
    returns the count of slides in a presentation,
    or the count of pages in a draw document.
    @param model a presentation or a draw document
    @return the number of slides/pages in the given document.
    '''

    def getSlideCount(self, model):
        return model.getDrawPages().getCount()

    def showMessageBox(
        self, xMSF, windowServiceName, windowAttribute, MessageText):

        return SystemDialog.showMessageBox(
            xMSF, windowServiceName, windowAttribute, MessageText)
