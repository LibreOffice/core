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

import uno
import unohelper

from com.sun.star.awt import Rectangle
from com.sun.star.awt.MessageBoxButtons import BUTTONS_OK
from com.sun.star.awt.MessageBoxType import INFOBOX
from com.sun.star.frame import XDispatch, XDispatchProvider
from com.sun.star.lang import XServiceInfo
from com.sun.star.registry import InvalidRegistryException

class Provider(unohelper.Base, XServiceInfo, XDispatchProvider):
    implementationName = "com.sun.star.comp.test.deployment.active_python"

    serviceNames = ("com.sun.star.test.deployment.active_python",)

    def __init__(self, context):
        self.context = context

    def getImplementationName(self):
        return self.implementationName

    def supportsService(self, ServiceName):
        return ServiceName in self.serviceNames

    def getSupportedServiceNames(self):
        return self.serviceNames

    def queryDispatch(self, URL, TargetFrame, SearchFlags):
        return self.context.getValueByName( \
            "/singletons/com.sun.star.test.deployment.active_python_singleton")

    def queryDispatches(self, Requests):
        tuple( \
            self.queryDispatch(i.FeatureURL, i.FrameName, i.SearchFlags) \
                for i in Requests)

class Dispatch(unohelper.Base, XServiceInfo, XDispatch):
    implementationName = \
        "com.sun.star.comp.test.deployment.active_python_singleton"

    serviceNames = ()

    def __init__(self, context):
        self.context = context

    def getImplementationName(self):
        return self.implementationName

    def supportsService(self, ServiceName):
        return ServiceName in self.serviceNames

    def getSupportedServiceNames(self):
        return self.serviceNames

    def dispatch(self, URL, Arguments):
        smgr = self.context.getServiceManager()
        box = smgr.createInstanceWithContext( \
            "com.sun.star.awt.Toolkit", self.context).createMessageBox( \
                smgr.createInstanceWithContext( \
                    "com.sun.star.frame.Desktop", self.context). \
                    getCurrentFrame().getComponentWindow(), \
                INFOBOX, BUTTONS_OK, "active", "python")
        box.execute();
        box.dispose();

    def addStatusListener(self, Control, URL):
        pass

    def removeStatusListener(self, Control, URL):
        pass

def getComponentFactory(implementationName, smgr, regKey):
    if implementationName == Provider.implementationName:
        return unohelper.createSingleServiceFactory( \
            Provider, Provider.implementationName, Provider.serviceNames)
    elif implementationName == Dispatch.implementationName:
        return unohelper.createSingleServiceFactory( \
            Dispatch, Dispatch.implementationName, Dispatch.serviceNames)
    else:
        return None

def writeRegistryInfo(smgr, regKey):
    try:
        for i in (Provider, Dispatch):
            key = regKey.createKey("/" + i.implementationName + "/UNO")
            for j in i.serviceNames:
                key.createKey("/SERVICES/" + j);
        regKey.createKey( \
            "/" + Dispatch.implementationName + "/UNO/SINGLETONS/" \
                "com.sun.star.test.deployment.active_python_singleton"). \
            setStringValue(Dispatch.implementationName)
    except InvalidRegistryException:
        return False
    return True
