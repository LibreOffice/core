# -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
import sys

import pyuno

try:
    import __builtin__
except ImportError:
    import builtins as __builtin__

try:
    unicode
except NameError:
    # Python 3 compatibility
    unicode = str

import socket # since on Windows sal3.dll no longer calls WSAStartup

# all functions and variables starting with a underscore (_) must be considered private
# and can be changed at any time. Don't use them
_g_ctx = pyuno.getComponentContext( )
_g_delegatee = __builtin__.__dict__["__import__"]

def getComponentContext():
    """ returns the UNO component context, that was used to initialize the python runtime.
    """
    return _g_ctx

def getConstantByName( constant ):
    "Looks up the value of a idl constant by giving its explicit name"
    return pyuno.getConstantByName( constant )

def getTypeByName( typeName):
    """ returns a uno.Type instance of the type given by typeName. In case the
        type does not exist, a com.sun.star.uno.RuntimeException is raised.
    """
    return pyuno.getTypeByName( typeName )

def createUnoStruct( typeName, *args, **kwargs ):
    """creates a uno struct or exception given by typeName. Can be called with:
    1) No additional argument.
       In this case, you get a default constructed uno structure.
       ( e.g. createUnoStruct( "com.sun.star.uno.Exception" ) )
    2) Exactly one additional argument that is an instance of typeName.
       In this case, a copy constructed instance of typeName is returned
       ( e.g. createUnoStruct( "com.sun.star.uno.Exception" , e ) )
    3) As many additional arguments as the number of elements within typeName
       (e.g. createUnoStruct( "com.sun.star.uno.Exception", "foo error" , self) ).
    4) Keyword arguments to give values for each element of the struct by name.
    5) A mix of 3) and 4), such that each struct element is given a value exactly once,
       either by a positional argument or by a keyword argument.
    The additional and/or keyword arguments must match the type of each struct element,
    otherwise an exception is thrown.
    """
    return getClass(typeName)( *args, **kwargs )

def getClass( typeName ):
    """returns the class of a concrete uno exception, struct or interface
    """
    return pyuno.getClass(typeName)

def isInterface( obj ):
    """returns true, when obj is a class of a uno interface"""
    return pyuno.isInterface( obj )

def generateUuid():
    "returns a 16 byte sequence containing a newly generated uuid or guid, see rtl/uuid.h "
    return pyuno.generateUuid()

def systemPathToFileUrl( systemPath ):
    "returns a file-url for the given system path"
    return pyuno.systemPathToFileUrl( systemPath )

def fileUrlToSystemPath( url ):
    "returns a system path (determined by the system, the python interpreter is running on)"
    return pyuno.fileUrlToSystemPath( url )

def absolutize( path, relativeUrl ):
    "returns an absolute file url from the given urls"
    return pyuno.absolutize( path, relativeUrl )

def getCurrentContext():
    """Returns the currently valid current context.
       see http://udk.openoffice.org/common/man/concept/uno_contexts.html#current_context
       for an explanation on the current context concept
    """
    return pyuno.getCurrentContext()

def setCurrentContext( newContext ):
    """Sets newContext as new uno current context. The newContext must
    implement the XCurrentContext interface. The implementation should
    handle the desired properties and delegate unknown properties to the
    old context. Ensure to reset the old one when you leave your stack ...
    see http://udk.openoffice.org/common/man/concept/uno_contexts.html#current_context
    """
    return pyuno.setCurrentContext( newContext )


class Enum:
    "Represents a UNO idl enum, use an instance of this class to explicitly pass a boolean to UNO"
    #typeName the name of the enum as a string
    #value    the actual value of this enum as a string
    def __init__(self,typeName, value):
        self.typeName = typeName
        self.value = value
        pyuno.checkEnum( self )

    def __repr__(self):
        return "<uno.Enum %s (%r)>" % (self.typeName, self.value)

    def __eq__(self, that):
        if not isinstance(that, Enum):
            return False
        return (self.typeName == that.typeName) and (self.value == that.value)

class Type:
    "Represents a UNO type, use an instance of this class to explicitly pass a boolean to UNO"
#    typeName                 # Name of the UNO type
#    typeClass                # python Enum of TypeClass,  see com/sun/star/uno/TypeClass.idl
    def __init__(self, typeName, typeClass):
        self.typeName = typeName
        self.typeClass = typeClass
        pyuno.checkType(self)
    def __repr__(self):
        return "<Type instance %s (%r)>" % (self.typeName, self.typeClass)

    def __eq__(self, that):
        if not isinstance(that, Type):
            return False
        return self.typeClass == that.typeClass and self.typeName == that.typeName

    def __hash__(self):
        return self.typeName.__hash__()

class Bool(object):
    """Represents a UNO boolean, use an instance of this class to explicitly
       pass a boolean to UNO.
       Note: This class is deprecated. Use python's True and False directly instead
    """
    def __new__(cls, value):
        if isinstance(value, (str, unicode)) and value == "true":
            return True
        if isinstance(value, (str, unicode)) and value == "false":
            return False
        if value:
            return True
        return False

class Char:
    "Represents a UNO char, use an instance of this class to explicitly pass a char to UNO"
    # @param value pass a Unicode string with length 1
    def __init__(self,value):
        assert isinstance(value, unicode)
        assert len(value) == 1
        self.value=value

    def __repr__(self):
        return "<Char instance %s>" % (self.value, )

    def __eq__(self, that):
        if isinstance(that, (str, unicode)):
            if len(that) > 1:
                return False
            return self.value == that[0]
        if isinstance(that, Char):
            return self.value == that.value
        return False

# Suggested by Christian, but still some open problems which need to be solved first
#
#class ByteSequence(str):
#
#    def __repr__(self):
#        return "<ByteSequence instance %s>" % str.__repr__(self)

    # for a little bit compatibility; setting value is not possible as
    # strings are immutable
#    def _get_value(self):
#        return self
#
#    value = property(_get_value)

class ByteSequence:
    def __init__(self, value):
        if isinstance(value, bytes):
            self.value = value
        elif isinstance(value, str):
            self.value = value.encode("utf-8") # Python 2 compatibility
        elif isinstance(value, ByteSequence):
            self.value = value.value
        else:
            raise TypeError("expected string or bytesequence")

    def __repr__(self):
        return "<ByteSequence instance '%s'>" % (self.value, )

    def __eq__(self, that):
        if isinstance( that, ByteSequence):
            return self.value == that.value
        if isinstance(that, bytes):
            return self.value == that
        if isinstance(that, str):
            return self.value == that.encode("utf-8")
        return False

    def __len__(self):
        return len(self.value)

    def __getitem__(self, index):
        return self.value[index]

    def __iter__( self ):
        return self.value.__iter__()

    def __add__( self , b ):
        if isinstance( b, bytes):
            return ByteSequence(self.value + b)
        elif isinstance( b, str ):
            return ByteSequence( self.value + b.encode("utf-8") )
        elif isinstance( b, ByteSequence ):
            return ByteSequence( self.value + b.value )
        raise TypeError( "expected string or ByteSequence as operand" )

    def __hash__( self ):
        return self.value.hash()


class Any:
    "use only in connection with uno.invoke() to pass an explicit typed any"
    def __init__(self, type, value ):
        if isinstance( type, Type ):
            self.type = type
        else:
            self.type = getTypeByName( type )
        self.value = value

def invoke( object, methodname, argTuple ):
    "use this function to pass exactly typed anys to the callee (using uno.Any)"
    return pyuno.invoke( object, methodname, argTuple )

#---------------------------------------------------------------------------------------
# don't use any functions beyond this point, private section, likely to change
#---------------------------------------------------------------------------------------
#def _uno_import( name, globals={}, locals={}, fromlist=[], level=-1 ):
def _uno_import( name, *optargs, **kwargs ):
    try:
#       print "optargs = " + repr(optargs)
        return _g_delegatee( name, *optargs, **kwargs )
    except ImportError as e:
        # process optargs
        globals, locals, fromlist = list(optargs)[:3] + [kwargs.get('globals',{}), kwargs.get('locals',{}), kwargs.get('fromlist',[])][len(optargs):]
        # from import form only, but skip if an uno lookup has already failed
        if not fromlist or hasattr(e, '_uno_import_failed'):
            raise
        # hang onto exception for possible use on subsequent uno lookup failure
        py_import_exc = e
    modnames = name.split( "." )
    mod = None
    d = sys.modules
    for x in modnames:
        if x in d:
           mod = d[x]
        else:
           mod = pyuno.__class__(x)  # How to create a module ??
        d = mod.__dict__

    RuntimeException = pyuno.getClass( "com.sun.star.uno.RuntimeException" )
    for x in fromlist:
       if x not in d:
          failed = False
          if x.startswith( "typeOf" ):
             try:
                d[x] = pyuno.getTypeByName( name + "." + x[6:len(x)] )
             except RuntimeException:
                failed = True
          else:
            try:
                # check for structs, exceptions or interfaces
                d[x] = pyuno.getClass( name + "." + x )
            except RuntimeException:
                # check for enums
                try:
                   d[x] = Enum( name , x )
                except RuntimeException:
                   # check for constants
                   try:
                      d[x] = getConstantByName( name + "." + x )
                   except RuntimeException:
                      # check for constant group
                      try:
                          d[x] = _impl_getConstantGroupByName( name, x )
                      except ValueError:
                          failed = True

          if failed:
              # We have an import failure, but cannot distinguish between
              # uno and non-uno errors as uno lookups are attempted for all
              # "from xxx import yyy" imports following a python failure.
              #
              # In Python 3, the original python exception traceback is reused
              # to help pinpoint the actual failing location.  Its original
              # message, unlike Python 2, is unlikely to be helpful for uno
              # failures, as it most commonly is just a top level module like
              # 'com'.  So our exception appends the uno lookup failure.
              # This is more ambiguous, but it plus the traceback should be
              # sufficient to identify a root cause for python or uno issues.
              #
              # Our exception is raised outside of the nested exception
              # handlers above, to avoid Python 3 nested exception
              # information for the RuntimeExceptions during lookups.
              #
              # Finally, a private attribute is used to prevent further
              # processing if this failure was in a nested import.  That
              # keeps the exception relevant to the primary failure point,
              # preventing us from re-processing our own import errors.

              uno_import_exc = ImportError("%s (or '%s.%s' is unknown)" %
                                           (py_import_exc, name, x))
              if sys.version_info[0] >= 3:
                  uno_import_exc = uno_import_exc.with_traceback(py_import_exc.__traceback__)
              uno_import_exc._uno_import_failed = True
              raise uno_import_exc

    return mod

# private
class _ConstantGroup(object):
    __slots__ = ['_constants']
    def __init__(self, constants):
        self._constants = constants
    def __dir__(self):
        return self._constants.keys()
    def __getattr__(self,name):
        if name in self._constants:
            return self._constants[name]
        raise AttributeError

# private
def _impl_getConstantGroupByName( module, group ):
    CONSTANTS = Enum('com.sun.star.uno.TypeClass', 'CONSTANTS')
    ONE = Enum('com.sun.star.reflection.TypeDescriptionSearchDepth', 'ONE')
    tdm = _g_ctx.getValueByName('/singletons/com.sun.star.reflection.theTypeDescriptionManager')
    tde = tdm.createTypeDescriptionEnumeration(module,(CONSTANTS,),ONE)
    qualifiedName = module + '.' + group
    for td in tde:
        if td.Name == qualifiedName:
            return _ConstantGroup({c.Name.split('.')[-1]: c.ConstantValue for c in td.Constants})
    else:
        raise ValueError

# private function, don't use
def _impl_extractName(name):
    r = list(range(len(name)-1,0,-1))
    for i in r:
        if name[i] == ".":
           name = name[i+1:len(name)]
           break
    return name

# private, referenced from the pyuno shared library
def _uno_struct__init__(self,*args, **kwargs):
    if len(kwargs) == 0 and len(args) == 1 and hasattr(args[0], "__class__") and args[0].__class__ == self.__class__ :
        self.__dict__["value"] = args[0]
    else:
        struct, used = pyuno._createUnoStructHelper(self.__class__.__pyunostruct__,args,**kwargs)
        for kw in kwargs.keys():
            if not (kw in used and used[kw]):
                RuntimeException = pyuno.getClass( "com.sun.star.uno.RuntimeException" )
                raise RuntimeException("_uno_struct__init__: unused keyword argument '" + kw + "'", None)
        self.__dict__["value"] = struct

# private, referenced from the pyuno shared library
def _uno_struct__getattr__(self,name):
    return __builtin__.getattr(self.__dict__["value"],name)

# private, referenced from the pyuno shared library
def _uno_struct__setattr__(self,name,value):
    return __builtin__.setattr(self.__dict__["value"],name,value)

# private, referenced from the pyuno shared library
def _uno_struct__repr__(self):
    return repr(self.__dict__["value"])

def _uno_struct__str__(self):
    return str(self.__dict__["value"])

# private, referenced from the pyuno shared library
def _uno_struct__eq__(self,cmp):
    if hasattr(cmp,"value"):
       return self.__dict__["value"] == cmp.__dict__["value"]
    return False

# referenced from pyuno shared lib and pythonscript.py
def _uno_extract_printable_stacktrace( trace ):
    mod = None
    try:
        mod = __import__("traceback")
    except ImportError as e:
        pass
    ret = ""
    if mod:
        lst = mod.extract_tb( trace )
        max = len(lst)
        for j in range(max):
            i = lst[max-j-1]
            ret = ret + "  " + str(i[0]) + ":" + \
                  str(i[1]) + " in function " + \
                  str(i[2])  + "() [" + str(i[3]) + "]\n"
    else:
        ret = "Couldn't import traceback module"
    return ret

# hook into the __import__ chain
__builtin__.__dict__["__import__"] = _uno_import

# vim:set shiftwidth=4 softtabstop=4 expandtab:
