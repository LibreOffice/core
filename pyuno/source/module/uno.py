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
import pyuno
import sys
import traceback
import types
import six
import warnings

import socket # since on Windows sal3.dll no longer calls WSAStartup

# All functions and variables starting with a underscore (_) must be
# considered private and can be changed at any time. Don't use them.
_component_context = pyuno.getComponentContext()


def getComponentContext():
    """Returns the UNO component context used to initialize the Python runtime."""

    return _component_context


def getCurrentContext():
    """Returns the current context.

    See http://udk.openoffice.org/common/man/concept/uno_contexts.html#current_context
    for an explanation on the current context concept.
    """

    return pyuno.getCurrentContext()


def setCurrentContext(newContext):
    """Sets newContext as new UNO context.

    The newContext must implement the XCurrentContext interface. The
    implementation should handle the desired properties and delegate
    unknown properties to the old context. Ensure that the old one
    is reset when you leave your stack, see
    http://udk.openoffice.org/common/man/concept/uno_contexts.html#current_context
    """

    return pyuno.setCurrentContext(newContext)


def getConstantByName(constant):
    """Looks up the value of an IDL constant by giving its explicit name."""

    return pyuno.getConstantByName(constant)


def getTypeByName(typeName):
    """Returns a `uno.Type` instance of the type given by typeName.

    If the type does not exist, a `com.sun.star.uno.RuntimeException` is raised.
    """

    return pyuno.getTypeByName(typeName)


def createUnoStruct(typeName, *args, **kwargs):
    """Creates a UNO struct or exception given by typeName.

    Can be called with:

    1) No additional argument.
       In this case, you get a default constructed UNO structure.
       (e.g. `createUnoStruct("com.sun.star.uno.Exception")`)
    2) Exactly one additional argument that is an instance of typeName.
       In this case, a copy constructed instance of typeName is returned
       (e.g. `createUnoStruct("com.sun.star.uno.Exception" , e)`)
    3) As many additional arguments as the number of elements within typeName
       (e.g. `createUnoStruct("com.sun.star.uno.Exception", "foo error" , self)`).
    4) Keyword arguments to give values for each element of the struct by name.
    5) A mix of 3) and 4), such that each struct element is given a value exactly once,
       either by a positional argument or by a keyword argument.

    The additional and/or keyword arguments must match the type of each struct element,
    otherwise an exception is thrown.
    """

    return getClass(typeName)(*args, **kwargs)


def getClass(typeName):
    """Returns the class of a concrete UNO exception, struct, or interface."""

    return pyuno.getClass(typeName)


def isInterface(obj):
    """Returns True, when obj is a class of a UNO interface."""

    return pyuno.isInterface(obj)


def generateUuid():
    """Returns a 16 byte sequence containing a newly generated uuid or guid.

    For more information, see rtl/uuid.h.
    """

    return pyuno.generateUuid()


def systemPathToFileUrl(systemPath):
    """Returns a file URL for the given system path."""

    return pyuno.systemPathToFileUrl(systemPath)


def fileUrlToSystemPath(url):
    """Returns a system path.

    This path is determined by the system that the Python interpreter is running on.
    """

    return pyuno.fileUrlToSystemPath(url)


def absolutize(path, relativeUrl):
    """Returns an absolute file url from the given urls."""

    return pyuno.absolutize(path, relativeUrl)


class Enum:
    """Represents a UNO enum.

    Use an instance of this class to explicitly pass an enum to UNO.

    :param typeName: The name of the enum as a string.
    :param value: The actual value of this enum as a string.
    """

    def __init__(self, typeName, value):
        self.typeName = typeName
        self.value = value
        pyuno.checkEnum(self)

    def __repr__(self):
        return "<Enum instance %s (%r)>" % (self.typeName, self.value)

    def __eq__(self, that):
        if not isinstance(that, Enum):
            return False

        return (self.typeName == that.typeName) and (self.value == that.value)


class Type:
    """Represents a UNO type.

    Use an instance of this class to explicitly pass a type to UNO.

    :param typeName: Name of the UNO type
    :param typeClass: Python Enum of TypeClass, see com/sun/star/uno/TypeClass.idl
    """

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
    """Represents a UNO boolean.

    Use an instance of this class to explicitly pass a boolean to UNO.

    Note: This class is deprecated. Use Python's True and False directly instead.
    """

    def __new__(cls, value):
        message = "The Bool class is deprecated. Use Python's True and False directly instead."
        warnings.warn(message, DeprecationWarning)

        if isinstance(value, six.string_types) and value == "true":
            return True

        if isinstance(value, six.string_types) and value == "false":
            return False

        if value:
            return True

        return False


