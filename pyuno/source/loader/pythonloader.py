#*************************************************************************
#
#   $RCSfile: pythonloader.py,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: jbu $ $Date: 2003-03-23 12:12:53 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Ralph Thomas
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): Ralph Thomas, Joerg Budischewski
#
#*************************************************************************
import uno
import unohelper
from com.sun.star.uno import Exception,RuntimeException
from com.sun.star.loader import XImplementationLoader
from com.sun.star.lang import XServiceInfo

MODULE_PROTOCOL = "vnd.openoffice.pymodule:"
DEBUG = 0

g_supportedServices  = "com.sun.star.loader.Python",      # referenced by the native C++ loader !
g_implementationName = "org.openoffice.comp.pyuno.Loader" # referenced by the native C++ loader !

class Loader( XImplementationLoader, XServiceInfo, unohelper.Base ):
      def __init__(self, ctx ):
	  if DEBUG:
	     print "pythonloader.Loader ctor" 
	  self.ctx = ctx

      def extractModuleFromUrl( self, url ):
	  if url.startswith( MODULE_PROTOCOL ):
	     return url[len(MODULE_PROTOCOL):len(url)]
	  else:
             nColon = url.find( ":" )
	     if -1 == nColon:
		raise RuntimeException( "PythonLoader: No protocol in url " + url )
             else:
	        raise RuntimeException( "PythonLoader: Unknown protocol "+url[0:nColon] 
				     + " in " + url )
	   
      def activate( self, implementationName, dummy, locationUrl, regKey ):
	  if DEBUG:
	     print "pythonloader.Loader.activate"
	  moduleName = self.extractModuleFromUrl( locationUrl )
  	  if DEBUG:
	     print "extracting modulname " + moduleName + " from url " + locationUrl

          try:	     
	     mod = __import__( moduleName )
	     implHelper = mod.__dict__.get( "g_ImplementationHelper" , None )
	     if implHelper == None:
		return mod.getComponentFactory( implementationName, self.ctx.ServiceManager, regKey )
	     else:
		return implHelper.getComponentFactory( implementationName,regKey,self.ctx.ServiceManager)
          except ImportError,e:
	     raise RuntimeException( "Couldn't import module " + moduleName + " for reason ("+str(e)+")", None)
	     
      def writeRegistryInfo( self, regKey, dummy, locationUrl ):
	  if DEBUG:
	     print "pythonloader.Loader.writeRegistryInfo"
	  moduleName = self.extractModuleFromUrl( locationUrl )
	  try:
	     mod = __import__( moduleName )
	     implHelper = mod.__dict__.get( "g_ImplementationHelper" , None )
	     if implHelper == None:
	        return mod.writeRegistryInfo( self.ctx.ServiceManager, regKey )
	     else:
	        return implHelper.writeRegistryInfo( regKey, self.ctx.ServiceManager )
	  except ImportError,e:
	     raise RuntimeException( "Couldn't import module " + moduleName + " for reason ("+str(e)+")", None)

      def getImplementationName( self ):
	  return g_implementationName

      def supportsService( self, ServiceName ):
	  return ServiceName in self.serviceNames

      def getSupportedServiceNames( self ):
	  return g_supportedServices


