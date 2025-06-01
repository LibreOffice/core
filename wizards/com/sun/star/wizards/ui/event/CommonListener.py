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
import unohelper

from com.sun.star.awt import XActionListener
from com.sun.star.awt import XItemListener
from com.sun.star.awt import XTextListener
from com.sun.star.frame import XTerminateListener
from com.sun.star.awt import XWindowListener
from com.sun.star.awt import XAdjustmentListener
from com.sun.star.awt import XFocusListener
from com.sun.star.awt import XKeyListener

class ActionListenerProcAdapter( unohelper.Base, XActionListener ):
    def __init__(self, oProcToCall):
        self.oProcToCall = oProcToCall

    def actionPerformed( self, oActionEvent ):
        if callable( self.oProcToCall ):
            self.oProcToCall()

    def disposing(self, Event):
        # TODO: Implement ?
        pass

class ItemListenerProcAdapter( unohelper.Base, XItemListener ):
    def __init__(self, oProcToCall):
        self.oProcToCall = oProcToCall

    def itemStateChanged( self, oItemEvent ):
        if callable( self.oProcToCall ):
            try:
                self.oProcToCall()
            except Exception:
                self.oProcToCall(oItemEvent)

    def disposing(self, Event):
        # TODO: Implement ?
        pass

class TextListenerProcAdapter( unohelper.Base, XTextListener ):
    def __init__(self, oProcToCall):
        self.oProcToCall = oProcToCall

    def textChanged( self, oTextEvent ):
        if callable( self.oProcToCall ):
            self.oProcToCall()

    def disposing(self, Event):
        # TODO: Implement ?
        pass

class TerminateListenerProcAdapter( unohelper.Base, XTerminateListener ):
    def __init__(self, oProcToCall):
        self.oProcToCall = oProcToCall

    def queryTermination(self, TerminateEvent):
        if callable( self.oProcToCall ):
            self.oProcToCall()

class WindowListenerProcAdapter( unohelper.Base, XWindowListener ):
    def __init__(self, oProcToCall):
        self.oProcToCall = oProcToCall

    def windowShown(self, TerminateEvent):
        if callable( self.oProcToCall ):
            self.oProcToCall()

    def windowHidden(self, Event):
        # TODO: Implement ?
        pass

    def windowResized(self, Event):
        # TODO: Implement ?
        pass

    def windowMoved(self, Event):
        # TODO: Implement ?
        pass

    def disposing(self, Event):
        # TODO: Implement ?
        pass

class AdjustmentListenerProcAdapter( unohelper.Base, XAdjustmentListener ):
    def __init__(self, oProcToCall):
        self.oProcToCall = oProcToCall

    def adjustmentValueChanged(self, TerminateEvent):
        if callable( self.oProcToCall ):
            self.oProcToCall()

class FocusListenerProcAdapter( unohelper.Base, XFocusListener ):
    def __init__( self, oProcToCall):
        self.oProcToCall = oProcToCall

    def focusGained(self, FocusEvent):
        if callable( self.oProcToCall ):
            self.oProcToCall(FocusEvent)

class KeyListenerProcAdapter( unohelper.Base, XKeyListener ):
    def __init__(self, oProcToCall):
        self.oProcToCall = oProcToCall

    def keyPressed(self, KeyEvent):
        if callable( self.oProcToCall ):
            self.oProcToCall(KeyEvent)

    def disposing(self, Event):
        # TODO: Implement ?
        pass