class Char:
    """Represents a UNO char.

    Use an instance of this class to explicitly pass a char to UNO.

    :param value: A Unicode string with length 1
    """

    def __init__(self, value):
        assert isinstance(value, six.string_types)
        assert len(value) == 1

        self.value = value

    def __repr__(self):
        return "<Char instance %s>" % (self.value,)

    def __eq__(self, that):
        if isinstance(that, six.string_types):
            if len(that) > 1:
                return False
            return self.value == that[0]

        if isinstance(that, Char):
            return self.value == that.value

        return False


class ByteSequence:
    """Represents a UNO ByteSequence value.

    Use an instance of this class to explicitly pass a byte sequence to UNO.

    :param value: A string or bytesequence
    """

    def __init__(self, value):
        if isinstance(value, bytes):
            self.value = value

        # Python 2 compatibility
        elif isinstance(value, six.string_types):
            self.value = value.encode("utf-8")

        elif isinstance(value, ByteSequence):
            self.value = value.value

        else:
            raise TypeError("Expected string or bytesequence, got %s instead." % type(value))

    def __repr__(self):
        return "<ByteSequence instance '%s'>" % (self.value,)

    def __eq__(self, that):
        if isinstance(that, bytes):
            return self.value == that

        # Python 2 compatibility
        if isinstance(that, six.string_types):
            return self.value == that.encode("utf-8")

        if isinstance(that, ByteSequence):
            return self.value == that.value

        return False

    def __len__(self):
        return len(self.value)

    def __getitem__(self, index):
        return self.value[index]

    def __iter__(self):
        return self.value.__iter__()

    def __add__(self, b):
        if isinstance(b, bytes):
            return ByteSequence(self.value + b)

        # Python 2 compatibility
        elif isinstance(b, six.string_types):
            return ByteSequence(self.value + b.encode("utf-8"))

        elif isinstance(b, ByteSequence):
            return ByteSequence(self.value + b.value)

        else:
            raise TypeError("Can't add ByteString and %s." % type(b))

    def __hash__(self):
        return self.value.hash()


class Any:
    """Represents a UNO Any value.

    Use only in connection with uno.invoke() to pass an explicit typed Any.
    """

    def __init__(self, type, value):
        if isinstance(type, Type):
            self.type = type
        else:
            self.type = getTypeByName(type)

        self.value = value


def invoke(object, methodname, argTuple):
    """Use this function to pass exactly typed Anys to the callee (using uno.Any)."""

    return pyuno.invoke(object, methodname, argTuple)


# -----------------------------------------------------------------------------
# Don't use any functions beyond this point; private section, likely to change.
# -----------------------------------------------------------------------------
class Importer(object):
    """Allows importing UNO classes directly in Python code.

    This file must be imported before any UNO classes are imported for this
    class to be hooked into the import system. For example,

    ```
    import uno

    from com.sun.star.beans import PropertyValue
    ```
    """

    def find_module(self, fullname, path=None):
        """Determines whether a module path should be loaded by this class."""

        # For a statement like `from com.sun.star.beans import PropertyValue`,
        # we'll first get a fullname of just 'com', without a path. We'll then
        # get a fullname of e.g. 'com.sun', 'com.sun.star', etc. When we get a
        # match, return `self` as it has a `load_module` method defined.
        if (fullname == 'com' and path is None) or fullname.startswith('com.'):
            return self

        return None

    def load_module(self, name):
        """Dynamically creates modules and imports UNO classes."""

        # Has to be done at runtime, when _uno_struct__init__ is defined
        RuntimeException = pyuno.getClass("com.sun.star.uno.RuntimeException")

        # TODO: Is https://git.io/v2SsQ necessary anymore? See https://git.io/v2SZi

        # Does name refer to a class?
        try:
            uno_class = pyuno.getClass(name)
            sys.modules.setdefault(name, uno_class)
            return uno_class
        except RuntimeException:
            pass

        # Does name refer to an enum?
        # TODO: Removing this doesn't break any unit tests?
        try:
            type_name, value = name.rsplit('.', 1)
            uno_enum = Enum(type_name, value)
            sys.modules.setdefault(name, uno_enum)
            return uno_enum
        except (RuntimeException, ValueError):
            pass

        # Does name refer to a constant?
        # TODO: Removing this doesn't break any unit tests?
        try:
            uno_constant = getConstantByName(name)
            sys.modules.setdefault(name, uno_constant)
            return uno_constant
        except RuntimeException:
            pass

        # Does name refer to a constant group?
        # TODO: Removing this doesn't break any unit tests?
        try:
            module, group = name.rsplit('.', 1)
            uno_constant_group = _impl_getConstantGroupByName(module, group)
            sys.modules.setdefault(name, uno_constant_group)
            return uno_constant_group
        except (RuntimeException, ValueError):
            pass

        # Otherwise, create a new module dynamically and insert it into sys.modules
        mod = types.ModuleType(name)
        mod.__path__ = []
        mod.__loader__ = self
        sys.modules.setdefault(name, mod)

        return mod

