# XScript implementation for python
import uno
import unohelper
import sys
import os
import imp
import time

class LogLevel:
    NONE = 0
    ERROR = 1
    DEBUG = 2

# Configuration ----------------------------------------------------
LogLevel.use = LogLevel.NONE                # production level
#LogLevel.use = LogLevel.ERROR               # for script developers
#LogLevel.use = LogLevel.DEBUG               # for script framework developers
LOG_STDOUT = True                           # True, writes to stdout (difficult on windows)
                                            # False, writes to user/Scripts/python/log.txt
ENABLE_EDIT_DIALOG=False                    # offers a minimal editor for editing.
#-------------------------------------------------------------------

def encfile(uni):
    return uni.encode( sys.getfilesystemencoding())

def lastException2String():
    (excType,excInstance,excTraceback) = sys.exc_info()
    ret = str(excType) + ": "+str(excInstance) + "\n" + \
          uno._uno_extract_printable_stacktrace( excTraceback )
    return ret

def logLevel2String( level ):
    ret = " NONE"
    if level == LogLevel.ERROR:
        ret = "ERROR"
    elif level >= LogLevel.DEBUG:
        ret = "DEBUG"
    return ret

def getLogTarget():
    ret = sys.stdout
    if not LOG_STDOUT:
        try:
            pathSubst = uno.getComponentContext().ServiceManager.createInstance(
                "com.sun.star.util.PathSubstitution" )
            userInstallation =  pathSubst.getSubstituteVariableValue( "user" )
            if len( userInstallation ) > 0:
                systemPath = uno.fileUrlToSystemPath( userInstallation + "/Scripts/python/log.txt" )
                ret = file( systemPath , "a" )
        except Exception,e:
            print "Exception during creation of pythonscript logfile: "+ lastException2String() + "\n, delagating log to stdout\n"
    return ret
  
class Logger(LogLevel):
    def __init__(self , target ):
        self.target = target

    def isDebugLevel( self ):
        return self.use >= self.DEBUG
    
    def debug( self, msg ):
        if self.isDebugLevel():
            self.log( self.DEBUG, msg )
    
    def isErrorLevel( self ):
        return self.use >= self.ERROR

    def error( self, msg ):
        if self.isErrorLevel():
            self.log( self.ERROR, msg )

    def log( self, level, msg ):
        if self.use >= level:
            try:
                self.target.write(
                    time.asctime() +
                    " [" +
                    logLevel2String( level ) +
                    "] " +
                    encfile(msg) +
                    "\n" )
                self.target.flush()
            except Exception,e:
                print "Error during writing to stdout: " +lastException2String() + "\n"

log = Logger( getLogTarget() )

log.debug( "pythonscript loading" )

#from com.sun.star.lang import typeOfXServiceInfo, typeOfXTypeProvider
from com.sun.star.uno import RuntimeException
from com.sun.star.lang import XServiceInfo
from com.sun.star.io import IOException
from com.sun.star.ucb import CommandAbortedException
from com.sun.star.beans import XPropertySet
from com.sun.star.container import XNameContainer
from com.sun.star.xml.sax import XDocumentHandler, InputSource
from com.sun.star.uno import Exception as UnoException
from com.sun.star.script import XInvocation
from com.sun.star.awt import XActionListener

from com.sun.star.script.provider import XScriptProvider, XScript, XScriptContext, ScriptFrameworkErrorException
from com.sun.star.script.browse import XBrowseNode
from com.sun.star.script.browse.BrowseNodeTypes import SCRIPT, CONTAINER, ROOT

LANGUAGENAME = "Python"
GLOBAL_SCRIPTCONTEXT_NAME = "XSCRIPTCONTEXT"
CALLABLE_CONTAINER_NAME =  "g_exportedScripts"

# pythonloader looks for a static g_ImplementationHelper variable
g_ImplementationHelper = unohelper.ImplementationHelper()
g_implName = "org.openoffice.pyuno.LanguageScriptProviderFor"+LANGUAGENAME



BLOCK_SIZE = 65536
def readTextFromStream( inputStream ):
    # read the file
    code = uno.ByteSequence( "" )
    while True:
        read,out = inputStream.readBytes( None , BLOCK_SIZE )
        code = code + out
        if read < BLOCK_SIZE:
           break
    return code.value
    
    
