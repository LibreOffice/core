/*************************************************************************
 *
 *  $RCSfile: filnot.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:53:36 $
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
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTACTION_HPP_
#include <com/sun/star/ucb/ContentAction.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSETINFOCHANGE_HPP_
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#endif
#ifndef _FILNOT_HXX_
#include "filnot.hxx"
#endif
#ifndef _FILID_HXX_
#include "filid.hxx"
#endif
#ifndef _BC_HXX_
#include "bc.hxx"
#endif
#ifndef _PROV_HXX_
#include "prov.hxx"
#endif



using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;


ContentEventNotifier::ContentEventNotifier( shell* pMyShell,
                                            BaseContent* pCreatorContent,
                                            const uno::Reference< XContentIdentifier >& xCreatorId,
                                            const uno::Sequence< uno::Reference< uno::XInterface > >& sListeners )
    : m_pMyShell( pMyShell ),
      m_pCreatorContent( pCreatorContent ),
      m_xCreatorId( xCreatorId ),
      m_sListeners( sListeners )
{
}


ContentEventNotifier::ContentEventNotifier( shell* pMyShell,
                                            BaseContent* pCreatorContent,
                                            const uno::Reference< XContentIdentifier >& xCreatorId,
                                            const uno::Reference< XContentIdentifier >& xOldId,
                                            const uno::Sequence< uno::Reference< uno::XInterface > >& sListeners )
    : m_pMyShell( pMyShell ),
      m_pCreatorContent( pCreatorContent ),
      m_xCreatorId( xCreatorId ),
      m_xOldId( xOldId ),
      m_sListeners( sListeners )
{
}



void ContentEventNotifier::notifyChildInserted( const rtl::OUString& aChildName )
{
    uno::Reference< uno::XInterface > shooter( static_cast< XContent* >( m_pCreatorContent ),uno::UNO_QUERY );

    FileContentIdentifier* p = new FileContentIdentifier( m_pMyShell,aChildName );
    uno::Reference< XContentIdentifier > xChildId( p );

    uno::Reference< XContent > xChildContent = m_pMyShell->m_pProvider->queryContent( xChildId );

    ContentEvent aEvt( shooter,
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
    uno::Reference< uno::XInterface > shooter( static_cast< XContent* >( m_pCreatorContent ),uno::UNO_QUERY );
    uno::Reference< XContent > xDeletedContent( static_cast< XContent* >( m_pCreatorContent ),uno::UNO_QUERY );

    ContentEvent aEvt( shooter,
                       ContentAction::DELETED,
                       xDeletedContent,
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
    uno::Reference< uno::XInterface > shooter( static_cast< XContent* >( m_pCreatorContent ),uno::UNO_QUERY );

    FileContentIdentifier* p = new FileContentIdentifier( m_pMyShell,aChildName );
    uno::Reference< XContentIdentifier > xChildId( p );

    BaseContent* pp = new BaseContent( m_pMyShell,xChildId,aChildName );
    {
        vos::OGuard aGuard( pp->m_aMutex );
        pp->m_nState |= BaseContent::Deleted;
    }

    uno::Reference< XContent > xDeletedContent( pp );


    ContentEvent aEvt( shooter,
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
    uno::Reference< uno::XInterface > shooter( static_cast< XContent* >( m_pCreatorContent ),uno::UNO_QUERY );

    ContentEvent aEvt( shooter,
                       ContentAction::EXCHANGED,
                       m_pCreatorContent,
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
    shell* pMyShell,
    BaseContent* pCreatorContent,
    const uno::Reference< XContentIdentifier >& xCreatorId,
    const uno::Sequence< uno::Reference< uno::XInterface > >& sListeners )
    : m_pMyShell( pMyShell ),
      m_pCreatorContent( pCreatorContent ),
      m_xCreatorId( xCreatorId ),
      m_sListeners( sListeners )
{

}


void SAL_CALL
PropertySetInfoChangeNotifier::notifyPropertyAdded( const rtl::OUString & aPropertyName )
{
    uno::Reference< uno::XInterface > shooter( static_cast< XContent* >( m_pCreatorContent ),uno::UNO_QUERY );
    beans::PropertySetInfoChangeEvent aEvt( shooter,
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
    uno::Reference< uno::XInterface > shooter( static_cast< XContent* >( m_pCreatorContent ),uno::UNO_QUERY );
    beans::PropertySetInfoChangeEvent aEvt( shooter,
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
    shell* pMyShell,
    BaseContent* pCreatorContent,
    const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& xCreatorId,
    ListenerMap* pListeners )
    : m_pMyShell( pMyShell ),
      m_pCreatorContent( pCreatorContent ),
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
        Changes[j].Source = static_cast< XContent *>( m_pCreatorContent );

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
