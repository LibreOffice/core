# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import gdb
import re
import six

class UnsupportedType(Exception):
    '''Represents exception thrown when an unsupported UNO type(like
        array or union) is used.'''

    def __init__(self, type):
        self.type = type

class UnknownType(Exception):
    '''Represents exception thrown when an unknown UNO type is used.'''

    def __init__(self, type):
        self.type = type

class TypeClass(object):
    '''Represents type class of UNO type.'''

    # type class of void
    VOID = 0
    # type class of char
    CHAR = 1
    # type class of boolean
    BOOLEAN = 2
    # type class of byte
    BYTE = 3
    # type class of short
    SHORT = 4
    # type class of unsigned short
    UNSIGNED_SHORT = 5
    # type class of long
    LONG = 6
    # type class of unsigned long
    UNSIGNED_LONG = 7
    # type class of hyper
    HYPER = 8
    # type class of unsigned hyper
    UNSIGNED_HYPER = 9
    # type class of float
    FLOAT = 10
    # type class of double
    DOUBLE = 11
    # type class of string
    STRING = 12
    # type class of type
    TYPE = 13
    # type class of any
    ANY = 14
    # type class of enum
    ENUM = 15
    # type class of typedef
    TYPEDEF = 16
    # type class of struct
    STRUCT = 17

    # type class of exception
    EXCEPTION = 19
    # type class of sequence
    SEQUENCE = 20

    # type class of interface
    INTERFACE = 22
    # type class of service (not implemented)
    SERVICE = 23
    # type class of module (not implemented)
    MODULE = 24
    # type class of interface method
    INTERFACE_METHOD = 25
    # type class of interface attribute
    INTERFACE_ATTRIBUTE = 26
    # type class of unknown type
    UNKNOWN = 27
    # type class of properties
    PROPERTY = 28
    # type class of constants
    CONSTANT = 29
    # type class of constants groups
    CONSTANTS = 30
    # type class of singletons
    SINGLETON = 31

class TemplateType(object):

    def __init__(self, template, *args):
        self.template = template
        self.args = args

    def __str__(self):
        argtypes = [str(gdb.lookup_type(arg).strip_typedefs()) for arg in self.args]
        return self.template + '<' + ', '.join(argtypes) + '>'

class Type(object):
    '''Describes a UNO type.'''

    def __init__(self, typeclass, tag):
        '''Constructs a new Type.
            @param[in] typeclass value of com::sun::star::uno::TypeClass
            @param[in] tag UNO name of the type
        '''
        self.typeclass = typeclass
        self.tag = tag
        # C++ name of the type
        self.typename = None

    def type(self):
        '''Gets gdb.Type for the type'''
        if self.typename:
            return gdb.lookup_type(str(self.typename))
        return None

    @staticmethod
    def uno2cpp(typename):
        return str(typename).replace('.', '::')[1:-1]

    def strip_typedefs(self):
        copy = self.copy()
        copy.typename = self._strip_typedefs(self.typename)
        return copy

    def _strip_typedefs(self, typename):
        template_args = re.compile('([^<]+)(<.*>)')
        match = template_args.match(typename)
        type = self._lookup_type(match.group(1))
        types = []
        if match.group(2):
            list_delim = re.compile(', *')
            # FIXME: this does not work with nested templates
            for arg in match.group(2).split(list_delim):
                types.append(self._lookup_type(arg))

        typename = str(type)
        if not types.empty():
            typename += '<' + types.join(', ') + '>'

        return typename

    def _lookup_type(self, typename):
        if typename != '':
            type = gdb.lookup_type(typename)
            if type:
                type = type.strip_typedefs()
        return type