# This will hook our class into the import chain
sys.meta_path.append(Importer())


class _ConstantGroup(object):
    """Represents a group of UNOIDL constants."""

    __slots__ = ['_constants']

    def __init__(self, constants):
        self._constants = constants

    def __dir__(self):
        return self._constants.keys()

    def __getattr__(self, name):
        if name in self._constants:
            return self._constants[name]

        raise AttributeError("The constant '%s' could not be found." % name)


def _impl_getConstantGroupByName(module, group):
    """Gets UNOIDL constant group by name."""

    constants = Enum('com.sun.star.uno.TypeClass', 'CONSTANTS')
    one = Enum('com.sun.star.reflection.TypeDescriptionSearchDepth', 'ONE')
    type_desc_mgr = _component_context.getValueByName('/singletons/com.sun.star.reflection.theTypeDescriptionManager')
    type_descs = type_desc_mgr.createTypeDescriptionEnumeration(module, (constants,), one)
    qualified_name = module + '.' + group

    for type_desc in type_descs:
        if type_desc.Name == qualified_name:
            return _ConstantGroup(dict(
                (c.Name.split('.')[-1], c.ConstantValue)
                for c in type_desc.Constants))

    raise ValueError("The constant group '%s' could not be found." % qualified_name)


def _uno_struct__init__(self, *args, **kwargs):
    """Initializes a UNO struct.

    Referenced from the pyuno shared library.

    This function can be called with either an already constructed UNO struct, which it
    will then just reference without copying, or with arguments to create a new UNO struct.
    """

    # Check to see if this function was passed an existing UNO struct
    if len(kwargs) == 0 and len(args) == 1 and getattr(args[0], "__class__", None) == self.__class__:
        self.__dict__['value'] = args[0]
    else:
        struct, used = pyuno._createUnoStructHelper(self.__class__.__pyunostruct__, args, **kwargs)

        for kwarg in kwargs.keys():
            if not used.get(kwarg):
                RuntimeException = pyuno.getClass("com.sun.star.uno.RuntimeException")
                raise RuntimeException("_uno_struct__init__: unused keyword argument '%s'." % kwarg, None)

        self.__dict__["value"] = struct


def _uno_struct__getattr__(self, name):
    """Gets attribute from UNO struct.

    Referenced from the pyuno shared library.
    """

    return getattr(self.__dict__["value"], name)


def _uno_struct__setattr__(self, name, value):
    """Sets attribute on UNO struct.

    Referenced from the pyuno shared library.
    """

    return setattr(self.__dict__["value"], name, value)


def _uno_struct__repr__(self):
    """Converts a UNO struct to a printable string.

    Referenced from the pyuno shared library.
    """

    return repr(self.__dict__["value"])


def _uno_struct__str__(self):
    """Converts a UNO struct to a string."""

    return str(self.__dict__["value"])


def _uno_struct__eq__(self, that):
    """Compares two UNO structs.

    Referenced from the pyuno shared library.
    """

    if hasattr(that, "value"):
        return self.__dict__["value"] == that.__dict__["value"]

    return False


def _uno_extract_printable_stacktrace(trace):
    """Extracts a printable stacktrace.

    Referenced from pyuno shared lib and pythonscript.py.
    """

    return ''.join(traceback.format_tb(trace))


# vim:set shiftwidth=4 softtabstop=4 expandtab:
