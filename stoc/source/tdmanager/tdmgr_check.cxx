/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "tdmgr_common.hxx"
#include "rtl/ustrbuf.hxx"
#include "typelib/typedescription.h"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/reflection/XConstantsTypeDescription.hpp"
#include "com/sun/star/reflection/XIndirectTypeDescription.hpp"
#include "com/sun/star/reflection/XEnumTypeDescription.hpp"
#include "com/sun/star/reflection/XStructTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceTypeDescription2.hpp"
#include "com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceAttributeTypeDescription2.hpp"
#include "com/sun/star/reflection/XServiceTypeDescription2.hpp"
#include "com/sun/star/reflection/XSingletonTypeDescription2.hpp"


using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::stoc_tdmgr;

namespace {

OUString getTypeClassName( TypeClass tc )
{
    typelib_EnumTypeDescription * typeDescr = 0;
    OUString name = "com.sun.star.uno.TypeClass";
    typelib_typedescription_getByName(
        reinterpret_cast<typelib_TypeDescription **>(&typeDescr), name.pData );
    OSL_ASSERT( typeDescr != 0 );
    if (typeDescr == 0)
        return "Cannot get type description of " + name;
    typelib_typedescription_complete(
        reinterpret_cast<typelib_TypeDescription **>(&typeDescr) );

    sal_Int32 const * pValues = typeDescr->pEnumValues;
    sal_Int32 nPos = typeDescr->nEnumValues;
    while (nPos--)
    {
        if (pValues[ nPos ] == (sal_Int32) tc)
            break;
    }
    if (nPos >= 0)
        name = typeDescr->ppEnumNames[ nPos ];
    else
        name = "unknown TypeClass value: " +
            OUString::valueOf( (sal_Int32) tc );

    typelib_typedescription_release(
        reinterpret_cast<typelib_TypeDescription *>(typeDescr) );
    return name;
}

OUString getPropertyFlagsAsString( sal_Int16 attributes )
{
    OUStringBuffer buf;
    if ((attributes & beans::PropertyAttribute::MAYBEVOID) != 0)
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("MAYBEVOID, ") );
    if ((attributes & beans::PropertyAttribute::BOUND) != 0)
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("BOUND, ") );
    if ((attributes & beans::PropertyAttribute::CONSTRAINED) != 0)
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("CONSTRAINED, ") );
    if ((attributes & beans::PropertyAttribute::TRANSIENT) != 0)
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("TRANSIENT, ") );
    if ((attributes & beans::PropertyAttribute::READONLY) != 0)
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("READONLY, ") );
    if ((attributes & beans::PropertyAttribute::MAYBEAMBIGUOUS) != 0)
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("MAYBEAMBIGUOUS, ") );
    if ((attributes & beans::PropertyAttribute::MAYBEDEFAULT) != 0)
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("MAYBEDEFAULT, ") );
    if ((attributes & beans::PropertyAttribute::REMOVABLE) != 0)
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("REMOVABLE, ") );
    if ((attributes & beans::PropertyAttribute::OPTIONAL) != 0)
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("OPTIONAL") );
    else if (buf.getLength() > 0)
        buf.setLength( buf.getLength() - 2 ); // truncate ", "
    return buf.makeStringAndClear();
}

void typeError( OUString const & msg, OUString const & context )
{
    OUStringBuffer buf;
    if (!context.isEmpty()) {
        buf.append( static_cast<sal_Unicode>('[') );
        buf.append( context );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] ") );
    }
    buf.append( msg );
    throw IncompatibleTypeException( buf.makeStringAndClear() );
}

template<typename T>
void checkSeq( Sequence< Reference<T> > const & newTypes,
            Sequence< Reference<T> > const & existingTypes,
            OUString const & context,
            bool optionalMode = false )
{
    sal_Int32 len = newTypes.getLength();
    if (len != existingTypes.getLength())
    {
        if (!optionalMode || len < newTypes.getLength())
            typeError( "Different number of types!", context );
        len = existingTypes.getLength();
    }

    Reference<T> const * pNewTypes = newTypes.getConstArray();
    Reference<T> const * pExistingTypes = existingTypes.getConstArray();
    for ( sal_Int32 pos = 0; pos < len; ++pos )
    {
        OUStringBuffer buf;
        buf.append( context );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", position ") );
        buf.append( pos );
        check( pNewTypes[pos].get(), pExistingTypes[pos].get(),
               buf.makeStringAndClear() );
    }
}

