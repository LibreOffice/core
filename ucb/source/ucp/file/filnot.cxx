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

#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/ContentAction.hpp>
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#include "filnot.hxx"
#include "filid.hxx"
#include "bc.hxx"
#include "prov.hxx"



using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;


ContentEventNotifier::ContentEventNotifier( shell* pMyShell,
                                            const uno::Reference< XContent >& xCreatorContent,
                                            const uno::Reference< XContentIdentifier >& xCreatorId,
                                            const uno::Sequence< uno::Reference< uno::XInterface > >& sListeners )
    : m_pMyShell( pMyShell ),
      m_xCreatorContent( xCreatorContent ),
      m_xCreatorId( xCreatorId ),
      m_sListeners( sListeners )
{
}


ContentEventNotifier::ContentEventNotifier( shell* pMyShell,
                                            const uno::Reference< XContent >& xCreatorContent,
                                            const uno::Reference< XContentIdentifier >& xCreatorId,
                                            const uno::Reference< XContentIdentifier >& xOldId,
                                            const uno::Sequence< uno::Reference< uno::XInterface > >& sListeners )
    : m_pMyShell( pMyShell ),
      m_xCreatorContent( xCreatorContent ),
      m_xCreatorId( xCreatorId ),
      m_xOldId( xOldId ),
      m_sListeners( sListeners )
{
}



void ContentEventNotifier::notifyChildInserted( const rtl::OUString& aChildName )
{
    FileContentIdentifier* p = new FileContentIdentifier( m_pMyShell,aChildName );
    uno::Reference< XContentIdentifier > xChildId( p );

    uno::Reference< XContent > xChildContent = m_pMyShell->m_pProvider->queryContent( xChildId );

    ContentEvent aEvt( m_xCreatorContent,
                       ContentAction::INSERTED,
                       xChildContent,
                       m_xCreatorId );

    for( sal_Int32 i = 0; i < m_sListeners.getLength(); ++i )
    {
        uno::Reference< XContentEventListener > ref( m_sListeners[i],uno::UNO_QUERY );
        if( ref.is() )
            ref->contentEvent( aEvt );
    }
}

void ContentEventNotifier::notifyDeleted( void )
{

    ContentEvent aEvt( m_xCreatorContent,
                       ContentAction::DELETED,
                       m_xCreatorContent,
                       m_xCreatorId );


    for( sal_Int32 i = 0; i < m_sListeners.getLength(); ++i )
    {
        uno::Reference< XContentEventListener > ref( m_sListeners[i],uno::UNO_QUERY );
        if( ref.is() )
            ref->contentEvent( aEvt );
    }
}



void ContentEventNotifier::notifyRemoved( const rtl::OUString& aChildName )
{
    FileContentIdentifier* p = new FileContentIdentifier( m_pMyShell,aChildName );
    uno::Reference< XContentIdentifier > xChildId( p );

    BaseContent* pp = new BaseContent( m_pMyShell,xChildId,aChildName );
    {
        osl::MutexGuard aGuard( pp->m_aMutex );
        pp->m_nState |= BaseContent::Deleted;
    }

    uno::Reference< XContent > xDeletedContent( pp );


    ContentEvent aEvt( m_xCreatorContent,
                       ContentAction::REMOVED,
                       xDeletedContent,
                       m_xCreatorId );

    for( sal_Int32 i = 0; i < m_sListeners.getLength(); ++i )
    {
        uno::Reference< XContentEventListener > ref( m_sListeners[i],uno::UNO_QUERY );
        if( ref.is() )
            ref->contentEvent( aEvt );
    }
}

void ContentEventNotifier::notifyExchanged()
{
    ContentEvent aEvt( m_xCreatorContent,
                       ContentAction::EXCHANGED,
                       m_xCreatorContent,
                       m_xOldId );

    for( sal_Int32 i = 0; i < m_sListeners.getLength(); ++i )
    {
        uno::Reference< XContentEventListener > ref( m_sListeners[i],uno::UNO_QUERY );
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
    const uno::Reference< XContentIdentifier >& xCreatorId,
    const uno::Sequence< uno::Reference< uno::XInterface > >& sListeners )
    : m_xCreatorContent( xCreatorContent ),
      m_xCreatorId( xCreatorId ),
      m_sListeners( sListeners )
{

}


void SAL_CALL
PropertySetInfoChangeNotifier::notifyPropertyAdded( const rtl::OUString & aPropertyName )
{
    beans::PropertySetInfoChangeEvent aEvt( m_xCreatorContent,
                                            aPropertyName,
                                            -1,
                                            beans::PropertySetInfoChange::PROPERTY_INSERTED );

    for( sal_Int32 i = 0; i < m_sListeners.getLength(); ++i )
    {
        uno::Reference< beans::XPropertySetInfoChangeListener > ref( m_sListeners[i],uno::UNO_QUERY );
        if( ref.is() )
            ref->propertySetInfoChange( aEvt );
    }
}


void SAL_CALL
PropertySetInfoChangeNotifier::notifyPropertyRemoved( const rtl::OUString & aPropertyName )
{
    beans::PropertySetInfoChangeEvent aEvt( m_xCreatorContent,
                                            aPropertyName,
                                            -1,
                                            beans::PropertySetInfoChange::PROPERTY_REMOVED );

    for( sal_Int32 i = 0; i < m_sListeners.getLength(); ++i )
    {
        uno::Reference< beans::XPropertySetInfoChangeListener > ref( m_sListeners[i],uno::UNO_QUERY );
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
    const com::sun::star::uno::Reference< XContent >& xCreatorContent,
    const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& xCreatorId,
    ListenerMap* pListeners )
    : m_xCreatorContent( xCreatorContent ),
      m_xCreatorId( xCreatorId ),
      m_pListeners( pListeners )
{
}


PropertyChangeNotifier::~PropertyChangeNotifier()
{
    delete m_pListeners;
}


void PropertyChangeNotifier::notifyPropertyChanged(
    uno::Sequence< beans::PropertyChangeEvent > Changes )
{
    sal_Int32 j;

    for( j = 0; j < Changes.getLength(); ++j )
        Changes[j].Source = m_xCreatorContent;

    // notify listeners for all Events

    uno::Sequence< uno::Reference< uno::XInterface > > seqList = (*m_pListeners)[ rtl::OUString() ];
    for( j = 0; j < seqList.getLength(); ++j )
    {
        uno::Reference< beans::XPropertiesChangeListener > aListener( seqList[j],uno::UNO_QUERY );
        if( aListener.is() )
        {
            aListener->propertiesChange( Changes );
        }
    }

    uno::Sequence< beans::PropertyChangeEvent > seq(1);
    for( j = 0; j < Changes.getLength(); ++j )
    {
        seq[0] = Changes[j];
        seqList = (*m_pListeners)[ seq[0].PropertyName ];

        for( sal_Int32 i = 0; i < seqList.getLength(); ++i )
        {
            uno::Reference< beans::XPropertiesChangeListener > aListener( seqList[j],uno::UNO_QUERY );
            if( aListener.is() )
            {
                aListener->propertiesChange( seq );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
