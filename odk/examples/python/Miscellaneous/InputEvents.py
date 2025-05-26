# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#

import uno
import unohelper
from com.sun.star.awt import XKeyHandler
from com.sun.star.awt import XKeyListener
from com.sun.star.awt import XMouseClickHandler
from com.sun.star.awt import XMouseMotionHandler
from com.sun.star.awt import XMouseListener
from com.sun.star.awt import XMouseMotionListener


"""
This example illustrates how to register to keyboard or mouse events.

There are two families of interfaces for this, called Listeners and Handlers.
In many cases the Handlers are easier to use and provide more flexibility. But
sometimes it's necessary to use the Listeners, as being shown below.

The Listeners usually need to be added exactly to the correct widget. So this
example adds them recursively them to all widgets below the given one.
"""


SOFFICE_CONNECTION_URI = "uno:socket,host=localhost,port=2083;urp;StarOffice.ComponentContext"


def demo():
    # Connect to LibreOffice process
    localComponentContext = uno.getComponentContext()
    localServiceManager = localComponentContext.getServiceManager()
    resolver = localServiceManager.createInstanceWithContext("com.sun.star.bridge.UnoUrlResolver", localComponentContext)
    remoteComponentContext = resolver.resolve(SOFFICE_CONNECTION_URI)

    # Get the currently opened view context.
    remoteServiceManager = remoteComponentContext.getServiceManager()
    desktop = remoteServiceManager.createInstance("com.sun.star.frame.Desktop")
    xComponent = desktop.getCurrentComponent()  # e.g. SwXTextDocument, ScModelObj, SdXImpressDocument
    if "com.sun.star.document.OfficeDocument" not in xComponent.getSupportedServiceNames():
        print("No OfficeDocument opened.")
        exit(1)
    else:
        handler = MyXKeyMouseClickMotionHandler("handler")
        # Events in the document view area. Not in the surrounding UI.
        xController = xComponent.getCurrentController()  # xModel.getCurrentController()
        xController.addKeyHandler(handler)         # XUserInputInterception.addKeyHandler()
        xController.addMouseClickHandler(handler)  # XUserInputInterception.addMouseClickHandler()

        listener = MyXKeyMouseClickMotionHandler("listener")
        # TODO: Register to e.value.Source of Handler events.
        #       That's the correct source.
        xWindow = xController.ComponentWindow
        # In writer we're usually looking for:
        #   xWindow.Windows[0].Windows[0]
        recursive_windows("", xWindow, lambda subXWindow: (
                subXWindow.addKeyListener(listener),
                subXWindow.addMouseListener(listener),
                #subXWindow.addMouseMotionListener(listener),  # very much events
            ))

        input("Waiting for events. Press Enter to quit...\n")


def recursive_windows(path, xWindow, func):
    print("recursive_windows: " + path)
    func(xWindow)
    try:
        windows = xWindow.getWindows()  # XVclContainer.getWindows()
    except Exception:
        return
    for i in range(len(windows)):
        subXWindow = windows[i]
        recursive_windows(path+str(i)+": ", subXWindow, func)


class MyXKeyMouseClickMotionHandler(unohelper.Base, XKeyHandler, XKeyListener, XMouseClickHandler, XMouseMotionHandler, XMouseListener, XMouseMotionListener):
    def __init__(self, name):  # XKeyHandler, XKeyListener
        self.name = name
    def keyPressed(self, e):  # XKeyHandler, XKeyListener
        self.key_evt(e, "pressed")
        return False  # False: don't consume (run other event handlers)
    def keyReleased(self, e):
        self.key_evt(e, "released")
        return False
    def mousePressed(self, e):  # XMouseClickHandler, XMouseListener
        self.mouse_evt(e, "pressed")
        return False
    def mouseReleased(self, e):  # XMouseClickHandler, XMouseListener
        self.mouse_evt(e, "released")
        return False
    def mouseEntered(self, e):  # XMouseListener
        self.mouse_evt(e, "entered")
        return False
    def mouseExited(self, e):  # XMouseListener
        self.mouse_evt(e, "exited")
        return False
    def mouseDragged(self, e):  # XMouseMotionHandler, XMouseMotionListener
        self.mouse_evt(e, "dragged")
        return False
    def mouseMoved(self, e):  # XMouseMotionHandler, XMouseMotionListener
        self.mouse_evt(e, "moved")
        return False

    def disposing(self, s):
        print(self.name + "# disposing")
    def key_evt(self, e, action):
        #print(self.name + "# key "+action+": " + str(e));  # very much output
        print(self.name + "# key "+action+" (code: " + str(e.value.KeyCode.real) + "): " + e.value.KeyChar.value)
    def mouse_evt(self, e, action):
        #print(self.name + "# mouse "+action+": " + str(e));  # very much output
        print(self.name + "# mouse "+action+": Modifiers: "+str(e.value.Modifiers)+"; Buttons: "+str(e.value.Buttons)+"; X: "+str(e.value.X)+"; Y: "+str(e.value.Y)+"; ClickCount: "+str(e.value.ClickCount)+"; PopupTrigger: "+str(e.value.PopupTrigger))


demo()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
