# instantiating
import uno

localContext = uno.getComponentContext()
resolver = localContext.ServiceManager.createInstanceWithContext(
				"com.sun.star.bridge.UnoUrlResolver", localContext )
remoteContext = resolver.resolve( "uno:socket,host=localhost,port=2002;urp;StarOffice.ComponentContext" )
remoteSmgr = remoteContext.ServiceManager

pyComp = remoteSmgr.createInstanceWithContext( "org.openoffice.demo.SWriter" , remoteContext )

pyComp.run( (), )

