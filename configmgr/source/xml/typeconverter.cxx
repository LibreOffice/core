/*************************************************************************
 *
 *  $RCSfile: typeconverter.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: jb $ $Date: 2001-04-05 09:19:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "typeconverter.hxx"

#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_FAILREASON_HPP_
#include <com/sun/star/script/FailReason.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif
#ifndef _CONFIGMGR_STRDECL_HXX_
#include <strdecl.hxx>
#endif

#ifndef _TYPELIB_TYPEDESCRIPTION_HXX_
#include <typelib/typedescription.hxx>
#endif


#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//#include <stdio.h>

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace uno = ::com::sun::star::uno;
namespace script = ::com::sun::star::script;
namespace lang = ::com::sun::star::lang;

namespace configmgr
{

//--------------------------------------------------------------------------------------------------
    rtl::OUString toString(const uno::Reference< script::XTypeConverter >& xTypeConverter, const uno::Any& rValue) throw( script::CannotConvertException )
    {
        rtl::OUString aRes;
        ::uno::TypeClass aDestinationClass = rValue.getValueType().getTypeClass();

        switch (aDestinationClass)
        {
        case ::uno::TypeClass_BOOLEAN:
        case ::uno::TypeClass_CHAR:
        case ::uno::TypeClass_BYTE:
        case ::uno::TypeClass_SHORT:
        case ::uno::TypeClass_LONG:
        case ::uno::TypeClass_HYPER:
        case ::uno::TypeClass_FLOAT:
        case ::uno::TypeClass_DOUBLE:
            if (!xTypeConverter.is())
            {
                throw( script::CannotConvertException( ::rtl::OUString::createFromAscii("stardiv.script.Converter!"), ::uno::Reference< ::uno::XInterface > (),
                                                           aDestinationClass, script::FailReason::UNKNOWN, 0 ) );
            }
            xTypeConverter->convertToSimpleType(rValue, ::uno::TypeClass_STRING) >>= aRes;
            break;
        case ::uno::TypeClass_STRING:
            rValue >>= aRes;
            break;
        default:
            throw( script::CannotConvertException( ::rtl::OUString::createFromAscii("TYPE is not supported!"), uno::Reference< ::uno::XInterface > (),
                                                       aDestinationClass, script::FailReason::TYPE_NOT_SUPPORTED, 0 ) );

        }
        return aRes;
    }

    uno::Any toAny(const uno::Reference< script::XTypeConverter >& xTypeConverter, const ::rtl::OUString& _rValue,const uno::TypeClass& _rTypeClass) throw( script::CannotConvertException )
    {
        uno::Any aRes;
        try
        {
            aRes = xTypeConverter->convertToSimpleType(uno::makeAny(_rValue), _rTypeClass);
        }
        catch (script::CannotConvertException&)
        {
            // ok, next try with trim()
            if (_rValue.getLength() != 0)
            {
                try
                {
                    aRes = xTypeConverter->convertToSimpleType(uno::makeAny(_rValue.trim()), _rTypeClass);
                }
                catch (script::CannotConvertException&)
                {
                    OSL_ENSURE(sal_False, "toAny : could not convert !");
                }
            }
        }
        catch (lang::IllegalArgumentException&)
        {
            OSL_ENSURE(sal_False, "toAny : could not convert !");
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

    uno::TypeClass toTypeClass(const ::rtl::OUString& _rType)
    {
        uno::TypeClass aRet = uno::TypeClass_VOID;

        if     (_rType.equalsIgnoreCase(TYPE_BOOLEAN))  aRet = uno::TypeClass_BOOLEAN;
        else if(_rType.equalsIgnoreCase(TYPE_SHORT))       aRet = uno::TypeClass_SHORT;
        else if(_rType.equalsIgnoreCase(TYPE_INT))     aRet = uno::TypeClass_LONG;
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("integer")))  aRet = uno::TypeClass_LONG;
        else if(_rType.equalsIgnoreCase(TYPE_LONG))    aRet = uno::TypeClass_HYPER;
        else if(_rType.equalsIgnoreCase(TYPE_DOUBLE))   aRet = uno::TypeClass_DOUBLE;
        else if(_rType.equalsIgnoreCase(TYPE_STRING))   aRet = uno::TypeClass_STRING;
        else if(_rType.equalsIgnoreCase(TYPE_BINARY))   aRet = uno::TypeClass_SEQUENCE;
        else if(_rType.equalsIgnoreCase(TYPE_ANY))   aRet = uno::TypeClass_ANY;
        else
        {
            ::rtl::OString aStr("Wrong typeclass! ");
            aStr += rtl::OUStringToOString(_rType,RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(0,aStr.getStr());
        }

        return aRet;
    }

// *************************************************************************
    namespace
    {

        inline uno::Type getBooleanType() { return ::getBooleanCppuType(); }

        inline uno::Type getShortType()     { return ::getCppuType(static_cast<sal_Int16 const*>(0)); }
        inline uno::Type getIntType()       { return ::getCppuType(static_cast<sal_Int32 const*>(0)); }
        inline uno::Type getLongType()      { return ::getCppuType(static_cast<sal_Int64 const*>(0)); }

        inline uno::Type getDoubleType()    { return ::getCppuType(static_cast<double const*>(0)); }

        inline uno::Type getStringType()    { return ::getCppuType(static_cast<rtl::OUString const*>(0)); }

// *************************************************************************
/*
  ::rtl::OUString findXMLTypeName(const uno::Type& _rType)
  {
  ::rtl::OUString aRet;
  switch(_rType.getTypeClass())
  {
  case uno::TypeClass_BOOLEAN:
  OSL_ASSERT( _rType == getBooleanType() );
  aRet = ::rtl::OUString::createFromAscii("boolean");
  break;

  case uno::TypeClass_SHORT:
  OSL_ASSERT( _rType == getShortType() );
  aRet = ::rtl::OUString::createFromAscii("short");
  break;

  case uno::TypeClass_LONG:
  OSL_ASSERT( _rType == getIntType() );
  aRet = ::rtl::OUString::createFromAscii("int");
  break;

  case uno::TypeClass_HYPER:
  OSL_ASSERT( _rType == getLongType() );
  Ret = ::rtl::OUString::createFromAscii("long");
  break;

  case uno::TypeClass_DOUBLE:
  OSL_ASSERT( _rType == getDoubleType() );
  aRet = ::rtl::OUString::createFromAscii("double");
  break;

  case uno::TypeClass_STRING:
  OSL_ASSERT( _rType == getStringType() );
  aRet = ::rtl::OUString::createFromAscii("string");
  break;

  case uno::TypeClass_SEQUENCE:
  if ( _rType == getStringType() );
  aRet = ::rtl::OUString::createFromAscii("sequence");
  break;

  default:
  {
  ::rtl::OString aStr("Wrong typeclass! ");
  aStr += ::rtl::OString::valueOf((sal_Int32)_rTypeClass);
  OSL_ENSURE(0,aStr.getStr());
  }
  }
  return aRet;
  }
 */
    } // unamed namespace
