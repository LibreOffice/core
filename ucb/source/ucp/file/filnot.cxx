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

#include <com/sun/star/ucb/ContentAction.hpp>
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#include <rtl/ref.hxx>
#include "filnot.hxx"
#include "filid.hxx"
#include "bc.hxx"
#include "prov.hxx"


using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;


ContentEventNotifier::ContentEventNotifier( TaskManager* pMyShell,
                                            const uno::Reference< XContent >& xCreatorContent,
                                            const uno::Reference< XContentIdentifier >& xCreatorId,
                                            std::vector< uno::Reference< uno::XInterface > >&& sListeners )
    : m_pMyShell( pMyShell ),
      m_xCreatorContent( xCreatorContent ),
      m_xCreatorId( xCreatorId ),
      m_sListeners( std::move(sListeners) )
{
}


ContentEventNotifier::ContentEventNotifier( TaskManager* pMyShell,
                                            const uno::Reference< XContent >& xCreatorContent,
                                            const uno::Reference< XContentIdentifier >& xCreatorId,
                                            const uno::Reference< XContentIdentifier >& xOldId,
                                            std::vector< uno::Reference< uno::XInterface > >&& sListeners )
    : m_pMyShell( pMyShell ),
      m_xCreatorContent( xCreatorContent ),
      m_xCreatorId( xCreatorId ),
      m_xOldId( xOldId ),
      m_sListeners( std::move(sListeners) )
{
}


void ContentEventNotifier::notifyChildInserted( const OUString& aChildName ) const
{
    rtl::Reference<FileContentIdentifier> xChildId = new FileContentIdentifier( aChildName );

    uno::Reference< XContent > xChildContent = m_pMyShell->m_pProvider->queryContent( xChildId );

    ContentEvent aEvt( m_xCreatorContent,
                       ContentAction::INSERTED,
                       xChildContent,
                       m_xCreatorId );

    for( const auto& r : m_sListeners )
    {
        uno::Reference< XContentEventListener > ref( r, uno::UNO_QUERY );
        if( ref.is() )
            ref->contentEvent( aEvt );
    }
}

void ContentEventNotifier::notifyDeleted() const
{

    ContentEvent aEvt( m_xCreatorContent,
                       ContentAction::DELETED,
                       m_xCreatorContent,
                       m_xCreatorId );


    for( const auto& r : m_sListeners )
    {
        uno::Reference< XContentEventListener > ref( r, uno::UNO_QUERY );
        if( ref.is() )
            ref->contentEvent( aEvt );
    }
}


void ContentEventNotifier::notifyRemoved( const OUString& aChildName ) const
{
    rtl::Reference<FileContentIdentifier> xChildId = new FileContentIdentifier( aChildName );

    rtl::Reference<BaseContent> pp = new BaseContent( m_pMyShell,xChildId,aChildName );
    {
        osl::MutexGuard aGuard( pp->m_aMutex );
        pp->m_nState |= BaseContent::Deleted;
    }

    ContentEvent aEvt( m_xCreatorContent,
                       ContentAction::REMOVED,
                       pp,
                       m_xCreatorId );

    for( const auto& r : m_sListeners )
    {
        uno::Reference< XContentEventListener > ref( r, uno::UNO_QUERY );
        if( ref.is() )
            ref->contentEvent( aEvt );
    }
}

void ContentEventNotifier::notifyExchanged() const
{
    ContentEvent aEvt( m_xCreatorContent,
                       ContentAction::EXCHANGED,
                       m_xCreatorContent,
                       m_xOldId );

    for( const auto& r : m_sListeners )
    {
        uno::Reference< XContentEventListener > ref( r, uno::UNO_QUERY );
        if( ref.is() )
            ref->contentEvent( aEvt );
    }
}

/*********************************************************************************/
/*                                                                               */
/*                      PropertySetInfoChangeNotifier                            */
/*                                                                               */
/*********************************************************************************/


PropertySetInfoChangeNotifier::PropertySetInfoChangeNotifier(
    const uno::Reference< XContent >& xCreatorContent,
    std::vector< uno::Reference< uno::XInterface > >&& sListeners )
    : m_xCreatorContent( xCreatorContent ),
      m_sListeners( std::move(sListeners) )
{

}


void
PropertySetInfoChangeNotifier::notifyPropertyAdded( const OUString & aPropertyName ) const
{
    beans::PropertySetInfoChangeEvent aEvt( m_xCreatorContent,
                                            aPropertyName,
                                            -1,
                                            beans::PropertySetInfoChange::PROPERTY_INSERTED );

    for( const auto& r : m_sListeners )
    {
        uno::Reference< beans::XPropertySetInfoChangeListener > ref( r, uno::UNO_QUERY );
        if( ref.is() )
            ref->propertySetInfoChange( aEvt );
    }
}


void
PropertySetInfoChangeNotifier::notifyPropertyRemoved( const OUString & aPropertyName ) const
{
    beans::PropertySetInfoChangeEvent aEvt( m_xCreatorContent,
                                            aPropertyName,
                                            -1,
                                            beans::PropertySetInfoChange::PROPERTY_REMOVED );

    for( const auto& r : m_sListeners )
    {
        uno::Reference< beans::XPropertySetInfoChangeListener > ref( r, uno::UNO_QUERY );
        if( ref.is() )
            ref->propertySetInfoChange( aEvt );
    }
}


/*********************************************************************************/
/*                                                                               */
/*                      PropertySetInfoChangeNotifier                            */
/*                                                                               */
/*********************************************************************************/


PropertyChangeNotifier::PropertyChangeNotifier(
    const css::uno::Reference< XContent >& xCreatorContent,
    ListenerMap&& pListeners )
    : m_xCreatorContent( xCreatorContent ),
      m_aListeners( std::move(pListeners) )
{
}


void PropertyChangeNotifier::notifyPropertyChanged(
    const uno::Sequence< beans::PropertyChangeEvent >& seqChanged ) const
{
    uno::Sequence< beans::PropertyChangeEvent > Changes  = seqChanged;

    for( auto& rChange : asNonConstRange(Changes) )
        rChange.Source = m_xCreatorContent;

    // notify listeners for all Events

    auto it = m_aListeners.find( OUString() );
    if (it != m_aListeners.end())
    {
        const std::vector< uno::Reference< uno::XInterface > >& seqList = it->second;
        for( const auto& rListener : seqList )
        {
            uno::Reference< beans::XPropertiesChangeListener > aListener( rListener,uno::UNO_QUERY );
            if( aListener.is() )
            {
                aListener->propertiesChange( Changes );
            }
        }
    }

    for( const auto& rChange : std::as_const(Changes) )
    {
        uno::Sequence< beans::PropertyChangeEvent > seq{ rChange };
        it = m_aListeners.find( rChange.PropertyName );
        if (it != m_aListeners.end())
        {
            const std::vector< uno::Reference< uno::XInterface > >& seqList = it->second;
            for( const auto& rListener : seqList )
            {
                uno::Reference< beans::XPropertiesChangeListener > aListener( rListener,uno::UNO_QUERY );
                if( aListener.is() )
                {
                    aListener->propertiesChange( seq );
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
