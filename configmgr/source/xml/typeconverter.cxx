/*************************************************************************
 *
 *  $RCSfile: typeconverter.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: lla $ $Date: 2000-10-16 11:33:02 $
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

namespace staruno = ::com::sun::star::uno;
namespace starscript = ::com::sun::star::script;
namespace starlang = ::com::sun::star::lang;

namespace configmgr
{

//--------------------------------------------------------------------------------------------------
    rtl::OUString toString(const staruno::Reference< starscript::XTypeConverter >& xTypeConverter, const staruno::Any& rValue) throw( starscript::CannotConvertException )
    {
        rtl::OUString aRes;
        ::staruno::TypeClass aDestinationClass = rValue.getValueType().getTypeClass();

        switch (aDestinationClass)
        {
        case ::staruno::TypeClass_BOOLEAN:
        case ::staruno::TypeClass_CHAR:
        case ::staruno::TypeClass_BYTE:
        case ::staruno::TypeClass_SHORT:
        case ::staruno::TypeClass_LONG:
        case ::staruno::TypeClass_HYPER:
        case ::staruno::TypeClass_FLOAT:
        case ::staruno::TypeClass_DOUBLE:
            if (!xTypeConverter.is())
            {
                throw( starscript::CannotConvertException( ::rtl::OUString::createFromAscii("stardiv.script.Converter!"), ::staruno::Reference< ::staruno::XInterface > (),
                                                           aDestinationClass, starscript::FailReason::UNKNOWN, 0 ) );
            }
            xTypeConverter->convertToSimpleType(rValue, ::staruno::TypeClass_STRING) >>= aRes;
            break;
        case ::staruno::TypeClass_STRING:
            rValue >>= aRes;
            break;
        default:
            throw( starscript::CannotConvertException( ::rtl::OUString::createFromAscii("TYPE is not supported!"), staruno::Reference< ::staruno::XInterface > (),
                                                       aDestinationClass, starscript::FailReason::TYPE_NOT_SUPPORTED, 0 ) );

        }
        return aRes;
    }

    staruno::Any toAny(const staruno::Reference< starscript::XTypeConverter >& xTypeConverter, const ::rtl::OUString& _rValue,const staruno::TypeClass& _rTypeClass) throw( starscript::CannotConvertException )
    {
        staruno::Any aRes;
        try
        {
            aRes = xTypeConverter->convertToSimpleType(staruno::makeAny(_rValue), _rTypeClass);
        }
        catch (starscript::CannotConvertException&)
        {
            if (_rValue.getLength() != 0)
                OSL_ENSHURE(sal_False, "toAny : could not convert !");
        }
        catch (starlang::IllegalArgumentException&)
        {
            OSL_ENSHURE(sal_False, "toAny : could not convert !");
        }
        return aRes;
    }

    ::rtl::OUString toTypeName(const staruno::TypeClass& _rTypeClass)
    {
        ::rtl::OUString aRet;
        switch(_rTypeClass)
        {
        case staruno::TypeClass_BOOLEAN:  aRet = ::rtl::OUString::createFromAscii("boolean"); break;
        case staruno::TypeClass_SHORT:    aRet = ::rtl::OUString::createFromAscii("short"); break;
        case staruno::TypeClass_LONG:     aRet = ::rtl::OUString::createFromAscii("integer"); break;
        case staruno::TypeClass_HYPER:    aRet = ::rtl::OUString::createFromAscii("long"); break;
        case staruno::TypeClass_DOUBLE:   aRet = ::rtl::OUString::createFromAscii("double"); break;
        case staruno::TypeClass_STRING:   aRet = ::rtl::OUString::createFromAscii("string"); break;
        case staruno::TypeClass_SEQUENCE: aRet = ::rtl::OUString::createFromAscii("binary"); break;

        case staruno::TypeClass_ANY: aRet = ::rtl::OUString::createFromAscii("any"); break;
        default:
        {
            ::rtl::OString aStr("Wrong typeclass! ");
            aStr += ::rtl::OString::valueOf((sal_Int32)_rTypeClass);
            OSL_ENSHURE(0,aStr.getStr());
        }
        }
        return aRet;
    }

    staruno::TypeClass toTypeClass(const ::rtl::OUString& _rType)
    {
        staruno::TypeClass aRet = staruno::TypeClass_VOID;

        if     (_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("boolean")))  aRet = staruno::TypeClass_BOOLEAN;
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("short")))    aRet = staruno::TypeClass_SHORT;
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("int")))      aRet = staruno::TypeClass_LONG;
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("integer")))  aRet = staruno::TypeClass_LONG;
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("long")))     aRet = staruno::TypeClass_HYPER;
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("double")))   aRet = staruno::TypeClass_DOUBLE;
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("string")))   aRet = staruno::TypeClass_STRING;
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("binary")))   aRet = staruno::TypeClass_SEQUENCE;

        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("any")))   aRet = staruno::TypeClass_ANY;
        else
        {
            ::rtl::OString aStr("Wrong typeclass! ");
            aStr += rtl::OUStringToOString(_rType,RTL_TEXTENCODING_ASCII_US);
            OSL_ENSHURE(0,aStr.getStr());
        }

        return aRet;
    }

// *************************************************************************

    namespace
    {

        inline staruno::Type getBooleanType() { return ::getBooleanCppuType(); }

        inline staruno::Type getShortType()     { return ::getCppuType(static_cast<sal_Int16 const*>(0)); }
        inline staruno::Type getIntType()       { return ::getCppuType(static_cast<sal_Int32 const*>(0)); }
        inline staruno::Type getLongType()      { return ::getCppuType(static_cast<sal_Int64 const*>(0)); }

        inline staruno::Type getDoubleType()    { return ::getCppuType(static_cast<double const*>(0)); }

        inline staruno::Type getStringType()    { return ::getCppuType(static_cast<rtl::OUString const*>(0)); }

// *************************************************************************
/*
  ::rtl::OUString findXMLTypeName(const staruno::Type& _rType)
  {
  ::rtl::OUString aRet;
  switch(_rType.getTypeClass())
  {
  case staruno::TypeClass_BOOLEAN:
  OSL_ASSERT( _rType == getBooleanType() );
  aRet = ::rtl::OUString::createFromAscii("boolean");
  break;

  case staruno::TypeClass_SHORT:
  OSL_ASSERT( _rType == getShortType() );
  aRet = ::rtl::OUString::createFromAscii("short");
  break;

  case staruno::TypeClass_LONG:
  OSL_ASSERT( _rType == getIntType() );
  aRet = ::rtl::OUString::createFromAscii("int");
  break;

  case staruno::TypeClass_HYPER:
  OSL_ASSERT( _rType == getLongType() );
  Ret = ::rtl::OUString::createFromAscii("long");
  break;

  case staruno::TypeClass_DOUBLE:
  OSL_ASSERT( _rType == getDoubleType() );
  aRet = ::rtl::OUString::createFromAscii("double");
  break;

  case staruno::TypeClass_STRING:
  OSL_ASSERT( _rType == getStringType() );
  aRet = ::rtl::OUString::createFromAscii("string");
  break;

  case staruno::TypeClass_SEQUENCE:
  if ( _rType == getStringType() );
  aRet = ::rtl::OUString::createFromAscii("sequence");
  break;

  default:
  {
  ::rtl::OString aStr("Wrong typeclass! ");
  aStr += ::rtl::OString::valueOf((sal_Int32)_rTypeClass);
  OSL_ENSHURE(0,aStr.getStr());
  }
  }
  return aRet;
  }
 */
    } // unamed namespace