class ModuleEntry:
    def __init__( self, lastRead, module ):
        self.lastRead = lastRead
        self.module = module

def hasChanged( oldDate, newDate ):
    return newDate.Year > oldDate.Year or \
           newDate.Month > oldDate.Month or \
           newDate.Day > oldDate.Day or \
           newDate.Hours > oldDate.Hours or \
           newDate.Minutes > oldDate.Minutes or \
           newDate.Seconds > oldDate.Seconds or \
           newDate.HundredthSeconds > oldDate.HundredthSeconds

def ensureSourceState( code ):
    if not code.endswith( "\n" ):
        code = code + "\n"
    code = code.replace( "\r", "" )
    return code


def checkForPythonPathBesideScript( url ):
    if url.startswith( "file:" ):
        path = unohelper.fileUrlToSystemPath( url+"/pythonpath.zip" );
        log.log( LogLevel.DEBUG,  "checking for existence of " + path )
        if 1 == os.access( encfile(path), os.F_OK) and not path in sys.path:
            log.log( LogLevel.DEBUG, "adding " + path + " to sys.path" )
            sys.path.append( path )

        path = unohelper.fileUrlToSystemPath( url+"/pythonpath" );
        log.log( LogLevel.DEBUG,  "checking for existence of " + path )
        if 1 == os.access( encfile(path), os.F_OK) and not path in sys.path:
            log.log( LogLevel.DEBUG, "adding " + path + " to sys.path" )
            sys.path.append( path )
        
    
class ScriptContext(unohelper.Base):
    def __init__( self, ctx, doc ):
        self.ctx = ctx
        self.doc = doc
       
   # XScriptContext
    def getDocument(self):
        return self.getDesktop().getCurrentComponent()

    def getDesktop(self):
        return self.ctx.ServiceManager.createInstanceWithContext(
            "com.sun.star.frame.Desktop", self.ctx )

    def getComponentContext(self):
        return self.ctx

#----------------------------------
# Global Module Administration
# does not fit together with script
# engine lifetime management
#----------------------------------
#g_scriptContext = ScriptContext( uno.getComponentContext(), None )
#g_modules = {}
#def getModuleByUrl( url, sfa ):
#    entry =  g_modules.get(url)
#    load = True
#    lastRead = sfa.getDateTimeModified( url )
#    if entry:
#        if hasChanged( entry.lastRead, lastRead ):
#            log.isDebugLevel() and log.debug("file " + url + " has changed, reloading")
#        else:
#            load = False
#            
#    if load:
#        log.isDebugLevel() and log.debug( "opening >" + url + "<" )
#
#        code = readTextFromStream( sfa.openFileRead( url ) )
            
        # execute the module
#        entry = ModuleEntry( lastRead, imp.new_module("ooo_script_framework") )
#        entry.module.__dict__[GLOBAL_SCRIPTCONTEXT_NAME] = g_scriptContext
#        entry.module.__file__ = url
#        exec code in entry.module.__dict__
#        g_modules[ url ] = entry
#        log.isDebugLevel() and log.debug( "mapped " + url + " to " + str( entry.module ) )
#    return entry.module

