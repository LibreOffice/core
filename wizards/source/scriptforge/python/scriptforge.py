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
    basicdispatcher = '@application:ScriptForge.SF_PythonHelper._PythonDispatcher'
    # Python helper functions module
    pythonhelpermodule = 'ScriptForgeHelper.py'
    #
    # VarType() constants
    V_EMPTY, V_NULL, V_INTEGER, V_LONG, V_SINGLE, V_DOUBLE = 0, 1, 2, 3, 4, 5
    V_CURRENCY, V_DATE, V_STRING, V_OBJECT, V_BOOLEAN = 6, 7, 8, 9, 11
    V_VARIANT, V_ARRAY, V_ERROR, V_UNO = 12, 8192, -1, 16
    # Object types
    objMODULE, objCLASS, objUNO = 1, 2, 3
    # Special argument symbols
    cstSymEmpty, cstSymNull, cstSymMissing = '+++EMPTY+++', '+++NULL+++', '+++MISSING+++'
    # Predefined references for services implemented as standard Basic modules
    servicesmodules = dict([('ScriptForge.Array', 0),
                            ('ScriptForge.Exception', 1),
                            ('ScriptForge.FileSystem', 2),
                            ('ScriptForge.Platform', 3),
                            ('ScriptForge.Services', 4),
                            ('ScriptForge.Session', 5),
                            ('ScriptForge.String', 6),
                            ('ScriptForge.UI', 7)])

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
            Implicit scope: Either
                "application"            a shared library                               (BASIC)
                "share"                  a library of LibreOffice Macros                (PYTHON)
            :param script: Either
                    [@][scope:][library.]module.method - Must not be a class module or method
                        [@] means that the targeted method accepts ParamArray arguments (Basic only)
                    [scope:][directory/]module.py$method - Must be a method defined at module level
            :return: the value returned by the invoked script, or an error if the script was not found
            """

        # The frequently called PythonDispatcher in the ScriptForge Basic library is buffered to privilege performance
        if cls.servicesdispatcher is not None and script == ScriptForge.basicdispatcher:
            xscript = cls.servicesdispatcher
            fullscript = script
            paramarray = True
        #    Build the URI specification described in
        #    https://wiki.openoffice.org/wiki/Documentation/DevGuide/Scripting/Scripting_Framework_URI_Specification
        elif len(script) > 0:
            # Check ParamArray arguments
            paramarray = False
            if script[0] == '@':
                script = script[1:]
                paramarray = True
            scope = ''
            if ':' in script:
                scope, script = script.split(':')
            if '.py$' in script.lower():  # Python
                if len(scope) == 0:
                    scope = 'share'     # Default for Python
                uri = 'vnd.sun.star.script:' + script + '?language=Python&location=' + scope
            else:  # Basic
                if len(scope) == 0:
                    scope = 'application'     # Default for Basic
                lib = ''
                if len(script.split('.')) < 3:
                    lib = cls.library + '.'     # Default library = ScriptForge
                uri = 'vnd.sun.star.script:' + lib + script + '?language=Basic&location=' + scope
            # Get the script object
            fullscript = ('@' if paramarray else '') + scope + ':' + script
            try:
                xscript = cls.scriptprovider.getScript(uri)
            except Exception:
                raise SystemExit('The script ' + "'" + script + "'"
                                 + ' could not be located in your LibreOffice installation')
        else:  # Should not happen
            return None

        # At 1st execution of the common Basic dispatcher, buffer xscript
        if fullscript == ScriptForge.basicdispatcher and cls.servicesdispatcher is None:
            cls.servicesdispatcher = xscript

        # Execute the script with the given arguments
        # Packaging for script provider depends on presence of ParamArray arguments in the called Basic script
        if paramarray:
            scriptreturn = xscript.invoke(args[0], (), ())
        else:
            scriptreturn = xscript.invoke(args, (), ())

        #
        return scriptreturn[0]  # Updatable arguments passed by reference are ignored

    @classmethod
    def InvokeBasicService(cls, basicobject, flags, method, *args):
        """
            Execute a given Basic script and interpret its result
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
                When an error occurs Python receives None as a scalar. This determines the occurrence of a failure
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
        # Interpret the result
        # Did an error occur in the Basic world ?
        if not isinstance(returntuple, (tuple, list)):
            raise RuntimeError("The execution of the method '" + method + "' failed. Execution stops.")
        #
        # Analyze the returned tuple
        if returntuple[cstVarType] == ScriptForge.V_OBJECT and len(returntuple) > cstClass:  # Avoid Nothing
            if returntuple[cstClass] == ScriptForge.objUNO:
                pass
            else:
                # Create the new class instance of the right subclass of SFServices()
                servname = returntuple[cstService]
                subcls = cls.serviceslist[servname]
                if subcls is not None:
                    return subcls(returntuple[cstValue], returntuple[cstType], returntuple[cstClass],
                                  returntuple[cstName])
                # When service not found
                raise RuntimeError("The service '" + servname + "' is not available in Python. Execution stops.")
        elif returntuple[cstVarType] >= ScriptForge.V_ARRAY:
            pass
        elif returntuple[cstVarType] == ScriptForge.V_DATE:
            return datetime.datetime.fromisoformat(returntuple[cstValue])
        else:         # All other scalar values
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

        Every subclass must initialize 3 class properties:
            servicename (e.g. 'ScriptForge.FileSystem', 'ScriptForge.Basic')
            servicesynonyms (e.g. 'FileSystem', 'Basic')
            serviceimplementation: either 'python' or 'basic'
        This is sufficient to register the service in the Python world

        The communication with Basic is managed by 2 ScriptForge() methods:
            InvokeSimpleScript(): low level invocation of a Basic script. This script must be located
                in a usual Basic module. The result is passed as-is
            InvokeBasicService(): the result comes back encapsulated with additional info
                The result is interpreted in the method
                The invoked script can be a property or a method of a Basic class or usual module
        It is up to every service method to determine which method to use

        For Basic services only:
            Each instance is identified by its
                - object reference: the real Basic object embedded as a UNO wrapper object
                - object type ('SF_String', 'DICTIONARY', ...)
                - class module: 1 for usual modules, 2 for class modules
                - name (form, control, ... name) - may be blank

            The role of the SFServices() superclass is mainly to propose a generic properties management
            Properties are got and set following next strategy:
                1. Property names are controlled strictly ('Value' or 'value', not 'VALUE')
                2. Getting a property value for the first time is always done via a Basic call
                3. Next occurrences are fetched from the Python dictionary of the instance if the property
                   is read-only, otherwise via a Basic call
                4. Read-only properties may be modified or deleted exceptionally by the class
                   when self.internal == True. The latter must immediately be reset after use

            Each subclass must define its interface with the user scripts:
            1.  The properties
                Property names are proper-cased
                Conventionally, camel-cased and lower-cased synonyms are supported where relevant
                    a dictionary named 'serviceproperties' with keys = (proper-cased) property names and value = boolean
                        True = editable, False = read-only
                    a list named 'localProperties' reserved to properties for internal use
                        e.g. oDlg.Controls() is a method that uses '_Controls' to hold the list of available controls
                When
                    forceGetProperty = False    # Standard behaviour
                read-only serviceproperties are buffered in Python after their 1st get request to Basic
                Otherwise set it to True to force a recomputation at each property getter invocation
                If there is a need to handle a specific property in a specific manner:
                    @property
                    def myProperty(self):
                        return self.GetProperty('myProperty')
            2   The methods
                a usual def: statement
                    def myMethod(self, arg1, arg2 = ''):
                        return self.Execute(self.vbMethod, 'myMethod', arg1, arg2)
                Method names are proper-cased, arguments are lower-cased
                Conventionally, camel-cased and lower-cased homonyms are supported where relevant
                All arguments must be present and initialized before the call to Basic, if any
        """
    # Python-Basic protocol constants and flags
    vbGet, vbLet, vbMethod, vbSet = 2, 4, 1, 8  # CallByName constants
    flgDateArg = 64  # Invoked service method may contain a date argument
    flgDateRet = 128  # Invoked service method can return a date
    flgArrayArg = 512  # 1st argument can be a 2D array
    flgArrayRet = 1024  # Invoked service method can return a 2D array
    flgUno = 256  # Invoked service method/property can return a UNO object
    # Basic class type
    moduleClass, moduleStandard = 2, 1
    #
    # Define the default behaviour for read-only properties: buffer their values in Python
    forceGetProperty = False
    # Empty dictionary for lower/camelcased homonyms or properties
    propertysynonyms = {}
    # To operate dynamic property getting/setting it is necessary to
    # enumerate all types of properties and adapt __getattr__() and __setattr__() according to their type
    internal_attributes = ('objectreference', 'objecttype', 'name', 'internal', 'servicename',
                           'serviceimplementation', 'classmodule', 'EXEC', 'SIMPLEEXEC')

    def __init__(self, reference = -1, objtype = None, classmodule = 0, name = ''):
        """
            Trivial initialization of internal properties
            If the subclass has its own __init()__ method, a call to this one should be its first statement.
            Afterwards localProperties should be filled with the list of its own properties
            """
        self.objectreference = reference  # the index in the Python storage where the Basic object is stored
        self.objecttype = objtype  # ('SF_String', 'DICTIONARY', ...)
        self.classmodule = classmodule  # Module (1), Class instance (2)
        self.name = name  # '' when no name
        self.internal = False  # True to exceptionally allow assigning a new value to a read-only property
        self.localProperties = []  # the properties reserved for internal use (often empty)
        self.SIMPLEEXEC = ScriptForge.InvokeSimpleScript  # Shortcuts to script provider interfaces
        self.EXEC = ScriptForge.InvokeBasicService

    def __getattr__(self, name):
        """
            Executed for EVERY property reference if name not yet in the instance dict
            At the 1st get, the property value is always got from Basic
            Due to the use of lower/camelcase synonyms, it is called for each variant of the same property
            The method manages itself the buffering in __dict__ based on the official ProperCase property name
            """
        if name in self.propertysynonyms:  # Reset real name if argument provided in lower or camel case
            name = self.propertysynonyms[name]
        if self.serviceimplementation == 'basic':
            if name in ('serviceproperties', 'localProperties', 'internal_attributes', 'propertysynonyms',
                        'forceGetProperty'):
                pass
            elif name in self.serviceproperties:
                if self.forceGetProperty is False and self.serviceproperties[name] is False:  # False = read-only
                    if name in self.__dict__:
                        return self.__dict__[name]
                    else:
                        # Get Property from Basic
                        prop = self.GetProperty(name)
                        self.__dict__[name] = prop
                        return prop
                else:
                    return self.GetProperty(name)
        # Execute the usual attributes getter
        return super(SFServices, self).__getattribute__(name)

    def __setattr__(self, name, value):
        """
            Executed for EVERY property assignment, including in __init__() !!
            Setting a property requires for serviceproperties() to be executed in Basic
            Management of __dict__ is automatically done in the final usual object.__setattr__ method
            """
        if self.serviceimplementation == 'basic':
            if name in ('serviceproperties', 'localProperties', 'internal_attributes', 'propertysynonyms'):
                pass
            elif name[0:2] == '__' or name in self.internal_attributes or name in self.localProperties:
                pass
            elif name in self.serviceproperties or name in self.propertysynonyms:
                if name in self.propertysynonyms:  # Reset real name if argument provided in lower or camel case
                    name = self.propertysynonyms[name]
                if self.internal:  # internal = True forces property local setting even if property is read-only
                    pass
                elif self.serviceproperties[name] is True:  # True == Editable
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

    @staticmethod
    def _getAttributeSynonyms(dico):
        """
            Returns a dictionary with key = name in lower case and in camelCase, value = real ProperCased name
            Example:
                 d = dict(ConfigFolder = False, InstallFolder = False)
                 dh = _getHomonyms(d)
                    # dh == dict(configfolder = 'ConfigFolder', installfolder = 'InstallFolder',
                                 configFolder = 'ConfigFolder', installFolder = 'InstallFolder')
            """
        def camelCase(key):
            return key[0].lower() + key[1:]

        lc = dict(zip(map(str.casefold, dico.keys()), dico.keys()))
        cc = dict(zip(map(camelCase, dico.keys()), dico.keys()))
        lc.update(cc)
        return lc

    def Dispose(self):
        if self.serviceimplementation == 'basic':
            if self.objectreference >= len(ScriptForge.servicesmodules):    # Do not dispose predefined module objects
                self.Execute(self.vbMethod, 'Dispose')
                self.objectreference = -1
    dispose = Dispose

    def Execute(self, flags = 0, methodname = '', *args):
        if flags == 0:
            flags = self.vbMethod
        if len(methodname) > 0:
            return self.EXEC(self.objectreference, flags, methodname, *args)

    def GetProperty(self, propertyname):
        """
            Get the given property from the Basic world
            """
        if self.serviceimplementation == 'basic':
            return self.EXEC(self.objectreference, self.vbGet, propertyname)
    getProperty, getproperty = GetProperty, GetProperty

    def Properties(self):
        return list(self.serviceproperties)
    properties = Properties

    def SetProperty(self, propertyname, value):
        """
            Set the given property to a new value in the Basic world
            """
        if self.serviceimplementation == 'basic':
            return self.EXEC(self.objectreference, self.vbLet, propertyname, value)
    setProperty, setproperty = SetProperty, SetProperty


# #####################################################################################################################
#                       SFScriptForge CLASS    (alias of ScriptForge Basic library)                                 ###
# #####################################################################################################################
class SFScriptForge:
    # #########################################################################
    # SF_Array CLASS
    # #########################################################################
    class SF_Array(SFServices, metaclass = _Singleton):
        """
            Provides a collection of methods for manipulating and transforming arrays of one dimension (vectors)
            and arrays of two dimensions (matrices). This includes set operations, sorting,
            importing to and exporting from text files.
            The Python version of the service provides a single method: ImportFromCSVFile
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'basic'
        servicename = 'ScriptForge.Array'
        servicesynonyms = ('array', 'scriptforge.array')
        serviceproperties = dict()
        propertysynonyms = SFServices._getAttributeSynonyms(serviceproperties)

        def ImportFromCSVFile(self, filename, delimiter = ',', dateformat = ''):
            """
                Difference with the Basic version: dates are returned in their iso format,
                not as any of the datetime objects.
                """
            return self.Execute(self.vbMethod + self.flgArrayRet, 'ImportFromCSVFile', filename, delimiter, dateformat)
        importFromCSVFile, importfromcsvfile = ImportFromCSVFile, ImportFromCSVFile

    # #########################################################################
    # SF_Basic CLASS
    # #########################################################################
    class SF_Basic(SFServices, metaclass = _Singleton):
        """
            This service proposes a collection of Basic methods to be executed in a Python context
            simulating the exact syntax and behaviour of the identical Basic builtin method.
            Typical example:
                SF_Basic.MsgBox('This has to be displayed in a message box')

            The signatures of Basic builtin functions are derived from
                core/basic/source/runtime/stdobj.cxx
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'python'
        servicename = 'ScriptForge.Basic'
        servicesynonyms = ('basic', 'scriptforge.basic')
        # Basic helper functions invocation
        module = 'SF_PythonHelper'
        # Message box constants
        MB_ABORTRETRYIGNORE, MB_DEFBUTTON1, MB_DEFBUTTON2, MB_DEFBUTTON3 = 2, 128, 256, 512
        MB_ICONEXCLAMATION, MB_ICONINFORMATION, MB_ICONQUESTION, MB_ICONSTOP = 48, 64, 32, 16
        MB_OK, MB_OKCANCEL, MB_RETRYCANCEL, MB_YESNO, MB_YESNOCANCEL = 0, 1, 5, 4, 3
        IDABORT, IDCANCEL, IDIGNORE, IDNO, IDOK, IDRETRY, IDYES = 3, 2, 5, 7, 1, 4, 6

        def ConvertFromUrl(self, url):
            return self.SIMPLEEXEC(self.module + '.PyConvertFromUrl', url)
        convertFromUrl, convertfromurl = ConvertFromUrl, ConvertFromUrl

        def ConvertToUrl(self, systempath):
            return self.SIMPLEEXEC(self.module + '.PyConvertToUrl', systempath)
        convertToUrl, converttourl = ConvertToUrl, ConvertToUrl

        def CreateUnoService(self, servicename):
            return self.SIMPLEEXEC(self.module + '.PyCreateUnoService', servicename)
        createUnoService, createunoservice = CreateUnoService, CreateUnoService

        def DateAdd(self, interval, number, date):
            if isinstance(date, datetime.datetime):
                date = date.isoformat()
            dateadd = self.SIMPLEEXEC(self.module + '.PyDateAdd', interval, number, date)
            return datetime.datetime.fromisoformat(dateadd)
        dateAdd, dateadd = DateAdd, DateAdd

        def DateDiff(self, interval, date1, date2, firstdayofweek = 1, firstweekofyear = 1):
            if isinstance(date1, datetime.datetime):
                date1 = date1.isoformat()
            if isinstance(date2, datetime.datetime):
                date2 = date2.isoformat()
            return self.SIMPLEEXEC(self.module + '.PyDateDiff', interval, date1, date2, firstdayofweek, firstweekofyear)
        dateDiff, datediff = DateDiff, DateDiff

        def DatePart(self, interval, date, firstdayofweek = 1, firstweekofyear = 1):
            if isinstance(date, datetime.datetime):
                date = date.isoformat()
            return self.SIMPLEEXEC(self.module + '.PyDatePart', interval, date, firstdayofweek, firstweekofyear)
        datePart, datepart = DatePart, DatePart

        def DateValue(self, string):
            if isinstance(string, datetime.datetime):
                string = string.isoformat()
            datevalue = self.SIMPLEEXEC(self.module + '.PyDateValue', string)
            return datetime.datetime.fromisoformat(datevalue)
        dateValue, datevalue = DateValue, DateValue

        def Format(self, expression, format = ''):
            if isinstance(expression, datetime.datetime):
                expression = expression.isoformat()
            return self.SIMPLEEXEC(self.module + '.PyFormat', expression, format)
        format = Format

        @staticmethod
        def GetDefaultContext():
            return ScriptForge.componentcontext
        getDefaultContext, getdefaultcontext = GetDefaultContext, GetDefaultContext

        def GetGuiType(self):
            return self.SIMPLEEXEC(self.module + '.PyGetGuiType')
        getGuiType, getguitype = GetGuiType, GetGuiType

        def GetSystemTicks(self):
            return self.SIMPLEEXEC(self.module + '.PyGetSystemTicks')
        getSystemTicks, getsystemticks = GetSystemTicks, GetSystemTicks

        @staticmethod
        def GetPathSeparator():
            return os.sep
        getPathSeparator, getpathseparator = GetPathSeparator, GetPathSeparator

        class GlobalScope(object, metaclass = _Singleton):
            @classmethod  # Mandatory because the GlobalScope class is normally not instantiated
            def BasicLibraries(cls):
                return ScriptForge.InvokeSimpleScript(SFScriptForge.SF_Basic.module + '.PyGlobalScope', 'Basic')

            @classmethod
            def DialogLibraries(cls):
                return ScriptForge.InvokeSimpleScript(SFScriptForge.SF_Basic.module + '.PyGlobalScope', 'Dialog')

        def InputBox(self, prompt, title = '', default = '', xpostwips = -1, ypostwips = -1):
            if xpostwips < 0 or ypostwips < 0:
                return self.SIMPLEEXEC(self.module + '.PyInputBox', prompt, title, default)
            return self.SIMPLEEXEC(self.module + '.PyInputBox', prompt, title, default, xpostwips, ypostwips)
        inputBox, inputbox = InputBox, InputBox

        def MsgBox(self, prompt, buttons = 0, title = ''):
            return self.SIMPLEEXEC(self.module + '.PyMsgBox', prompt, buttons, title)
        msgBox, msgbox = MsgBox, MsgBox

        @staticmethod
        def Now():
            return datetime.datetime.now()
        now = Now

        @staticmethod
        def RGB(red, green, blue):
            return int('%02x%02x%02x' % (red, green, blue), 16)
        rgb = RGB

        @staticmethod
        def StarDesktop():
            ctx = ScriptForge.componentcontext
            if ctx is None:
                return None
            smgr = ctx.getServiceManager()  # com.sun.star.lang.XMultiComponentFactory
            DESK = 'com.sun.star.frame.Desktop'
            desktop = smgr.createInstanceWithContext(DESK, ctx)
            return desktop
        starDesktop, stardesktop = StarDesktop, StarDesktop

        def Xray(self, unoobject = None):
            return self.SIMPLEEXEC('XrayTool._main.xray', unoobject)
        xray = Xray

    # #########################################################################
    # SF_Dictionary CLASS
    # #########################################################################
    class SF_Dictionary(SFServices, dict):
        """
            The service adds to a Python dict instance the interfaces for conversion to and from
            a list of UNO PropertyValues

            Usage:
                dico = dict(A = 1, B = 2, C = 3)
                myDict = CreateScriptService('Dictionary', dico)    # Initialize myDict with the content of dico
                myDict['D'] = 4
                print(myDict)   # {'A': 1, 'B': 2, 'C': 3, 'D': 4}
                propval = myDict.ConvertToPropertyValues()
            or
                dico = dict(A = 1, B = 2, C = 3)
                myDict = CreateScriptService('Dictionary')          # Initialize myDict as an empty dict object
                myDict.update(dico) # Load the values of dico into myDict
                myDict['D'] = 4
                print(myDict)   # {'A': 1, 'B': 2, 'C': 3, 'D': 4}
                propval = myDict.ConvertToPropertyValues()
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'python'
        servicename = 'ScriptForge.Dictionary'
        servicesynonyms = ('dictionary', 'scriptforge.dictionary')

        def __init__(self, dic = None):
            SFServices.__init__(self)
            dict.__init__(self)
            if dic is not None:
                self.update(dic)

        def ConvertToPropertyValues(self):
            """
                Store the content of the dictionary in an array of PropertyValues.
                Each entry in the array is a com.sun.star.beans.PropertyValue.
                he key is stored in Name, the value is stored in Value.

                If one of the items has a type datetime, it is converted to a com.sun.star.util.DateTime structure.
                If one of the items is an empty list, it is converted to None.

                The resulting array is empty when the dictionary is empty.
                """
            def CDateToUno(date):
                """
                Converts a datetime object into the corresponding com.sun.star.util.x date format
                :param date: datetime.datetime, date or time
                :return: a com.sun.star.util.DateTime, Date or Time
                """
                pvdate = None
                if isinstance(date, datetime.datetime):
                    pvdate = uno.createUnoStruct('com.sun.star.util.DateTime')
                elif isinstance(date, datetime.date):
                    pvdate = uno.createUnoStruct('com.sun.star.util.Date')
                elif isinstance(date, datetime.time):
                    pvdate = uno.createUnoStruct('com.sun.star.util.Time')
                if isinstance(date, (datetime.datetime, datetime.date)):
                    pvdate.Year = date.year
                    pvdate.Month = date.month
                    pvdate.Day = date.day
                if isinstance(date, (datetime.datetime, datetime.time)):
                    pvdate.Hours = date.hour
                    pvdate.Minutes = date.minute
                    pvdate.Seconds = date.second
                    pvdate.NanoSeconds = date.microsecond
                return pvdate

            result = []
            for key in iter(self):
                value = self[key]
                item = value
                if isinstance(value, dict):  # check that first level is not itself a (sub)dict
                    item = None
                elif isinstance(value, (tuple, list)):  # check every member of the list is not a (sub)dict
                    if len(value) == 0:  # Property values do not like empty lists
                        value = None
                    else:
                        for i in range(len(value)):
                            if isinstance(value[i], dict):
                                value[i] = None
                elif isinstance(value, (datetime.datetime, datetime.date, datetime.time)):
                    item = CDateToUno(value)
                pv = uno.createUnoStruct('com.sun.star.beans.PropertyValue')
                pv.Name = key
                pv.Value = item
                result.append(pv)
            return result
        convertToPropertyValues, converttopropertyvalues = ConvertToPropertyValues, ConvertToPropertyValues

        def ImportFromPropertyValues(self, propertyvalues, overwrite = False):
            """
                nserts the contents of an array of PropertyValue objects into the current dictionary.
                PropertyValue Names are used as keys in the dictionary, whereas Values contain the corresponding values.
                :param propertyvalues: a list.tuple containing com.sun.star.beans.PropertyValue objects
                :param overwrite: When True, entries with same name may exist in the dictionary and their values
                    are overwritten. When False (default), repeated keys are not overwritten.
                :return: True when successful
                """
            result = []
            for pv in iter(propertyvalues):
                key = pv.Name
                if overwrite is True or key not in self:
                    item = pv.Value
                    if 'com.sun.star.util.DateTime' in repr(type(item)):
                        item = datetime.datetime(item.Year, item.Month, item.Day,
                                                 item.Hours, item.Minutes, item.Seconds, item.NanoSeconds)
                    elif 'com.sun.star.util.Date' in repr(type(item)):
                        item = datetime.datetime(item.Year, item.Month, item.Day)
                    elif 'com.sun.star.util.Time' in repr(type(item)):
                        item = datetime.datetime(item.Hours, item.Minutes, item.Seconds, item.NanoSeconds)
                    result.append((key, item))
            self.update(result)
            return True
        importFromPropertyValues, importfrompropertyvalues = ImportFromPropertyValues, ImportFromPropertyValues

    # #########################################################################
    # SF_Exception CLASS
    # #########################################################################
    class SF_Exception(SFServices, metaclass = _Singleton):
        """
            The Exception service is a collection of methods for code debugging and error handling.

            The Exception service console stores events, variable values and information about errors.
            Use the console when the Python shell is not available, for example in Calc user defined functions (UDF)
            or during events processing.
            Use DebugPrint() method to aggregate additional user data of any type.

            Console entries can be dumped to a text file or visualized in a dialogue.
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'basic'
        servicename = 'ScriptForge.Exception'
        servicesynonyms = ('exception', 'scriptforge.exception')
        serviceproperties = dict()
        propertysynonyms = SFServices._getAttributeSynonyms(serviceproperties)

        def Console(self, modal = True):
            # Modal is always True in Python: Basic execution lasts only the time to display the box
            return self.Execute(self.vbMethod, 'Console', True)
        console = Console

        def ConsoleClear(self, keep = 0):
            return self.Execute(self.vbMethod, 'ConsoleClear', keep)
        consoleClear, consoleclear = ConsoleClear, ConsoleClear

        def ConsoleToFile(self, filename):
            return self.Execute(self.vbMethod, 'ConsoleToFile', filename)
        consoleToFile, consoletofile = ConsoleToFile, ConsoleToFile

        def DebugDisplay(self, *args):
            # Arguments are concatenated in a single string similar to what the Python print() function would produce
            self.DebugPrint(*args)
            param = '\n'.join(list(map(lambda a: a.strip("'") if isinstance(a, str) else repr(a), args)))
            bas = CreateScriptService('ScriptForge.Basic')
            return bas.MsgBox(param, bas.MB_OK + bas.MB_ICONINFORMATION, 'DebugDisplay')
        debugDisplay, debugdisplay = DebugDisplay, DebugDisplay

        def DebugPrint(self, *args):
            # Arguments are concatenated in a single string similar to what the Python print() function would produce
            param = '\t'.join(list(map(repr, args))).expandtabs(tabsize = 4)
            return self.Execute(self.vbMethod, 'DebugPrint', param)
        debugPrint, debugprint = DebugPrint, DebugPrint

        def RaiseFatal(self, errorcode, *args):
            """
                Generate a run-time error caused by an anomaly in a user script detected by ScriptForge
                The message is logged in the console. The execution is STOPPED
                For INTERNAL USE only
                """
            # Direct call because RaiseFatal forces an execution stop in Basic
            return self.SIMPLEEXEC('SF_Exception.RaiseFatal', errorcode, *args)

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
        servicesynonyms = ('filesystem', 'scriptforge.filesystem')
        serviceproperties = dict(FileNaming = True, ConfigFolder = False, ExtensionsFolder = False, HomeFolder = False,
                                 InstallFolder = False, TemplatesFolder = False, TemporaryFolder = False,
                                 UserTemplatesFolder = False)
        propertysynonyms = SFServices._getAttributeSynonyms(serviceproperties)
        # Force for each property to get its value from Basic - due to FileNaming updatability
        forceGetProperty = True
        # Open TextStream constants
        ForReading, ForWriting, ForAppending = 1, 2, 8

        def BuildPath(self, foldername, name):
            return self.Execute(self.vbMethod, 'BuildPath', foldername, name)

        def CompareFiles(self, filename1, filename2, comparecontents = False):
            py = ScriptForge.pythonhelpermodule + '$' + '_SF_FileSystem__CompareFiles'
            if self.FileExists(filename1) and self.FileExists(filename2):
                file1 = self._ConvertFromUrl(filename1)
                file2 = self._ConvertFromUrl(filename2)
                return self.SIMPLEEXEC(py, file1, file2, comparecontents)
            else:
                return False

        def CopyFile(self, source, destination, overwrite = True):
            return self.Execute(self.vbMethod, 'CopyFile', source, destination, overwrite)

        def CopyFolder(self, source, destination, overwrite = True):
            return self.Execute(self.vbMethod, 'CopyFolder', source, destination, overwrite)

        def CreateFolder(self, foldername):
            return self.Execute(self.vbMethod, 'CreateFolder', foldername)

        def CreateTextFile(self, filename, overwrite = True, encoding = 'UTF-8'):
            return self.Execute(self.vbMethod, 'CreateTextFile', filename, overwrite, encoding)

        def DeleteFile(self, filename):
            return self.Execute(self.vbMethod, 'DeleteFile', filename)

        def DeleteFolder(self, foldername):
            return self.Execute(self.vbMethod, 'DeleteFolder', foldername)

        def FileExists(self, filename):
            return self.Execute(self.vbMethod, 'FileExists', filename)
        fileexists, fileExists = FileExists, FileExists

        def Files(self, foldername, filter = ''):
            return self.Execute(self.vbMethod, 'Files', foldername, filter)

        def FolderExists(self, foldername):
            return self.Execute(self.vbMethod, 'FolderExists', foldername)

        def GetBaseName(self, filename):
            return self.Execute(self.vbMethod, 'GetBaseName', filename)

        def GetExtension(self, filename):
            return self.Execute(self.vbMethod, 'GetExtension', filename)

        def GetFileLen(self, filename):
            py = ScriptForge.pythonhelpermodule + '$' + '_SF_FileSystem__GetFilelen'
            if self.FileExists(filename):
                file = self._ConvertFromUrl(filename)
                return int(self.SIMPLEEXEC(py, file))
            else:
                return 0

        def GetFileModified(self, filename):
            return self.Execute(self.vbMethod + self.flgDateRet, 'GetFileModified', filename)

        def GetName(self, filename):
            return self.Execute(self.vbMethod, 'GetName', filename)

        def GetParentFolderName(self, filename):
            return self.Execute(self.vbMethod, 'GetParentFolderName', filename)

        def GetTempName(self):
            return self.Execute(self.vbMethod, 'GetTempName')

        def HashFile(self, filename, algorithm):
            py = ScriptForge.pythonhelpermodule + '$' + '_SF_FileSystem__HashFile'
            if self.FileExists(filename):
                file = self._ConvertFromUrl(filename)
                return self.SIMPLEEXEC(py, file, algorithm.lower())
            else:
                return ''

        def MoveFile(self, source, destination):
            return self.Execute(self.vbMethod, 'MoveFile', source, destination)

        def MoveFolder(self, source, destination):
            return self.Execute(self.vbMethod, 'MoveFolder', source, destination)

        def OpenTextFile(self, filename, iomode = 1, create = False, encoding = 'UTF-8'):
            return self.Execute(self.vbMethod, 'OpenTextFile', filename, iomode, create, encoding)

        def PickFile(self, defaultfile = ScriptForge.cstSymEmpty, mode = 'OPEN', filter = ''):
            return self.Execute(self.vbMethod, 'PickFile', defaultfile, mode, filter)

        def PickFolder(self, defaultfolder = ScriptForge.cstSymEmpty, freetext = ''):
            return self.Execute(self.vbMethod, 'PickFolder', defaultfolder, freetext)

        def SubFolders(self, foldername, filter = ''):
            return self.Execute(self.vbMethod, 'SubFolders', foldername, filter)

        def _ConvertFromUrl(self, filename):
            # Alias for same function in FileSystem Basic module
            return self.SIMPLEEXEC('ScriptForge.SF_FileSystem._ConvertFromUrl', filename)

    # #########################################################################
    # SF_L10N CLASS
    # #########################################################################
    class SF_L10N(SFServices):
        """
            This service provides a number of methods related to the translation of strings
            with minimal impact on the program's source code.
            The methods provided by the L10N service can be used mainly to:
                Create POT files that can be used as templates for translation of all strings in the program.
                Get translated strings at runtime for the language defined in the Locale property.
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'basic'
        servicename = 'ScriptForge.L10N'
        servicesynonyms = ()
        serviceproperties = dict(Folder = False, Languages = False, Locale = False)
        propertysynonyms = SFServices._getAttributeSynonyms(serviceproperties)

        def AddText(self, context = '', msgid = '', comment = ''):
            return self.Execute(self.vbMethod, 'AddText', context, msgid, comment)
        addText, addtext = AddText, AddText

        def ExportToPOTFile(self, filename, header = '', encoding= 'UTF-8'):
            return self.Execute(self.vbMethod, 'ExportToPOTFile', filename, header, encoding)
        exportToPOTFile, exporttopotfile = ExportToPOTFile, ExportToPOTFile

        def GetText(self, msgid, *args):
            return self.Execute(self.vbMethod, 'GetText', msgid, *args)
        _, gettext, getText = GetText, GetText, GetText

    # #########################################################################
    # SF_Platform CLASS
    # #########################################################################
    class SF_Platform(SFServices, metaclass = _Singleton):
        """
            The 'Platform' service implements a collection of properties about the actual execution environment
            and context :
                the hardware platform
                the operating system
                the LibreOffice version
                the current user
            All those properties are read-only.
            The implementation is mainly based on the 'platform' module of the Python standard library
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'basic'
        servicename = 'ScriptForge.Platform'
        servicesynonyms = ()
        serviceproperties = dict(Architecture = False, ComputerName = False, CPUCount = False, CurrentUser = False,
                                 Locale = False, Machine = False, OfficeVersion = False, OSName = False,
                                 OSPlatform = False, OSRelease = False, OSVersion = False, Processor = False)
        propertysynonyms = SFServices._getAttributeSynonyms(serviceproperties)
        # Python helper functions
        py = ScriptForge.pythonhelpermodule + '$' + '_SF_Platform'

        @property
        def Architecture(self):
            return self.SIMPLEEXEC(self.py, 'Architecture')
        architecture = Architecture

        @property
        def ComputerName(self):
            return self.SIMPLEEXEC(self.py, 'ComputerName')
        computerName, computername = ComputerName, ComputerName

        @property
        def CPUCount(self):
            return self.SIMPLEEXEC(self.py, 'CPUCount')
        cpuCount, cpucount = CPUCount, CPUCount

        @property
        def CurrentUser(self):
            return self.SIMPLEEXEC(self.py, 'CurrentUser')
        currentUser, currentuser = CurrentUser, CurrentUser

        @property
        def Machine(self):
            return self.SIMPLEEXEC(self.py, 'Machine')
        machine = Machine

        @property
        def OSName(self):
            return self.SIMPLEEXEC(self.py, 'OSName')
        osName, osname = OSName, OSName

        @property
        def OSPlatform(self):
            return self.SIMPLEEXEC(self.py, 'OSPlatform')
        osPlatform, osplatform = OSPlatform, OSPlatform

        @property
        def OSRelease(self):
            return self.SIMPLEEXEC(self.py, 'OSRelease')
        osRelease, osrelease = OSRelease, OSRelease

        @property
        def OSVersion(self):
            return self.SIMPLEEXEC(self.py, 'OSVersion')
        osVersion, osversion = OSVersion, OSVersion

        @property
        def Processor(self):
            return self.SIMPLEEXEC(self.py, 'Processor')
        processor = Processor

    # #########################################################################
    # SF_Session CLASS
    # #########################################################################
    class SF_Session(SFServices, metaclass = _Singleton):
        """
            The Session service gathers various general-purpose methods about:
            - UNO introspection
            - the invocation of external scripts or programs
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'basic'
        servicename = 'ScriptForge.Session'
        servicesynonyms = ('session', 'scriptforge.session')
        serviceproperties = dict()
        propertysynonyms = SFServices._getAttributeSynonyms(serviceproperties)

        # Class constants                       Where to find an invoked library ?
        SCRIPTISEMBEDDED = 'document'           # in the document
        SCRIPTISAPPLICATION = 'application'     # in any shared library (Basic)
        SCRIPTISPERSONAL = 'user'               # in My Macros (Python)
        SCRIPTISPERSOXT = 'user:uno_packages'   # in an extension installed for the current user (Python)
        SCRIPTISSHARED = 'share'                # in LibreOffice macros (Python)
        SCRIPTISSHAROXT = 'share:uno_packages'  # in an extension installed for all users (Python)
        SCRIPTISOXT = 'uno_packages'            # in an extension but the installation parameters are unknown (Python)

        def ExecuteBasicScript(self, scope = '', script = '', *args):
            if scope is None or scope == '':
                scope = self.SCRIPTISAPPLICATION
            if len(args) == 0:
                args = (scope,) + (script,) + (None,)
            else:
                args = (scope,) + (script,) + args
            # ExecuteBasicScript method has a ParamArray parameter in Basic
            return self.SIMPLEEXEC('@SF_Session.ExecuteBasicScript', args)
        executeBasicScript, executebasicscript = ExecuteBasicScript, ExecuteBasicScript

        def ExecuteCalcFunction(self, calcfunction, *args):
            if len(args) == 0:
                # Arguments of Calc functions are strings or numbers. None == Empty is a good alias for no argument
                args = (calcfunction,) + (None,)
            else:
                args = (calcfunction,) + args
            # ExecuteCalcFunction method has a ParamArray parameter in Basic
            return self.SIMPLEEXEC('@SF_Session.ExecuteCalcFunction', args)
        executeCalcFunction, executecalcfunction = ExecuteCalcFunction, ExecuteCalcFunction

        def ExecutePythonScript(self, scope = '', script = '', *args):
            return self.SIMPLEEXEC(scope + ':' + script, *args)
        executePythonScript, executepythonscript = ExecutePythonScript, ExecutePythonScript

        def HasUnoMethod(self, unoobject, methodname):
            return self.Execute(self.vbMethod, 'HasUnoMethod', unoobject, methodname)
        hasUnoMethod, hasunomethod = HasUnoMethod, HasUnoMethod

        def HasUnoProperty(self, unoobject, propertyname):
            return self.Execute(self.vbMethod, 'HasUnoProperty', unoobject, propertyname)
        hasUnoProperty, hasunoproperty = HasUnoProperty, HasUnoProperty

        def OpenURLInBrowser(self, url):
            py = ScriptForge.pythonhelpermodule + '$' + '_SF_Session__OpenURLInBrowser'
            return self.SIMPLEEXEC(py, url)
        openURLInBrowser, openurlinbrowser = OpenURLInBrowser, OpenURLInBrowser

        def RunApplication(self, command, parameters):
            return self.Execute(self.vbMethod, 'RunApplication', command, parameters)
        runApplication, runapplication = RunApplication, RunApplication

        def SendMail(self, recipient, cc = '', bcc = '', subject = '', body = '', filenames = '', editmessage = True):
            return self.Execute(self.vbMethod, 'SendMail', recipient, cc, bcc, subject, body, filenames, editmessage)
        sendMail, sendmail = SendMail, SendMail

        def UnoObjectType(self, unoobject):
            return self.Execute(self.vbMethod, 'UnoObjectType', unoobject)
        unoObjectType, unoobjecttype = UnoObjectType, UnoObjectType

        def UnoMethods(self, unoobject):
            return self.Execute(self.vbMethod, 'UnoMethods', unoobject)
        unoMethods, unomethods = UnoMethods, UnoMethods

        def UnoProperties(self, unoobject):
            return self.Execute(self.vbMethod, 'UnoProperties', unoobject)
        unoProperties, unoproperties = UnoProperties, UnoProperties

        def WebService(self, uri):
            return self.Execute(self.vbMethod, 'WebService', uri)
        webService, webservice = WebService, WebService

    # #########################################################################
    # SF_String CLASS
    # #########################################################################
    class SF_String(SFServices, metaclass = _Singleton):
        """
            Focus on string manipulation, regular expressions, encodings and hashing algorithms.
            The methods implemented in Basic that are redundant with Python builtin functions
            are not duplicated
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'basic'
        servicename = 'ScriptForge.String'
        servicesynonyms = ('string', 'scriptforge.string')
        serviceproperties = dict()
        propertysynonyms = SFServices._getAttributeSynonyms(serviceproperties)

        def HashStr(self, inputstr, algorithm):
            py = ScriptForge.pythonhelpermodule + '$' + '_SF_String__HashStr'
            return self.SIMPLEEXEC(py, inputstr, algorithm.lower())
        hashStr, hashstr = HashStr, HashStr

        def IsADate(self, inputstr, dateformat = 'YYYY-MM-DD'):
            return self.Execute(self.vbMethod, 'IsADate', inputstr, dateformat)
        isADate, isadate = IsADate, IsADate

        def IsEmail(self, inputstr):
            return self.Execute(self.vbMethod, 'IsEmail', inputstr)
        isEmail, isemail = IsEmail, IsEmail

        def IsFileName(self, inputstr, osname = ScriptForge.cstSymEmpty):
            return self.Execute(self.vbMethod, 'IsFileName', inputstr, osname)
        isFileName, isfilename = IsFileName, IsFileName

        def IsIBAN(self, inputstr):
            return self.Execute(self.vbMethod, 'IsIBAN', inputstr)
        isIBAN, isiban = IsIBAN, IsIBAN

        def IsIPv4(self, inputstr):
            return self.Execute(self.vbMethod, 'IsIPv4', inputstr)
        isIPv4, isipv4 = IsIPv4, IsIPv4

        def IsLike(self, inputstr, pattern, casesensitive = False):
            return self.Execute(self.vbMethod, 'IsLike', inputstr, pattern, casesensitive)
        isLike, islike = IsLike, IsLike

        def IsSheetName(self, inputstr):
            return self.Execute(self.vbMethod, 'IsSheetName', inputstr)
        isSheetName, issheetname = IsSheetName, IsSheetName

        def IsUrl(self, inputstr):
            return self.Execute(self.vbMethod, 'IsUrl', inputstr)
        isUrl, isurl = IsUrl, IsUrl

        def SplitNotQuoted(self, inputstr, delimiter = ' ', occurrences = 0, quotechar = '"'):
            return self.Execute(self.vbMethod, 'SplitNotQuoted', inputstr, delimiter, occurrences, quotechar)
        splitNotQuoted, splitnotquoted = SplitNotQuoted, SplitNotQuoted

        def Wrap(self, inputstr, width = 70, tabsize = 8):
            return self.Execute(self.vbMethod, 'Wrap', inputstr, width, tabsize)
        wrap = Wrap

    # #########################################################################
    # SF_TextStream CLASS
    # #########################################################################
    class SF_TextStream(SFServices):
        """
            The TextStream service is used to sequentially read from and write to files opened or created
            using the ScriptForge.FileSystem service..
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'basic'
        servicename = 'ScriptForge.TextStream'
        servicesynonyms = ()
        serviceproperties = dict(AtEndOfStream = False, Encoding = False, FileName = False, IOMode = False,
                                 Line = False, NewLine = True)
        propertysynonyms = SFServices._getAttributeSynonyms(serviceproperties)

        @property
        def AtEndOfStream(self):
            return self.GetProperty('AtEndOfStream')
        atEndOfStream, atendofstream = AtEndOfStream, AtEndOfStream

        @property
        def Line(self):
            return self.GetProperty('Line')
        line = Line

        def CloseFile(self):
            return self.Execute(self.vbMethod, 'CloseFile')
        closeFile, closefile = CloseFile, CloseFile

        def ReadAll(self):
            return self.Execute(self.vbMethod, 'ReadAll')
        readAll, readall = ReadAll, ReadAll

        def ReadLine(self):
            return self.Execute(self.vbMethod, 'ReadLine')
        readLine, readline = ReadLine, ReadLine

        def SkipLine(self):
            return self.Execute(self.vbMethod, 'SkipLine')
        skipLine, skipline = SkipLine, SkipLine

        def WriteBlankLines(self, lines):
            return self.Execute(self.vbMethod, 'WriteBlankLines', lines)
        writeBlankLines, writeblanklines = WriteBlankLines, WriteBlankLines

        def WriteLine(self, line):
            return self.Execute(self.vbMethod, 'WriteLine', line)
        writeLine, writeline = WriteLine, WriteLine

    # #########################################################################
    # SF_Timer CLASS
    # #########################################################################
    class SF_Timer(SFServices):
        """
            The "Timer" service measures the amount of time it takes to run user scripts.
            """
        # Mandatory class properties for service registration
        serviceimplementation = 'basic'
        servicename = 'ScriptForge.Timer'
        servicesynonyms = ()
        serviceproperties = dict(Duration = False, IsStarted = False, IsSuspended = False,
                                 SuspendDuration = False, TotalDuration = False)
        propertysynonyms = SFServices._getAttributeSynonyms(serviceproperties)
        # Force for each property to get its value from Basic
        forceGetProperty = True

        def Continue(self):
            return self.Execute(self.vbMethod, 'Continue')

        def Restart(self):
            return self.Execute(self.vbMethod, 'Restart')
        restart = Restart

        def Start(self):
            return self.Execute(self.vbMethod, 'Start')
        start = Start

        def Suspend(self):
            return self.Execute(self.vbMethod, 'Suspend')
        suspend = Suspend

        def Terminate(self):
            return self.Execute(self.vbMethod, 'Terminate')
        terminate = Terminate


# ##############################################False#######################################################################
#                           CreateScriptService()                                                                   ###
# #####################################################################################################################
def CreateScriptService(service, *args):
    """
        A service being the name of a collection of properties and methods,
        this method returns either
            - the Python object mirror of the Basic object implementing the requested service
            - the Python object implementing the service itself

        A service may be designated by its official name, stored in its class.servicename
        or by one of its synonyms stored in its class.servicesynonyms list
        If the service is not identified, the service creation is delegated to Basic, that might raise an error
        if still not identified there

        :param service: the name of the service as a string 'library.service' - cased exactly
                or one of its synonyms
        :param args: the arguments to pass to the service constructor
        :return: the service as a Python object
        """
    # Init at each CreateScriptService() invocation
    #       CreateScriptService is usually the first statement in user scripts requesting ScriptForge services
    #       ScriptForge() is optional in user scripts when Python process inside LibreOffice process
    ScriptForge()

    def ResolveSynonyms(servicename):
        """
            Synonyms within service names implemented in Python or predefined are resolved here
            :param servicename: The short name of the service
            :return: The official service name if found, the argument otherwise
            """
        for cls in SFServices.__subclasses__():
            if servicename.lower() in cls.servicesynonyms:
                return cls.servicename
        return servicename

    #
    # Check the list of available services
    scriptservice = ResolveSynonyms(service)
    if scriptservice in ScriptForge.serviceslist:
        serv = ScriptForge.serviceslist[scriptservice]
        # Check if the requested service is within the Python world
        if serv.serviceimplementation == 'python':
            return serv(*args)
        # Check if the service is a predefined standard Basic service
        elif scriptservice in ScriptForge.servicesmodules:
            return serv(ScriptForge.servicesmodules[scriptservice], classmodule = SFServices.moduleStandard)
    # The requested service is to be found in the Basic world
    if len(args) == 0:
        serv = ScriptForge.InvokeBasicService('SF_Services', SFServices.vbMethod, 'CreateScriptService', service)
    else:
        serv = ScriptForge.InvokeBasicService('SF_Services', SFServices.vbMethod, 'CreateScriptService', service, *args)
    return serv


createScriptService, createscriptservice = CreateScriptService, CreateScriptService


# #####################################################################################################################
#                           Services shortcuts                                                                      ###
# #####################################################################################################################
SF_Basic = SFScriptForge.SF_Basic()
# SF_String = None


# ######################################################################
# Lists the scripts, that shall be visible inside the Basic/Python IDE
# ######################################################################

g_exportedScripts = ()
