'''
import com.sun.star.awt.*;

import com.sun.star.lang.EventObject;
'''
from AbstractListener import AbstractListener
from EventNames import *

class CommonListener(AbstractListener):

    def __init__(self):
        pass

    '''Implementation of com.sun.star.awt.XActionListener'''
    def actionPerformed(self, actionEvent):
        self.invoke(self.getEventSourceName(actionEvent), EVENT_ACTION_PERFORMED, actionEvent)

    '''Implementation of com.sun.star.awt.XItemListener'''
    def itemStateChanged(self, itemEvent):
        self.invoke(self.getEventSourceName(itemEvent), EVENT_ITEM_CHANGED, itemEvent)

    '''Implementation of com.sun.star.awt.XTextListener'''
    def textChanged(self, textEvent):
        self.invoke(self.getEventSourceName(textEvent), EVENT_TEXT_CHANGED, textEvent)

    '''@see com.sun.star.awt.XWindowListener#windowResized(com.sun.star.awt.WindowEvent)'''
    def windowResized(self, event):
        self.invoke(self.getEventSourceName(event), EVENT_WINDOW_RESIZED, event)

    '''@see com.sun.star.awt.XWindowListener#windowMoved(com.sun.star.awt.WindowEvent)'''
    def windowMoved(self, event):
        self.invoke(self.getEventSourceName(event), EVENT_WINDOW_MOVED, event)

    '''@see com.sun.star.awt.XWindowListener#windowShown(com.sun.star.lang.EventObject)'''
    def windowShown(self, event):
        self.invoke(self.getEventSourceName(event), EVENT_WINDOW_SHOWN, event)

    '''@see com.sun.star.awt.XWindowListener#windowHidden(com.sun.star.lang.EventObject)'''
    def windowHidden(self, event):
        self.invoke(self.getEventSourceName(event), EVENT_WINDOW_HIDDEN, event)

    '''@see com.sun.star.awt.XMouseListener#mousePressed(com.sun.star.awt.MouseEvent)'''
    def mousePressed(self, event):
        self.invoke(self.getEventSourceName(event), EVENT_MOUSE_PRESSED, event)

    '''@see com.sun.star.awt.XMouseListener#mouseReleased(com.sun.star.awt.MouseEvent)'''
    def mouseReleased(self, event):
        self.invoke(self.getEventSourceName(event), EVENT_KEY_RELEASED, event)

    '''@see com.sun.star.awt.XMouseListener#mouseEntered(com.sun.star.awt.MouseEvent)'''
    def mouseEntered(self, event):
        self.invoke(self.getEventSourceName(event), EVENT_MOUSE_ENTERED, event)

    '''@see com.sun.star.awt.XMouseListener#mouseExited(com.sun.star.awt.MouseEvent)'''
    def mouseExited(self, event):
        self.invoke(self.getEventSourceName(event), EVENT_MOUSE_EXITED, event)

    '''@see com.sun.star.awt.XFocusListener#focusGained(com.sun.star.awt.FocusEvent)'''
    def focusGained(self, event):
        self.invoke(self.getEventSourceName(event), EVENT_FOCUS_GAINED, event)

    '''@see com.sun.star.awt.XFocusListener#focusLost(com.sun.star.awt.FocusEvent)'''
    def focusLost(self, event):
        self.invoke(self.getEventSourceName(event), EVENT_FOCUS_LOST, event)

    '''@see com.sun.star.awt.XKeyListener#keyPressed(com.sun.star.awt.KeyEvent)'''
    def keyPressed(self, event):
        self.invoke(c(event), EVENT_KEY_PRESSED, event)

    '''@see com.sun.star.awt.XKeyListener#keyReleased(com.sun.star.awt.KeyEvent)'''
    def keyReleased(self, event):
        self.invoke(self.getEventSourceName(event), EVENT_KEY_RELEASED, event)