class ProviderContext:
    def __init__( self, storageType, sfa, uriHelper, scriptContext ):
        self.storageType = storageType
        self.sfa = sfa
        self.uriHelper = uriHelper
        self.scriptContext = scriptContext
        self.modules = {}
        self.rootUrl = None
        self.mapPackageName2Path = None

    def getTransientPartFromUrl( self, url ):
        rest = url.replace( self.rootUrl , "",1 ).replace( "/","",1)
        return rest[0:rest.find("/")]
    
    def getPackageNameFromUrl( self, url ):
        rest = url.replace( self.rootUrl , "",1 ).replace( "/","",1)
        start = rest.find("/") +1
        return rest[start:rest.find("/",start)]
        
        
    def removePackageByUrl( self, url ):
        items = self.mapPackageName2Path.items()
        for i in items:
            if url in i[1].pathes:
                self.mapPackageName2Path.pop(i[0])
                break

    def addPackageByUrl( self, url ):
        packageName = self.getPackageNameFromUrl( url )
        transientPart = self.getTransientPartFromUrl( url )
        log.isDebugLevel() and log.debug( "addPackageByUrl : " + packageName + ", " + transientPart )
        if self.mapPackageName2Path.has_key( packageName ):
            package = self.mapPackageName2Path[ packageName ]
            package.pathes = package.pathes + (url, )
        else:
            package = Package( (url,), transientPart)
            self.mapPackageName2Path[ packageName ] = package
    
    def isUrlInPackage( self, url ):
        values = self.mapPackageName2Path.values()
        for i in values:
            if url in i.pathes:
               return True
        return False
            
    def setPackageAttributes( self, mapPackageName2Path, rootUrl ):
        self.mapPackageName2Path = mapPackageName2Path
        self.rootUrl = rootUrl
        
    def getPersistentUrlFromStorageUrl( self, url ):
        # package name is the second directory
        ret = url
        if self.rootUrl:
            pos = len( self.rootUrl) +1
            ret = url[0:pos]+url[url.find("/",pos)+1:len(url)]
        log.isDebugLevel() and log.debug( "getPersistentUrlFromStorageUrl " + url +  " -> "+ ret)
        return ret

    def getStorageUrlFromPersistentUrl( self, url):
        ret = url
        if self.rootUrl:
            pos = len(self.rootUrl)+1
            packageName = url[pos:url.find("/",pos+1)]
            package = self.mapPackageName2Path[ packageName ]
            ret = url[0:pos]+ package.transientPathElement + "/" + url[pos:len(url)]
        log.isDebugLevel() and log.debug( "getStorageUrlFromPersistentUrl " + url + " -> "+ ret)
        return ret
    
    def getModuleByUrl( self, url ):
        entry =  self.modules.get(url)
        load = True
        lastRead = self.sfa.getDateTimeModified( url )
        if entry:
            if hasChanged( entry.lastRead, lastRead ):
                log.isDebugLevel() and log.debug( "file " + url + " has changed, reloading" )
            else:
                load = False
                
        if load:
            log.isDebugLevel() and log.debug( "opening >" + url + "<" )
            
            src = readTextFromStream( self.sfa.openFileRead( url ) )
            checkForPythonPathBesideScript( url[0:url.rfind('/')] )
            src = ensureSourceState( src )            
            
            # execute the module
            entry = ModuleEntry( lastRead, imp.new_module("ooo_script_framework") )
            entry.module.__dict__[GLOBAL_SCRIPTCONTEXT_NAME] = self.scriptContext

            code = None
            if url.startswith( "file:" ):
                code = compile( src, encfile(uno.fileUrlToSystemPath( url ) ), "exec" )
            else:
                code = compile( src, url, "exec" )
            exec code in entry.module.__dict__
            entry.module.__file__ = url
            self.modules[ url ] = entry
            log.isDebugLevel() and log.debug( "mapped " + url + " to " + str( entry.module ) )
        return  entry.module
        
#--------------------------------------------------
def isScript( candidate ):
    ret = False
    if isinstance( candidate, type(isScript) ):
        ret = True
    return ret
    
