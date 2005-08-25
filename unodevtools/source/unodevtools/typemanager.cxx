/*************************************************************************
 *
 *  $RCSfile: typemanager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jsc $ $Date: 2005-08-25 15:30:34 $
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


#include "unodevtools/typemanager.hxx"

#include <rtl/alloc.h>
#include <registry/reader.hxx>
//#include <registry/version.h>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/reflection/XTypeDescription.hpp>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::reflection;

namespace unodevtools {

static RTTypeClass mapTypeClass(TypeClass typeclass) {
    switch(typeclass) {
    case TypeClass_ENUM:
        return RT_TYPE_ENUM;
        break;
    case TypeClass_TYPEDEF:
        return RT_TYPE_TYPEDEF;
        break;
    case TypeClass_STRUCT:
        return RT_TYPE_STRUCT;
        break;
    case TypeClass_UNION:
        return RT_TYPE_UNION;
        break;
    case TypeClass_EXCEPTION:
        return RT_TYPE_EXCEPTION;
        break;
    case TypeClass_INTERFACE:
        return RT_TYPE_INTERFACE;
        break;
    case TypeClass_SERVICE:
        return RT_TYPE_SERVICE;
        break;
    case TypeClass_MODULE:
        return RT_TYPE_MODULE;
        break;
    case TypeClass_CONSTANTS:
        return RT_TYPE_CONSTANTS;
        break;
    case TypeClass_SINGLETON:
        return RT_TYPE_SINGLETON;
        break;
    }
    return RT_TYPE_INVALID;
}


UnoTypeManager::UnoTypeManager()
{
    m_pImpl = new UnoTypeManagerImpl();
    acquire();
}

UnoTypeManager::~UnoTypeManager()
{
    release();
}

void UnoTypeManager::release()
{
    if (0 == TypeManager::release())
        delete m_pImpl;
}

sal_Bool UnoTypeManager::init(
    const ::std::vector< ::rtl::OUString > registries)
{
    Reference< XComponentContext > xContext=
        defaultBootstrap_InitialComponentContext();

    if ( !xContext.is() ) {
        OUString msg(RTL_CONSTASCII_USTRINGPARAM(
            "internal UNO problem, can't create initial UNO component context"));
        throw RuntimeException( msg, Reference< XInterface >());
    }
    Any a = xContext->getValueByName(
        OUString(RTL_CONSTASCII_USTRINGPARAM(
            "/singletons/com.sun.star.reflection.theTypeDescriptionManager")));

    a >>= m_pImpl->m_tdmgr;

    if ( !m_pImpl->m_tdmgr.is() ) {
        OUString msg(RTL_CONSTASCII_USTRINGPARAM(
            "internal UNO problem, can't get TypeDescriptionManager"));
        throw RuntimeException( msg, Reference< XInterface >());
    }

    if ( !registries.empty() ) {

        Reference< XMultiComponentFactory > xServiceManager(
            xContext->getServiceManager() );
        if ( !xServiceManager.is() ) {
            OUString msg(RTL_CONSTASCII_USTRINGPARAM(
                             "internal UNO problem, can't get ServiceManager"));
            throw RuntimeException( msg, Reference< XInterface >());
        }

        Sequence<Any> seqArgs(registries.size());

        std::vector< OUString >::const_iterator iter = registries.begin();
        int i = 0;
        while (iter != registries.end()) {
            Reference< XSimpleRegistry > xReg(
                xServiceManager->createInstanceWithContext(
                    OUString(RTL_CONSTASCII_USTRINGPARAM(
                             "com.sun.star.registry.SimpleRegistry")),
                    xContext), UNO_QUERY);
            xReg->open(convertToFileUrl(
                           OUStringToOString(*iter, RTL_TEXTENCODING_UTF8)),
                           sal_True, sal_False);

            seqArgs[i++] = makeAny(xReg);
            iter++;
        }

        Reference< XHierarchicalNameAccess > xTDProvider(
            xServiceManager->createInstanceWithArgumentsAndContext(
                OUString(RTL_CONSTASCII_USTRINGPARAM(
                             "com.sun.star.reflection.TypeDescriptionProvider")),
                seqArgs, xContext),
            UNO_QUERY);
        if ( !xTDProvider.is() ) {
            OUString msg(RTL_CONSTASCII_USTRINGPARAM(
                             "internal UNO problem, can't create type local"
                             " type description provider"));
            throw RuntimeException( msg, Reference< XInterface >());
        }

        a = makeAny(xTDProvider);
        Reference< XSet > xSet(m_pImpl->m_tdmgr, UNO_QUERY);
        xSet->insert(a);
    }

    return sal_True;
}

sal_Bool UnoTypeManager::isValidType(const ::rtl::OString& name) const
{
    return m_pImpl->m_tdmgr->hasByHierarchicalName(
        OStringToOUString(name, RTL_TEXTENCODING_UTF8));
}

OString UnoTypeManager::getTypeName(RegistryKey& rTypeKey) const
{
    OString typeName = OUStringToOString(rTypeKey.getName(), RTL_TEXTENCODING_UTF8);
    static OString sBase("/UCR");
    if (typeName.indexOf(sBase) == 0)
        typeName = typeName.copy(typeName.indexOf('/', 1) + 1);
    else
        typeName = typeName.copy(1);

    return typeName;
}

// extern
void* getTypeBlob(Reference< XHierarchicalNameAccess > xTDmgr,
                  const OString& typeName, sal_uInt32* pBlob);

typereg::Reader UnoTypeManager::getTypeReader(
    const OString& name, sal_Bool * pIsExtraType ) const
{
    typereg::Reader reader;

    void* pBlob = NULL;
    sal_uInt32 blobsize = 0;

    if ( (pBlob = getTypeBlob(m_pImpl->m_tdmgr, name, &blobsize)) != NULL )
    {
        reader = typereg::Reader(pBlob, blobsize, sal_True, TYPEREG_VERSION_1);
    }

    if ( pBlob )
    {
        rtl_freeMemory(pBlob);
    }

    return reader;
}

typereg::Reader UnoTypeManager::getTypeReader(RegistryKey& rTypeKey) const
{
    typereg::Reader reader;

    if (rTypeKey.isValid())
    {
        RegValueType    valueType;
        sal_uInt32      valueSize;

        if (!rTypeKey.getValueInfo(OUString(), &valueType, &valueSize))
        {
            sal_uInt8*  pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
            if (!rTypeKey.getValue(OUString(), pBuffer))
            {
                reader = typereg::Reader(
                    pBuffer, valueSize, true, TYPEREG_VERSION_1);
            }
            rtl_freeMemory(pBuffer);
        }
    }
    return reader;
}


RTTypeClass UnoTypeManager::getTypeClass(const OString& name) const
{
    if (m_pImpl->m_t2TypeClass.count(name) > 0)
    {
        return m_pImpl->m_t2TypeClass[name];
    } else
    {
        Reference< XTypeDescription > xTD;
        Any a = m_pImpl->m_tdmgr->getByHierarchicalName(
            OStringToOUString(name, RTL_TEXTENCODING_UTF8));
        a >>= xTD;

        if ( xTD.is())
        {
            RTTypeClass tc = mapTypeClass(xTD->getTypeClass());
            if (tc != RT_TYPE_INVALID)
                m_pImpl->m_t2TypeClass[name] = tc;
            return tc;
        }
    }

    return RT_TYPE_INVALID;
}

RTTypeClass UnoTypeManager::getTypeClass(RegistryKey& rTypeKey) const
{
    OString name = getTypeName(rTypeKey);

    if (m_pImpl->m_t2TypeClass.count(name) > 0)
    {
        return m_pImpl->m_t2TypeClass[name];
    } else
    {
        if (rTypeKey.isValid())
        {
            RegValueType    valueType;
            sal_uInt32      valueSize;

            if (!rTypeKey.getValueInfo(OUString(), &valueType, &valueSize))
            {
                sal_uInt8*  pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
                if (!rTypeKey.getValue(OUString(), pBuffer))
                {
                    typereg::Reader reader(
                        pBuffer, valueSize, false, TYPEREG_VERSION_1);

                    RTTypeClass ret = reader.getTypeClass();

                    rtl_freeMemory(pBuffer);

                    m_pImpl->m_t2TypeClass[name] = ret;
                    return ret;
                }
                rtl_freeMemory(pBuffer);
            }
        }
    }

    return RT_TYPE_INVALID;
}

} // end of namespace unodevtools
