# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os
import subprocess

try:
    import pyuno
    import uno
    import unohelper
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("PYTHONPATH=/installation/opt/program")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

try:
    from com.sun.star.document import XDocumentEventListener
except ImportError:
    print("UNO API class not found: try to set URE_BOOTSTRAP variable")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

class EventListener(XDocumentEventListener,unohelper.Base):

    def __init__(self, xContext, eventName):
        self.xGEB = xContext.ServiceManager.createInstanceWithContext(
            "com.sun.star.frame.GlobalEventBroadcaster", xContext)
        self.xContext = xContext
        self.executed = False
        self.eventName = eventName

    def __enter__(self):
        self.xGEB.addDocumentEventListener(self)
        return self

    def __exit__(self, type, value, traceback):
        self.xGEB.removeDocumentEventListener(self)

    def documentEventOccured(self, event):
        print(str(event.EventName))
        if event.EventName == self.eventName:
            self.executed = True

    def disposing(event):
        pass

def mkPropertyValue(name, value):
    """ Create a UNO ProertyValue from two input values.
    """
    return uno.createUnoStruct("com.sun.star.beans.PropertyValue",
            name, 0, value, 0)

def mkPropertyValues(vals):
    """ Create UNO property values from a map.
    """
    return tuple([mkPropertyValue(name, value) for (name, value) in vals.items()])

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
