/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: typeconverter.cxx,v $
 * $Revision: 1.25 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include "typeconverter.hxx"
#include "utility.hxx"
#include "simpletypehelper.hxx"
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/FailReason.hpp>
#include <com/sun/star/uno/Type.hxx>
#include <strdecl.hxx>
#include <typelib/typedescription.hxx>


#include <rtl/ustring.hxx>
#include <osl/diagnose.h>

namespace uno = ::com::sun::star::uno;
namespace script = ::com::sun::star::script;
namespace lang = ::com::sun::star::lang;

namespace configmgr
{

//--------------------------------------------------------------------------------------------------
    rtl::OUString toString(const uno::Reference< script::XTypeConverter >& xTypeConverter, const uno::Any& rValue)
        SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException))
    {
        rtl::OUString aRes;
        uno::TypeClass aDestinationClass = rValue.getValueType().getTypeClass();

        switch (aDestinationClass)
        {
        case uno::TypeClass_BOOLEAN:
        case uno::TypeClass_CHAR:
        case uno::TypeClass_BYTE:
        case uno::TypeClass_SHORT:
        case uno::TypeClass_LONG:
        case uno::TypeClass_HYPER:
        case uno::TypeClass_FLOAT:
        case uno::TypeClass_DOUBLE:
            if (!xTypeConverter.is())
            {
                throw script::CannotConvertException(
                                ::rtl::OUString::createFromAscii("Missing Converter Service!"),
                                uno::Reference< uno::XInterface > (),
                                aDestinationClass,
                                script::FailReason::UNKNOWN, 0
                            );
            }
            xTypeConverter->convertToSimpleType(rValue, uno::TypeClass_STRING) >>= aRes;
            break;

        case ::uno::TypeClass_STRING:
            rValue >>= aRes;
            break;

        default:
            throw script::CannotConvertException(
                                ::rtl::OUString::createFromAscii("Unsupported type: ") + rValue.getValueType().getTypeName(),
                                uno::Reference< uno::XInterface > (),
                                aDestinationClass,
                                script::FailReason::TYPE_NOT_SUPPORTED, 0
                            );

        }
        return aRes;
    }

    uno::Any toAny(const uno::Reference< script::XTypeConverter >& xTypeConverter, const ::rtl::OUString& _rValue,const uno::TypeClass& _rTypeClass)
        SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException))
    {
        uno::Any aRes;

        if (uno::TypeClass_STRING == _rTypeClass)
        {
            aRes <<= _rValue;
        }
        else if (!xTypeConverter.is())
        {
            throw script::CannotConvertException(
                            ::rtl::OUString::createFromAscii("Missing Converter Service!"),
                            uno::Reference< uno::XInterface > (),
                            _rTypeClass,
                            script::FailReason::UNKNOWN, 0
                        );
        }
        else try
        {
            aRes = xTypeConverter->convertToSimpleType(uno::makeAny(_rValue), _rTypeClass);
        }
        catch (script::CannotConvertException& )
        {
            // ok, next try with trim()
            ::rtl::OUString const sTrimmed = _rValue.trim();
            if (sTrimmed.getLength() != 0)
            {
                try
                {
                    aRes = xTypeConverter->convertToSimpleType(uno::makeAny(sTrimmed), _rTypeClass);

                    OSL_ENSURE(aRes.hasValue(),"Converted non-empty string to NULL\n");
                }
                catch (script::CannotConvertException&)
                {
                    OSL_ASSERT(!aRes.hasValue());
                }
                catch (uno::Exception&)
                {
                    OSL_ENSURE(false,"Unexpected exception from XTypeConverter::convertToSimpleType()\n");
                    OSL_ASSERT(!aRes.hasValue());
                }

                if (!aRes.hasValue()) throw;
            }
        }
        catch (lang::IllegalArgumentException& iae)
        {
            OSL_ENSURE(sal_False, "Illegal argument for typeconverter. Maybe invalid typeclass ?");
            throw script::CannotConvertException(
                            ::rtl::OUString::createFromAscii("Invalid Converter Argument:") + iae.Message,
                            uno::Reference< uno::XInterface > (),
                            _rTypeClass,
                            script::FailReason::UNKNOWN, 0
                        );
        }
        catch (uno::Exception& e)
        {
            OSL_ENSURE(false,"Unexpected exception from XTypeConverter::convertToSimpleType()\n");
            throw script::CannotConvertException(
                            ::rtl::OUString::createFromAscii("Unexpected TypeConverter Failure:") + e.Message,
                            uno::Reference< uno::XInterface > (),
                            _rTypeClass,
                            script::FailReason::UNKNOWN, 0
                        );
        }
        return aRes;
    }

    ::rtl::OUString toTypeName(const uno::TypeClass& _rTypeClass)
    {
        ::rtl::OUString aRet;
        switch(_rTypeClass)
        {
        case uno::TypeClass_BOOLEAN:  aRet = TYPE_BOOLEAN; break;
        case uno::TypeClass_SHORT:    aRet = TYPE_SHORT; break;
        case uno::TypeClass_LONG:     aRet = TYPE_INT; break;
        case uno::TypeClass_HYPER:    aRet = TYPE_LONG; break;
        case uno::TypeClass_DOUBLE:   aRet = TYPE_DOUBLE; break;
        case uno::TypeClass_STRING:   aRet = TYPE_STRING; break;
        case uno::TypeClass_SEQUENCE: aRet = TYPE_BINARY; break;
        case uno::TypeClass_ANY: aRet = TYPE_ANY; break;
        default:
        {
            ::rtl::OString aStr("Wrong typeclass! ");
            aStr += ::rtl::OString::valueOf((sal_Int32)_rTypeClass);
            OSL_ENSURE(0,aStr.getStr());
        }
        }
        return aRet;
    }

