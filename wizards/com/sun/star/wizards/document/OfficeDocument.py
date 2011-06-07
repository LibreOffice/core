from com.sun.star.awt.WindowClass import TOP
import traceback
import uno

class OfficeDocument(object):
    '''Creates a new instance of OfficeDocument '''

    def __init__(self, _xMSF):
        self.xMSF = _xMSF

    @classmethod
    def attachEventCall(self, xComponent, EventName, EventType, EventURL):
        try:
            oEventProperties = range(2)
            oEventProperties[0] = uno.createUnoStruct('com.sun.star.beans.PropertyValue')
            oEventProperties[0].Name = "EventType"
            oEventProperties[0].Value = EventType
            # "Service", "StarBasic"
            oEventProperties[1] = uno.createUnoStruct('com.sun.star.beans.PropertyValue')
            oEventProperties[1].Name = "Script" #"URL";
            oEventProperties[1].Value = EventURL
            uno.invoke(xComponent.getEvents(), "replaceByName", (EventName, uno.Any( \
                "[]com.sun.star.beans.PropertyValue", tuple(oEventProperties))))
        except Exception, exception:
            traceback.print_exc()

    def dispose(self, xMSF, xComponent):
        try:
            if xComponent != None:
                xFrame = xComponent.getCurrentController().getFrame()
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
    @return the document Component (implements XComponent) object ( XTextDocument, or XSpreadsheedDocument )
    '''

    def createNewDocument(self, frame, sDocumentType, preview, readonly):
        loadValues = range(2)
        loadValues[0] = uno.createUnoStruct('com.sun.star.beans.PropertyValue')
        loadValues[0].Name = "ReadOnly"
        if readonly:
            loadValues[0].Value = True
        else:
            loadValues[0].Value = False

        loadValues[1] = uno.createUnoStruct('com.sun.star.beans.PropertyValue')
        loadValues[1].Name = "Preview"
        if preview:
            loadValues[1].Value = True
        else:
            loadValues[1].Value = False

        sURL = "private:factory/" + sDocumentType
        try:

            xComponent = frame.loadComponentFromURL(sURL, "_self", 0, loadValues)

        except Exception, exception:
            traceback.print_exc()

        return xComponent

    def createNewFrame(self, xMSF, listener):
        return createNewFrame(xMSF, listener, "_blank")

    def createNewFrame(self, xMSF, listener, FrameName):
        xFrame = None
        if FrameName.equalsIgnoreCase("WIZARD_LIVE_PREVIEW"):
            xFrame = createNewPreviewFrame(xMSF, listener)
        else:
            xF = Desktop.getDesktop(xMSF)
            xFrame = xF.findFrame(FrameName, 0)
            if listener != None:
                xFF = xF.getFrames()
                xFF.remove(xFrame)
                xF.addTerminateListener(listener)

        return xFrame

    def createNewPreviewFrame(self, xMSF, listener):
        xToolkit = None
        try:
            xToolkit = xMSF.createInstance("com.sun.star.awt.Toolkit")
        except Exception, e:
            # TODO Auto-generated catch block
            traceback.print_exc()

        #describe the window and its properties
        aDescriptor = WindowDescriptor.WindowDescriptor()
        aDescriptor.Type = TOP
        aDescriptor.WindowServiceName = "window"
        aDescriptor.ParentIndex = -1
        aDescriptor.Parent = None
        aDescriptor.Bounds = Rectangle.Rectangle_unknown(10, 10, 640, 480)
        aDescriptor.WindowAttributes = WindowAttribute.BORDER | WindowAttribute.MOVEABLE | WindowAttribute.SIZEABLE | VclWindowPeerAttribute.CLIPCHILDREN
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
        if listener != None:
            Desktop.getDesktop(xMSF).addTerminateListener(listener)

        return xFrame

    @classmethod
    def load(self, xInterface, sURL, sFrame, xValues):
        xComponent = None
        try:
            xComponent = xInterface.loadComponentFromURL(sURL, sFrame, 0, tuple(xValues))
        except Exception, exception:
            traceback.print_exc()

        return xComponent

    @classmethod
    def store(self, xMSF, xComponent, StorePath, FilterName, bStoreToUrl):
        try:
            if FilterName.length() > 0:
                oStoreProperties = range(2)
                oStoreProperties[0] = uno.createUnoStruct('com.sun.star.beans.PropertyValue')
                oStoreProperties[0].Name = "FilterName"
                oStoreProperties[0].Value = FilterName
                oStoreProperties[1] = uno.createUnoStruct('com.sun.star.beans.PropertyValue')
                oStoreProperties[1].Name = "InteractionHandler"
                oStoreProperties[1].Value = xMSF.createInstance("com.sun.star.comp.uui.UUIInteractionHandler")
            else:
                oStoreProperties = range(0)

            if bStoreToUrl == True:
                xComponent.storeToURL(StorePath, oStoreProperties)
            else:
                xStoreable.storeAsURL(StorePath, oStoreProperties)

            return True
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
                    xNewRange = oTable.getCellRangeByPosition(xpos, ypos, (colcount + xpos) - 1, (rowcount + ypos) - 1)
                    xNewRange.setDataArray(datalist)

        except Exception, e:
            traceback.print_exc()

    def getFileMediaDecriptor(self, xmsf, url):
        typeDetect = xmsf.createInstance("com.sun.star.document.TypeDetection")
        mediaDescr = range(1)
        mediaDescr[0][0] = uno.createUnoStruct('com.sun.star.beans.PropertyValue')
        mediaDescr[0][0].Name = "URL"
        mediaDescr[0][0].Value = url
        Type = typeDetect.queryTypeByDescriptor(mediaDescr, True)
        if Type.equals(""):
            return None
        else:
            return typeDetect.getByName(type)

    def getTypeMediaDescriptor(self, xmsf, type):
        typeDetect = xmsf.createInstance("com.sun.star.document.TypeDetection")
        return typeDetect.getByName(type)

    '''
    returns the count of slides in a presentation,
    or the count of pages in a draw document.
    @param model a presentation or a draw document
    @return the number of slides/pages in the given document.
    '''

    def getSlideCount(self, model):
        return model.getDrawPages().getCount()

    def getDocumentProperties(self, document):
        return document.getDocumentProperties()

    def showMessageBox(self, xMSF, windowServiceName, windowAttribute, MessageText):

        return SystemDialog.showMessageBox(xMSF, windowServiceName, windowAttribute, MessageText)

    def getWindowPeer(self):
        return self.xWindowPeer

    '''
    @param windowPeer The xWindowPeer to set.
    Should be called as soon as a Windowpeer of a wizard dialog is available
    The windowpeer is needed to call a Messagebox
    '''

    def setWindowPeer(self, windowPeer):
        self.xWindowPeer = windowPeer