#-------------------------------------------------------
class ScriptBrowseNode( unohelper.Base, XBrowseNode , XPropertySet, XInvocation, XActionListener ):
    def __init__( self, provCtx, uri, fileName, funcName, func ):
        self.fileName = fileName
        self.funcName = funcName
        self.provCtx = provCtx
        self.func = func
        self.uri = uri
        
    def getName( self ):
        return self.funcName

    def getChildNodes(self):
        return ()

    def hasChildNodes(self):
        return False
    
    def getType( self):
        return SCRIPT

    def getPropertyValue( self, name ):
        ret = None
        if name == "URI":
            ret = self.provCtx.uriHelper.getScriptURI(
                self.provCtx.getPersistentUrlFromStorageUrl( self.uri + "$" + self.funcName ) )
        elif name == "Description":
            ret = getattr( self.func, "__doc__", None )
        elif name == "Editable" and ENABLE_EDIT_DIALOG:
            ret = not self.provCtx.sfa.isReadOnly( self.uri )
        
        log.isDebugLevel() and log.debug( "ScriptBrowseNode.getPropertyValue called for " + name + ", returning " + str(ret) )
        return ret
    def setPropertyValue( self, name, value ):
        log.isDebugLevel() and log.debug( "ScriptBrowseNode.setPropertyValue called " + name + "=" +str(value ) )
    def getPropertySetInfo( self ):
        log.isDebugLevel() and log.debug( "ScriptBrowseNode.getPropertySetInfo called "  )
        return None
               
    def getIntrospection( self ):
        return None

    def invoke( self, name, params, outparamindex, outparams ):
        if name == "Editable":
            servicename = "com.sun.star.awt.DialogProvider"
            ctx = self.provCtx.scriptContext.getComponentContext()
            dlgprov = ctx.ServiceManager.createInstanceWithContext(
                servicename, ctx )

            self.editor = dlgprov.createDialog(
                "vnd.sun.star.script:" +
                "ScriptBindingLibrary.MacroEditor?location=application")

            code = readTextFromStream(self.provCtx.sfa.openFileRead(self.uri))
            code = ensureSourceState( code )
            self.editor.getControl("EditorTextField").setText(code)

            self.editor.getControl("RunButton").setActionCommand("Run")
            self.editor.getControl("RunButton").addActionListener(self)
            self.editor.getControl("SaveButton").setActionCommand("Save")
            self.editor.getControl("SaveButton").addActionListener(self)

            self.editor.execute()

        return None

    def actionPerformed( self, event ):
        try:
            if event.ActionCommand == "Run":
                code = self.editor.getControl("EditorTextField").getText()
                code = ensureSourceState( code )
                mod = imp.new_module("ooo_script_framework")
                mod.__dict__[GLOBAL_SCRIPTCONTEXT_NAME] = self.provCtx.scriptContext
                exec code in mod.__dict__
                values = mod.__dict__.get( CALLABLE_CONTAINER_NAME , None )
                if not values:
                    values = mod.__dict__.values()
                    
                for i in values:
                    if isScript( i ):
                        i()
                        break
                    
            elif event.ActionCommand == "Save":
                toWrite = uno.ByteSequence(
                    str(
                    self.editor.getControl("EditorTextField").getText().encode(
                    sys.getdefaultencoding())) )
                copyUrl = self.uri + ".orig"
                self.provCtx.sfa.move( self.uri, copyUrl )
                out = self.provCtx.sfa.openFileWrite( self.uri )
                out.writeBytes( toWrite )
                out.close()
                self.provCtx.sfa.kill( copyUrl )
#                log.isDebugLevel() and log.debug("Save is not implemented yet")
#                text = self.editor.getControl("EditorTextField").getText()
#                log.isDebugLevel() and log.debug("Would save: " + text)
        except Exception,e:
            # TODO: add an error box here !
            log.error( lastException2String() )
            

    def setValue( self, name, value ):
        return None

    def getValue( self, name ):
        return None

    def hasMethod( self, name ):
        return False

    def hasProperty( self, name ):
        return False

    
#-------------------------------------------------------
class FileBrowseNode( unohelper.Base, XBrowseNode ):
    def __init__( self, provCtx, uri , name ):
        self.provCtx = provCtx
        self.uri = uri
        self.name = name
        self.module = None
        
    def getName( self ):
        return self.name
 
    def getChildNodes(self):
        ret = ()
        try:
            self.module = self.provCtx.getModuleByUrl( self.uri )
            values = self.module.__dict__.get( CALLABLE_CONTAINER_NAME , None )
            
            # no g_exportedScripts, export every function
            if not isinstance(values, type(())):
                values = self.module.__dict__.values()
                    
            scriptNodeList = []
            for i in values:
                if isScript( i ):
                    scriptNodeList.append(
                        ScriptBrowseNode(
                        self.provCtx, self.uri, self.name, i.__name__, i  ))
            ret = tuple( scriptNodeList )
            # must compile  !
            log.isDebugLevel() and log.debug( "returning " +str(len(ret)) + " ScriptChildNodes on " + self.uri )
        except Exception, e:
            text = lastException2String()
            log.error( "Error while evaluating " + self.uri + ":" + text )
            raise
        return ret

    def hasChildNodes(self):
        try:
            return len(self.getChildNodes()) > 0
        except Exception, e:
            return False
    
    def getType( self):
        return CONTAINER

        

