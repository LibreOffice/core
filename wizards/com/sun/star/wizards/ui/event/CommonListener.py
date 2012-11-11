#**********************************************************************
#
#   Danny.OOo.Listeners.ListenerProcAdapters.py
#
#   A module to easily work with OpenOffice.org.
#
#**********************************************************************
#   Copyright (c) 2003-2004 Danny Brewer
#   d29583@groovegarden.com
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License as published by the Free Software Foundation; either
#   version 2.1 of the License, or (at your option) any later version.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#   See:  http://www.gnu.org/licenses/lgpl.html
#
#**********************************************************************
#   If you make changes, please append to the change log below.
#
#   Change Log
#   Danny Brewer         Revised 2004-06-05-01
#
import unohelper
from com.sun.star.awt import XActionListener

class ActionListenerProcAdapter( unohelper.Base, XActionListener ):
    def __init__( self, oProcToCall, tParams=() ):
        self.oProcToCall = oProcToCall # a python procedure
        self.tParams = tParams # a tuple


    # oActionEvent is a com.sun.star.awt.ActionEvent struct.
    def actionPerformed( self, oActionEvent ):
        if callable( self.oProcToCall ):
            apply( self.oProcToCall )

from com.sun.star.awt import XItemListener
class ItemListenerProcAdapter( unohelper.Base, XItemListener ):
    def __init__( self, oProcToCall, tParams=() ):
        self.oProcToCall = oProcToCall # a python procedure
        self.tParams = tParams # a tuple

    # oItemEvent is a com.sun.star.awt.ItemEvent struct.
    def itemStateChanged( self, oItemEvent ):
        if callable( self.oProcToCall ):
            try:
                apply( self.oProcToCall)
            except:
                apply( self.oProcToCall, (oItemEvent,) + self.tParams )

from com.sun.star.awt import XTextListener
class TextListenerProcAdapter( unohelper.Base, XTextListener ):
    def __init__( self, oProcToCall, tParams=() ):
        self.oProcToCall = oProcToCall # a python procedure
        self.tParams = tParams # a tuple

    # oTextEvent is a com.sun.star.awt.TextEvent struct.
    def textChanged( self, oTextEvent ):
        if callable( self.oProcToCall ):
            apply( self.oProcToCall )

from com.sun.star.frame import XTerminateListener
class TerminateListenerProcAdapter( unohelper.Base, XTerminateListener ):
    def __init__( self, oProcToCall, tParams=() ):
        self.oProcToCall = oProcToCall # a python procedure
        self.tParams = tParams # a tuple

    def queryTermination(self, TerminateEvent):
        self.oProcToCall = getattr(self.oProcToCall,"queryTermination")
        if callable( self.oProcToCall ):
            apply( self.oProcToCall )

from com.sun.star.awt import XWindowListener
class WindowListenerProcAdapter( unohelper.Base, XWindowListener ):
    def __init__( self, oProcToCall, tParams=() ):
        self.oProcToCall = oProcToCall # a python procedure
        self.tParams = tParams # a tuple

    def windowShown(self, TerminateEvent):
        if callable( self.oProcToCall ):
            apply( self.oProcToCall )

from com.sun.star.awt import XAdjustmentListener
class AdjustmentListenerProcAdapter( unohelper.Base, XAdjustmentListener ):
    def __init__( self, oProcToCall, tParams=() ):
        self.oProcToCall = oProcToCall # a python procedure
        self.tParams = tParams # a tuple

    def adjustmentValueChanged(self, TerminateEvent):
        if callable( self.oProcToCall ):
            apply( self.oProcToCall )

from com.sun.star.awt import XFocusListener
class FocusListenerProcAdapter( unohelper.Base, XFocusListener ):
    def __init__( self, oProcToCall, tParams=() ):
        self.oProcToCall = oProcToCall # a python procedure
        self.tParams = tParams # a tuple

    def focusGained(self, FocusEvent):
        if callable( self.oProcToCall ):
            apply( self.oProcToCall, (FocusEvent,) + self.tParams )

from com.sun.star.awt import XKeyListener
class KeyListenerProcAdapter( unohelper.Base, XKeyListener ):
    def __init__( self, oProcToCall, tParams=() ):
        self.oProcToCall = oProcToCall # a python procedure
        self.tParams = tParams # a tuple

    def keyPressed(self, KeyEvent):
        if callable( self.oProcToCall ):
            apply( self.oProcToCall, (KeyEvent,) + self.tParams )
