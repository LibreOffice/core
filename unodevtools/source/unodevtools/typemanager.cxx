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

#include "unodevtools/typemanager.hxx"

#include "rtl/alloc.h"
#include "registry/reader.hxx"
#include "cppuhelper/bootstrap.hxx"

#include "com/sun/star/container/XSet.hpp"
#include "com/sun/star/reflection/XTypeDescription.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"

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
    case TypeClass_TYPEDEF:
        return RT_TYPE_TYPEDEF;
    case TypeClass_STRUCT:
        return RT_TYPE_STRUCT;
    case TypeClass_UNION:
        return RT_TYPE_UNION;
    case TypeClass_EXCEPTION:
        return RT_TYPE_EXCEPTION;
    case TypeClass_INTERFACE:
        return RT_TYPE_INTERFACE;
    case TypeClass_SERVICE:
        return RT_TYPE_SERVICE;
    case TypeClass_MODULE:
        return RT_TYPE_MODULE;
    case TypeClass_CONSTANTS:
        return RT_TYPE_CONSTANTS;
    case TypeClass_SINGLETON:
        return RT_TYPE_SINGLETON;
    default:
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
        OUString msg(
            "internal UNO problem, can't create initial UNO component context");
        throw RuntimeException( msg, Reference< XInterface >());
    }
    Any a = xContext->getValueByName(
        OUString(
            "/singletons/com.sun.star.reflection.theTypeDescriptionManager"));

    a >>= m_pImpl->m_tdmgr;

    if ( !m_pImpl->m_tdmgr.is() ) {
        OUString msg("internal UNO problem, can't get TypeDescriptionManager");
        throw RuntimeException( msg, Reference< XInterface >());
    }

    if ( !registries.empty() ) {

        Reference< XMultiComponentFactory > xServiceManager(
            xContext->getServiceManager() );
        if ( !xServiceManager.is() ) {
            OUString msg("internal UNO problem, can't get ServiceManager");
            throw RuntimeException( msg, Reference< XInterface >());
        }

        Sequence<Any> seqArgs(registries.size());

        std::vector< OUString >::const_iterator iter = registries.begin();
        int i = 0;
        while ( iter != registries.end() )
        {
            Reference< XSimpleRegistry > xReg(
                xServiceManager->createInstanceWithContext(
                    OUString("com.sun.star.registry.SimpleRegistry"),
                    xContext), UNO_QUERY);
            xReg->open(convertToFileUrl(
                           OUStringToOString(*iter, RTL_TEXTENCODING_UTF8)),
                           sal_True, sal_False);

            seqArgs[i++] = makeAny(xReg);
            ++iter;
        }

        Reference< XHierarchicalNameAccess > xTDProvider(
            xServiceManager->createInstanceWithArgumentsAndContext(
                OUString("com.sun.star.reflection.TypeDescriptionProvider"),
                seqArgs, xContext),
            UNO_QUERY);
        if ( !xTDProvider.is() ) {
            OUString msg("internal UNO problem, can't create local"
                         " type description provider");
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
    if (typeName.indexOf(sBase) == 0) {
        typeName = typeName.copy(typeName.indexOf('/', 1) + 1);
    } else {
        typeName = typeName.copy(1);
    }
    return typeName;
}

// extern
void* getTypeBlob(Reference< XHierarchicalNameAccess > xTDmgr,
                  const OString& typeName, sal_uInt32* pBlob);

typereg::Reader UnoTypeManager::getTypeReader(
    const OString& name, sal_Bool * /*pIsExtraType*/ ) const
{
    typereg::Reader reader;

    void* pBlob = NULL;
    sal_uInt32 blobsize = 0;

    if ( (pBlob = getTypeBlob(m_pImpl->m_tdmgr, name, &blobsize)) != NULL )
        reader = typereg::Reader(pBlob, blobsize, sal_True, TYPEREG_VERSION_1);

    if ( pBlob )
        rtl_freeMemory(pBlob);

    return reader;
}

typereg::Reader UnoTypeManager::getTypeReader(RegistryKey& rTypeKey) const
{
    typereg::Reader reader;

    if (rTypeKey.isValid()) {
        RegValueType    valueType;
        sal_uInt32      valueSize;

        if (!rTypeKey.getValueInfo(OUString(), &valueType, &valueSize)) {
            sal_uInt8*  pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
            if ( !rTypeKey.getValue(OUString(), pBuffer) ) {
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
    if ( m_pImpl->m_t2TypeClass.count(name) > 0 ) {
        return m_pImpl->m_t2TypeClass[name];
    } else {
        Reference< XTypeDescription > xTD;
        Any a = m_pImpl->m_tdmgr->getByHierarchicalName(
            OStringToOUString(name, RTL_TEXTENCODING_UTF8));
        a >>= xTD;

        if ( xTD.is() ) {
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

    if ( m_pImpl->m_t2TypeClass.count(name) > 0 ) {
        return m_pImpl->m_t2TypeClass[name];
    } else {
        if ( rTypeKey.isValid() ) {
            RegValueType    valueType;
            sal_uInt32      valueSize;

            if ( !rTypeKey.getValueInfo(OUString(), &valueType, &valueSize) ) {
                sal_uInt8*  pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
                if ( !rTypeKey.getValue(OUString(), pBuffer) ) {
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