class DirBrowseNode( unohelper.Base, XBrowseNode ):
    def __init__( self, provCtx, name, rootUrl ):
        self.provCtx = provCtx
        self.name = name
        self.rootUrl = rootUrl

    def getName( self ):
        return self.name

    def getChildNodes( self ):
        try:
            log.isDebugLevel() and log.debug( "DirBrowseNode.getChildNodes called for " + self.rootUrl )
            contents = self.provCtx.sfa.getFolderContents( self.rootUrl, True )
            browseNodeList = []
            for i in contents:
                if i.endswith( ".py" ):
                    log.isDebugLevel() and log.debug( "adding filenode " + i )
                    browseNodeList.append(
                        FileBrowseNode( self.provCtx, i, i[i.rfind("/")+1:len(i)-3] ) )
                elif self.provCtx.sfa.isFolder( i ) and not i.endswith("/pythonpath"):
                    log.isDebugLevel() and log.debug( "adding DirBrowseNode " + i )
                    browseNodeList.append( DirBrowseNode( self.provCtx, i[i.rfind("/")+1:len(i)],i))
            return tuple( browseNodeList )
        except Exception, e:
            text = lastException2String()
            log.error( "DirBrowseNode error: " + str(e) + " while evaluating " + self.rootUrl)
            log.error( text)
            return ()

    def hasChildNodes( self ):
        return True

    def getType( self ):
        return CONTAINER

    def getScript( self, uri ):
        log.debug( "DirBrowseNode getScript " + uri + " invoked" )
        raise IllegalArgumentException( "DirBrowseNode couldn't instantiate script " + uri , self , 0 )


class ManifestHandler( XDocumentHandler, unohelper.Base ):
    def __init__( self, rootUrl ):
        self.rootUrl = rootUrl
        
    def startDocument( self ):
        self.urlList = []
        
    def endDocument( self ):
        pass
        
    def startElement( self , name, attlist):
        if name == "manifest:file-entry":
            if attlist.getValueByName( "manifest:media-type" ) == "application/vnd.sun.star.framework-script":
                self.urlList.append(
                    self.rootUrl + "/" + attlist.getValueByName( "manifest:full-path" ) )

    def endElement( self, name ):
        pass

    def characters ( self, chars ):
        pass

    def ignoreableWhitespace( self, chars ):
        pass

    def setDocumentLocator( self, locator ):
        pass


# extracts META-INF directory from 
def getPathesFromPackage( rootUrl, sfa ):
    ret = ()
    try:
        fileUrl = rootUrl + "/META-INF/manifest.xml" 
        inputStream = sfa.openFileRead( fileUrl )
        parser = uno.getComponentContext().ServiceManager.createInstance( "com.sun.star.xml.sax.Parser" )
        handler = ManifestHandler( rootUrl )
        parser.setDocumentHandler( handler )
        parser.parseStream( InputSource( inputStream , "", fileUrl, fileUrl ) )
        ret = tuple( handler.urlList )
    except UnoException, e:
        text = lastException2String()
        log.debug( "getPathesFromPackage " + fileUrl + " Exception: " +text )
        pass
    return ret
    

class Package:
    def __init__( self, pathes, transientPathElement ):
        self.pathes = pathes
        self.transientPathElement = transientPathElement

def getPackageName2PathMap( sfa, rootUrl ):
    ret = {}
    contents = sfa.getFolderContents( rootUrl, True )
    for i in contents:
        if sfa.isFolder( i ):
            transientPathElement = lastElement( i )
            subcontents = sfa.getFolderContents( i , True )
            for j in subcontents:
                if sfa.isFolder( j ):
                    # ok, found a package. Now let's have a look, if
                    # it contains scripts
                    pathes = getPathesFromPackage( j, sfa )
                    if len( pathes ) > 0:
                        # map package name to url, we need this later
                        log.isErrorLevel() and log.error( "adding Package " + transientPathElement + " " + str( pathes ) )
                        ret[ lastElement( j ) ] = Package( pathes, transientPathElement )
    return ret

def lastElement( aStr):
    return aStr[ aStr.rfind( "/" )+1:len(aStr)]