def make_uno_type(val):
    '''Creates a UNO type from gdb.Value of type
        com::sun::star::uno::Type, typelib_TypeDescription, or
        typelib_TypeDescriptionReference
    '''

    cssu_type = 'com::sun::star::uno::Type'
    type_desc = '_typelib_TypeDescription'
    type_descs =(
            type_desc,
            '_typelib_CompoundTypeDescription',
            '_typelib_StructTypeDescription',
            '_typelib_IndirectTypeDescription',
            '_typelib_EnumTypeDescription',
            '_typelib_InterfaceMemberTypeDescription',
            '_typelib_InterfaceMethodTypeDescription',
            '_typelib_InterfaceAttributeTypeDescription',
            '_typelib_InterfaceTypeDescription'
    )
    type_desc_ref = '_typelib_TypeDescriptionReference'

    type = val.type.strip_typedefs()

    if type.tag == cssu_type:
        pvalue = val['_pType']
        assert pvalue
        val = pvalue.dereference()
        type = val.type.strip_typedefs()

    while type.tag == type_desc_ref:
        pvalue = val['pType']
        assert pvalue
        val = pvalue.dereference()
        type = val.type.strip_typedefs()

    if type.tag not in type_descs:
        return None

    # determination of the UNO type
    full_val = val
    if type.tag != type_desc:
        while 'aBase' in val:
            val = val['aBase']
    type_class = int(val['eTypeClass'])
    name = val['pTypeName'].dereference()
    uno_type = None
    if type_class == TypeClass.VOID:
        uno_type = VoidType()
    elif type_class == TypeClass.CHAR:
        uno_type = PrimitiveType(type_class, name, 'sal_Char')
    elif type_class == TypeClass.BOOLEAN:
        uno_type = PrimitiveType(type_class, name, 'sal_Bool')
    elif type_class == TypeClass.BYTE:
        uno_type = PrimitiveType(type_class, name, 'sal_Int8')
    elif type_class == TypeClass.SHORT:
        uno_type = PrimitiveType(type_class, name, 'sal_Int16')
    elif type_class == TypeClass.UNSIGNED_SHORT:
        uno_type = PrimitiveType(type_class, name, 'sal_uInt16')
    elif type_class == TypeClass.LONG:
        uno_type = PrimitiveType(type_class, name, 'sal_Int32')
    elif type_class == TypeClass.UNSIGNED_LONG:
        uno_type = PrimitiveType(type_class, name, 'sal_uInt32')
    elif type_class == TypeClass.HYPER:
        uno_type = PrimitiveType(type_class, name, 'sal_Int64')
    elif type_class == TypeClass.UNSIGNED_HYPER:
        uno_type = PrimitiveType(type_class, name, 'sal_uInt64')
    elif type_class == TypeClass.FLOAT:
        uno_type = PrimitiveType(type_class, name, 'float')
    elif type_class == TypeClass.DOUBLE:
        uno_type = PrimitiveType(type_class, name, 'double')
    elif type_class == TypeClass.STRING:
        uno_type = PrimitiveType(type_class, name, 'rtl::OUString')
    elif type_class == TypeClass.TYPE:
        uno_type = PrimitiveType(type_class, name, 'com::sun::star::uno::Type')
    elif type_class == TypeClass.ANY:
        uno_type = PrimitiveType(type_class, name, 'com::sun::star::uno::Any')
    elif type_class == TypeClass.ENUM:
        uno_type = EnumType(val, full_val)
    elif type_class == TypeClass.TYPEDEF:
        pass
    elif type_class == TypeClass.STRUCT:
        uno_type = StructType(val, full_val)
    elif type_class == TypeClass.EXCEPTION:
        uno_type = CompoundType(val, full_val)
    elif type_class == TypeClass.SEQUENCE:
        uno_type = IndirectType(val, full_val)
    elif type_class == TypeClass.INTERFACE:
        uno_type = InterfaceType(val, full_val)
    elif type_class == TypeClass.SERVICE:
        raise UnsupportedType('service')
    elif type_class == TypeClass.MODULE:
        raise UnsupportedType('module')
    elif type_class == TypeClass.INTERFACE_METHOD:
        uno_type = InterfaceMethodType(val, full_val)
    elif type_class == TypeClass.INTERFACE_ATTRIBUTE:
        uno_type = InterfaceAttributeType(val, full_val)
    elif type_class == TypeClass.UNKNOWN:
        raise UnknownType(type)
    elif type_class == TypeClass.PROPERTY:
        pass
    elif type_class == TypeClass.CONSTANT:
        pass
    elif type_class == TypeClass.CONSTANTS:
        pass
    elif type_class == TypeClass.SINGLETON:
        pass
    else:
        raise UnknownType(type)

    assert uno_type
    return uno_type

