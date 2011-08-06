# -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-

import uno

localContext = uno.getComponentContext()
resolver = localContext.ServiceManager.createInstanceWithContext(
           "com.sun.star.bridge.UnoUrlResolver", localContext )
remoteContext = resolver.resolve( "uno:socket,host=localhost,port=2002;urp;StarOffice.ComponentContext" )
remoteSmgr = remoteContext.ServiceManager

pyComp = remoteSmgr.createInstanceWithContext( "org.openoffice.demo.SWriter" , remoteContext )

pyComp.run( (), )

# vim:set shiftwidth=4 softtabstop=4 expandtab:
