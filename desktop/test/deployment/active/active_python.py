#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#***********************************************************************/

import uno
import unohelper

from com.sun.star.awt import Rectangle
from com.sun.star.awt.MessageBoxButtons import BUTTONS_OK
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
                Rectangle(), "infobox", BUTTONS_OK, "active", "python")
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