def uno_cast(type, val):
    '''Casts val or pointer to UNO type represented by type'''
    if val.type.code == gdb.TYPE_CODE_PTR:
        return val.cast(type.type().pointer())
    else:
        return val.cast(type.type())

class VoidType(Type):

    def __init__(self):
        super(VoidType, self).__init__(TypeClass.VOID, "void")
        self.typename = "void"

class PrimitiveType(Type):

    def __init__(self, typeclass, typename_uno, typename_cpp):
        super(PrimitiveType, self).__init__(typeclass, typename_uno)
        self.typename = str(typename_cpp)

class CompoundType(Type):

    def __init__(self, type, full_type):
        super(CompoundType, self).__init__(type['eTypeClass'], type['pTypeName'].dereference())
        self.typename = self.uno2cpp(self.tag)
        self._type = full_type

    class _iterator(six.Iterator):

        def __init__(self, count, types, names):
            self.count = count
            self.members = members
            self.names = names
            self.pos = 0

        def __iter__(self):
            return self

        def __next__(self):
            assert self.pos >= 0 and self.pos <= self.count
            if self.pos == self.count:
                raise StopIteration

            pmember = self.members[self.pos]
            assert pmember
            pname = self.names[self.i]
            assert pname
            self.pos = self.pos + 1
            member = make_uno_type(pmember.dereference())
            assert member
            name = str(pname.dereference())
            return (name, member)

    def attributes(self):
        return _iterator(self._type['nMembers'], self._type['ppTypeRefs'],
                self._type['ppMemberNames'])

class StructType(CompoundType):

    def __init__(self, type, full_type):
        full_type = full_type.cast(gdb.lookup_type('_typelib_StructTypeDescription'))
        super(StructType, self).__init__(type, full_type['aBase'])

class IndirectType(Type):

    def __init__(self, type, full_type):
        super(IndirectType, self).__init__(type['eTypeClass'], type['pTypeName'].dereference())
        full_type = full_type.cast(gdb.lookup_type('_typelib_IndirectTypeDescription'))
        pelem = full_type['pType']
        assert pelem
        self.element = make_uno_type(pelem.dereference())
        assert self.element
        self.typename = TemplateType('com::sun::star::uno::Sequence', self.element.typename)

class EnumType(Type):

    def __init__(self, type, full_type):
        super(EnumType, self).__init__(TypeClass.ENUM, type['pTypeName'].dereference())
        self.typename = self.uno2cpp(self.tag)
        self._type = full_type.cast(gdb.lookup_type('_typelib_EnumTypeDescription'))

    class _iterator(six.Iterator):

        def __init__(self, count, values, names):
            self.count = count
            self.values = values
            self.names = names
            self.pos = 0

        def __iter__(self):
            return self

        def __next__(self):
            assert self.pos >= 0 and self.pos <= self.count
            if self.pos == self.count:
                raise StopIteration

            pvalue = self.values[self.pos]
            assert pvalue
            pname = self.names[self.pos]
            assert pname
            self.pos = self.pos + 1
            val = int(pvalue.dereference())
            name = str(pname.dereference())
            return (name, val)

    def values(self):
        return _iterator(self._type['nEnumValues'],
                self._type['ppEnumNames'], self._type['pEnumValues'])

    def default_value(self):
        return self._type['nDefaultEnumValue']