// *************************************************************************

    staruno::Type toType(const ::rtl::OUString& _rType)
    {
        staruno::Type aRet;

        if     (_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("boolean")))  aRet = getBooleanType();

        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("short")))    aRet = getShortType();
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("int")))      aRet = getIntType();
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("integer")))  aRet = getIntType();
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("long")))     aRet = getLongType();

        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("double")))   aRet = getDoubleType();

        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("string")))   aRet = getStringType();
        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("binary")))   aRet = getBinaryType();
//  else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("sequence"))) aRet = staruno::TypeClass_SEQUENCE;

        else if(_rType.equalsIgnoreCase(::rtl::OUString::createFromAscii("any")))   aRet = getAnyType();
        else
        {
            ::rtl::OString aStr("Unknown type! ");
            aStr += rtl::OUStringToOString(_rType,RTL_TEXTENCODING_ASCII_US);
            OSL_ENSHURE(0,aStr.getStr());
        }

        return aRet;
    }
    staruno::Type toListType(const ::rtl::OUString& _rsElementType)
    {
        staruno::Type aRet;

        if     (_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("boolean")))
            aRet = ::getCppuType(static_cast<staruno::Sequence<sal_Bool> const*>(0));

        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("short")))
            aRet = ::getCppuType(static_cast<staruno::Sequence<sal_Int16> const*>(0));

        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("int")))
            aRet = ::getCppuType(static_cast<staruno::Sequence<sal_Int32> const*>(0));
        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("integer")))
            aRet = ::getCppuType(static_cast<staruno::Sequence<sal_Int32> const*>(0));

        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("long")))
            aRet = ::getCppuType(static_cast<staruno::Sequence<sal_Int64> const*>(0));

        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("double")))
            aRet = ::getCppuType(static_cast<staruno::Sequence<double> const*>(0));

        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("string")))
            aRet = ::getCppuType(static_cast<staruno::Sequence<rtl::OUString> const*>(0));

        else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("binary")))
            aRet = ::getCppuType(static_cast<staruno::Sequence<staruno::Sequence<sal_Int8> > const*>(0));

//  else if(_rsElementType.equalsIgnoreCase(::rtl::OUString::createFromAscii("sequence"))) aRet = staruno::TypeClass_SEQUENCE;
        else
        {
            ::rtl::OString aStr("Unknown type! ");
            aStr += rtl::OUStringToOString(_rsElementType,RTL_TEXTENCODING_ASCII_US);
            OSL_ENSHURE(0,aStr.getStr());
        }

        return aRet;
    }

    staruno::Type getSequenceElementType(staruno::Type const& rSequenceType)
    {
        OSL_ENSHURE(rSequenceType.getTypeClass() == staruno::TypeClass_SEQUENCE,
                    "getSequenceElementType() must be called with a  sequence type");

        if (!(rSequenceType.getTypeClass() == staruno::TypeClass_SEQUENCE))
            return staruno::Type();

        staruno::TypeDescription aTD(rSequenceType);
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
            return staruno::Type(pSequenceTD->pType);
#else
            OSL_ASSERT(pSequenceTD->pType);
            return staruno::Type(pSequenceTD->pType->pWeakRef);
#endif
        } //if

        return staruno::Type();
    }
    staruno::Type getBasicType(staruno::Type const& rType, bool& bSequence)
    {
        bSequence = rType.getTypeClass() == staruno::TypeClass_SEQUENCE &&
                    rType != getBinaryType();

        if (!bSequence)
            return rType;

        return getSequenceElementType(rType);
    }


} // namespace configmgr
