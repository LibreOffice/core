### *************************************************************************
### *
### *  $RCSfile: officehelper.py,v $
### *
### *  $Revision: 1.2 $
### *
### *  last change: $Author: obo $ $Date: 2006-03-22 10:46:00 $
### *
### *  The Contents of this file are made available subject to
### *  the terms of GNU Lesser General Public License Version 2.1.
### *
### *
### *    GNU Lesser General Public License Version 2.1
### *    =============================================
### *    Copyright 2005 by Sun Microsystems, Inc.
### *    901 San Antonio Road, Palo Alto, CA 94303, USA
### *
### *    This library is free software; you can redistribute it and/or
### *    modify it under the terms of the GNU Lesser General Public
### *    License version 2.1, as published by the Free Software Foundation.
### *
### *    This library is distributed in the hope that it will be useful,
### *    but WITHOUT ANY WARRANTY; without even the implied warranty of
### *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
### *    Lesser General Public License for more details.
### *
### *    You should have received a copy of the GNU Lesser General Public
### *    License along with this library; if not, write to the Free Software
### *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
### *    MA  02111-1307  USA
### *
### ************************************************************************/

#
# Translated to python from "Bootstrap.java" by Kim Kulak
#

import os
import random
from sys import platform
from time import sleep

import uno
from com.sun.star.connection import NoConnectException
from com.sun.star.uno import Exception as UnoException


class BootstrapException(UnoException):
    pass

def bootstrap():
    """Bootstrap OOo and PyUNO Runtime.
    The soffice process is started opening a named pipe of random name, then the local context is used
	to access the pipe. This function directly returns the remote component context, from whereon you can
	get the ServiceManager by calling getServiceManager() on the returned object.
	"""
    try:
	# soffice script used on *ix, Mac; soffice.exe used on Windoof
        sOffice = os.path.join(os.path.dirname(__file__), "soffice")
        if platform.startswith("win"): 
            sOffice += ".exe"
		
        # Generate a random pipe name.
        random.seed()
        sPipeName = "uno" + str(random.random())[2:]
        
        # Start the office proces, don't check for exit status since an exception is caught anyway if the office terminates unexpectedly.
        cmdArray = (sOffice, "-nologo", "-nodefault", "".join(["-accept=pipe,name=", sPipeName, ";urp;"]))        
        os.spawnv(os.P_NOWAIT, sOffice, cmdArray)
			
        # ---------

        xLocalContext = uno.getComponentContext()
        resolver = xLocalContext.ServiceManager.createInstanceWithContext(
			"com.sun.star.bridge.UnoUrlResolver", xLocalContext)
        sConnect = "".join(["uno:pipe,name=", sPipeName, ";urp;StarOffice.ComponentContext"])

        # Wait until an office is started, but loop only nLoop times (can we do this better???)
        nLoop = 20
        while True:
            try:
                xContext = resolver.resolve(sConnect)
                break
            except NoConnectException:
                nLoop -= 1
                if nLoop <= 0:
                    raise BootstrapException("Cannot connect to soffice server.", None)
                sleep(0.5)  # Sleep 1/2 second.

    except BootstrapException:
        raise     
    except Exception, e:  # Any other exception
        raise BootstrapException("Caught exception " + str(e), None)

    return xContext
