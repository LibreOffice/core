'''
  This file is part of the LibreOffice project.

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This file incorporates work covered by the following license notice:

    Licensed to the Apache Software Foundation (ASF) under one or more
    contributor license agreements. See the NOTICE file distributed
    with this work for additional information regarding copyright
    ownership. The ASF licenses this file to you under the Apache
    License, Version 2.0 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.apache.org/licenses/LICENSE-2.0 .
'''
import logging
from com.sun.star.accessibility import XAccessible
from com.sun.star.accessibility import XAccessibleComponent
from com.sun.star.accessibility import XAccessibleContext
from com.sun.star.accessibility.AccessibleStateType import SHOWING
from com.sun.star.awt import XWindow
from com.sun.star.frame import XController
from com.sun.star.frame import XFrame
from com.sun.star.frame import XModel
from com.sun.star.uno import UnoRuntime
from com.sun.star.uno import XInterface
from com.sun.star.lang import XServiceInfo

class AccessibilityTools:

    @classmethod
    def setUpClass(cls):
        cls.searchedAccessible = None
        cls.deBug = False

    def getAccessibleObject(self, xObject):
        return xObject

    def getCurrentContainerWindow(self, xModel):
        return getWindow(xModel, True)

    def getCurrentWindow(self, xModel):
        return getWindow(xModel, False)

    def getWindow(self, xModel, containerWindow):
        xWindow = None

        try:
            xController = xModel.getCurrentController()
            xFrame = xController.getFrame()

            if (xFrame is None):
                print ("can't get frame from controller")
            else:
                if containerWindow:
                    xWindow = xFrame.getContainerWindow()
                else:
                    xWindow = xFrame.getComponentWindow()
            if (xWindow is None):
                print ("can't get window from frame")
        except Exception as e:
            print ("caught exception while getting current window", e)
        return xWindow

    def getAccessibleObjectForRole(self, xAcc, role):
        self.searchedAccessible = None
        return getAccessibleObjectForRole_(xAcc, role)

    def getAccessibleObjectForRole(self, xAcc, role, ignoreShowing):
        self.searchedAccessible = None
        if ignoreShowing:
            return getAccessibleObjectForRoleIgnoreShowing_(xAcc, role)
        else:
            return getAccessibleObjectForRole_(xAcc, role)

    def getAccessibleObjectForRoleIgnoreShowing_(self, xAcc, role):
        ac = xAcc.getAccessibleContext()
        if (ac is None):
            return None
        if (ac.getAccessibleRole() == role):
            self.searchedAccessible = xAcc
            return ac
        else:
            k = ac.getAccessibleChildCount()
            if (ac.getAccessibleChildCount() > 100):
                k = 50

            for i in range(k):
                try:
                    ac2 = getAccessibleObjectForRoleIgnoreShowing_(ac.getAccessibleChild(i), role)
                    if ac2 is not None:
                        return ac2
                except IndexError as e:
                    print ("Couldn't get Child")
    return None

    def getAccessibleObjectForRole_(self, xAcc, role):
        ac = xAcc.getAccessibleContext()
        isShowing = ac.getAccessibleStateSet().contains(SHOWING)
        if((ac.getAccessibleRole() == role) and isShowing):
             self.searchedAccessible = xAcc
             return ac
        else:
            k = ac.getAccessibleChildCount()
            if (ac.getAccessibleChildCount() > 100):
                k = 50
            for i in range(k):
                try:
                    ac2 = getAccessibleObjectForRole_(ac.getAccessibleChild(i), role)
                    if ac2 is not None:
                        return ac2
                except IndexError as e:
                    print ("Couldn't get Child")
    return None

    def getAccessibleObjectForRole(self, xAcc, role, name):
        return getAccessibleObjectForRole(xAcc, role, name, "")

    def getAccessibleObjectForRole(self, xAcc, role, name, ignoreShowing):
        if ignoreShowing:
            return getAccessibleObjectForRoleIgnoreShowing(xAcc, role, name, "")
        else:
            return getAccessibleObjectForRole(xAcc, role, name, "")

    def getImplName(self, aObject):
        xSI = aObject
        if xSI is None:
            print ("Unknown, does not implement XServiceInfo")
        else:
            return xSI.getImplementationName()

    def getAccessibleObjectForRoleIgnoreShowing(self, xAcc, role, name, implName):
        ac = xAcc.getAccessibleContext()
        if ((ac.getAccessibleRole() == role) and
            (ac.getAccessibleName().index(name) > -1) and
            (getImplName(ac).index(implName) > -1):
            self.searchedAccessible = xAcc
            return ac
        else:
            k = ac.getAccessibleChildCount()
            if (ac.getAccessibleChildCount() > 100):
                k = 50
            for i in range(k):
                try:
                    ac1 = getAccessibleObjectForRoleIgnoreShowing(ac.getAccessibleChild(i), role, name, implName)
                    if ac1 is not None:
                        return ac1
                except IndexError as e:
                    print ("Couldn't get Child")
        return None

    def getAccessibleObjectForRole(self, xAcc, role, name, implName):
        ac = xAcc.getAccessibleContext()
        isShowing = ac.getAccessibleStateSet().contains(SHOWING)
        if  (role == 0 || ac.getAccessibleRole() == role) and
            (ac.getAccessibleName().index(name) > -1) and
            (getImplName(ac).index(implName) > -1) and
            isShowing:
            self.searchedAccessible = xAcc
            return ac
        else:
            k = ac.getAccessibleChildCount()
            if ac.getAccessibleChildCount() > 100:
                k = 50
            for i in range(k):
                try:
                    ac1 = getAccessibleObjectForRole(ac.getAccessibleChild(i), role, name, implName)
                    if ac1 is not None:
                        return ac1
                except IndexError as e:
                    print ("Couldn't get Child")
        return None

    def printAccessibleTree(self, log, xAcc, debugIsActive):
        self.deBug = debugIsActive
        if deBug:
            printAccessibleTree(log, xAcc, "")

    def printAccessibleTree(self, log, xAcc):
        printAccessibleTree(log, xAcc, "")

    def printAccessibleTree(self, log, xAcc, indent):
        ac = xAcc.getAccessibleContext()
        logging(log, indent + ac.getAccessibleRole() + "," + 
            ac.getAccessibleName() + "(" +
            ac.getAccessibleDescription() + "):" +
            getImplName(ac))
        aComp = xAcc
        if aComp is not None:
            bounds = "(" + aComp.getBounds().X + "," +
                aComp.getBounds().Y + ")" + "(" +
                aComp.getBounds().Width + "," +
                aComp.getBounds().Height + ")"
            bounds = "The boundary Rectangle is " + bounds
            logging(log, indent + indent + bounds)

        isShowing = ac.getAccessibleStateSet().contains(SHOWING)
        logging(log,indent + indent + "StateType contains SHOWING: " + isShowing)
        k = ac.getAccessibleChildCount()
        if ac.getAccessibleChildCount() > 100:
            k = 50
        for i in range(k):
            try:
                printAccessibleTree(log, ac.getAccessibleChild(i), indent + "")
            except IndexError as e:
                print ("Couldn't get Child")
        if ac.getAccessibleChildCount() > 100:
            k = ac.getAccessibleChildCount()
            st = ac.getAccessibleChildCount() - 50
            logging(log, indent + "" + " ...... [skipped] ......")
            for i in range(st, k):
                try:
                    printAccessibleTree(log, ac.getAccessibleChild(i), indent + " ")
                except IndexError as e:
                    print ("Couldn't get Child")

    def accessibleToString(self, AC):
        xAC = AC
        if xAC is not None:
            return xAC.getAccessibleRole() + "," +
                xAC.getAccessibleName() + "(" +
                xAC.getAccessibleDescription() + "):"
        xA = AC
        if xA is None:
            return "(Not supported)"
        xAC = xA.getAccessibleContext()
        return xAC.getAccessibleRole() + "," + xAC.getAccessibleName() + "(" + xAC.getAccessibleDescription() + ")"

    def equals(self, c1, c2):
        if (c1 is None) or (c2 is None):
            return c1 == c2
        return equals(c1.getAccessibleContext(), c2.getAccessibleContext())

    def equals(self, c1, c2):
        if (c1 is None) or (c2 is None):
            return c1 == c2
        if (not(c1.getAccessibleRole() == c2.getAccessibleRole())):
            return False
        if (not(c1.getAccessibleName().equals(c2.getAccessibleName()))):
            return False
        if (not(c1.getAccessibleDescription().equals(c2.getAccessibleDescription()))):
            return False
        if (not(c1.getAccessibleChildCount() == c2.getAccessibleChildCount())):
            return False
        return equals(c1.getAccessibleParent(), c2.getAccessibleParent())

    def logging(self, log, content):
        if deBug:
            logging.debug(content)