// *************************************************************************

    uno::Type toType(const ::rtl::OUString& _rType)
    {
        uno::Type aRet;

        if     (_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("boolean")))  aRet = getBooleanType();

        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("short")))    aRet = getShortType();
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("int")))      aRet = getIntType();
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("integer")))  aRet = getIntType();
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("long")))     aRet = getLongType();

        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("double")))   aRet = getDoubleType();

        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("string")))   aRet = getStringType();
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("binary")))   aRet = getBinaryType();
//  else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("sequence"))) aRet = uno::TypeClass_SEQUENCE;

        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("any")))   aRet = getAnyType();
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

        if     (_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("boolean")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Bool> const*>(0));

        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("short")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int16> const*>(0));

        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("int")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int32> const*>(0));
        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("integer")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int32> const*>(0));

        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("long")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int64> const*>(0));

        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("double")))
            aRet = ::getCppuType(static_cast<uno::Sequence<double> const*>(0));

        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("string")))
            aRet = ::getCppuType(static_cast<uno::Sequence<rtl::OUString> const*>(0));

        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("binary")))
            aRet = ::getCppuType(static_cast<uno::Sequence<uno::Sequence<sal_Int8> > const*>(0));

//  else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("sequence"))) aRet = uno::TypeClass_SEQUENCE;
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
            // SUPD expects a decimal constant, hence,
            // build version comparison is made using
            // a decimal number

#if ( SUPD >= 601 )
            return uno::Type(pSequenceTD->pType);
#else
            OSL_ASSERT(pSequenceTD->pType);
            return uno::Type(pSequenceTD->pType->pWeakRef);
#endif
        } //if

        return uno::Type();
    }
    uno::Type getBasicType(uno::Type const& rType, bool& bSequence)
    {
        bSequence = rType.getTypeClass() == uno::TypeClass_SEQUENCE &&
                    rType != getBinaryType();

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

    bool parseTemplateName(::rtl::OUString const& sTypeName, uno::TypeClass& _rType, bool& bList)
    {
        ::rtl::OUString sBasicTypeName;

        if (parseTemplateName(sTypeName, sBasicTypeName,bList))
            _rType = toTypeClass(sBasicTypeName);

        else
            _rType = uno::TypeClass_VOID;

        return _rType != uno::TypeClass_VOID;
    }

// *************************************************************************
    ::rtl::OUString toTemplateName(const ::rtl::OUString& _rBasicTypeName, bool bList)
    {
        ::rtl::OUString sName = _rBasicTypeName;
        if (bList)
            sName += TEMPLATE_LIST_SUFFIX;
        return sName;
    }

    bool parseTemplateName(::rtl::OUString const& sTypeName, ::rtl::OUString& _rBasicName, bool& bList)
    {
        ::rtl::OUString const sSuffix = TEMPLATE_LIST_SUFFIX;

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
        return true;

    }
// *************************************************************************

} // namespace configmgr