void checkEnum(
    Reference<reflection::XEnumTypeDescription> const & xNewTD,
    Reference<reflection::XEnumTypeDescription> const & xExistingTD )
{
    if (xNewTD->getEnumNames() != xExistingTD->getEnumNames())
        typeError( "ENUM names don't match!", xNewTD->getName() );
    if (xNewTD->getEnumValues() != xExistingTD->getEnumValues())
        typeError( "ENUM values don't match!", xNewTD->getName() );
}

void checkStruct(
    Reference<reflection::XCompoundTypeDescription> const & xNewTD,
    Reference<reflection::XCompoundTypeDescription> const & xExistingTD )
{
    check( xNewTD->getBaseType(), xExistingTD->getBaseType(),
           xNewTD->getName() + ", base type" );
    checkSeq( xNewTD->getMemberTypes(), xExistingTD->getMemberTypes(),
              xNewTD->getName() + ", member types" );

    if (xNewTD->getMemberNames() != xExistingTD->getMemberNames())
        typeError( "Different member names!", xNewTD->getName() );

    if (xNewTD->getTypeClass() == TypeClass_STRUCT)
    {
        Reference<reflection::XStructTypeDescription> xNewStructTD(
            xNewTD, UNO_QUERY );
        Reference<reflection::XStructTypeDescription> xExistingStructTD(
            xExistingTD, UNO_QUERY );
        if (xNewStructTD.is() && xExistingStructTD.is())
        {
            if (xNewStructTD->getTypeParameters() !=
                xExistingStructTD->getTypeParameters())
                typeError( "Different type parameters of instantiated "
                           "polymorphic STRUCT!", xNewTD->getName() );
            checkSeq( xNewStructTD->getTypeArguments(),
                      xExistingStructTD->getTypeArguments(),
                      xNewTD->getName() + ", argument types" );
        }
        else if (xNewStructTD.is() || xExistingStructTD.is())
            typeError( "Mixing polymorphic STRUCT types "
                       "with non-polymorphic!", xNewTD->getName() );
    }
}

void checkInterface(
    Reference<reflection::XInterfaceTypeDescription2> const & xNewTD,
    Reference<reflection::XInterfaceTypeDescription2> const & xExistingTD )
{
    checkSeq( xNewTD->getBaseTypes(), xExistingTD->getBaseTypes(),
              xNewTD->getName() + ", base types" );
    checkSeq(xNewTD->getOptionalBaseTypes(),xExistingTD->getOptionalBaseTypes(),
             xNewTD->getName() + ", optional base types" );
    checkSeq( xNewTD->getMembers(), xExistingTD->getMembers(),
              xNewTD->getName() + ", members" );
}

void checkRestParam( Reference<reflection::XParameter> const & xNewParam,
                     Reference<reflection::XParameter> const & xExistingParam,
                     OUString const & context )
{
    if (xNewParam->isRestParameter() != xExistingParam->isRestParameter())
        typeError( "Different ... parameters specified!", context );
}

void checkRestParam(
    SAL_UNUSED_PARAMETER Reference<reflection::XMethodParameter> const &,
    SAL_UNUSED_PARAMETER Reference<reflection::XMethodParameter> const &,
    SAL_UNUSED_PARAMETER OUString const & )
{}

template<typename T>
void checkParameters( Sequence< Reference<T> > const & newParams,
                      Sequence< Reference<T> > const & existingParams,
                      OUString const & context_ )
{
    sal_Int32 len = newParams.getLength();
    if (len != existingParams.getLength())
        typeError( "Different number of parameters!", context_ );
    Reference<T> const * pNewParams = newParams.getConstArray();
    Reference<T> const * pExistingParams = existingParams.getConstArray();
    for ( sal_Int32 pos = 0; pos < len; ++pos )
    {
        Reference<T> const & xNewParam = pNewParams[pos];
        Reference<T> const & xExistingParam = pExistingParams[pos];

        OUStringBuffer buf;
        buf.append( context_ );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", parameter ") );
        buf.append( pos );
        OSL_ASSERT( pos == xNewParam->getPosition() &&
                    pos == xExistingParam->getPosition() );
        OUString context( buf.makeStringAndClear() );

        if (xNewParam->getName() != xExistingParam->getName())
        {
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("Name differs: ") );
            buf.append( xNewParam->getName() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", ") );
            buf.append( xExistingParam->getName() );
            typeError( buf.makeStringAndClear(), context );
        }
        check( xNewParam->getType(), xExistingParam->getType(), context );

        if (xNewParam->isIn() != xExistingParam->isIn())
            typeError( "IN attribute differs!", context );
        if (xNewParam->isOut() != xExistingParam->isOut())
            typeError( "OUT attribute differs!", context );
        checkRestParam( xNewParam, xExistingParam, context );
    }
}

static void checkMethod(
    Reference<reflection::XInterfaceMethodTypeDescription> const & xNewTD,
    Reference<reflection::XInterfaceMethodTypeDescription> const & xExistingTD )
{
    check( xNewTD->getReturnType(), xExistingTD->getReturnType(),
           xNewTD->getName() );

    if (xNewTD->isOneway() != xExistingTD->isOneway())
        typeError( "Methods have differing OneWay attribute!",
                   xNewTD->getName() );

    checkParameters( xNewTD->getParameters(), xExistingTD->getParameters(),
                     xNewTD->getName() );

    checkSeq( xNewTD->getExceptions(), xExistingTD->getExceptions(),
              xNewTD->getName() + ", declared exceptions" );
}

void checkAttribute(
    Reference<reflection::XInterfaceAttributeTypeDescription2> const & xNewTD,
    Reference<reflection::XInterfaceAttributeTypeDescription2>
    const & xExistingTD )
{
    if (xNewTD->isReadOnly() != xExistingTD->isReadOnly())
        typeError( "ReadOnly attribute differs!", xNewTD->getName() );

    check( xNewTD->getType(), xExistingTD->getType(),
           xNewTD->getName() + ", attribute type" );

    if (xNewTD->isBound() != xExistingTD->isBound())
        typeError( "Bound attribute differs!", xNewTD->getName() );

    checkSeq( xNewTD->getGetExceptions(), xExistingTD->getGetExceptions(),
              xNewTD->getName() + ", getter exceptions" );
    checkSeq( xNewTD->getSetExceptions(), xExistingTD->getSetExceptions(),
              xNewTD->getName() + ", setter exceptions" );
}

void checkProperty(
    Reference<reflection::XPropertyTypeDescription> const & xNewTD,
    Reference<reflection::XPropertyTypeDescription> const & xExistingTD )
{
    if (xNewTD->getPropertyFlags() != xExistingTD->getPropertyFlags())
    {
        OUStringBuffer buf;
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                             "Different set of property flags: { ") );
        buf.append( getPropertyFlagsAsString(
                        xNewTD->getPropertyFlags() ) );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" } (new), { ") );
        buf.append( getPropertyFlagsAsString(
                        xExistingTD->getPropertyFlags() ) );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" } (existing)!") );
        typeError( buf.makeStringAndClear(), xNewTD->getName() );
    }

    check( xNewTD->getPropertyTypeDescription(),
           xExistingTD->getPropertyTypeDescription(),
           xNewTD->getName() );
}

void checkSingleton(
    Reference<reflection::XSingletonTypeDescription2> const & xNewTD,
    Reference<reflection::XSingletonTypeDescription2> const & xExistingTD )
{
    sal_Bool ifaceBased = xNewTD->isInterfaceBased();
    if (ifaceBased != xExistingTD->isInterfaceBased())
        typeError(
            "Mixing interface and NON-interface based singletons!",
            xNewTD->getName() );
    if (ifaceBased)
        check( xNewTD->getInterface(), xExistingTD->getInterface(),
               xNewTD->getName() );
    else
        check( xNewTD->getService().get(), xExistingTD->getService().get(),
               xNewTD->getName() );
}

void checkService(
    Reference<reflection::XServiceTypeDescription2> const & xNewTD,
    Reference<reflection::XServiceTypeDescription2> const & xExistingTD )
{
    sal_Bool singleIfaceBased = xNewTD->isSingleInterfaceBased();
    if (singleIfaceBased != xExistingTD->isSingleInterfaceBased())
        typeError( "Mixing interface and NON-interface based services!",
                   xNewTD->getName() );
    if (singleIfaceBased)
    {
        check( xNewTD->getInterface(), xExistingTD->getInterface(),
               xNewTD->getName() );
        Sequence< Reference<reflection::XServiceConstructorDescription> >
            newCtors( xNewTD->getConstructors() );
        Sequence< Reference<reflection::XServiceConstructorDescription> >
            existingCtors( xExistingTD->getConstructors() );
        sal_Int32 len = newCtors.getLength();
        if (len != existingCtors.getLength())
            typeError( "Different number of service constructors!",
                       xNewTD->getName() );
        Reference<reflection::XServiceConstructorDescription> const *
            pNewCtors = newCtors.getConstArray();
        Reference<reflection::XServiceConstructorDescription> const *
            pExistingCtors = existingCtors.getConstArray();
        for ( sal_Int32 pos = 0; pos < len; ++pos )
        {
            Reference<reflection::XServiceConstructorDescription> const &
                xNewCtor = pNewCtors[pos];
            Reference<reflection::XServiceConstructorDescription> const &
                xExistingCtor = pExistingCtors[pos];

            if (xNewCtor->getName() != xExistingCtor->getName())
            {
                OUStringBuffer buf;
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                                     "Different constructor names: ") );
                buf.append( xNewCtor->getName() );
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" (new), ") );
                buf.append( xExistingCtor->getName() );
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" (existing)!") );
                typeError( buf.makeStringAndClear(), xNewTD->getName() );
            }

            OUStringBuffer buf;
            buf.append( xNewTD->getName() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", constructor ") );
            buf.append( xNewCtor->getName() );
            OUString context( buf.makeStringAndClear() );
            checkParameters( xNewCtor->getParameters(),
                             xExistingCtor->getParameters(),
                             context );
            checkSeq( xNewCtor->getExceptions(), xExistingCtor->getExceptions(),
                      context + ", exceptions" );
        }
    }
    else // old-style service descriptions:
    {
        checkSeq( xNewTD->getMandatoryServices(),
                  xExistingTD->getMandatoryServices(),
                  xNewTD->getName() + ", mandatory services" );
        checkSeq( xNewTD->getOptionalServices(),
                  xExistingTD->getOptionalServices(),
                  xNewTD->getName() + ", optional services",
                  true /* optionalMode */ );
        checkSeq( xNewTD->getMandatoryInterfaces(),
                  xExistingTD->getMandatoryInterfaces(),
                  xNewTD->getName() + ", mandatory interfaces" );
        checkSeq( xNewTD->getOptionalInterfaces(),
                  xExistingTD->getOptionalInterfaces(),
                  xNewTD->getName() + ", optional interfaces",
                  true /* optionalMode */ );

        Sequence< Reference<reflection::XPropertyTypeDescription> >
            newProperties( xNewTD->getProperties() );
        Sequence< Reference<reflection::XPropertyTypeDescription> >
            existingProperties( xExistingTD->getProperties() );
        checkSeq( newProperties, existingProperties,
                  xNewTD->getName() + ", properties",
                  true /* optionalMode */ );
        if (newProperties.getLength() > existingProperties.getLength())
        {
            // check whether all added properties are OPTIONAL:
            Reference<reflection::XPropertyTypeDescription> const *
                pNewProperties = newProperties.getConstArray();
            for ( sal_Int32 pos = existingProperties.getLength() + 1;
                  pos < newProperties.getLength(); ++pos )
            {
                if ((pNewProperties[pos]->getPropertyFlags() &
                     beans::PropertyAttribute::OPTIONAL) == 0)
                    typeError( "New property is not OPTIONAL!",
                               pNewProperties[pos]->getName() );
            }
        }
    }
}

}

namespace stoc_tdmgr {

void check( Reference<reflection::XTypeDescription> const & xNewTD,
            Reference<reflection::XTypeDescription> const & xExistingTD,
            OUString const & context )
{
    if (xNewTD == xExistingTD)
        return;
    if (xNewTD->getName() != xExistingTD->getName())
    {
        OUStringBuffer buf;
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("Different type names: ") );
        buf.append( xNewTD->getName() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" (new), ") );
        buf.append( xExistingTD->getName() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" (existing)!") );
        typeError( buf.makeStringAndClear(), context );
    }

    TypeClass tc = xNewTD->getTypeClass();
    if (tc != xExistingTD->getTypeClass())
    {
        OUStringBuffer buf;
        buf.append( xNewTD->getName() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                             " has different type classes: ") );
        buf.append( getTypeClassName( tc ) );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" (new), ") );
        buf.append( getTypeClassName( xExistingTD->getTypeClass() ) );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" (existing)!") );
        typeError( buf.makeStringAndClear(), context );
    }

    switch (tc)
    {
    case TypeClass_ENUM:
        checkEnum( Reference<reflection::XEnumTypeDescription>(
                       xNewTD, UNO_QUERY_THROW ),
                   Reference<reflection::XEnumTypeDescription>(
                       xExistingTD, UNO_QUERY_THROW ) );
        break;

    case TypeClass_TYPEDEF:
    case TypeClass_SEQUENCE:
        check( Reference<reflection::XIndirectTypeDescription>(
                   xNewTD, UNO_QUERY_THROW )->getReferencedType(),
               Reference<reflection::XIndirectTypeDescription>(
                   xExistingTD, UNO_QUERY_THROW )->getReferencedType() );
        break;

    case TypeClass_STRUCT:
    case TypeClass_EXCEPTION:
        checkStruct( Reference<reflection::XCompoundTypeDescription>(
                         xNewTD, UNO_QUERY_THROW ),
                     Reference<reflection::XCompoundTypeDescription>(
                         xExistingTD, UNO_QUERY_THROW ) );
        break;

    case TypeClass_INTERFACE:
        checkInterface( Reference<reflection::XInterfaceTypeDescription2>(
                            xNewTD, UNO_QUERY_THROW ),
                        Reference<reflection::XInterfaceTypeDescription2>(
                            xExistingTD, UNO_QUERY_THROW ) );
        break;

    case TypeClass_SERVICE:
        checkService( Reference<reflection::XServiceTypeDescription2>(
                          xNewTD, UNO_QUERY_THROW ),
                      Reference<reflection::XServiceTypeDescription2>(
                          xExistingTD, UNO_QUERY_THROW ) );
        break;

    case TypeClass_INTERFACE_METHOD:
        checkMethod( Reference<reflection::XInterfaceMethodTypeDescription>(
                         xNewTD, UNO_QUERY_THROW ),
                     Reference<reflection::XInterfaceMethodTypeDescription>(
                         xExistingTD, UNO_QUERY_THROW ) );
        break;
    case TypeClass_INTERFACE_ATTRIBUTE:
        checkAttribute(
            Reference<reflection::XInterfaceAttributeTypeDescription2>(
                xNewTD, UNO_QUERY_THROW ),
            Reference<reflection::XInterfaceAttributeTypeDescription2>(
                xExistingTD, UNO_QUERY_THROW ) );
        break;

    case TypeClass_PROPERTY:
        checkProperty( Reference<reflection::XPropertyTypeDescription>(
                           xNewTD, UNO_QUERY_THROW ),
                       Reference<reflection::XPropertyTypeDescription>(
                           xExistingTD, UNO_QUERY_THROW ) );
        break;

    case TypeClass_CONSTANT:
        if (Reference<reflection::XConstantTypeDescription>(
                xNewTD, UNO_QUERY_THROW )->getConstantValue() !=
            Reference<reflection::XConstantTypeDescription>(
                xExistingTD, UNO_QUERY_THROW )->getConstantValue())
            typeError( "Different constant value!", xNewTD->getName() );
        break;
    case TypeClass_CONSTANTS:
        checkSeq( Reference<reflection::XConstantsTypeDescription>(
                      xNewTD, UNO_QUERY_THROW )->getConstants(),
                  Reference<reflection::XConstantsTypeDescription>(
                      xExistingTD, UNO_QUERY_THROW )->getConstants(),
                  xNewTD->getName() );
        break;

    case TypeClass_SINGLETON:
        checkSingleton( Reference<reflection::XSingletonTypeDescription2>(
                            xNewTD, UNO_QUERY_THROW ),
                        Reference<reflection::XSingletonTypeDescription2>(
                            xExistingTD, UNO_QUERY_THROW ) );
        break;

    default:
        break;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
