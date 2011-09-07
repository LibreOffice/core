import traceback
from common.Properties import Properties

from com.sun.star.awt import WindowDescriptor
from com.sun.star.awt import Rectangle
from com.sun.star.awt.WindowClass import SIMPLE
from com.sun.star.awt.VclWindowPeerAttribute import CLIPCHILDREN
from com.sun.star.awt.WindowAttribute import SHOW

'''
@author rpiterman
To change the template for this generated type comment go to
Window>Preferences>Java>Code Generation>Code and Comments
'''

class DocumentPreview(object):
    PREVIEW_MODE = 1

    '''
    create new frame with window inside
    load a component as preview into this frame
    '''

    def __init__(self, xmsf, control):
        self.xControl = control
        self.createPreviewFrame(xmsf, self.xControl)

    def setDocument(self, url_, propNames, propValues=None):
        if propValues is None:
            if propNames == DocumentPreview.PREVIEW_MODE:
                self.setDocument(url_, ("Preview", "ReadOnly"), (True, True))
            else:
                self.loadArgs = propNames
                self.xFrame.activate()
                self.xComponent = self.xFrame.loadComponentFromURL(url_, "_self", 0, tuple(self.loadArgs))
                return self.xComponent
        else:
            self.url = url_
            ps = Properties()
            for index,item in enumerate(propNames):
                ps[item] = propValues[index]
            return self.setDocument(self.url, ps.getProperties(ps))

    def reload(self, xmsf):
        self.closeFrame()
        self.createPreviewFrame(xmsf, self.xControl)
        self.setDocument(self.url, self.loadArgs)

    def closeFrame(self):
        if self.xFrame is not None:
            self.xFrame.close(False)

    '''
    create a new frame with a new container window inside,
    which isnt part of the global frame tree.

    Attention:
    a) This frame wont be destroyed by the office. It must be closed by you!
       Do so - please call XCloseable::close().
    b) The container window is part of the frame. Dont hold it alive - nor try to kill it.
       It will be destroyed inside close().
    '''

    def createPreviewFrame(self, xmsf, xControl):
        controlPeer = xControl.Peer
        r = xControl.PosSize
        toolkit = xmsf.createInstance("com.sun.star.awt.Toolkit")
        aDescriptor = WindowDescriptor()
        aDescriptor.Type = SIMPLE
        aDescriptor.WindowServiceName = "window"
        aDescriptor.ParentIndex = -1
        aDescriptor.Parent = controlPeer
        #xWindowPeer; #argument !
        aDescriptor.Bounds = Rectangle(0, 0, r.Width, r.Height)
        aDescriptor.WindowAttributes = CLIPCHILDREN | SHOW
        self.xWindow = toolkit.createWindow(aDescriptor)
        self.xFrame = xmsf.createInstance("com.sun.star.frame.Frame")
        self.xFrame.initialize(self.xWindow)
        self.xWindow.setVisible(True)

    def dispose(self):
        try:
            self.closeFrame()
        except Exception:
            traceback.print_exc()
