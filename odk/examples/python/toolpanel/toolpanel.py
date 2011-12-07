# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Laurent Godard <lgodard.libre@laposte.net>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# inspired from core/testautomation/extensions/optional/input/extension_sources/TaskPaneComponent
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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