class InterfaceMemberType(Type):

    def __init__(self, type, full_type):
        super(InterfaceMemberType, self).__init__(type['eTypeClass'], type['pTypeName'].dereference())
        (interface, delim, member) = self.tag.partition('::')
        self.typename = self.uno2cpp(interface) + '::*' + member
        full_type = full_type.cast(gdb.lookup_type('_typelib_InterfaceMemberTypeDescription'))
        self.position = full_type['nPosition']
        pname = full_type['pMemberName']
        assert pname
        self.name = pname.dereference()

class InterfaceMethodType(InterfaceMemberType):

    def __init__(self, type, full_type):
        full_type = full_type.cast(gdb.lookup_type('_typelib_InterfaceMethodTypeDescription'))
        super(InterfaceMethodType, self).__init__(type, full_type['aBase'])
        pret = full_type['pReturnTypeRef']
        assert pret
        self.return_type = make_uno_type(pret.dereference())
        assert self.return_type
        self.oneway = full_type['bOneWay']
        self._type = full_type

    class _iterator(six.Iterator):

        def __init__(self, count, values):
            self.count = count
            self.values = values
            self.pos = 0
            assert values

        def __iter__(self):
            return self

        def __next__(self):
            assert self.pos >= 0 and self.pos <= self.count
            if self.pos == self.count:
                raise StopIteration

            val = self.values[self.pos]
            self.pos = self.pos + 1
            return val

    class parameter(tuple):

        def __init__(self, type):
            self.__init_tuple(type)
            self.input = type['bIn']
            self.output = type['bOut']

        def _init_tuple(self, type):
            pname = self['pName']
            assert pname
            ptype = self['pTypeRef']
            assert ptype
            name = str(pname.dereference())
            type = make_uno_type(ptype.dereference())
            assert type
            super(parameter, self).__init__(name, type)

    def parameters(self):
        for param in _iterator(self._type['nParams'], self._type['pParams']):
            yield parameter(param)

    def exceptions(self):
        def make_exception(self, pex):
            assert pex
            ex = make_uno_type(pex.dereference())
            assert ex
            return ex

        for ex in _iterator(
                self._type['nExceptions'], self._type['ppExceptions']):
            yield make_exception(ex)

class InterfaceAttributeType(InterfaceMemberType):

    def __init__(self, type, full_type):
        full_type = full_type.cast(gdb.lookup_type('_typelib_InterfaceAttributeTypeDescription'))
        super(InterfaceAttributeType, self).__init__(type, full_type['aBase'])
        self.readonly = full_type['bReadOnly']
        ptype = full_type['pAttributeTypeRef']
        assert ptype
        self.type = make_uno_type(ptype.dereference())
        assert self.type

class MembersNotInitialized(Exception):
    '''Represents exception raised when interface type' members haven't
        been initialized(i.e. just level 1 initialization has been
        performed)'''
    pass

class InterfaceType(Type):

    def __init__(self, type, full_type):
        super(InterfaceType, self).__init__(TypeClass.INTERFACE, type['pTypeName'].dereference())
        assert int(type['eTypeClass']) == TypeClass.INTERFACE
        self.typename = self.uno2cpp(self.tag)
        full_type = full_type.cast(gdb.lookup_type('_typelib_InterfaceTypeDescription'))
        self.uik = full_type['aUik']
        self._type = full_type

    class _iterator(six.Iterator):

        def __init__(self, count, values):
            assert values
            self.count = count
            self.values = values
            self.pos = 0

        def __iter__(self):
            return self

        def __next__(self):
            assert self.pos >= 0 and self.pos <= self.count
            pvalue = self.values[self.pos]
            assert pvalue
            self.pos = self.pos + 1
            uno = make_uno_type(pvalue.dereference())
            assert uno
            return uno

    def members(self):
        return __members(self._type['nMembers'], self._type['ppMembers'])

    def all_members(self):
        return __members(self._type['nAllMembers'], self._type['ppAllMembers'])

    def __members(count, values):
        if values == 0:
            raise MembersNotInitialized
        return _iterator(count, values)

    def bases(self):
        return _iterator(self._type['nBaseTypes'], self._type['ppBaseTypes'])

# vim:set shiftwidth=4 softtabstop=4 expandtab:
