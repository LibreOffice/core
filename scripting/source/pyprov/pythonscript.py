# XScript implementation for python
import uno
import unohelper
import sys
import imp

from com.sun.star.lang import typeOfXServiceInfo, typeOfXTypeProvider
from com.sun.star.io import IOException
from com.sun.star.ucb import CommandAbortedException


# pythonloader looks for a static g_ImplementationHelper variable
g_MyImplementationHelper = unohelper.ImplementationHelper()
g_implName = "org.openoffice.pyuno.PythonScriptImpl"
g_implementationId = uno.generateUuid()
g_types = None 

BLOCK_SIZE = 65536
DEBUG = 0

class Context:
    def __init__( self, ctx, doc ):
        if DEBUG:
            print "constructing Context!"

        self.ctx = ctx
        self.desktop = None
        self.doc = doc
        
    # XTypeProvider
    # (can't use unohelper base class, because XScriptContext does not 
    # belong to core package )
    def getTypes(self):
        global g_types
        if not g_types:
            g_types = typeOfXTypeProvider, uno.getTypeByName( \
                "com.sun.star.script.provider.XScriptContext" )
        return g_types

    def getImplementationId(self):
        return g_implementationId

    # XScriptContext
    def getDocument(self):
        if not self.doc:
            self.doc = self.getDesktop().getCurrentComponent()
        return self.doc

    def getDesktop(self):
        if not self.desktop:
            self.desktop = self.ctx.ServiceManager.createInstanceWithContext(
            "com.sun.star.frame.Desktop", self.ctx )
        return self.desktop

    def getComponentContext(self):
        return self.ctx


class PScript:
    def __init__( self, ctx, *args ):
        if DEBUG:
            print "Constructing PythonScript"
                
        self.ctx = ctx
        self.sfa = self.ctx.ServiceManager.createInstanceWithContext( \
            "com.sun.star.ucb.SimpleFileAccess" , self.ctx )
            
        fileName = args[0]
        self.doc = args[1]

        try:
            if DEBUG:
                print "Trying to open file " + fileName
                
            inputStream = self.sfa.openFileRead( fileName  )
            code = uno.ByteSequence("")

            if DEBUG:
                print "Reading file " + fileName
            while True:
                read,out = inputStream.readBytes( None , BLOCK_SIZE )
                if DEBUG:
                    print "read " + str(len( out ))
                code = code + out
                # does this impose a limit on the size of the script??
                if read < BLOCK_SIZE:
                    break

            if DEBUG:
                print "File " + fileName + " read ("+str(len(code.value))+")"

            idx = fileName.rfind("/")
            tmp = fileName[idx+1:]
            idx = tmp.find(".")
            funcName = tmp[0:idx]

            if DEBUG:
                print "funcName " + funcName

            mod = imp.new_module("scripting_framework")
            exec code.value in mod.__dict__
            self.func = mod.__dict__.get( funcName )
        except CommandAbortedException,e:
            if DEBUG:
                print e
            raise InvocationTargetException( \
                "[PythonScript] Error during opening file " + \
                filename + "("+e.Message+")", self, e )
        
        except IOException,e:
            if DEBUG:
                print e
            raise InvocationTargetException( \
                "[PythonScript] Error during opening file " + \
                filename + "("+e.Message+")", self, e )

        except Exception,e:
            if DEBUG:
                print e
            raise
            
        if DEBUG:
            print "Leaving PythonScript-setProperty"

    # XServiceInfo
    def getSupportedServices( self ):
        if DEBUG:
            print "PythonScript-getSupportedServices"
        return g_MyImplementationHelper.getSupportedServices(g_implName)

    def supportsService( self, ServiceName ):
        if DEBUG:
            print "PythonScript-supportsService"
        return g_MyImplementationHelper.supportsService( g_implName, ServiceName )
    def getImplementationName(self):
        if DEBUG:
            print "PythonScript-getImplementationName"
        return g_implName
    
    # XScript
    def invoke( self, params, outIndex, outParams ):
        if DEBUG:
            print "PythonScript-invoke"

        # obviously this needs to be worked on!!
        outIndex = ()
        outParams = ()
        ret = None

        try:
            if self.func:
                #and this, once the outParams are sorted
                self.func( Context(self.ctx, self.doc), *params)
            else:
                raise InvocationTargetException( \
                    "Function has not been initialized correctly" )
        except Exception,e:
            if DEBUG:
                print e
            raise
            
        if DEBUG:
            print "Leaving script-runtime-invoke"

        return ret, outIndex, outParams

    # XTypeProvider
    # (can't use unohelper base class, because XScript does
    # not belong to core package )
    def getTypes(self):
        if DEBUG:
            print "PythonScript-getTypes"
        global g_types
        if not g_types:
            g_types = typeOfXTypeProvider,typeOfXServiceInfo, \
                uno.getTypeByName( \
                    "com.sun.star.script.provider.XScript")
        return g_types

    def getImplementationId(self):
        if DEBUG:
            print "PythonScript-getTypes"
        return g_implementationId
          
g_MyImplementationHelper.addImplementation( \
	PScript,g_implName, \
    ("org.openoffice.pyuno.PythonScript",),)

def writeRegistryInfo( smgr, key ):
    ret =  g_MyImplementationHelper.writeRegistryInfo( key,smgr )
    return ret

def getComponentFactory( implName, smgr , key ):
    return g_MyImplementationHelper.getComponentFactory(implName,key,smgr)