class PackageBrowseNode( unohelper.Base, XBrowseNode ):
    def __init__( self, provCtx, name, rootUrl ):
        self.provCtx = provCtx
        self.name = name
        self.rootUrl = rootUrl

    def getName( self ):
        return self.name

    def getChildNodes( self ):
        items = self.provCtx.mapPackageName2Path.items()
        browseNodeList = []
        for i in items:
            if len( i[1].pathes ) == 1:
                browseNodeList.append(
                    DirBrowseNode( self.provCtx, i[0], i[1].pathes[0] ))
            else:
                for j in i[1].pathes:
                    browseNodeList.append(
                        DirBrowseNode( self.provCtx, i[0]+"."+lastElement(j), j ) )
        return tuple( browseNodeList )

    def hasChildNodes( self ):
        return len( self.mapPackageName2Path ) > 0

    def getType( self ):
        return CONTAINER

    def getScript( self, uri ):
        log.debug( "DirBrowseNode getScript " + uri + " invoked" )
        raise IllegalArgumentException( "PackageBrowseNode couldn't instantiate script " + uri , self , 0 )




class PythonScript( unohelper.Base, XScript ):
    def __init__( self, func, mod ):
        self.func = func
        self.mod = mod
    def invoke(self, args, out, outindex ):
        log.isDebugLevel() and log.debug( "PythonScript.invoke " + str( args ) )
        try:
            ret = self.func( *args )
        except UnoException,e:
            # UNO Exception continue to fly ...
            text = lastException2String()
            complete = "Error during invoking function " + \
                str(self.func.__name__) + " in module " + \
                self.mod.__file__ + " (" + text + ")"
            log.isDebugLevel() and log.debug( complete )
            # some people may beat me up for modifying the exception text,
            # but otherwise office just shows
            # the type name and message text with no more information,
            # this is really bad for most users. 
            e.Message = e.Message + " (" + complete + ")"
            raise
        except Exception,e:
            # General python exception are converted to uno RuntimeException
            text = lastException2String()
            complete = "Error during invoking function " + \
                str(self.func.__name__) + " in module " + \
                self.mod.__file__ + " (" + text + ")"
            log.isDebugLevel() and log.debug( complete )
            raise RuntimeException( complete , self )
        log.isDebugLevel() and log.debug( "PythonScript.invoke ret = " + str( ret ) )
        return ret, (), ()

def expandUri(  uri ):
    if uri.startswith( "vnd.sun.star.expand:" ):
        uri = uri.replace( "vnd.sun.star.expand:", "",1)
        uri = uno.getComponentContext().getByName(
                    "/singletons/com.sun.star.util.theMacroExpander" ).expandMacros( uri )
    if uri.startswith( "file:" ):
        uri = uno.absolutize("",uri)   # necessary to get rid of .. in uri
    return uri
    
