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

#include "baslibnode.hxx"
#include "basmodnode.hxx"
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <comphelper/sequence.hxx>
#include <vcl/svapp.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbstar.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;


namespace basprov
{


    // BasicLibraryNodeImpl


    BasicLibraryNodeImpl::BasicLibraryNodeImpl( const Reference< XComponentContext >& rxContext,
         const OUString& sScriptingContext, BasicManager* pBasicManager,
        const Reference< script::XLibraryContainer >& xLibContainer, const OUString& sLibName, bool isAppScript )
        :m_xContext( rxContext )
    ,m_sScriptingContext( sScriptingContext )
        ,m_pBasicManager( pBasicManager )
        ,m_xLibContainer( xLibContainer )
        ,m_sLibName( sLibName )
        ,m_bIsAppScript( isAppScript )
    {
        if ( m_xLibContainer.is() )
        {
            Any aElement = m_xLibContainer->getByName( m_sLibName );
            aElement >>= m_xLibrary;
        }
    }


    BasicLibraryNodeImpl::~BasicLibraryNodeImpl()
    {
    }


    // XBrowseNode


    OUString BasicLibraryNodeImpl::getName(  )
    {
        SolarMutexGuard aGuard;

        return m_sLibName;
    }


    Sequence< Reference< browse::XBrowseNode > > BasicLibraryNodeImpl::getChildNodes(  )
    {
        SolarMutexGuard aGuard;

        std::vector< Reference< browse::XBrowseNode > > aChildNodes;

        if ( m_xLibContainer.is() && m_xLibContainer->hasByName( m_sLibName ) && !m_xLibContainer->isLibraryLoaded( m_sLibName ) )
            m_xLibContainer->loadLibrary( m_sLibName );

        if ( m_pBasicManager )
        {
            StarBASIC* pBasic = m_pBasicManager->GetLib( m_sLibName );
            if ( pBasic && m_xLibrary.is() )
            {
                Sequence< OUString > aNames = m_xLibrary->getElementNames();
                sal_Int32 nCount = aNames.getLength();
                const OUString* pNames = aNames.getConstArray();
                aChildNodes.resize( nCount );

                for ( sal_Int32 i = 0 ; i < nCount ; ++i )
                {
                    SbModule* pModule = pBasic->FindModule( pNames[i] );
                    if ( pModule )
                        aChildNodes[i] = new BasicModuleNodeImpl(m_xContext, m_sScriptingContext,
                                                                 pModule, m_bIsAppScript);
                }
            }
        }

        return comphelper::containerToSequence(aChildNodes);
    }


    sal_Bool BasicLibraryNodeImpl::hasChildNodes(  )
    {
        SolarMutexGuard aGuard;

        bool bReturn = false;
        if ( m_xLibrary.is() )
            bReturn = m_xLibrary->hasElements();

        return bReturn;
    }


    sal_Int16 BasicLibraryNodeImpl::getType(  )
    {
        SolarMutexGuard aGuard;

        return browse::BrowseNodeTypes::CONTAINER;
    }


}   // namespace basprov


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
