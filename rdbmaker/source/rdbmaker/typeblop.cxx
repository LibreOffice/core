/*************************************************************************
 *
 *  $RCSfile: typeblop.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:08 $
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

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef __REGISTRY_REFLWRIT_HXX__
#include <registry/reflwrit.hxx>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XINTERFACETYPEDESCRIPTION_HPP_
#include <com/sun/star/reflection/XInterfaceTypeDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XINTERFACEMETHODTYPEDESCRIPTION_HPP_
#include <com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XINTERFACEATTRIBUTETYPEDESCRIPTION_HPP_
#include <com/sun/star/reflection/XInterfaceAttributeTypeDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XMETHODPARAMETER_HPP_
#include <com/sun/star/reflection/XMethodParameter.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XCOMPOUNDTYPEDESCRIPTION_HPP_
#include <com/sun/star/reflection/XCompoundTypeDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XINDIRECTTYPEDESCRIPTION_HPP_
#include <com/sun/star/reflection/XIndirectTypeDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XENUMTYPEDESCRIPTION_HPP_
#include <com/sun/star/reflection/XEnumTypeDescription.hpp>
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::reflection;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace cppu;
//using namespace osl;
using namespace rtl;

static Reference< XHierarchicalNameAccess > xNameAccess;

sal_uInt32 writeConstantData( const RegistryTypeWriterLoader& rLoader,
                              const OUString& typeName,
                              const Any& aTypeAny,
                              sal_uInt8** pBlop )
{
    sal_uInt32 lastIndex = typeName.lastIndexOf('.');
    OUString uModuleName( typeName.copy(0, lastIndex) );
    OUString uConstName( typeName.copy(lastIndex + 1) );

    RegistryTypeWriter writer(rLoader, RT_TYPE_MODULE, uModuleName.replace('.', '/'),
                              OUString(), 1, 0, 0);

    RTConstValue constValue;
    OUString uConstTypeName;
    switch ( aTypeAny.getValueTypeClass() )
    {
        case TypeClass_BOOLEAN:
            {
                uConstTypeName = OUString( RTL_CONSTASCII_USTRINGPARAM("boolean") );
                constValue.m_type = RT_TYPE_BOOL;
                aTypeAny >>= constValue.m_value.aBool;
            }
            break;
        case TypeClass_BYTE:
            {
                uConstTypeName = OUString( RTL_CONSTASCII_USTRINGPARAM("byte") );
                constValue.m_type = RT_TYPE_BYTE;
                aTypeAny >>= constValue.m_value.aByte;
            }
            break;
        case TypeClass_SHORT:
            {
                uConstTypeName = OUString( RTL_CONSTASCII_USTRINGPARAM("short") );
                constValue.m_type = RT_TYPE_INT16;
                aTypeAny >>= constValue.m_value.aShort;
            }
            break;
        case TypeClass_UNSIGNED_SHORT:
            {
                uConstTypeName = OUString( RTL_CONSTASCII_USTRINGPARAM("unsigned short") );
                constValue.m_type = RT_TYPE_UINT16;
                aTypeAny >>= constValue.m_value.aUShort;
            }
            break;
        case TypeClass_LONG:
            {
                uConstTypeName = OUString( RTL_CONSTASCII_USTRINGPARAM("long") );
                constValue.m_type = RT_TYPE_INT32;
                aTypeAny >>= constValue.m_value.aLong;
            }
            break;
        case TypeClass_UNSIGNED_LONG:
            {
                uConstTypeName = OUString( RTL_CONSTASCII_USTRINGPARAM("unsigned long") );
                constValue.m_type = RT_TYPE_UINT32;
                aTypeAny >>= constValue.m_value.aULong;
            }
            break;
        case TypeClass_FLOAT:
            {
                uConstTypeName = OUString( RTL_CONSTASCII_USTRINGPARAM("float") );
                constValue.m_type = RT_TYPE_FLOAT;
                aTypeAny >>= constValue.m_value.aFloat;
            }
            break;
        case TypeClass_DOUBLE:
            {
                uConstTypeName = OUString( RTL_CONSTASCII_USTRINGPARAM("double") );
                constValue.m_type = RT_TYPE_DOUBLE;
                aTypeAny >>= constValue.m_value.aDouble;
            }
            break;
        case TypeClass_STRING:
            {
                uConstTypeName = OUString( RTL_CONSTASCII_USTRINGPARAM("string") );
                constValue.m_type = RT_TYPE_STRING;
                constValue.m_value.aString = ((OUString*)aTypeAny.getValue())->getStr();
            }
            break;
    }

    writer.setFieldData(1, uConstName, uConstTypeName, OUString(),
                        OUString(), RT_ACCESS_CONST, constValue);

    sal_uInt32 length = writer.getBlopSize();
    *pBlop = (sal_uInt8*)rtl_allocateMemory( length );
    rtl_copyMemory(*pBlop, writer.getBlop(), length);

    return length;
}

sal_uInt32 getInheritedMemberCount( Reference< XTypeDescription >& xType )
{
    sal_uInt32 memberCount = 0;
    if ( xType->getTypeClass() == TypeClass_INTERFACE )
    {
        Reference< XInterfaceTypeDescription > xIFace(xType, UNO_QUERY);

        if ( !xIFace.is() )
            return memberCount;

        Reference< XTypeDescription > xSuperType = xIFace->getBaseType();

        if ( xSuperType.is() )
            memberCount = getInheritedMemberCount( xSuperType );

        memberCount += xIFace->getMembers().getLength();
    }
//  } else
//  if ( xType->getTypeClass() == TypeClass_Struct || xType->getTypeClass() == TypeClass_Exception )
//  {
//      Reference< XCompoundTypeDescription > xComp(xType, UNO_QUERY);
//
//      if ( xComp.is() )
//          return membercount;
//
//      Reference< XTypeDescription > xSuperType = xComp->getBaseType();
//
//      if ( xSuperType.is() )
//          memberCount = getInheritedMemberCount( xSuperType );
//
//      memberCount += xComp->getMemberNames().getLength();
//  }

    return memberCount;
}

void writeMethodData( RegistryTypeWriter& rWriter, sal_uInt32 inheritedMmeberCount,
                      const Reference< XInterfaceMemberTypeDescription >& xMember,
                      const Reference< XInterfaceMethodTypeDescription >& xMethod )
{
    RTMethodMode methodMode = RT_MODE_TWOWAY;
    if ( xMethod->isOneway() )
    {
        methodMode = RT_MODE_ONEWAY;
    }

    Sequence< Reference< XMethodParameter > > parameters( xMethod->getParameters() );
    Sequence< Reference< XTypeDescription > > exceptions( xMethod->getExceptions() );
    sal_uInt32 methodIndex = xMember->getPosition() - inheritedMmeberCount;
    sal_uInt32 paramCount = parameters.getLength();
    sal_uInt32 exceptionCount = exceptions.getLength();

    rWriter.setMethodData(methodIndex, xMember->getMemberName(),
                          xMethod->getReturnType()->getName().replace('.', '/'),
                          methodMode, paramCount, exceptionCount, OUString());

    RTParamMode paramMode = RT_PARAM_IN;
    for (sal_Int32 i=0; i < paramCount; i++)
    {
        Reference< XMethodParameter > xParam = parameters[i];
        if ( xParam->isIn() && xParam->isOut())
        {
            paramMode = RT_PARAM_INOUT;
        } else
        if ( xParam->isIn() )
        {
            paramMode = RT_PARAM_IN;
        } else
        if ( xParam->isOut() )
        {
            paramMode = RT_PARAM_OUT;
        }

        rWriter.setParamData(methodIndex, xParam->getPosition(), xParam->getType()->getName().replace('.', '/'),
                             xParam->getName(), paramMode);
    }

    for (i=0; i < exceptionCount; i++)
    {
        rWriter.setExcData(methodIndex, i, exceptions[i]->getName().replace('.', '/'));
    }
}

extern "C"
{

sal_Bool SAL_CALL initTypeMapper( const sal_Char* pRegName )
{
    try
    {
        if (!pRegName)
            return sal_False;

        Reference< XMultiServiceFactory > xSMgr( createRegistryServiceFactory( OUString::createFromAscii(pRegName) ) );

        if ( !xSMgr.is() )
            return sal_False;

        Reference< XInterface > xIFace( xSMgr->createInstance(
                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.TypeDescriptionManager"))) );

        if ( !xIFace.is() )
            return sal_False;

        Reference< XHierarchicalNameAccess > xNAccess(xIFace, UNO_QUERY);

        if ( !xNAccess.is() )
            return sal_False;

        xNameAccess = xNAccess;
    }
    catch( Exception& )
    {
        return sal_False;
    }

    return sal_True;
}

sal_uInt32 SAL_CALL getTypeBlop(const sal_Char* pTypeName, sal_uInt8** pBlop)
{
    sal_uInt32 length = 0;

    if ( !pTypeName )
        return length;

    OUString uTypeName( OUString::createFromAscii(pTypeName).replace('/', '.') );
    try
    {
        Any aTypeAny( xNameAccess->getByHierarchicalName( uTypeName ) );

        if ( !aTypeAny.hasValue() )
            return length;

        RegistryTypeWriterLoader rtwLoader;
        if ( !rtwLoader.isLoaded() )
            return length;

        if ( aTypeAny.getValueTypeClass() != TypeClass_INTERFACE )
        {
            return writeConstantData( rtwLoader, uTypeName, aTypeAny, pBlop );
        }

        Reference< XTypeDescription > xType;
        aTypeAny >>= xType;

        if ( !xType.is() )
            return length;

        switch (xType->getTypeClass())
        {
            case TypeClass_INTERFACE:
                {
                    Reference< XInterfaceTypeDescription > xIFace(xType, UNO_QUERY);

                    if ( !xIFace.is() )
                        return length;

                    Reference< XInterfaceAttributeTypeDescription > xAttr;
                    Reference< XInterfaceMethodTypeDescription > xMethod;
                    Sequence< Reference< XInterfaceMemberTypeDescription > > memberTypes( xIFace->getMembers());
                    sal_uInt32 memberCount = memberTypes.getLength();
                    sal_uInt32 attrCount = 0;
                    sal_uInt32 inheritedMemberCount = 0;

                    for (sal_Int32 i=0; i < memberCount; i++)
                    {
                        xAttr = Reference< XInterfaceAttributeTypeDescription >(memberTypes[i], UNO_QUERY);
                        if ( xAttr.is() )
                        {
                            attrCount++;
                        }
                    }

                    OUString uSuperType;
                    Reference< XTypeDescription > xSuperType = xIFace->getBaseType();
                    if ( xSuperType.is() )
                    {
                        uSuperType = xSuperType->getName().replace('.','/');
                        inheritedMemberCount = getInheritedMemberCount( xSuperType );
                    }

                    RegistryTypeWriter writer(rtwLoader, RT_TYPE_INTERFACE, uTypeName.replace('.', '/'),
                                              uSuperType, attrCount, memberCount-attrCount, 0);

                    Uik   uik = xIFace->getUik();
                    RTUik rtUik = { uik.m_Data1, uik.m_Data2, uik.m_Data3, uik.m_Data4, uik.m_Data5 };
                    writer.setUik( rtUik );

                    RTFieldAccess attrAccess = RT_ACCESS_READWRITE;
                    for (i=0; i < memberCount; i++)
                    {
                        xAttr = Reference< XInterfaceAttributeTypeDescription >(memberTypes[i], UNO_QUERY);
                        if ( xAttr.is() )
                        {
                            if (xAttr->isReadOnly())
                            {
                                attrAccess = RT_ACCESS_READONLY;
                            } else
                            {
                                attrAccess = RT_ACCESS_READWRITE;
                            }
                            writer.setFieldData(memberTypes[i]->getPosition() - inheritedMemberCount,
                                                memberTypes[i]->getMemberName(),
                                                xAttr->getType()->getName().replace('.', '/'),
                                                OUString(), OUString(), attrAccess);
                            continue;
                        }

                        xMethod = Reference< XInterfaceMethodTypeDescription >(memberTypes[i], UNO_QUERY);
                        if ( xMethod.is() )
                        {
                            writeMethodData( writer, inheritedMemberCount, memberTypes[i], xMethod );
                        }
                    }

                    length = writer.getBlopSize();
                    *pBlop = (sal_uInt8*)rtl_allocateMemory( length );
                    rtl_copyMemory(*pBlop, writer.getBlop(), length);
                }
                break;
            case TypeClass_STRUCT:
            case TypeClass_EXCEPTION:
                {
                    RTTypeClass rtTypeClass = RT_TYPE_STRUCT;
                    if (xType->getTypeClass() == TypeClass_EXCEPTION)
                    {
                        rtTypeClass = RT_TYPE_EXCEPTION;
                    }

                    Reference< XCompoundTypeDescription > xComp(xType, UNO_QUERY);

                    if ( !xComp.is() )
                        return length;

                    Sequence< OUString > memberNames( xComp->getMemberNames());
                    Sequence< Reference< XTypeDescription > > memberTypes( xComp->getMemberTypes());
                    sal_uInt32 memberCount = memberNames.getLength();

                    OUString uSuperType;
                    Reference< XTypeDescription > xSuperType = xComp->getBaseType();
                    if ( xSuperType.is() )
                    {
                        uSuperType = xSuperType->getName().replace('.','/');
                    }

                    RegistryTypeWriter writer(rtwLoader, rtTypeClass, uTypeName.replace('.', '/'),
                                              uSuperType, memberCount, 0, 0);

                    for (sal_Int32 i=0; i < memberCount; i++)
                    {
                        writer.setFieldData(i , memberNames[i], memberTypes[i]->getName().replace('.', '/'),
                                            OUString(), OUString(), RT_ACCESS_READWRITE);
                    }

                    length = writer.getBlopSize();
                    *pBlop = (sal_uInt8*)rtl_allocateMemory( length );
                    rtl_copyMemory(*pBlop, writer.getBlop(), length);
                }
                break;
            case TypeClass_ENUM:
                {
                    Reference< XEnumTypeDescription > xEnum(xType, UNO_QUERY);

                    if ( !xEnum.is() )
                        return length;

                    Sequence< OUString > enumNames( xEnum->getEnumNames());
                    Sequence< sal_Int32 > enumValues( xEnum->getEnumValues());
                    sal_uInt32 enumCount = enumNames.getLength();

                    RegistryTypeWriter writer(rtwLoader, RT_TYPE_ENUM, uTypeName.replace('.', '/'),
                                              OUString(), enumCount, 0, 0);

                    RTConstValue constValue;
                    for (sal_Int32 i=0; i < enumCount; i++)
                    {
                        constValue.m_type = RT_TYPE_INT32;
                        constValue.m_value.aLong = enumValues[i];

                        writer.setFieldData(i, enumNames[i], OUString(), OUString(), OUString(),
                                            RT_ACCESS_CONST, constValue);
                    }

                    length = writer.getBlopSize();
                    *pBlop = (sal_uInt8*)rtl_allocateMemory( length );
                    rtl_copyMemory(*pBlop, writer.getBlop(), length);
                }
                break;
            case TypeClass_TYPEDEF:
                {
                    Reference< XIndirectTypeDescription > xTD(xType, UNO_QUERY);

                    if ( !xTD.is() )
                        return length;

                    RegistryTypeWriter writer(rtwLoader, RT_TYPE_TYPEDEF, uTypeName.replace('.', '/'),
                                              xTD->getReferencedType()->getName().replace('.', '/'),
                                              0, 0, 0);
                    length = writer.getBlopSize();
                    *pBlop = (sal_uInt8*)rtl_allocateMemory( length );
                    rtl_copyMemory(*pBlop, writer.getBlop(), length);
                }
                break;
        }

    }
    catch( Exception& )
    {
    }

    return length;
}

} // extern "C"



