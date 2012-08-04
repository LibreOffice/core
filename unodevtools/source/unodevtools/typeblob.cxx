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

#include <string.h>

#include "rtl/alloc.h"
#include "registry/writer.hxx"

#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/reflection/XPublished.hpp"
#include "com/sun/star/reflection/XInterfaceTypeDescription2.hpp"
#include "com/sun/star/reflection/XSingletonTypeDescription2.hpp"
#include "com/sun/star/reflection/XServiceTypeDescription2.hpp"
#include "com/sun/star/reflection/XStructTypeDescription.hpp"
#include "com/sun/star/reflection/XConstantsTypeDescription.hpp"
#include "com/sun/star/reflection/XConstantTypeDescription.hpp"
#include "com/sun/star/reflection/XModuleTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceAttributeTypeDescription2.hpp"
#include "com/sun/star/reflection/XMethodParameter.hpp"
#include "com/sun/star/reflection/XCompoundTypeDescription.hpp"
#include "com/sun/star/reflection/XIndirectTypeDescription.hpp"
#include "com/sun/star/reflection/XEnumTypeDescription.hpp"

#include "codemaker/generatedtypeset.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::reflection;
using namespace codemaker;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringToOUString;
using ::rtl::OUStringToOString;

namespace unodevtools {

void writeConstantData(typereg::Writer& rWriter, sal_uInt16 fieldIndex,
                       const Reference< XConstantTypeDescription >& xConstant)

{
    RTConstValue constValue;
    OUString uConstTypeName;
    OUString uConstName = xConstant->getName();
    Any aConstantAny = xConstant->getConstantValue();

    switch ( aConstantAny.getValueTypeClass() )
    {
    case TypeClass_BOOLEAN:
    {
        uConstTypeName = OUString(RTL_CONSTASCII_USTRINGPARAM("boolean"));
        constValue.m_type = RT_TYPE_BOOL;
        aConstantAny >>= constValue.m_value.aBool;
    }
    break;
    case TypeClass_BYTE:
    {
        uConstTypeName = OUString(RTL_CONSTASCII_USTRINGPARAM("byte"));
        constValue.m_type = RT_TYPE_BYTE;
        aConstantAny >>= constValue.m_value.aByte;
    }
    break;
    case TypeClass_SHORT:
    {
        uConstTypeName = OUString(RTL_CONSTASCII_USTRINGPARAM("short"));
        constValue.m_type = RT_TYPE_INT16;
        aConstantAny >>= constValue.m_value.aShort;
    }
    break;
    case TypeClass_UNSIGNED_SHORT:
    {
        uConstTypeName = OUString(RTL_CONSTASCII_USTRINGPARAM("unsigned short"));
        constValue.m_type = RT_TYPE_UINT16;
        aConstantAny >>= constValue.m_value.aUShort;
    }
    break;
    case TypeClass_LONG:
    {
        uConstTypeName = OUString(RTL_CONSTASCII_USTRINGPARAM("long"));
        constValue.m_type = RT_TYPE_INT32;
        aConstantAny >>= constValue.m_value.aLong;
    }
    break;
    case TypeClass_UNSIGNED_LONG:
    {
        uConstTypeName = OUString(RTL_CONSTASCII_USTRINGPARAM("unsigned long"));
        constValue.m_type = RT_TYPE_UINT32;
        aConstantAny >>= constValue.m_value.aULong;
    }
    break;
    case TypeClass_FLOAT:
    {
        uConstTypeName = OUString(RTL_CONSTASCII_USTRINGPARAM("float"));
        constValue.m_type = RT_TYPE_FLOAT;
        aConstantAny >>= constValue.m_value.aFloat;
    }
    break;
    case TypeClass_DOUBLE:
    {
        uConstTypeName = OUString(RTL_CONSTASCII_USTRINGPARAM("double"));
        constValue.m_type = RT_TYPE_DOUBLE;
        aConstantAny >>= constValue.m_value.aDouble;
    }
    break;
    case TypeClass_STRING:
    {
        uConstTypeName = OUString(RTL_CONSTASCII_USTRINGPARAM("string"));
        constValue.m_type = RT_TYPE_STRING;
        constValue.m_value.aString = ((OUString*)aConstantAny.getValue())->getStr();
    }
    break;
    default:
        OSL_FAIL( "unsupported constant type" );
        break;
    }

    rWriter.setFieldData(fieldIndex, OUString(), OUString(), RT_ACCESS_CONST,
                         uConstName, uConstTypeName, constValue);
}

sal_uInt32 getInheritedMemberCount(
    GeneratedTypeSet& checkedTypes,
    Sequence< Reference< XTypeDescription > >& superTypes)
{
    sal_uInt32 memberCount = 0;

    sal_uInt16 count = (sal_uInt16)superTypes.getLength();
    OString name;
    for (sal_uInt16 i=0; i < count; i++) {
        name = OString(OUStringToOString(superTypes[i]->getName(),
                                         RTL_TEXTENCODING_UTF8));
        if (!checkedTypes.contains(name)) {
            checkedTypes.add(name);

            Reference< XInterfaceTypeDescription2 > xIFace(
                superTypes[i], UNO_QUERY);

            Sequence< Reference< XTypeDescription> > baseTypes =
                xIFace->getBaseTypes();
            if ( baseTypes.getLength() > 0)
                memberCount += getInheritedMemberCount(checkedTypes, baseTypes);

            memberCount += xIFace->getMembers().getLength();
        }
    }

    return memberCount;
}

void writeMethodData( typereg::Writer& rWriter, sal_uInt32 calculatedMemberOffset,
                      const Reference< XInterfaceMethodTypeDescription >& xMethod )
{
    RTMethodMode methodMode = RT_MODE_TWOWAY;
    if ( xMethod->isOneway() )
        methodMode = RT_MODE_ONEWAY;

    Sequence< Reference< XMethodParameter > > parameters(xMethod->getParameters());
    Sequence< Reference< XTypeDescription > > exceptions(xMethod->getExceptions());

    OUString name(xMethod->getMemberName());
    sal_uInt16 methodIndex = (sal_uInt16)(xMethod->getPosition()
                                          - calculatedMemberOffset);
    sal_uInt16 paramCount = (sal_uInt16)parameters.getLength();
    sal_uInt16 exceptionCount = (sal_uInt16)exceptions.getLength();

    rWriter.setMethodData(methodIndex, OUString(), methodMode,
                          xMethod->getMemberName(),
                          xMethod->getReturnType()->getName().replace('.', '/'),
                          paramCount, exceptionCount);

    RTParamMode paramMode = RT_PARAM_IN;
    sal_uInt16 i;

    for ( i=0; i < paramCount; i++) {
        Reference< XMethodParameter > xParam = parameters[i];
        if ( xParam->isIn() && xParam->isOut())
            paramMode = RT_PARAM_INOUT;
        else if ( xParam->isIn() )
            paramMode = RT_PARAM_IN;
        else if ( xParam->isOut() )
            paramMode = RT_PARAM_OUT;

        rWriter.setMethodParameterData(methodIndex,
                                       (sal_uInt16)xParam->getPosition(),
                                       paramMode, xParam->getName(),
                                       xParam->getType()->
                                       getName().replace('.', '/'));
    }

    for (i=0; i < exceptionCount; i++) {
        rWriter.setMethodExceptionTypeName(
            methodIndex, i, exceptions[i]->getName().replace('.', '/'));
    }
}

void writeAttributeMethodData(
    typereg::Writer& rWriter, sal_uInt16& methodindex, RTMethodMode methodmode,
    const Reference<XInterfaceAttributeTypeDescription2>& xAttr)
{
    Sequence<Reference<XCompoundTypeDescription> > seqExcp;
    if (methodmode == RT_MODE_ATTRIBUTE_GET)
        seqExcp = xAttr->getGetExceptions();
    else
        seqExcp = xAttr->getSetExceptions();

    if (seqExcp.getLength() > 0) {
        rWriter.setMethodData(methodindex, OUString(), methodmode,
                              xAttr->getMemberName(),
                              rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("void")),
                              0, (sal_uInt16)seqExcp.getLength());

        for (sal_Int32 i=0; i < seqExcp.getLength(); i++) {
            rWriter.setMethodExceptionTypeName(
                methodindex, (sal_uInt16)i,
                seqExcp[i]->getName().replace('.', '/'));
        }
        ++methodindex;
    }
}

RTFieldAccess checkParameterizedTypeFlag(const Sequence< OUString >& typeParams,
                                         const OUString & memberType)
{
    for (sal_uInt16 i=0; i < typeParams.getLength(); i++) {
        if (typeParams[i].equals(memberType))
            return RT_ACCESS_PARAMETERIZED_TYPE;
    }

    return RT_ACCESS_READWRITE;
}

RTFieldAccess checkPropertyFlags(short flags) {
    RTFieldAccess propertyFlags=RT_ACCESS_INVALID;
    switch(flags) {
    case PropertyAttribute::MAYBEVOID:
        propertyFlags |= RT_ACCESS_MAYBEVOID;
    case PropertyAttribute::BOUND:
        propertyFlags |= RT_ACCESS_BOUND;
    case PropertyAttribute::CONSTRAINED:
        propertyFlags |= RT_ACCESS_CONSTRAINED;
    case PropertyAttribute::TRANSIENT:
        propertyFlags |= RT_ACCESS_TRANSIENT;
    case PropertyAttribute::READONLY :
        propertyFlags |= RT_ACCESS_READONLY;
    case PropertyAttribute::MAYBEAMBIGUOUS:
        propertyFlags |= RT_ACCESS_MAYBEAMBIGUOUS;
    case PropertyAttribute::MAYBEDEFAULT:
        propertyFlags |= RT_ACCESS_MAYBEDEFAULT;
    case PropertyAttribute::REMOVEABLE:
        propertyFlags |= RT_ACCESS_REMOVEABLE;
    case PropertyAttribute::OPTIONAL:
        propertyFlags |= RT_ACCESS_OPTIONAL;
    }
    return propertyFlags;
}

void* getTypeBlob(Reference< XHierarchicalNameAccess > xTDmgr,
                  const OString& typeName, sal_uInt32* blobsize)
{
    if ( typeName.isEmpty() )
        return NULL;

    OUString uTypeName(OStringToOUString(typeName, RTL_TEXTENCODING_UTF8)
                       .replace('/', '.'));

    Any aTypeAny( xTDmgr->getByHierarchicalName( uTypeName ) );

    if ( !aTypeAny.hasValue() )
        return NULL;

    Reference< XTypeDescription > xType;
    aTypeAny >>= xType;

    if ( !xType.is() )
        return NULL;

    Reference< XPublished > xPublished(xType, UNO_QUERY);
    void* pBlob = NULL;
    switch (xType->getTypeClass())
    {
    case TypeClass_CONSTANTS:
    {
        Reference< XConstantsTypeDescription > xCFace(xType, UNO_QUERY);

        if ( !xCFace.is() )
            return NULL;

        Sequence< Reference< XConstantTypeDescription > > constTypes(
            xCFace->getConstants());
        sal_uInt16 constCount = (sal_uInt16)constTypes.getLength();

        typereg::Writer writer(TYPEREG_VERSION_1, OUString(), OUString(),
                               RT_TYPE_CONSTANTS, xPublished->isPublished(),
                               uTypeName.replace('.', '/'),
                               0, constCount, 0, 0);

        for (sal_uInt16 i=0; i < constCount; i++)
            writeConstantData(writer, i, constTypes[i]);

        const void* p = writer.getBlob(blobsize);
        pBlob = (sal_uInt8*)rtl_allocateMemory(*blobsize);
        memcpy(pBlob, p, *blobsize);
    }
    break;
    case TypeClass_MODULE:
    {
        Reference< XModuleTypeDescription > xMFace(xType, UNO_QUERY);

        if ( !xMFace.is() )
            return NULL;

        Sequence< Reference< XTypeDescription > > memberTypes(
            xMFace->getMembers());

        sal_uInt16 memberCount = (sal_uInt16)memberTypes.getLength();
        sal_uInt16 constCount = 0;
        sal_Int16 i;

        for ( i=0; i < memberCount; i++) {
            if ( TypeClass_CONSTANT == memberTypes[i]->getTypeClass() )
                constCount++;
        }

        typereg::Writer writer(TYPEREG_VERSION_1, OUString(), OUString(),
                               RT_TYPE_MODULE, xPublished->isPublished(),
                               uTypeName.replace('.', '/'),
                               0, constCount, 0, 0);

        if ( 0 < constCount ) {
            Reference< XConstantTypeDescription > xConst;
            sal_uInt16 fieldIndex = 0;
            for (i=0; i < memberCount; i++) {
                if ( TypeClass_CONSTANT == memberTypes[i]->getTypeClass() ) {
                    xConst = Reference< XConstantTypeDescription >(
                        memberTypes[i], UNO_QUERY);

                    writeConstantData(writer, ++fieldIndex, xConst);
                }
            }
        }

        const void* p = writer.getBlob(blobsize);
        pBlob = (sal_uInt8*)rtl_allocateMemory(*blobsize);
        memcpy(pBlob, p, *blobsize);
    }
    break;
    case TypeClass_INTERFACE:
    {
        Reference< XInterfaceTypeDescription2 > xIFace(xType, UNO_QUERY);

        if ( !xIFace.is() )
            return NULL;

        Reference< XInterfaceAttributeTypeDescription2 > xAttr;
        Reference< XInterfaceMethodTypeDescription > xMethod;
        Sequence< Reference< XInterfaceMemberTypeDescription > > memberTypes(
            xIFace->getMembers());
        Sequence< Reference< XTypeDescription > > baseTypes =
            xIFace->getBaseTypes();
        Sequence< Reference< XTypeDescription > > optBaseTypes =
            xIFace->getOptionalBaseTypes();

        sal_uInt16 baseCount = (sal_uInt16)baseTypes.getLength();
        sal_uInt16 optBaseCount = (sal_uInt16)optBaseTypes.getLength();
        sal_uInt16 memberCount = (sal_uInt16)memberTypes.getLength();
        sal_uInt16 attrCount = 0, attrmethods = 0;
        sal_uInt16 inheritedMemberCount = 0;
        sal_uInt16 i;

        for (i=0; i < memberCount; i++) {
            xAttr = Reference< XInterfaceAttributeTypeDescription2 >(
                memberTypes[i], UNO_QUERY);
            if ( xAttr.is() ) {
                attrCount++;

                if (xAttr->getGetExceptions().getLength() > 0)
                    attrmethods++;

                if (xAttr->getSetExceptions().getLength() > 0)
                    attrmethods++;
            }
        }

        // check inherited members count
        if (baseCount > 0) {
            GeneratedTypeSet checkedTypes;
            inheritedMemberCount = (sal_uInt16)getInheritedMemberCount(
                checkedTypes, baseTypes );
        }

        typereg::Writer writer(TYPEREG_VERSION_1, OUString(), OUString(),
                               RT_TYPE_INTERFACE, xPublished->isPublished(),
                               uTypeName.replace('.', '/'),
                               baseCount, attrCount, memberCount-attrCount+attrmethods,
                               (sal_uInt16)optBaseTypes.getLength());

        // set super types
        for (i=0; i < baseCount; i++) {
            writer.setSuperTypeName(i, baseTypes[i]->
                                    getName().replace('.', '/'));
        }

        // set optional super types
        RTReferenceType referenceType = RT_REF_SUPPORTS;
        RTFieldAccess fieldAccess = RT_ACCESS_OPTIONAL;
        for (i=0; i < optBaseCount; i++) {
            writer.setReferenceData(i, OUString(), referenceType,
                                    fieldAccess, optBaseTypes[i]->
                                    getName().replace('.', '/'));
        }

        fieldAccess = RT_ACCESS_READWRITE;
        // reset attrCount, used for method index calculation
        attrCount = 0;
        attrmethods = 0;
        for (i=0; i < memberCount; i++) {
            xAttr = Reference< XInterfaceAttributeTypeDescription2 >(
                memberTypes[i], UNO_QUERY);
            if ( xAttr.is() ) {
                ++attrCount;
                if (xAttr->isReadOnly())
                    fieldAccess = RT_ACCESS_READONLY;
                else
                    fieldAccess = RT_ACCESS_READWRITE;

                if (xAttr->isBound())
                    fieldAccess |= RT_ACCESS_BOUND;

                writer.setFieldData((sal_uInt16)memberTypes[i]->getPosition()
                                    - inheritedMemberCount,
                                    OUString(), OUString(), fieldAccess,
                                    memberTypes[i]->getMemberName(),
                                    xAttr->getType()->getName().replace('.','/'),
                                    RTConstValue());

                writeAttributeMethodData(writer, attrmethods,
                                         RT_MODE_ATTRIBUTE_GET, xAttr);
                if (!xAttr->isReadOnly()) {
                    writeAttributeMethodData(writer, attrmethods,
                                            RT_MODE_ATTRIBUTE_SET, xAttr);
                }

                continue;
            }

            xMethod = Reference< XInterfaceMethodTypeDescription >(
                memberTypes[i], UNO_QUERY);
            if ( xMethod.is() ) {
                writeMethodData(writer, attrCount+inheritedMemberCount-attrmethods,
                                xMethod);
            }
        }

        const void* p = writer.getBlob(blobsize);
        pBlob = (sal_uInt8*)rtl_allocateMemory(*blobsize);
        memcpy(pBlob, p, *blobsize);
    }
    break;
    case TypeClass_STRUCT:
    {
        Reference< XStructTypeDescription > xStruct(xType, UNO_QUERY);

        if ( !xStruct.is() )
            return NULL;

        if ( xStruct->getTypeArguments().getLength() > 0)
            return NULL;


        Sequence< OUString > typeParams( xStruct->getTypeParameters());
        Sequence< OUString > memberNames( xStruct->getMemberNames());
        Sequence< Reference< XTypeDescription > > memberTypes(
            xStruct->getMemberTypes());
        sal_uInt16 memberCount = (sal_uInt16)memberNames.getLength();

        OUString uSuperType;
        sal_uInt16 superCount=0;
        if ( typeParams.getLength() == 0) {
            Reference< XTypeDescription > xSuperType = xStruct->getBaseType();
            if ( xSuperType.is() ) {
                ++superCount;
                uSuperType = xSuperType->getName().replace('.','/');
            }
        }
        typereg::Writer writer(TYPEREG_VERSION_1, OUString(), OUString(),
                               RT_TYPE_STRUCT, xPublished->isPublished(),
                               uTypeName.replace('.', '/'),
                                   superCount, memberCount, 0,
                               (sal_uInt16)typeParams.getLength());

        // set super type
        if (superCount > 0) {
            writer.setSuperTypeName(0, uSuperType);
        }

        sal_uInt16 i=0;
        for (i=0; i < memberCount; i++) {
            RTFieldAccess fieldAccess = RT_ACCESS_READWRITE;
            if (typeParams.getLength() > 0)
                fieldAccess |= checkParameterizedTypeFlag(
                    typeParams, memberTypes[i]->getName());

            writer.setFieldData(i, OUString(), OUString(), fieldAccess,
                                memberNames[i],
                                memberTypes[i]->getName().replace('.', '/'),
                                RTConstValue());
        }

        for (i=0; i < typeParams.getLength(); i++) {
            writer.setReferenceData(i, OUString(), RT_REF_TYPE_PARAMETER,
                                    RT_ACCESS_INVALID, typeParams[i]);
        }

        const void* p = writer.getBlob(blobsize);
        pBlob = (sal_uInt8*)rtl_allocateMemory(*blobsize);
        memcpy(pBlob, p, *blobsize);
        }
    break;
    case TypeClass_EXCEPTION:
    {
        Reference< XCompoundTypeDescription > xComp(xType, UNO_QUERY);

        if ( !xComp.is() )
            return NULL;

        Sequence< OUString > memberNames( xComp->getMemberNames());
        Sequence< Reference< XTypeDescription > > memberTypes(
            xComp->getMemberTypes());
        sal_uInt16 memberCount = (sal_uInt16)memberNames.getLength();

        OUString uSuperType;
        sal_uInt16 superCount=0;
        Reference< XTypeDescription > xSuperType = xComp->getBaseType();
        if ( xSuperType.is() ) {
            ++superCount;
            uSuperType = xSuperType->getName().replace('.','/');
        }

        typereg::Writer writer(TYPEREG_VERSION_1, OUString(), OUString(),
                               RT_TYPE_EXCEPTION, xPublished->isPublished(),
                               uTypeName.replace('.', '/'),
                               superCount, memberCount, 0, 0);

        // set super type
        if (superCount > 0) {
            writer.setSuperTypeName(0, uSuperType);
        }

        for (sal_Int16 i=0; i < memberCount; i++) {
            writer.setFieldData(i, OUString(), OUString(), RT_ACCESS_READWRITE,
                                memberNames[i],
                                memberTypes[i]->getName().replace('.', '/'),
                                RTConstValue());
        }

        const void* p = writer.getBlob(blobsize);
        pBlob = (sal_uInt8*)rtl_allocateMemory(*blobsize);
        memcpy(pBlob, p, *blobsize);
    }
    break;
    case TypeClass_ENUM:
    {
        Reference< XEnumTypeDescription > xEnum(xType, UNO_QUERY);

        if ( !xEnum.is() )
            return NULL;

        Sequence< OUString > enumNames( xEnum->getEnumNames());
        Sequence< sal_Int32 > enumValues( xEnum->getEnumValues());
        sal_uInt16 enumCount = (sal_uInt16)enumNames.getLength();

        typereg::Writer writer(TYPEREG_VERSION_1, OUString(), OUString(),
                               RT_TYPE_ENUM, xPublished->isPublished(),
                               uTypeName.replace('.', '/'),
                               0, enumCount, 0, 0);

        RTConstValue constValue;
        for (sal_Int16 i=0; i < enumCount; i++) {
            constValue.m_type = RT_TYPE_INT32;
            constValue.m_value.aLong = enumValues[i];

            writer.setFieldData(i, OUString(), OUString(),
                                RT_ACCESS_CONST, enumNames[i],
                                OUString(), constValue);
        }

        const void* p = writer.getBlob(blobsize);
        pBlob = (sal_uInt8*)rtl_allocateMemory(*blobsize);
        memcpy(pBlob, p, *blobsize);
    }
    break;
    case TypeClass_TYPEDEF:
    {
        Reference< XIndirectTypeDescription > xTD(xType, UNO_QUERY);

        if ( !xTD.is() )
            return NULL;

        typereg::Writer writer(TYPEREG_VERSION_1, OUString(), OUString(),
                               RT_TYPE_TYPEDEF, xPublished->isPublished(),
                               uTypeName.replace('.', '/'),
                               1, 0, 0, 0);

        writer.setSuperTypeName(0, xTD->getReferencedType()
                                ->getName().replace('.','/'));

        const void* p = writer.getBlob(blobsize);
        pBlob = (sal_uInt8*)rtl_allocateMemory(*blobsize);
        memcpy(pBlob, p, *blobsize);
    }
    break;
    case TypeClass_SERVICE:
    {
        Reference< XServiceTypeDescription2 > xService(xType, UNO_QUERY);

        if ( !xService.is() )
            return NULL;

        Sequence<Reference<XServiceConstructorDescription> > constructors(
            xService->getConstructors());
        Sequence<Reference<XPropertyTypeDescription> > properties;
        Sequence<Reference<XServiceTypeDescription> > mandatoryServices;
        Sequence<Reference<XServiceTypeDescription> > optionalServices;
        Sequence<Reference<XInterfaceTypeDescription> > mandatoryInterfaces;
        Sequence<Reference<XInterfaceTypeDescription> > optionalInterfaces;
        sal_uInt16 methodCount = (sal_uInt16)constructors.getLength();
        sal_uInt16 referenceCount = 0;
        sal_uInt16 propertyCount = 0;

        if ( !xService->isSingleInterfaceBased() ) {
            mandatoryServices = xService->getMandatoryServices();
            optionalServices = xService->getOptionalServices();
            mandatoryInterfaces = xService->getMandatoryInterfaces();
            optionalInterfaces = xService->getOptionalInterfaces();
            properties = xService->getProperties();
            referenceCount = (sal_uInt16)(
                mandatoryServices.getLength()+
                optionalServices.getLength()+
                mandatoryInterfaces.getLength()+
                optionalInterfaces.getLength());
            propertyCount = (sal_uInt16)properties.getLength();
        }

        typereg::Writer writer(TYPEREG_VERSION_1, OUString(), OUString(),
                               RT_TYPE_SERVICE, xPublished->isPublished(),
                               uTypeName.replace('.', '/'),
                                   (xService->isSingleInterfaceBased() ? 1 : 0),
                               propertyCount, methodCount, referenceCount);

        sal_uInt16 i=0;
        if ( xService->isSingleInterfaceBased() ) {
            writer.setSuperTypeName(0, xService->getInterface()
                                    ->getName().replace('.','/'));

            sal_uInt16 j=0;
            for ( i=0; i<methodCount; i++ ) {
                Reference<XServiceConstructorDescription> xConstructor(
                    constructors[i], UNO_QUERY);
                Sequence<Reference<XParameter> > parameters;
                Sequence<Reference<XCompoundTypeDescription> > exceptions;
                sal_uInt16 parameterCount=0;
                sal_uInt16 exceptionCount=0;
                if ( !xConstructor->isDefaultConstructor() ) {
                    parameters = xConstructor->getParameters();
                    parameterCount = (sal_uInt16)parameters.getLength();
                }

                writer.setMethodData(i, OUString(), RT_MODE_TWOWAY,
                                     xConstructor->getName(),
                                     OUString(
                                         RTL_CONSTASCII_USTRINGPARAM("void")),
                                     parameterCount, exceptionCount);

                if ( !xConstructor->isDefaultConstructor() ) {
                    for ( j=0; j<parameterCount; j++ ) {
                        Reference<XParameter> xParam(parameters[j], UNO_QUERY);
                        RTParamMode paramMode = RT_PARAM_IN;
                        if (xParam->isRestParameter())
                            paramMode = RT_PARAM_REST;

                        writer.setMethodParameterData(
                            i,  (sal_uInt16)xParam->getPosition(),
                            paramMode, xParam->getName(),
                            xParam->getType()->getName().replace('.', '/'));
                    }

                    for (j=0; j<exceptionCount; j++) {
                        Reference<XCompoundTypeDescription> xExcp(
                            exceptions[j], UNO_QUERY);

                        writer.setMethodExceptionTypeName(
                            i, j, xExcp->getName().replace('.', '/'));
                    }
                }
            }
        } else
        {
            for (i=0; i<propertyCount; i++) {
                Reference<XPropertyTypeDescription> xProp(
                    properties[i], UNO_QUERY);

                RTFieldAccess propertyFlags = checkPropertyFlags(
                    xProp->getPropertyFlags());

                writer.setFieldData(i, OUString(), OUString(),
                                    propertyFlags,
                                    xProp->getName().copy(xProp->getName().lastIndexOf('.')+1),
                                    xProp->getPropertyTypeDescription()
                                    ->getName().replace('.', '/'),
                                    RTConstValue());
            }

            sal_uInt16 refIndex = 0;
            sal_uInt16 length = (sal_uInt16)mandatoryServices.getLength();
            for (i=0; i < length; i++) {
                writer.setReferenceData(refIndex++, OUString(),
                                        RT_REF_EXPORTS, RT_ACCESS_INVALID,
                                        mandatoryServices[i]->getName()
                                        .replace('.', '/'));
            }
            length = (sal_uInt16)optionalServices.getLength();
            for (i=0; i < length; i++) {
                writer.setReferenceData(refIndex++, OUString(),
                                        RT_REF_EXPORTS, RT_ACCESS_OPTIONAL,
                                        optionalServices[i]->getName()
                                        .replace('.', '/'));
            }
            length = (sal_uInt16)mandatoryInterfaces.getLength();
            for (i=0; i < length; i++) {
                writer.setReferenceData(refIndex++, OUString(),
                                        RT_REF_SUPPORTS, RT_ACCESS_INVALID,
                                        mandatoryInterfaces[i]->getName()
                                        .replace('.', '/'));
            }
            length = (sal_uInt16)optionalInterfaces.getLength();
            for (i=0; i < length; i++) {
                writer.setReferenceData(refIndex++, OUString(),
                                        RT_REF_SUPPORTS, RT_ACCESS_OPTIONAL,
                                        optionalInterfaces[i]->getName()
                                        .replace('.', '/'));
            }
        }

        const void* p = writer.getBlob(blobsize);
        pBlob = (sal_uInt8*)rtl_allocateMemory(*blobsize);
        memcpy(pBlob, p, *blobsize);
    }
    break;
    case TypeClass_SINGLETON:
    {
        Reference<XSingletonTypeDescription2> xSingleton(xType, UNO_QUERY);

        if ( !xSingleton.is() )
            return NULL;

        typereg::Writer writer(TYPEREG_VERSION_1, OUString(), OUString(),
                               RT_TYPE_SINGLETON, xPublished->isPublished(),
                               uTypeName.replace('.', '/'),
                               1, 0, 0, 0);

        if (xSingleton->isInterfaceBased()) {
            writer.setSuperTypeName(0, xSingleton->getInterface()
                                    ->getName().replace('.','/'));
        } else {
            writer.setSuperTypeName(0, xSingleton->getService()
                                    ->getName().replace('.','/'));
        }

        const void* p = writer.getBlob(blobsize);
        pBlob = (sal_uInt8*)rtl_allocateMemory(*blobsize);
        memcpy(pBlob, p, *blobsize);
    }
    break;
    default:
        OSL_FAIL( "unsupported type" );
        break;
    }

    return pBlob;
}

} // end of namespace unodevtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
