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

class Screenshot(object):

    def __init__(self):
        pass

    def _create_python_path(self):
        """ creates a clean PATH env variable

        We need to avoid picking the LibreOffice python and
        the corresponding PYTHONPATH, PYTHONHOME variables
        """
        env = os.environ.copy()

        # remove any python properties pointing to soffice internal python
        del env['PYTHONPATH']
        del env['PYTHONHOME']

        # remove path pointing to instdir to avoid picking up the soffice provided python
        path = env['PATH']
        path_parts = path.split(':')
        cleaned_path = (path for path in path_parts if path.find("instdir") == -1)
        new_path = ":".join( cleaned_path )
        env['PATH'] = new_path
        return env

    def take_screenshot(self, x1, y1, x2, y2):
        env = self._create_python_path()
        popen = subprocess.Popen(" ".join(["./screenshot.py", x1, y1, x2, y2]), shell=True, env=env)
        popen.wait()

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