// *************************************************************************
    uno::Type toType(const ::rtl::OUString& _rType)
    {
        uno::Type aRet;

        if     (_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("boolean")))  aRet = SimpleTypeHelper::getBooleanType();

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("short")))       aRet = SimpleTypeHelper::getShortType();
        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("int")))     aRet = SimpleTypeHelper::getIntType();
        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("integer")))  aRet = SimpleTypeHelper::getIntType();
        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("long")))    aRet = SimpleTypeHelper::getLongType();

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("double")))   aRet = SimpleTypeHelper::getDoubleType();

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("string")))   aRet = SimpleTypeHelper::getStringType();
        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("binary")))   aRet = SimpleTypeHelper::getBinaryType();
//  else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("sequence"))) aRet = uno::TypeClass_SEQUENCE;

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("any")))   aRet = SimpleTypeHelper::getAnyType();
        else
        {
            ::rtl::OString aStr("Unknown type! ");
            aStr += rtl::OUStringToOString(_rType,RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(0,aStr.getStr());
        }

        return aRet;
    }
    uno::Type toListType(const ::rtl::OUString& _rsElementType)
    {
        uno::Type aRet;

        if     (_rsElementType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("boolean")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Bool> const*>(0));

        else if(_rsElementType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("short")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int16> const*>(0));

        else if(_rsElementType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("int")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int32> const*>(0));
        else if(_rsElementType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("integer")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int32> const*>(0));

        else if(_rsElementType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("long")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int64> const*>(0));

        else if(_rsElementType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("double")))
            aRet = ::getCppuType(static_cast<uno::Sequence<double> const*>(0));

        else if(_rsElementType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("string")))
            aRet = ::getCppuType(static_cast<uno::Sequence<rtl::OUString> const*>(0));

        else if(_rsElementType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("binary")))
            aRet = ::getCppuType(static_cast<uno::Sequence<uno::Sequence<sal_Int8> > const*>(0));

//  else if(_rsElementType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("sequence"))) aRet = uno::TypeClass_SEQUENCE;
        else
        {
            ::rtl::OString aStr("Unknown type! ");
            aStr += rtl::OUStringToOString(_rsElementType,RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(0,aStr.getStr());
        }

        return aRet;
    }

    uno::Type getSequenceElementType(uno::Type const& rSequenceType)
    {
        OSL_ENSURE(rSequenceType.getTypeClass() == uno::TypeClass_SEQUENCE,
                    "getSequenceElementType() must be called with a  sequence type");

        if (!(rSequenceType.getTypeClass() == uno::TypeClass_SEQUENCE))
            return uno::Type();

        uno::TypeDescription aTD(rSequenceType);
        typelib_IndirectTypeDescription* pSequenceTD =
            reinterpret_cast< typelib_IndirectTypeDescription* >(aTD.get());

        OSL_ASSERT(pSequenceTD);
        OSL_ASSERT(pSequenceTD->pType);

        if ( pSequenceTD && pSequenceTD->pType )
        {
            return uno::Type(pSequenceTD->pType);
        } //if

        return uno::Type();
    }
    uno::Type getBasicType(uno::Type const& rType, bool& bSequence)
    {
        bSequence = rType.getTypeClass() == uno::TypeClass_SEQUENCE &&
                    rType != SimpleTypeHelper::getBinaryType();

        if (!bSequence)
            return rType;

        return getSequenceElementType(rType);
    }


    // template names
// *************************************************************************
    ::rtl::OUString toTemplateName(const uno::Type& _rType)
    {
        bool bList;
        uno::Type aBaseType = getBasicType(_rType,bList);
        return toTemplateName(aBaseType.getTypeClass(), bList);
    }

    ::rtl::OUString toTemplateName(const uno::TypeClass& _rBasicType, bool bList)
    {
        return  toTemplateName(toTypeName(_rBasicType), bList);
    }

// *************************************************************************
    uno::Type parseTemplateName(::rtl::OUString const& sTypeName)
    {
        uno::Type aRet;

        ::rtl::OUString sBasicTypeName;
        bool bList;
        if (parseTemplateName(sTypeName, sBasicTypeName,bList))
            aRet = toType(sBasicTypeName,bList);
        // else leave as void

        return aRet;
    }

// *************************************************************************
    ::rtl::OUString toTemplateName(const ::rtl::OUString& _rBasicTypeName, bool bList)
    {
        ::rtl::OUString sName = TEMPLATE_MODULE_NATIVE_PREFIX + _rBasicTypeName;
        if (bList)
            sName += TEMPLATE_LIST_SUFFIX;


        return sName;
    }

    bool parseTemplateName(::rtl::OUString const& sTypeName, ::rtl::OUString& _rBasicName, bool& bList)
    {
        ::rtl::OUString const sSuffix( TEMPLATE_LIST_SUFFIX );

        sal_Int32 nIndex = sTypeName.lastIndexOf(sSuffix);
        if (nIndex >= 0 && nIndex + sSuffix.getLength() == sTypeName.getLength())
        {
            bList = true;
            _rBasicName = sTypeName.copy(0,nIndex);
        }
        else
        {
            bList = false;
            _rBasicName = sTypeName;
        }
        // erase the default prefix 'cfg:'
        if (_rBasicName.indexOf(TEMPLATE_MODULE_NATIVE_PREFIX) == 0)
            _rBasicName = _rBasicName.copy(TEMPLATE_MODULE_NATIVE_PREFIX.m_nLen);

        return true;

    }
// *************************************************************************

} // namespace configmgr
