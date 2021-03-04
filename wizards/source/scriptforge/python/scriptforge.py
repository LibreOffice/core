# -*- coding: utf-8 -*-

#     Copyright 2020-2021 Jean-Pierre LEDURE, Alain ROMEDENNE

# =====================================================================================================================
# ===           The ScriptForge library and its associated libraries are part of the LibreOffice project.           ===
# ===                   Full documentation is available on https://help.libreoffice.org/                            ===
# =====================================================================================================================

# ScriptForge is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# ScriptForge is free software; you can redistribute it and/or modify it under the terms of either (at your option):

# 1) The Mozilla Public License, v. 2.0. If a copy of the MPL was not
# distributed with this file, you can obtain one at http://mozilla.org/MPL/2.0/ .

# 2) The GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version. If a copy of the LGPL was not
# distributed with this file, see http://www.gnu.org/licenses/ .

"""
    ScriptForge libraries are an extensible and robust collection of macro scripting resources for LibreOffice
    to be invoked from user Basic or Python macros. Users familiar with other BASIC macro variants often face hard
    times to dig into the extensive LibreOffice Application Programming Interface even for the simplest operations.
    By collecting most-demanded document operations in a set of easy to use, easy to read routines, users can now
    program document macros with much less hassle and get quicker results.

    ScriptForge abundant methods are organized in reusable modules that cleanly isolate Basic/Python programming
    language constructs from ODF document content accesses and user interface(UI) features.

    The scriptforge.py module
        - implements a protocol between Python (user) scripts and the ScriptForge Basic library
        - contains the interfaces (classes and attributes) to be used in Python user scripts
          to run the services implemented in the standard libraries shipped with LibreOffice

    Usage:

        When Python and LibreOffice run in the same process (usual case): either
            from scriptforge import *   # or, better ...
            from scriptforge import CreateScriptService

        When Python and LibreOffice are started in separate processes,
        LibreOffice being started from console ... (example for Linux with port = 2021)
            ./soffice --accept='socket,host=localhost,port=2021;urp;'
        then use next statement:
            from scriptforge import *   # or, better ...
            from scriptforge import CreateScriptService, ScriptForge
            ScriptForge(hostname = 'localhost', port = 2021)

    Specific documentation about the use of ScriptForge from Python scripts:
        TBD
    """

import uno

from platform import system as _opsys
import datetime
import os


class _Singleton(type):
    """
        A Singleton metaclass design pattern
        Credits: « Python in a Nutshell » by Alex Martelli, O'Reilly
        """
    instances = {}

    def __call__(cls, *args, **kwargs):
        if cls not in cls.instances:
            cls.instances[cls] = super(_Singleton, cls).__call__(*args, **kwargs)
        return cls.instances[cls]


# #####################################################################################################################
#                           ScriptForge CLASS                                                                       ###
# #####################################################################################################################

class ScriptForge(object, metaclass = _Singleton):
    """
        The ScriptForge (singleton) class encapsulates the core of the ScriptForge run-time
            - Bridge with the LibreOffice process
            - Implementation of the inter-language protocol with the Basic libraries
            - Identification of the available services interfaces
            - Dispatching of services
            - Coexistence with UNO

        It embeds the Service class that manages the protocol with Basic
        """

    # #########################################################################
    # Class attributes
    # #########################################################################
    hostname = ''
    port = 0
    componentcontext = None
    scriptprovider = None

    # #########################################################################
    # Class constants
    # #########################################################################
    library = 'ScriptForge'
    Version = '7.2'  # Actual version number
    #
    # Basic dispatcher for Python scripts
    basicdispatcher = 'ScriptForge.SF_PythonHelper._PythonDispatcher'
    #
    # VarType() constants
    V_EMPTY, V_NULL, V_INTEGER, V_LONG, V_SINGLE, V_DOUBLE = 0, 1, 2, 3, 4, 5
    V_CURRENCY, V_DATE, V_STRING, V_OBJECT, V_BOOLEAN = 6, 7, 8, 9, 11
    V_VARIANT, V_ARRAY, V_ERROR, V_UNO = 12, 8192, -1, 16
    # Object types
    objMODULE, objCLASS, objUNO = 1, 2, 3
    # Special argument symbols
    cstSymEmpty, cstSymNull, cstSymMissing = '+++EMPTY+++', '+++NULL+++', '+++MISSING+++'

    def __init__(self, hostname = '', port = 0):
        """
            Because singleton, constructor is executed only once while Python active
            Arguments are mandatory when Python and LibreOffice run in separate processes
            :param hostname: probably 'localhost'
            :param port: port number
            """
        ScriptForge.hostname = hostname
        ScriptForge.port = port
        # Determine main pyuno entry points
        ScriptForge.componentcontext = self.ConnectToLOProcess(hostname, port)  # com.sun.star.uno.XComponentContext
        ScriptForge.scriptprovider = self.ScriptProvider(self.componentcontext)  # ...script.provider.XScriptProvider
        #
        # Establish a list of the available services as a dictionary (servicename, serviceclass)
        ScriptForge.serviceslist = dict((cls.servicename, cls) for cls in SFServices.__subclasses__())
        ScriptForge.servicesdispatcher = None

    @classmethod
    def ConnectToLOProcess(cls, hostname = '', port = 0):
        """
            Called by the ScriptForge class constructor to establish the connection with
            the requested LibreOffice instance
            The default arguments are for the usual interactive mode

            :param hostname: probably 'localhost' or ''
            :param port: port number or 0
            :return: the derived component context
            """
        if len(hostname) > 0 and port > 0:  # Explicit connection request via socket
            ctx = uno.getComponentContext()  # com.sun.star.uno.XComponentContext
            resolver = ctx.ServiceManager.createInstanceWithContext(
                'com.sun.star.bridge.UnoUrlResolver', ctx)  # com.sun.star.comp.bridge.UnoUrlResolver
            try:
                conn = 'socket,host=%s,port=%d' % (hostname, port)
                url = 'uno:%s;urp;StarOffice.ComponentContext' % conn
                ctx = resolver.resolve(url)
            except Exception:  # thrown when LibreOffice specified instance isn't started
                raise ConnectionError(
                    'Connection to LibreOffice failed (host = ' + hostname + ', port = ' + str(port) + ')')
            return ctx
        elif len(hostname) == 0 and port == 0:  # Usual interactive mode
            return uno.getComponentContext()
        else:
            raise SystemExit('The creation of the ScriptForge() instance got invalid arguments: '
                             + '(host = ' + hostname + ', port = ' + str(port) + ')')

    @classmethod
    def ScriptProvider(cls, context = None):
        """
            Returns the general script provider
            """
        servicemanager = context.ServiceManager  # com.sun.star.lang.XMultiComponentFactory
        masterscript = servicemanager.createInstanceWithContext(
            "com.sun.star.script.provider.MasterScriptProviderFactory", context)
        return masterscript.createScriptProvider("")

    @classmethod
    def InvokeSimpleScript(cls, script, *args):
        """
            Create a UNO object corresponding with the given Python or Basic script
            The execution is done with the invoke() method applied on the created object
            Implicit scope: Extensions and documents are excluded. Either
                "application"            a shared library                               (BASIC)
                "share"                  a library of LibreOffice Macros                (PYTHON)
            :param script: Either
                    [library.]module.method - Must not be a class module or method
                    [directory/]module.py$method
            :return: the script object as a com.sun.star.script.provider.XScript UNO object
            """
        #    Compute the URI specification described in
        #    https://wiki.openoffice.org/wiki/Documentation/DevGuide/Scripting/Scripting_Framework_URI_Specification
        if cls.servicesdispatcher is not None and script == ScriptForge.basicdispatcher:
            xscript = cls.servicesdispatcher
        elif len(script) > 0:
            if '.py$' in script.lower():  # Python
                uri = 'vnd.sun.star.script:' + script + '?language=Python&location=share'
            else:  # Basic
                lib = ''
                if len(script.split('.')) < 3:
                    lib = cls.library + '.'
                uri = 'vnd.sun.star.script:' + lib + script + '?language=Basic&location=application'
            # Get the script object
            try:
                xscript = cls.scriptprovider.getScript(uri)
            except Exception:
                raise SystemExit('The script ' + "'" + script + "'"
                                 + ' could not be located in your LibreOffice installation')
        else:  # Should not happen
            return None
        # Execute the script with the given arguments
        # Packaging for script provider depends on presence of ParamArray arguments in the called Basic script
        if script == ScriptForge.basicdispatcher:
            # At 1st execution, buffer xscript
            if cls.servicesdispatcher is None:
                cls.servicesdispatcher = xscript
            scriptreturn = xscript.invoke(args[0], (), ())
        else:
            scriptreturn = xscript.invoke(args, (), ())
        #
        return scriptreturn[0]  # Updatable arguments passed by reference are ignored

    @classmethod
    def InvokeBasicService(cls, basicobject, flags, method, *args):
        """
            Execute a given Basic script and interprete its result
            This method has as counterpart the ScriptForge.SF_PythonHelper._PythonDispatcher() Basic method
            :param basicobject: a Service subclass
            :param flags: see the vb* and flg* constants below
            :param method: the name of the method or property to invoke, as a string
            :param args: the arguments of the method. Symbolic cst* constants may be necessary
            :return: The invoked Basic counterpart script (with InvokeSimpleScript()) will return a tuple
                [0]     The returned value - scalar, object reference or a tuple
                [1]     The Basic VarType() of the returned value
                        Null, Empty and Nothing have different vartypes but return all None to Python
                Additionally, when [0] is a tuple:
                [2]     Number of dimensions in Basic
                Additionally, when [0] is a UNO or Basic object:
                [2]     Module (1), Class instance (2) or UNO (3)
                [3]     The object's ObjectType
                [4]     The object's ServiceName
                [5]     The object's name
                When an error occurs Python receives None as a scalar. This determines the occurence of a failure
                The method returns either
                    - the 0th element of the tuple when scalar, tuple or UNO object
                    - a new Service() object or one of its subclasses otherwise
            """
        # Constants
        script = ScriptForge.basicdispatcher
        cstNoArgs = '+++NOARGS+++'
        cstValue, cstVarType, cstDims, cstClass, cstType, cstService, cstName = 0, 1, 2, 2, 3, 4, 5

        #
        # Run the basic script
        # The targeted script has a ParamArray argument. Do not change next 4 lines except if you know what you do !
        if len(args) == 0:
            args = (basicobject,) + (flags,) + (method,) + (cstNoArgs,)
        else:
            args = (basicobject,) + (flags,) + (method,) + args
        returntuple = cls.InvokeSimpleScript(script, args)
        #
        # Interprete the result
        # Did an error occur in the Basic world ?
        if not isinstance(returntuple, (tuple, list)):
            raise RuntimeError("The execution of the method '" + method + "' failed. Execution stops.")
        #
        # Analyze the returned tuple
        if returntuple[cstVarType] == ScriptForge.V_OBJECT and len(returntuple) > cstClass:  # Avoid Nothing
            if returntuple[cstClass] == ScriptForge.objUNO:
                pass
            else:
                # Create the new class instance of the right subclass of Service()
                servname = returntuple[cstService]
                for subcls in SFServices.__subclasses__():
                    if servname == subcls.servicename:
                       return subcls(returntuple[cstValue], returntuple[cstType], returntuple[cstClass],
                                      returntuple[cstName])
                # When service not found
                raise RuntimeError("The service '" + servname + "' is not available in Python. Execution stops.")
        elif returntuple[cstVarType] >= ScriptForge.V_ARRAY:
            pass
        else:         # All scalar values
            pass
        return returntuple[cstValue]


# #####################################################################################################################
#                           SFServices CLASS    (ScriptForge services superclass)                                   ###
# #####################################################################################################################

class SFServices(object):
    """
        Generic implementation of a parent Service class
        Every service must subclass this class to be recognized as a valid service
        A service instance is created by the CreateScriptService method
        It can have a mirror in the Basic world or be totally defined in Python

        Every subclass must initialize 2 class properties:
            servicename (e.g. ScriptForge.FileSystem, ScriptForge.Basic)
            serviceimplementation: either 'python' or 'basic'
        This is sufficient to register the set of services in the Python world

        The communication with Basic is managed by 2 ScriptForge() methods:
            InvokeSimpleScript(): low level invocation of a Basic script. This script must be located
                in a usual Basic module. The result is passed as-is
            InvokeSBasicService(): the result comes back encapsulated with additional info
                The result is interpreted in the method
                The invoked script can be a property or a method of a Basic class module
        It is up to every service method to determine which method to use

        For Basic services only:
            Each instance is identified by its
                - object reference: the real Basic object embedded as a UNO wrapper object
                - objecttype ('SF_String', 'DICTIONARY', ...)
                - name (form, control, ... name) - may be blank

            The role of the Service() superclass is mainly to propose a generic properties management
            Properties are got and set following next strategy:
                1. Property names are controlled strictly ('Value' and not 'value')
                2. Getting a property value for the first time is always done via a Basic call
                3. Next occurrences are fetched from the Python dictionary of the instance if the property
                    is read-only, otherwise via a Basic call
                4. Read-only properties may be modified or deleted exceptionally by the class
                   when self.internal == True. The latter must immediately be reset after use

            Each subclass must define its interface with the user scripts:
            1.  The properties
                     a dictionary named 'serviceProperties' with keys = (camel-cased) property names and value = boolean
                        True = editable, False = read-only
                     a list named 'localProperties' reserved to properties for internal use
                        e.g. oDlg.Controls() is a method that uses '_Controls' to hold the list of available controls
                serviceProperties are buffered in Python after their 1st get request to Basic
                Only if there is a need to go to Basic at each get, then declare the property explicitly:
                    @property
                    def myProperty(self):
                        return self.GetProperty('myProperty')
            2   The methods
                a usual def: statement
                    def myMethod(self, arg1, arg2 = ''):
                        return self.Execute(self.vbMethod, 'myMethod', arg1, arg2)
                Method names are camel-cased, arguments are lower-cased
                All arguments must be present and initialized before the call to Basic, if any
        """
    # Python-Basic protocol constants and flags
    vbGet, vbLet, vbMethod, vbSet = 2, 4, 1, 8  # CallByName constants
    flgArrayArg = 512  # 1st argument can be a 2D array
    flgArrayRet = 1024  # Invoked service method can return an array
    flgUno = 256  # Invoked service method/property can return a UNO object
    # Basic class type
    moduleClass, moduleStandard = 2, 1
    #
    # To operate dynamic property getting/setting it is necessary to
    # enumerate all types of properties and adapt __getattr__() and __setattr__() according to their type
    internal_attributes = ('objectreference', 'objecttype', 'name', 'internal', 'servicename',
                           'serviceimplementation', 'classmodule', 'EXEC', 'SIMPLEEXEC')

    def __init__(self, reference = -1, objtype = None, classmodule = 0, name = ''):
        """
            Trivial initialization of internal properties
            If the subclass has its own __init()__ method, a call to this one should be its first statement.
            Afterwards localProperties should be filled with the list of its own propertties
            """
        self.objectreference = reference  # the index in the Python storage where the Basic object is stored
        self.objecttype = objtype  # ('SF_String', 'DICTIONARY', ...)
        self.classmodule = classmodule  # Module (1), Class instance (2)
        self.name = name  # '' when no name
        self.internal = False  # True to exceptionally allow assigning a new value to a read-only property
        self.localProperties = ()  # the properties reserved for internal use (often empty)
        self.SIMPLEEXEC = ScriptForge.InvokeSimpleScript  # Shortcuts to script provider interfaces
        self.EXEC = ScriptForge.InvokeBasicService

    def __getattr__(self, name):
        """
            Executed for EVERY property reference if name not yet in the instance dict
            At the 1st get, the property value is always got from Basic
            """
        if self.serviceimplementation == 'basic':
            if name in ('serviceProperties', 'localProperties', 'internal_attributes'):
                pass
            elif name in self.serviceProperties:
                # Get Property from Basic
                return self.GetProperty(name)
        # Execute the usual attributes getter
        return super(SFServices, self).__getattribute__(name)

    def __setattr__(self, name, value):
        """
            Executed for EVERY property assignment, including in __init__() !!
            Setting a property requires for serviceProperties() to be executed in Basic
            """
        if self.serviceimplementation == 'basic':
            if name in ('serviceProperties', 'localProperties', 'internal_attributes'):
                pass
            elif name[0:2] == '__' or name in self.internal_attributes or name in self.localProperties:
                pass
            elif name in self.serviceProperties:
                if self.internal:  # internal = True forces property local setting even if property is read-only
                    pass
                elif self.serviceProperties[name] is True:  # True == Editable
                    self.SetProperty(name, value)
                else:
                    raise AttributeError(
                        "type object '" + self.objecttype + "' has no editable property '" + name + "'")
            else:
                raise AttributeError("type object '" + self.objecttype + "' has no property '" + name + "'")
        object.__setattr__(self, name, value)
        return

    def __repr__(self):
        return self.serviceimplementation + '/' + self.servicename + '/' + str(self.objectreference) + '/' + \
               super(SFServices, self).__repr__()

    def Dispose(self):
        if self.serviceimplementation == 'basic':
            if self.classmodule == self.moduleClass and self.objectreference >= 0:
                self.Execute(self.vbMethod, 'Dispose')
                self.objectreference = -1

    def Execute(self, flags = 0, methodname = '', *args):
        if flags == 0:
            flags = self.vbMethod
        if len(methodname) > 0:
            return self.EXEC(self.objectreference, flags, methodname, *args)

    def GetProperty(self, propertyname):
        """
            Get the given property from the Basic world
            """
        return self.EXEC(self.objectreference, self.vbGet, propertyname)

    def SetProperty(self, propertyname, value):
        """
            Set the given property to a new value in the Basic world
            """
        return self.EXEC(self.objectreference, self.vbLet, propertyname, value)


# #####################################################################################################################
#                       SFScriptForge CLASS    (alias of ScriptForge Basic library)                                 ###
# #####################################################################################################################
class SFScriptForge:
    # #########################################################################
    # SF_Basic CLASS
    # #########################################################################
    class SF_Basic(SFServices, metaclass = _Singleton):
        """
            This service proposes a collection of Basic methods to be executed in a Python context
            to simulate the exact syntax and behaviour of the identical Basic builtin method.
            Typical example:
                SF_Basic.MsgBox('This has to be displayed in a message box')
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'python'
        servicename = 'ScriptForge.Basic'
        # Basic helper functions invocation
        module = 'SF_PythonHelper'
        # Message box constants
        MB_ABORTRETRYIGNORE, MB_DEFBUTTON1, MB_DEFBUTTON2, MB_DEFBUTTON3 = 2, 128, 256, 512
        MB_ICONEXCLAMATION, MB_ICONINFORMATION, MB_ICONQUESTION, MB_ICONSTOP = 48, 64, 32, 16
        MB_OK, MB_OKCANCEL, MB_RETRYCANCEL, MB_YESNO, MB_YESNOCANCEL = 0, 1, 5, 4, 3
        IDABORT, IDCANCEL, IDIGNORE, IDNO, IDOK, IDRETRY, IDYES = 3, 2, 5, 7, 1, 4, 6

        def ConvertFromUrl(self, filename):
            return self.SIMPLEEXEC(self.module + '.PyConvertFromUrl', filename)

        def ConvertToUrl(self, filename):
            return self.SIMPLEEXEC(self.module + '.PyConvertToUrl', filename)

        def CreateUnoService(self, unoservice):
            return self.SIMPLEEXEC(self.module + '.PyCreateUnoService', unoservice)

        def DateAdd(self, add, count, datearg):
            if isinstance(datearg, datetime.datetime):
                datearg = datearg.isoformat()
            dateadd = self.SIMPLEEXEC(self.module + '.PyDateAdd', add, count, datearg)
            return datetime.datetime.fromisoformat(dateadd)

        def DateDiff(self, add, date1, date2, weekstart = 1, yearstart = 1):
            if isinstance(date1, datetime.datetime):
                date1 = date1.isoformat()
            if isinstance(date2, datetime.datetime):
                date2 = date2.isoformat()
            return self.SIMPLEEXEC(self.module + '.PyDateDiff', add, date1, date2, weekstart, yearstart)

        def DatePart(self, add, datearg, weekstart = 1, yearstart = 1):
            if isinstance(datearg, datetime.datetime):
                datearg = datearg.isoformat()
            return self.SIMPLEEXEC(self.module + '.PyDatePart', add, datearg, weekstart, yearstart)

        def DateValue(self, datearg):
            if isinstance(datearg, datetime.datetime):
                datearg = datearg.isoformat()
            datevalue = self.SIMPLEEXEC(self.module + '.PyDateValue', datearg)
            return datetime.datetime.fromisoformat(datevalue)

        def Format(self, value, pattern = ''):
            if isinstance(value, datetime.datetime):
                value = value.isoformat()
            return self.SIMPLEEXEC(self.module + '.PyFormat', value, pattern)

        def GetGuiType(self):
            return self.SIMPLEEXEC(self.module + '.PyGetGuiType')

        def GetSystemTicks(self):
            return self.SIMPLEEXEC(self.module + '.PyGetSystemTicks')

        @staticmethod
        def GetDefaultContext():
            return ScriptForge.componentcontext

        @staticmethod
        def GetPathSeparator():
            return os.sep

        class GlobalScope(object, metaclass = _Singleton):
            @classmethod  # Mandatory because the GlobalScope class is normally not instantiated
            def BasicLibraries(cls):
                return SFScriptForge.SF_Basic().SIMPLEEXEC(SFScriptForge.SF_Basic.module + '.PyGlobalScope', 'Basic')

            @classmethod
            def DialogLibraries(cls):
                return SFScriptForge.SF_Basic().SIMPLEEXEC(SFScriptForge.SF_Basic.module + '.PyGlobalScope', 'Dialog')

        def InputBox(self, msg, title = '', default = '', xpos = -1, ypos = -1):
            if xpos < 0 or ypos < 0:
                return self.SIMPLEEXEC(self.module + '.PyInputBox', msg, title, default)
            return self.SIMPLEEXEC(self.module + '.PyInputBox', msg, title, default, xpos, ypos)

        def MsgBox(self, text, dialogtype = 0, dialogtitle = ''):
            return self.SIMPLEEXEC(self.module + '.PyMsgBox', text, dialogtype, dialogtitle)

        @staticmethod
        def Now():
            return datetime.datetime.now()

        @staticmethod
        def RGB(red, green, blue):
            return int('%02x%02x%02x' % (red, green, blue), 16)

        def Xray(self, unoobject = None):
            return self.SIMPLEEXEC('XrayTool._main.xray', unoobject)

    # #########################################################################
    # SF_String CLASS
    # #########################################################################
    class SF_String(SFServices, metaclass = _Singleton):
        """
            A collection of methods focussed on string manipulation, user input validation,
            regular expressions, encodings, parsing and hashing algorithms.
            Many of them are less efficient than their Python equivalents.
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'basic'
        servicename = 'ScriptForge.String'

    # #########################################################################
    # SF_FileSystem CLASS
    # #########################################################################
    class SF_FileSystem(SFServices, metaclass = _Singleton):
        """
            The "FileSystem" service includes common file and folder handling routines.
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'basic'
        servicename = 'ScriptForge.FileSystem'
        serviceProperties = dict(FileNaming = True, ConfigFolder = False, ExtensionsFolder = False, HomeFolder = False,
                                 InstallFolder = False, TemplatesFolder = False, TemporaryFolder = False,
                                 UserTemplatesFolder = False)

        @property
        def ConfigFolder(self):
            return self.GetProperty('ConfigFolder')

        def BuildPath(self, foldername, name):
            return self.Execute(self.vbMethod, 'BuildPath', foldername, name)

        def FolderExists(self, foldername):
            return self.Execute(self.vbMethod, 'FolderExists', foldername)

    # #########################################################################
    # SF_Timer CLASS
    # #########################################################################
    class SF_Timer(SFServices):
        """
            The "Timer" service measures the amount of time it takes to run user scripts..
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'basic'
        servicename = 'ScriptForge.Timer'
        serviceProperties = dict(Duration = False, IsStarted = False, IsSuspended = False,
                                 SuspendDuration = False, TotalDuration = False)

        @property
        def Duration(self):
            return self.GetProperty('Duration')

        @property
        def IsStarted(self):
            return self.GetProperty('IsStarted')

        @property
        def SuspendDuration(self):
            return self.GetProperty('SuspendDuration')

        @property
        def TotalDuration(self):
            return self.GetProperty('TotalDuration')

        def Continue(self):
            return self.Execute(self.vbMethod, 'Continue')

        def Restart(self):
            return self.Execute(self.vbMethod, 'Restart')

        def Start(self):
            return self.Execute(self.vbMethod, 'Start')

        def Suspend(self):
            return self.Execute(self.vbMethod, 'Suspend')

        def Terminate(self):
            return self.Execute(self.vbMethod, 'Terminate')


# ##############################################False#######################################################################
#                           CreateScriptService()                                                                   ###
# #####################################################################################################################
def CreateScriptService(service, *args):
    """
        A service being the name of a collection of properties and methods,
        this method returns the Python object mirror of the Basic object implementing
        the requested service
        As an exception to above, 'Basic' is accepted as a shortcut to the Basic service
        which is implemented in Python
        :param service: the name of the service as a string 'library.service' - cased exactly
        :param args: the arguments to pass to the service constructor
        :return: the service as a Python object
        """
    # Init at each CreateScriptService() invocation
    #       CreateScriptService is usually the first statement in user scripts requesting ScriptForge services
    #       ScriptForge() is optional in user scripts when Python process inside LibreOffice process
    ScriptForge()

    def ResolveSynonyms(servicename):
        """
            Synonyms within service names implemented in Python are resolved here
            :param servicename: The short name of the service
            :return: The official service name
            """
        if servicename.lower() in ('basic', 'scriptforge.basic'):
            return 'ScriptForge.Basic'
        return servicename

    #
    # Check the list of available services to examine if the requested service is within the Python world
    scriptservice = ResolveSynonyms(service)
    if scriptservice in ScriptForge.serviceslist:
        serv = ScriptForge.serviceslist[scriptservice]
        if serv.serviceimplementation == 'python':
            return serv()
    # The requested service is to be found in the Basic world
    if len(args) == 0:
        serv = ScriptForge.InvokeBasicService('SF_Services', SFServices.vbMethod, 'CreateScriptService', service)
    else:
        serv = ScriptForge.InvokeBasicService('SF_Services', SFServices.vbMethod, 'CreateScriptService', service, *args)
    return serv

# #####################################################################################################################
#                           Services shortcuts                                                                      ###
# #####################################################################################################################
# SF_Basic = CreateScriptService('SFPython.Basic')
# SF_String = _ScriptForge.SF_String


# ######################################################################
# lists the scripts, that shall be visible inside the Basic/Python IDE
# ######################################################################

g_exportedScripts = ()