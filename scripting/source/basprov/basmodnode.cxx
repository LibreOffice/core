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

#include "basmodnode.hxx"
#include "basmethnode.hxx"
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <vcl/svapp.hxx>
#include <basic/sbx.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;


namespace basprov
{


    // BasicModuleNodeImpl


    BasicModuleNodeImpl::BasicModuleNodeImpl( const Reference< XComponentContext >& rxContext,
        const OUString& sScriptingContext, SbModule* pModule, bool isAppScript )
        :m_xContext( rxContext )
        ,m_sScriptingContext( sScriptingContext )
        ,m_pModule( pModule )
        ,m_bIsAppScript( isAppScript )
    {
    }


    BasicModuleNodeImpl::~BasicModuleNodeImpl()
    {
    }


    // XBrowseNode


    OUString BasicModuleNodeImpl::getName(  )
    {
        SolarMutexGuard aGuard;

        OUString sModuleName;
        if ( m_pModule )
            sModuleName = m_pModule->GetName();

        return sModuleName;
    }


    Sequence< Reference< browse::XBrowseNode > > BasicModuleNodeImpl::getChildNodes(  )
    {
        SolarMutexGuard aGuard;

        Sequence< Reference< browse::XBrowseNode > > aChildNodes;

        if ( m_pModule )
        {
            SbxArray* pMethods = m_pModule->GetMethods().get();
            if ( pMethods )
            {
                sal_Int32 nCount = pMethods->Count();
                sal_Int32 nRealCount = 0;
                for ( sal_Int32 i = 0; i < nCount; ++i )
                {
                    SbMethod* pMethod = static_cast< SbMethod* >( pMethods->Get( static_cast< sal_uInt16 >( i ) ) );
                    if ( pMethod && !pMethod->IsHidden() )
                        ++nRealCount;
                }
                aChildNodes.realloc( nRealCount );
                Reference< browse::XBrowseNode >* pChildNodes = aChildNodes.getArray();

                sal_Int32 iTarget = 0;
                for ( sal_Int32 i = 0; i < nCount; ++i )
                {
                    SbMethod* pMethod = static_cast< SbMethod* >( pMethods->Get( static_cast< sal_uInt16 >( i ) ) );
                    if ( pMethod && !pMethod->IsHidden() )
                        pChildNodes[iTarget++] = new BasicMethodNodeImpl(
                            m_xContext, m_sScriptingContext, pMethod, m_bIsAppScript);
                }
            }
        }

        return aChildNodes;
    }


    sal_Bool BasicModuleNodeImpl::hasChildNodes(  )
    {
        SolarMutexGuard aGuard;

        bool bReturn = false;
        if ( m_pModule )
        {
            SbxArray* pMethods = m_pModule->GetMethods().get();
            if ( pMethods && pMethods->Count() > 0 )
                bReturn = true;
        }

        return bReturn;
    }


    sal_Int16 BasicModuleNodeImpl::getType(  )
    {
        SolarMutexGuard aGuard;

        return browse::BrowseNodeTypes::CONTAINER;
    }


}   // namespace basprov


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
