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

#include <cppuhelper/factory.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/container/XSet.hpp>
#include <osl/diagnose.h>

#include "source.hxx"
#include "target.hxx"

using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::registry  ;
using namespace ::cppu                      ;
using namespace ::com::sun::star::lang;

Reference< XInterface > SAL_CALL createDragSource( const Reference< XMultiServiceFactory >& rServiceManager )
{
    DragSource* pSource= new DragSource( comphelper::getComponentContext(rServiceManager) );
    return Reference<XInterface>( static_cast<XInitialization*>(pSource), UNO_QUERY);
}

Reference< XInterface > SAL_CALL createDropTarget( const Reference< XMultiServiceFactory >& rServiceManager )
{
    DropTarget* pTarget= new DropTarget( comphelper::getComponentContext(rServiceManager) );
    return Reference<XInterface>( static_cast<XInitialization*>(pTarget), UNO_QUERY);
}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void* SAL_CALL
dnd_component_getFactory( const sal_Char* pImplName, void* pSrvManager, void* /*pRegistryKey*/ )
{
    void* pRet = 0;
    Reference< XSingleServiceFactory > xFactory;

    if ( pSrvManager && ( 0 == rtl_str_compare( pImplName, DNDSOURCE_IMPL_NAME ) ) )
    {
        Sequence< OUString > aSNS { DNDSOURCE_SERVICE_NAME };

        xFactory= createSingleFactory(
            reinterpret_cast< XMultiServiceFactory* > ( pSrvManager ),
            OUString::createFromAscii( pImplName ),
            createDragSource,
            aSNS);

    }
    else if( pSrvManager && ( 0 == rtl_str_compare( pImplName, DNDTARGET_IMPL_NAME ) ) )
    {
        Sequence< OUString > aSNS { DNDTARGET_SERVICE_NAME };

        xFactory= createSingleFactory(
            reinterpret_cast< XMultiServiceFactory* > ( pSrvManager ),
            OUString::createFromAscii( pImplName ),
            createDropTarget,
            aSNS);

    }

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
