#*************************************************************************
#
#   $RCSfile: unohelper.py,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: jbu $ $Date: 2003-03-23 12:12:59 $
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
import pyuno

from com.sun.star.lang import XTypeProvider, XSingleComponentFactory, XServiceInfo
from com.sun.star.uno import RuntimeException

def createSingleServiceFactory( clazz, implementationName, serviceNames ):
    return _FactoryHelper_( clazz, implementationName, serviceNames )

class _ImplementationHelperEntry:
      def __init__(self, ctor,serviceNames):
	  self.ctor = ctor
	  self.serviceNames = serviceNames
	  
class ImplementationHelper:
      def __init__(self):
	  self.impls = {}
	  
      def addImplementation( self, ctor, implementationName, serviceNames ):
          self.impls[implementationName] =  _ImplementationHelperEntry(ctor,serviceNames)
	  
      def writeRegistryInfo( self, regKey, smgr ):
          for i in self.impls.items():
	      keyName = "/"+ i[0] + "/UNO/SERVICES"
	      key = regKey.createKey( keyName )
	      for serviceName in i[1].serviceNames:
		  key.createKey( serviceName )
          return 1

      def getComponentFactory( self, implementationName , regKey, smgr ):
	  entry = self.impls.get( implementationName, None )
	  if entry == None:
	     raise RuntimeException( implementationName + " is unknown" , None )
	  return createSingleServiceFactory( entry.ctor, implementationName, entry.serviceNames )

      def getSupportedServiceNames( self, implementationName ):
	  entry = self.impls.get( implementationName, None )
	  if entry == None:
	     raise RuntimeException( implementationName + " is unknown" , None )
	  return entry.serviceNames	     
	  
      def supportsService( self, implementationName, serviceName ):
	  entry = self.impls.get( implementationName,None )
	  if entry == None:
	     raise RuntimeException( implementationName + " is unknown", None )
          return serviceName in entry.serviceNames	     

	  
class ImplementationEntry:
      def __init__(self, implName, supportedServices, clazz ):
	  self.implName = implName
	  self.supportedServices = supportedServices
	  self.clazz = clazz

def writeRegistryInfoHelper( smgr, regKey, seqEntries ):
    for entry in seqEntries:
        keyName = "/"+ entry.implName + "/UNO/SERVICES"
	key = regKey.createKey( keyName )
	for serviceName in entry.supportedServices:
	    key.createKey( serviceName )

def systemPathToFileUrl( systemPath ):
    "returns a file-url for the given system path"
    return pyuno.systemPathToFileUrl( systemPath )

def fileUrlToSystemPath( url ):
    "returns a system path (determined by the system, the python interpreter is running on)"
    return pyuno.fileUrlToSystemPath( url )

def absolutize( path, relativeUrl ):
    "returns an absolute file url from the given urls"
    return pyuno.absolutize( path, relativeUrl )
        
def getComponentFactoryHelper( implementationName, smgr, regKey, seqEntries ):
    for x in seqEntries:
	if x.implName == implementationName:
	   return createSingleServiceFactory( x.clazz, implementationName, x.supportedServices )

def addComponentsToContext( toBeExtendedContext, contextRuntime, componentUrls, loaderName ):
    smgr = contextRuntime.ServiceManager
    loader = smgr.createInstanceWithContext( loaderName, contextRuntime )
    implReg = smgr.createInstanceWithContext( "com.sun.star.registry.ImplementationRegistration",contextRuntime)
    
    #   create a temporary registry
    for componentUrl in componentUrls:
        reg = smgr.createInstanceWithContext( "com.sun.star.registry.SimpleRegistry", contextRuntime )
	reg.open( "", 0, 1 )
	implReg.registerImplementation( loaderName,componentUrl, reg )
	rootKey = reg.getRootKey()
	implementationKey = rootKey.openKey( "IMPLEMENTATIONS" )
	implNames = implementationKey.getKeyNames()
	extSMGR = toBeExtendedContext.ServiceManager
	for x in implNames:
	    fac = loader.activate( max(x.split("/")),"",componentUrl,rootKey)
	    extSMGR.insert( fac )
	reg.close()
	    	    
# never shrinks !
_g_typeTable = {}
def _unohelper_getHandle( self):
   ret = None
   if _g_typeTable.has_key( self.__class__ ):
     ret = _g_typeTable[self.__class__]
   else:
     names = {}
     traverse = list(self.__class__.__bases__)
     while len( traverse ) > 0:
         item = traverse.pop()
         bases = item.__bases__
         if uno.isInterface( item ):
             names[item.__pyunointerface__] = None
         elif len(bases) > 0:
             # the "else if", because we only need the most derived interface
             traverse = traverse + list(bases)#

     lst = names.keys()
     types = []
     for x in lst:
         t = uno.getTypeByName( x )
         types.append( t )
         
     ret = tuple(types) , uno.generateUuid()
     _g_typeTable[self.__class__] = ret
   return ret
  
class Base(XTypeProvider):
      def getTypes( self ):
	  return _unohelper_getHandle( self )[0]
      def getImplementationId(self):
	  return _unohelper_getHandle( self )[1]
	  
# -------------------------------------------------
# implementation details
# -------------------------------------------------
class _FactoryHelper_( XSingleComponentFactory, XServiceInfo, Base ):
      def __init__( self, clazz, implementationName, serviceNames ):
	  self.clazz = clazz
	  self.implementationName = implementationName
	  self.serviceNames = serviceNames
	  
      def getImplementationName( self ):
	  return self.implementationName

      def supportsService( self, ServiceName ):
	  return ServiceName in serviceNames

      def getSupportedServiceNames( self ):
	  return self.serviceNames

      def createInstanceWithContext( self, context ):
	  return self.clazz( context )
	      
      def createInstanceWithArgumentsAndContext( self, args, context ):
	  return self.clazz( context, *args )
      
