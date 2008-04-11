#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: pythonloader.py,v $
#
# $Revision: 1.6 $
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
#*************************************************************************
import uno
import unohelper
import sys
import imp
import os
from com.sun.star.uno import Exception,RuntimeException
from com.sun.star.loader import XImplementationLoader
from com.sun.star.lang import XServiceInfo

MODULE_PROTOCOL = "vnd.openoffice.pymodule:"
DEBUG = 0

g_supportedServices  = "com.sun.star.loader.Python",      # referenced by the native C++ loader !
g_implementationName = "org.openoffice.comp.pyuno.Loader" # referenced by the native C++ loader !

def splitUrl( url ):
      nColon = url.find( ":" )
      if -1 == nColon:
            raise RuntimeException( "PythonLoader: No protocol in url " + url, None )
      return url[0:nColon], url[nColon+1:len(url)]

g_loadedComponents = {}
def checkForPythonPathBesideComponent( url ):
      path = unohelper.fileUrlToSystemPath( url+"/pythonpath.zip" );
      if DEBUG == 1:
            print "checking for existence of " + path  
      if 1 == os.access( path, os.F_OK) and not path in sys.path:
            if DEBUG == 1:
                  print "adding " + path + " to sys.path"
            sys.path.append( path )

      path = unohelper.fileUrlToSystemPath( url+"/pythonpath" );
      if 1 == os.access( path, os.F_OK) and not path in sys.path:
            if DEBUG == 1:
                  print "adding " + path + " to sys.path"
            sys.path.append( path )

class Loader( XImplementationLoader, XServiceInfo, unohelper.Base ):
      def __init__(self, ctx ):
	  if DEBUG:
	     print "pythonloader.Loader ctor" 
	  self.ctx = ctx

      def getModuleFromUrl( self, url ):
          if DEBUG:
                print "pythonloader: interpreting url " +url
          protocol, dependent = splitUrl( url )
          if "vnd.sun.star.expand" == protocol:
                exp = self.ctx.getValueByName( "/singletons/com.sun.star.util.theMacroExpander" )
                url = exp.expandMacros(dependent)
                protocol,dependent = splitUrl( url )

          if DEBUG:
                print "pythonloader: after expansion " +protocol +":" + dependent
                
          try:
                if "file" == protocol:
                      # remove \..\ sequence, which may be useful e.g. in the build env
                      url = unohelper.absolutize( url, url )

                      # did we load the module already ?
                      mod = g_loadedComponents.get( url )
                      if not mod:
                            mod = imp.new_module("uno_component")

                            # check for pythonpath.zip beside .py files
                            checkForPythonPathBesideComponent( url[0:url.rfind('/')] )
                            
                            # read the file
                            filename = unohelper.fileUrlToSystemPath( url )
                            fileHandle = file( filename )
                            src = fileHandle.read().replace("\r","")
                            if not src.endswith( "\n" ):
                                  src = src + "\n"

                            # compile and execute the module
                            codeobject = compile( src, filename, "exec" )
                            exec codeobject in mod.__dict__
                            mod.__file__ = filename
                            g_loadedComponents[url] = mod
                      return mod
                elif "vnd.openoffice.pymodule" == protocol:
                      return  __import__( dependent )
                else:
                      raise RuntimeException( "PythonLoader: Unknown protocol " +
                                              protocol + " in url " +url, self )
          except ImportError, e:
                raise RuntimeException( "Couldn't load "+url+ " for reason "+str(e), None)
          return None
	   
      def activate( self, implementationName, dummy, locationUrl, regKey ):
	  if DEBUG:
	     print "pythonloader.Loader.activate"

	  mod = self.getModuleFromUrl( locationUrl )
          implHelper = mod.__dict__.get( "g_ImplementationHelper" , None )
          if implHelper == None:
		return mod.getComponentFactory( implementationName, self.ctx.ServiceManager, regKey )
          else:
		return implHelper.getComponentFactory( implementationName,regKey,self.ctx.ServiceManager)
	     
      def writeRegistryInfo( self, regKey, dummy, locationUrl ):
	  if DEBUG:
	     print "pythonloader.Loader.writeRegistryInfo"
             
	  mod = self.getModuleFromUrl( locationUrl )
          implHelper = mod.__dict__.get( "g_ImplementationHelper" , None )
          if implHelper == None:
	        return mod.writeRegistryInfo( self.ctx.ServiceManager, regKey )
          else:
	        return implHelper.writeRegistryInfo( regKey, self.ctx.ServiceManager )

      def getImplementationName( self ):
	  return g_implementationName

      def supportsService( self, ServiceName ):
	  return ServiceName in self.serviceNames

      def getSupportedServiceNames( self ):
	  return g_supportedServices