#--------------------------------------------------------------
class PythonScriptProvider( unohelper.Base, XBrowseNode, XScriptProvider, XNameContainer):
    def __init__( self, ctx, *args ):
        if log.isDebugLevel():
            mystr = ""
            for i in args:
                if len(mystr) > 0:
                    mystr = mystr +","
                mystr = mystr + str(i)
            log.debug( "Entering PythonScriptProvider.ctor" + mystr )

        storageType = ""
        if isinstance(args[0],unicode ):
            storageType = args[0]
        else:
            storageType = args[0].SCRIPTING_DOC_URI
        isPackage = storageType.endswith( ":uno_packages" )

        try:
            urlHelper = ctx.ServiceManager.createInstanceWithArgumentsAndContext(
                "com.sun.star.script.provider.ScriptURIHelper", (LANGUAGENAME, storageType), ctx)
            
            log.isDebugLevel() and log.debug( "got urlHelper " + str( urlHelper ) )
        
            rootUrl = urlHelper.getRootStorageURI()
            log.isDebugLevel() and log.debug( storageType + " transformed to " + rootUrl )

            ucbService = "com.sun.star.ucb.SimpleFileAccess"
            sfa = ctx.ServiceManager.createInstanceWithContext( ucbService, ctx )
            if not sfa:
                log.debug("PythonScriptProvider couldn't instantiate " +ucbService)
                raise RuntimeException(
                    "PythonScriptProvider couldn't instantiate " +ucbService, self)
            self.provCtx = ProviderContext(
                storageType, sfa, urlHelper, ScriptContext( uno.getComponentContext(), None ) )
            if isPackage:
                mapPackageName2Path = getPackageName2PathMap( sfa, rootUrl )
                self.provCtx.setPackageAttributes( mapPackageName2Path , rootUrl )
                self.dirBrowseNode = PackageBrowseNode( self.provCtx, LANGUAGENAME, rootUrl )
            else:
                self.dirBrowseNode = DirBrowseNode( self.provCtx, LANGUAGENAME, rootUrl )
            
        except Exception, e:
            text = lastException2String()
            log.debug( "PythonScriptProvider could not be instantiated because of : " + text )
            raise e

    def getName( self ):
        return self.dirBrowseNode.getName()

    def getChildNodes( self ):
        return self.dirBrowseNode.getChildNodes()    

    def hasChildNodes( self ):
        return self.dirBrowseNode.hasChildNodes()

    def getType( self ):
        return self.dirBrowseNode.getType()

    def getScript( self, uri ):
        log.debug( "DirBrowseNode getScript " + uri + " invoked" )
        
        raise IllegalArgumentException( "DirBrowseNode couldn't instantiate script " + uri , self , 0 )

    def getScript( self, scriptUri ):
        try:
            log.isDebugLevel() and log.debug( "getScript " + scriptUri + " invoked")
            
            storageUri = self.provCtx.getStorageUrlFromPersistentUrl(
                self.provCtx.uriHelper.getStorageURI(scriptUri) );
            log.isDebugLevel() and log.debug( "getScript: storageUri = " + storageUri)
            fileUri = storageUri[0:storageUri.find( "$" )]
            funcName = storageUri[storageUri.find( "$" )+1:len(storageUri)]        
            
            mod = self.provCtx.getModuleByUrl( fileUri )
            log.isDebugLevel() and log.debug( " got mod " + str(mod) )
            
            func = mod.__dict__[ funcName ]

            log.isDebugLevel() and log.debug( "got func " + str( func ) )
            return PythonScript( func, mod )
        except Exception, e:
            text = lastException2String()
            log.error( text )
            raise ScriptFrameworkErrorException( text, self, scriptUri, LANGUAGENAME, 0 )
        

    # XServiceInfo
    def getSupportedServices( self ):
        return g_ImplementationHelper.getSupportedServices(g_implName)

    def supportsService( self, ServiceName ):
        return g_ImplementationHelper.supportsService( g_implName, ServiceName )

    def getImplementationName(self):
        return g_implName

    def getByName( self, name ):
        log.debug( "getByName called" + str( name ))
        return None

        
    def getElementNames( self ):
        log.debug( "getElementNames called")
        return ()
    
    def hasByName( self, name ):
        try:
            log.debug( "hasByName called " + str( name ))
            uri = expandUri(name)
            ret = self.provCtx.isUrlInPackage( uri )
            log.debug( "hasByName " + uri + " " +str( ret ) )
            return ret
        except Exception, e:
            text = lastException2String()
            log.debug( "Error in hasByName:" +  text )
            return False

    def removeByName( self, name ):
        log.debug( "removeByName called" + str( name ))
        uri = expandUri( name )
        self.provCtx.removePackageByUrl( uri )
        log.debug( "removeByName called" + str( uri ) + " successful" )
        
    def insertByName( self, name, value ):
        log.debug( "insertByName called " + str( name ) + " " + str( value ))
        uri = expandUri( name )
        self.provCtx.addPackageByUrl( uri )
        log.debug( "insertByName called" + str( uri ) + " successful" )

    def replaceByName( self, name, value ):
        log.debug( "replaceByName called " + str( name ) + " " + str( value ))
        removeByName( name )
        insertByName( name )
        log.debug( "replaceByName called" + str( uri ) + " successful" )

    def getElementType( self ):
        log.debug( "getElementType called" )
        return uno.getTypeByName( "void" )
    
    def hasElements( self ):
        log.debug( "hasElements got called")
        return False
    
g_ImplementationHelper.addImplementation( \
	PythonScriptProvider,g_implName, \
    ("com.sun.star.script.provider.LanguageScriptProvider",
     "com.sun.star.script.provider.ScriptProviderFor"+ LANGUAGENAME,),)


log.debug( "pythonscript finished intializing" )

