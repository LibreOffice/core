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
# inspired by core/testautomation/extensions/optional/input/extension_sources/TaskPaneComponent
#

import uno
import unohelper

from com.sun.star.ui import XUIElementFactory
from com.sun.star.ui import XUIElement
from com.sun.star.ui.UIElementType import TOOLPANEL as unoTOOLPANEL
from com.sun.star.ui import XToolPanel

implementation_name = "org.libreoffice.example.toolpanel.pocFactory" # as defined in Factory.xcu
implementation_services = ("org.libreoffice.example.toolpanel.pocFactory",)

xdlPath = "toolpanels/poc.xdl" # the path inside the oxt
extensionID = "org.libreoffice.example.toolpanel" # as defined in description.xml

########################################################################
########################################################################

class pocToolPanel( unohelper.Base, XToolPanel ):

        def __init__ ( self, xPanelWindow, ctx ):

                self.ctx = ctx
                self.PanelWindow = xPanelWindow
                self.Window = xPanelWindow

        def createAccessible( self, i_parentAccessible ):

                return self.PanelWindow

########################################################################
########################################################################

class pyPanel( unohelper.Base, XUIElement ):

        def __init__ ( self, ctx, xFrame, xParentWindow, url ):

                self.ctx = ctx
                self.xParentWindow = xParentWindow
                self.toolpanel = None
                self.m_panelRootWindow = None

                self.Frame = xFrame
                self.ResourceURL = url
                self.Type = unoTOOLPANEL


        def getRealInterface( self ):

                if not self.toolpanel:
                        rootWindow = self.getOrCreatePanelRootWindow()
                        self.toolpanel = pocToolPanel(rootWindow, self.ctx)

                return self.toolpanel

        def getOrCreatePanelRootWindow( self ):

                pip = self.ctx.getValueByName("/singletons/com.sun.star.deployment.PackageInformationProvider" )
                s = pip.getPackageLocation(extensionID)
                dialogUrl = s + "/" + xdlPath

                provider = self.ctx.ServiceManager.createInstanceWithContext("com.sun.star.awt.ContainerWindowProvider",self.ctx)  
                self.m_panelRootWindow = provider.createContainerWindow(dialogUrl,"",self.xParentWindow, None)

                return self.m_panelRootWindow

        def postDisposing( self ):

                super.postDisposing()

                if self.m_panelRootWindow:
                    self.m_panelRootWindow.dispose()
                    self.m_panelRootWindow = None

                self.toolpanel = None

                return

#######################################################################
#######################################################################

class pocFactory( unohelper.Base, XUIElementFactory ):

        def __init__ ( self, ctx ):

                self.ctx = ctx

        def createUIElement( self, url, properties ):

            xParentWindow = None
            xFrame = None
            xUIElement = None

            for arg in properties:
                if arg.Name == "Frame":
                    xFrame = arg.Value
                elif arg.Name == "ParentWindow":
                    xParentWindow = arg.Value

            if xFrame and xParentWindow:
                try:
                    xUIElement = pyPanel(self.ctx, xFrame, xParentWindow, url)
                except Exception as e:
                    print(e)

            return xUIElement

########################################################################
########################################################################

# pythonloader looks for a static g_ImplementationHelper variable
g_ImplementationHelper = unohelper.ImplementationHelper ()

# add the FormatFactory class to the implementation container,
# which the loader uses to register/instantiate the component.
g_ImplementationHelper.addImplementation (pocFactory,
					implementation_name,
					implementation_services,
					)

