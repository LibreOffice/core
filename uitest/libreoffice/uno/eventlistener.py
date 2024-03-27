# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

try:
    import unohelper
    from com.sun.star.document import XDocumentEventListener
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("PYTHONPATH=/installation/opt/program")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

class EventListener(XDocumentEventListener,unohelper.Base):

    def __init__(self, xContext, eventNames, **kwargs):
        self.xGEB = xContext.ServiceManager.createInstanceWithContext(
            "com.sun.star.frame.GlobalEventBroadcaster", xContext)
        self.xContext = xContext
        self.executed = False
        self.eventExecuted = []
        self.printEvents = kwargs.get('printNames', False)
        if isinstance(eventNames, str):
            self.eventNames = [eventNames]
        elif isinstance(eventNames, list):
            self.eventNames = eventNames

    def __enter__(self):
        self.xGEB.addDocumentEventListener(self)
        return self

    def __exit__(self, type, value, traceback):
        self.xGEB.removeDocumentEventListener(self)

    def documentEventOccured(self, event):
        print("documentEventOccured: name=" + event.EventName + ", title=" + str(event.Supplement))
        print("documentEventOccured: found event we are waiting for")
        if event.EventName in self.eventNames:
            self.executed = True
            self.eventExecuted.append(event.EventName)

    def hasExecuted(self, eventName):
        return eventName in self.eventExecuted

    def disposing(event):
        pass

# vim: set shiftwidth=4 softtabstop=4 expandtab:
