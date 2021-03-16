# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
import unohelper
from com.sun.star.awt import XMouseListener
from com.sun.star.awt import MouseButton
from com.sun.star.awt import MouseEvent
from com.sun.star.awt import KeyEvent
from com.sun.star.awt import XKeyListener


mouseListenerCount = 0
mouseEventsIntercepted = 0
mousePressedEventsIntercepted = 0
mouseReleasedEventsIntercepted = 0
mouseEnteredEventsIntercepted = 0
mouseExitedEventsIntercepted = 0
keymousePressedEventsIntercepted = 0
keymouseReleasedEventsIntercepted = 0


class XMouseListenerExtended(unohelper.Base, XMouseListener):
    def __init__(self):
        global mouseListenerCount
        mouseListenerCount += 1
        super().__init__()

    # is invoked when a mouse button has been pressed on a window.
    @classmethod
    def mousePressed(self, xMouseEvent):
        global mousePressedEventsIntercepted
        mousePressedEventsIntercepted += 1

    # is invoked when a mouse button has been released on a window.
    @classmethod
    def mouseReleased(self, xMouseEvent):
        global mouseReleasedEventsIntercepted
        mouseReleasedEventsIntercepted += 1

    # is invoked when the mouse enters a window.
    @classmethod
    def mouseEntered(self, xMouseEvent):
        global mouseEventsIntercepted
        mouseEventsIntercepted += 1
        return super(XMouseListenerExtended, self).mouseEntered(xMouseEvent)

    # is invoked when the mouse exits a window.
    @classmethod
    def mouseExited(self, xMouseEvent):
        global mouseEventsIntercepted
        mouseEventsIntercepted += 1
        return super(XMouseListenerExtended, self).mouseExited(xMouseEvent)


class XKeyListenerExtended(unohelper.Base, XKeyListener):
    # is invoked when a key has been pressed
    @classmethod
    def keyPressed(self, xKeyEvent):
        global keymousePressedEventsIntercepted
        keymousePressedEventsIntercepted += 1
        return super(XKeyListenerExtended, self).keyPressed(xKeyEvent)

    # is invoked when a key has been released
    @classmethod
    def keyReleased(self, xKeyEvent):
        global keymouseReleasedEventsIntercepted
        keymouseReleasedEventsIntercepted += 1
        return super(XKeyListenerExtended, self).keyReleased(xKeyEvent)

# Test that registered mouse/key listeners for top window receive mouse/key events
class XWindow(UITestCase):
    def test_listeners(self):
        global mouseListenerCount

        self.ui_test.create_doc_in_start_center("writer")
        xDoc = self.ui_test.get_component()

        # create new mouse listener
        xFrame = xDoc.getCurrentController().getFrame()
        self.assertIsNotNone(xFrame)
        xWindow = xFrame.getContainerWindow()
        self.assertIsNotNone(xWindow)

        # add new mouse listener
        xMouseListener = XMouseListenerExtended()
        self.assertIsNotNone(xMouseListener)
        xWindow.addMouseListener(xMouseListener)
        self.assertEqual(1, mouseListenerCount)

        # add new key listener
        xKeyListener = XKeyListenerExtended()
        self.assertIsNotNone(xKeyListener)
        xWindow.addKeyListener(xKeyListener)

        # create dummy mouse event
        xMouseEvent = MouseEvent()
        xMouseEvent.Modifiers = 0
        xMouseEvent.Buttons = MouseButton.LEFT
        xMouseEvent.X = 10
        xMouseEvent.Y = 10
        xMouseEvent.ClickCount = 1
        xMouseEvent.PopupTrigger = False
        xMouseEvent.Source = xWindow

        xMouseEvent2 = MouseEvent()
        xMouseEvent2.Modifiers = 0
        xMouseEvent2.Buttons = MouseButton.LEFT
        xMouseEvent2.X = 300
        xMouseEvent2.Y = 300
        xMouseEvent2.ClickCount = 1
        xMouseEvent2.PopupTrigger = False
        xMouseEvent2.Source = xWindow

        # send mouse event
        xToolkitRobot = xWindow.getToolkit()
        self.assertIsNotNone(xToolkitRobot)

        # Click in the menubar/toolbar area
        xToolkitRobot.mouseMove(xMouseEvent)
        xToolkitRobot.mousePress(xMouseEvent)
        xToolkitRobot.mouseRelease(xMouseEvent)

        # Click into the document content
        xToolkitRobot.mousePress(xMouseEvent2)
        xToolkitRobot.mouseRelease(xMouseEvent2)

        # send key press event
        xKeyEvent = KeyEvent()
        xKeyEvent.Modifiers = 0
        xKeyEvent.KeyCode = 70
        xKeyEvent.KeyChar = 70
        xKeyEvent.Source = xWindow

        xToolkitRobot.keyPress(xKeyEvent)
        xToolkitRobot.keyRelease(xKeyEvent)

        # Wait for async events to be processed
        xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
        xToolkit.processEventsToIdle()

        # remove mouse listener
        xWindow.removeMouseListener(xMouseListener)
        self.assertEqual(1, mouseListenerCount)
        del xMouseListener

        # remove key listener
        xWindow.removeKeyListener(xKeyListener)
        del xKeyListener

        global keymousePressedEventsIntercepted
        # Not expected any interceptions
        self.assertEqual(0, keymousePressedEventsIntercepted)

        global keymouseReleasedEventsIntercepted
        # Not expected any interceptions
        self.assertEqual(0, keymouseReleasedEventsIntercepted)

        global mousePressedEventsIntercepted
        self.assertEqual(0, mousePressedEventsIntercepted)

        global mouseReleasedEventsIntercepted
        self.assertEqual(0, mouseReleasedEventsIntercepted)

        global mouseEventsIntercepted
        # Not expected 3 interceptions
        self.assertEqual(0, mouseEventsIntercepted)

        # close document
        self.ui_test.close_doc()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
