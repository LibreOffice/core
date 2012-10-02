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


#include <toolkit/controls/roadmapcontrol.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/helper/property.hxx>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;

//  ----------------------------------------------------
//  helper
//  ----------------------------------------------------

static void lcl_throwIllegalArgumentException( )
{   // throwing is expensive (in terms of code size), thus we hope the compiler does not inline this ....
    throw IllegalArgumentException();
}

static void lcl_throwIndexOutOfBoundsException( )
{   // throwing is expensive (in terms of code size), thus we hope the compiler does not inline this ....
    throw IndexOutOfBoundsException();
}

    // ===================================================================
    // = UnoControlRoadmapModel
    // ===================================================================
    // -------------------------------------------------------------------
    UnoControlRoadmapModel::UnoControlRoadmapModel( const Reference< XMultiServiceFactory >& i_factory )
        :UnoControlRoadmapModel_Base( i_factory )
        ,maContainerListeners( *this )
    {
        ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
        ImplRegisterProperty( BASEPROPERTY_BORDER );
        ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
        ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
        ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
        ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
        ImplRegisterProperty( BASEPROPERTY_HELPURL );
        ImplRegisterProperty( BASEPROPERTY_IMAGEURL );
        ImplRegisterProperty( BASEPROPERTY_GRAPHIC );
        ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
        ImplRegisterProperty( BASEPROPERTY_COMPLETE );
        ImplRegisterProperty( BASEPROPERTY_ACTIVATED );
        ImplRegisterProperty( BASEPROPERTY_CURRENTITEMID );
        ImplRegisterProperty( BASEPROPERTY_TABSTOP );
        ImplRegisterProperty( BASEPROPERTY_TEXT );
    }

    // -------------------------------------------------------------------
    ::rtl::OUString UnoControlRoadmapModel::getServiceName() throw(RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( szServiceName_UnoControlRoadmapModel );
    }


    // -------------------------------------------------------------------
    Any UnoControlRoadmapModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        Any aReturn;
        switch (nPropId)
        {
                case BASEPROPERTY_COMPLETE:
                    aReturn <<= (sal_Bool) sal_True;
                    break;
                case BASEPROPERTY_ACTIVATED:
                    aReturn <<= (sal_Bool) sal_True;
                    break;
                case BASEPROPERTY_CURRENTITEMID:
                    aReturn <<= (sal_Int16) -1;
                    break;
                case BASEPROPERTY_TEXT:
                   break;
                case BASEPROPERTY_BORDER:
                    aReturn <<= (sal_Int16) 2;              // No Border
                    break;
                case BASEPROPERTY_DEFAULTCONTROL:
                    aReturn <<= ::rtl::OUString( ::rtl::OUString::createFromAscii( szServiceName_UnoControlRoadmap ) );
                    break;
            default : aReturn = UnoControlRoadmapModel_Base::ImplGetDefaultValue( nPropId ); break;
        }

        return aReturn;
    }


    Reference< XInterface > SAL_CALL UnoControlRoadmapModel::createInstance(  ) throw (Exception, ::com::sun::star::uno::RuntimeException)
    {
        ORoadmapEntry* pRoadmapItem = new ORoadmapEntry();
        Reference< XInterface > xNewRoadmapItem = (::cppu::OWeakObject*)pRoadmapItem;
        return xNewRoadmapItem;
    }


    Reference< XInterface > SAL_CALL UnoControlRoadmapModel::createInstanceWithArguments( const Sequence< Any >& /*aArguments*/ ) throw (Exception, RuntimeException)
    {
        // Todo: implementation of the arguments handling
        ORoadmapEntry* pRoadmapItem = new ORoadmapEntry();
        Reference< XInterface > xNewRoadmapItem = (::cppu::OWeakObject*)pRoadmapItem;
        return xNewRoadmapItem;
    }


 IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoControlRoadmapModel, UnoControlRoadmapModel_Base, UnoControlRoadmapModel_IBase )


    // -------------------------------------------------------------------
    ::com::sun::star::uno::Any  SAL_CALL UnoControlRoadmapModel::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
    {
        Any aRet = UnoControlRoadmapModel_Base::queryAggregation( rType );
        if ( !aRet.hasValue() )
            aRet = UnoControlRoadmapModel_IBase::queryInterface( rType );
         return aRet;
    }


    // -------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& UnoControlRoadmapModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper* pHelper = NULL;
        if ( !pHelper )
        {
            Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }


    // beans::XMultiPropertySet
    // -------------------------------------------------------------------
    Reference< XPropertySetInfo > UnoControlRoadmapModel::getPropertySetInfo(  ) throw(RuntimeException)
    {
        static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    sal_Int32 SAL_CALL UnoControlRoadmapModel::getCount() throw(RuntimeException)
    {
        return maRoadmapItems.size();
    }

    Any SAL_CALL UnoControlRoadmapModel::getByIndex( sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
    {
        if (( Index >= (sal_Int32)maRoadmapItems.size()) || (Index < 0))
            lcl_throwIndexOutOfBoundsException( );
        Any aAny;
        aAny = makeAny( maRoadmapItems.at( Index ));
        return aAny;
    }



    void UnoControlRoadmapModel::MakeRMItemValidation( sal_Int32 Index, Reference< XInterface > xRoadmapItem )
    {
        if ((Index > (sal_Int32)maRoadmapItems.size()) || ( Index < 0 ) )
            lcl_throwIndexOutOfBoundsException( );
        if ( !xRoadmapItem.is() )
            lcl_throwIllegalArgumentException();
        Reference< XServiceInfo > xServiceInfo( xRoadmapItem, UNO_QUERY );
        sal_Bool bIsRoadmapItem = xServiceInfo->supportsService( ::rtl::OUString("com.sun.star.awt.RoadmapItem") );
        if ( !bIsRoadmapItem )
            lcl_throwIllegalArgumentException();
    }


    void UnoControlRoadmapModel::SetRMItemDefaultProperties( const sal_Int32 , Reference< XInterface > xRoadmapItem)
    {
        Any aAny;
        Reference< XPropertySet > xPropertySet( xRoadmapItem, UNO_QUERY );
        Reference< XPropertySet > xProps( xRoadmapItem, UNO_QUERY );
        if ( xProps.is() )
        {
            sal_Int32 LocID = 0;
            Any aValue = xPropertySet->getPropertyValue( ::rtl::OUString("ID") );
            aValue >>= LocID;
            if (LocID < 0)              // index may not be smaller than zero
            {
                aAny <<= GetUniqueID();
                xPropertySet->setPropertyValue( ::rtl::OUString("ID"), aAny );
            }
        }
    }


// The performance of this method could certainly be improved.
// As long as only vectors with up to 10 elements are
// involved it should be sufficient
       sal_Int32 UnoControlRoadmapModel::GetUniqueID()
      {
          Any aAny;
          sal_Bool bIncrement = sal_True;
          sal_Int32 CurID = 0;
          sal_Int32 n_CurItemID = 0;
          Reference< XInterface > CurRoadmapItem;
          while ( bIncrement )
          {
              bIncrement = sal_False;
              for ( RoadmapItemHolderList::iterator i = maRoadmapItems.begin(); i < maRoadmapItems.end(); ++i )
              {
                CurRoadmapItem = *i;
                Reference< XPropertySet > xPropertySet( CurRoadmapItem, UNO_QUERY );
                aAny = xPropertySet->getPropertyValue( ::rtl::OUString("ID") );
                aAny >>= n_CurItemID;
                if (n_CurItemID == CurID)
                {
                    bIncrement = sal_True;
                    CurID++;
                    break;
                }
            }
        }
        return CurID;
    }


    ContainerEvent UnoControlRoadmapModel::GetContainerEvent(sal_Int32 Index, Reference< XInterface > xRoadmapItem)
    {
        ContainerEvent aEvent;
        aEvent.Source = *this;
        aEvent.Element <<= xRoadmapItem;
        aEvent.Accessor = makeAny(Index);
        return aEvent;
    }


    sal_Int16 UnoControlRoadmapModel::GetCurrentItemID( Reference< XPropertySet > xPropertySet )
    {
        Any aAny = xPropertySet->getPropertyValue( GetPropertyName( BASEPROPERTY_CURRENTITEMID ) );
        sal_Int16 n_CurrentItemID = 0;
        aAny >>= n_CurrentItemID;
        return n_CurrentItemID;
    }


    void SAL_CALL UnoControlRoadmapModel::insertByIndex( const sal_Int32 Index, const Any& _Element)
                                    throw (IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
    {
        if ( ( Index >= ( (sal_Int32)maRoadmapItems.size() + 1 ) ) || (Index < 0))
            lcl_throwIndexOutOfBoundsException( );
        Reference< XInterface > xRoadmapItem;
        _Element >>= xRoadmapItem;
        MakeRMItemValidation( Index, xRoadmapItem);
        SetRMItemDefaultProperties( Index, xRoadmapItem );
        maRoadmapItems.insert( maRoadmapItems.begin() + Index, xRoadmapItem);
        ContainerEvent aEvent = GetContainerEvent(Index, xRoadmapItem);
        maContainerListeners.elementInserted( aEvent );
        Reference< XPropertySet > xPropertySet( (XAggregation*) (::cppu::OWeakAggObject*)this, UNO_QUERY );
        sal_Int16 n_CurrentItemID = GetCurrentItemID( xPropertySet );
        if ( Index <= n_CurrentItemID )
        {
            Any aAny;
            aAny <<= ( sal_Int16 ) ( n_CurrentItemID + 1 );
            xPropertySet->setPropertyValue( GetPropertyName( BASEPROPERTY_CURRENTITEMID ), aAny );
        }
    }



    void SAL_CALL UnoControlRoadmapModel::removeByIndex( sal_Int32 Index)
                                                throw    (IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
    {
        if (( Index > (sal_Int32)maRoadmapItems.size()) || (Index < 0))
            lcl_throwIndexOutOfBoundsException( );
        Reference< XInterface > xRoadmapItem;
        maRoadmapItems.erase( maRoadmapItems.begin() + Index );
        ContainerEvent aEvent = GetContainerEvent(Index, xRoadmapItem);
        maContainerListeners.elementRemoved( aEvent );
        Reference< XPropertySet > xPropertySet( (XAggregation*) (::cppu::OWeakAggObject*)this, UNO_QUERY );
        sal_Int16 n_CurrentItemID = GetCurrentItemID( xPropertySet );
        Any aAny;
        if ( Index <= n_CurrentItemID )
        {
            if ( n_CurrentItemID >= (sal_Int32)maRoadmapItems.size() )
            {
                n_CurrentItemID = sal::static_int_cast< sal_Int16 >(
                    maRoadmapItems.size()-1);
                if ( n_CurrentItemID < 0 )
                    return;
                aAny <<= n_CurrentItemID;
            }
            else if (Index == n_CurrentItemID)
                aAny <<= ( sal_Int16 ) -1;
            else if( Index < n_CurrentItemID)
                aAny <<= ( sal_Int16 ) ( n_CurrentItemID - 1 );
            xPropertySet->setPropertyValue( GetPropertyName( BASEPROPERTY_CURRENTITEMID ), aAny );
        }
    }


    void SAL_CALL UnoControlRoadmapModel::replaceByIndex( const sal_Int32 Index, const Any& _Element)
                                throw (IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
    {
        Reference< XInterface > xRoadmapItem;
        _Element >>= xRoadmapItem;
        MakeRMItemValidation( Index, xRoadmapItem);
        SetRMItemDefaultProperties( Index, xRoadmapItem );
        maRoadmapItems.erase( maRoadmapItems.begin() + Index );
        maRoadmapItems.insert( maRoadmapItems.begin() + Index, xRoadmapItem);        //push_back( xRoadmapItem );
        ContainerEvent aEvent = GetContainerEvent(Index, xRoadmapItem);
        maContainerListeners.elementReplaced( aEvent );
    }


    Type SAL_CALL UnoControlRoadmapModel::getElementType() throw(RuntimeException)
    {
        Type aType = getCppuType( ( Reference< XPropertySet>* ) NULL );
        return aType;
    }


    sal_Bool SAL_CALL UnoControlRoadmapModel::hasElements() throw(RuntimeException)
    {
        return !maRoadmapItems.empty();
    }


    void SAL_CALL UnoControlRoadmapModel::addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
    {
        maContainerListeners.addInterface( xListener );
    }

    void SAL_CALL UnoControlRoadmapModel::removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
    {
        maContainerListeners.removeInterface( xListener );
    }

    // ===================================================================
    // = UnoRoadmapControl
    // ===================================================================
    // -------------------------------------------------------------------
    UnoRoadmapControl::UnoRoadmapControl( const Reference< XMultiServiceFactory >& i_factory )
        :UnoControlRoadmap_Base( i_factory )
        ,maItemListeners( *this )
    {
    }

IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoRoadmapControl, UnoControlRoadmap_Base, UnoControlRoadmap_IBase )
IMPLEMENT_FORWARD_XINTERFACE2( UnoRoadmapControl, UnoControlRoadmap_Base, UnoControlRoadmap_IBase )


sal_Bool SAL_CALL UnoRoadmapControl::setModel(const Reference< XControlModel >& _rModel) throw ( RuntimeException )
    {


           Reference< XContainer > xC( getModel(), UNO_QUERY );
        if ( xC.is() )
            xC->removeContainerListener( this );

        sal_Bool bReturn = UnoControlBase::setModel( _rModel );

        xC = xC.query( getModel());
        if ( xC.is() )
            xC->addContainerListener( this );

        return bReturn;
    }


    // -------------------------------------------------------------------
    ::rtl::OUString UnoRoadmapControl::GetComponentServiceName()
    {
        return ::rtl::OUString("Roadmap");
    }



    void UnoRoadmapControl::dispose() throw(RuntimeException)
    {
        EventObject aEvt;
        aEvt.Source = (::cppu::OWeakObject*)this;
        maItemListeners.disposeAndClear( aEvt );
        UnoControl::dispose();
    }



void UnoRoadmapControl::elementInserted( const ContainerEvent& rEvent )throw(RuntimeException)
{
    Reference< XInterface > xRoadmapItem;
    rEvent.Element >>= xRoadmapItem;
    Reference< XPropertySet > xRoadmapPropertySet( xRoadmapItem, UNO_QUERY );
    if ( xRoadmapPropertySet.is() )
        xRoadmapPropertySet->addPropertyChangeListener( rtl::OUString(), this );

    Reference< XContainerListener >  xPeer(getPeer(), UNO_QUERY);
    if ( xPeer.is() )
    {
        xPeer->elementInserted( rEvent );
        Reference < XPropertySet > xPropertySet( xPeer, UNO_QUERY );
        if ( xPropertySet.is() )
            xPropertySet->addPropertyChangeListener( rtl::OUString(), this );
    }
}


void UnoRoadmapControl::elementRemoved( const ContainerEvent& rEvent )throw(RuntimeException)
{
    Reference< XContainerListener >  xPeer(getPeer(), UNO_QUERY);
    if ( xPeer.is() )
        xPeer->elementRemoved( rEvent );
    Reference< XInterface > xRoadmapItem;
    rEvent.Element >>= xRoadmapItem;
    Reference< XPropertySet > xPropertySet( xRoadmapItem, UNO_QUERY );
    if ( xPropertySet.is() )
        xPropertySet->removePropertyChangeListener( rtl::OUString(), this );
}


void UnoRoadmapControl::elementReplaced( const ContainerEvent& rEvent )throw(RuntimeException)
{
    Reference< XContainerListener >  xPeer(getPeer(), UNO_QUERY);
    if ( xPeer.is() )
        xPeer->elementReplaced( rEvent );
}


void SAL_CALL UnoRoadmapControl::itemStateChanged( const ItemEvent& rEvent ) throw (RuntimeException)
{
    sal_Int16 CurItemIndex = sal::static_int_cast< sal_Int16 >(rEvent.ItemId);
    Any aAny;
    aAny <<= CurItemIndex;
    Reference< XControlModel > xModel( getModel( ), UNO_QUERY );
    Reference< XPropertySet > xPropertySet( xModel, UNO_QUERY );
    xPropertySet->setPropertyValue( GetPropertyName( BASEPROPERTY_CURRENTITEMID ), aAny );
    if ( maItemListeners.getLength() )
        maItemListeners.itemStateChanged( rEvent );
}


void SAL_CALL UnoRoadmapControl::addItemListener( const Reference< XItemListener >& l ) throw (RuntimeException)
{
    maItemListeners.addInterface( l );
    if( getPeer().is() && maItemListeners.getLength() == 1 )
    {
        Reference < XItemEventBroadcaster > xRoadmap( getPeer(), UNO_QUERY );
        xRoadmap->addItemListener( this );
    }
}


void SAL_CALL UnoRoadmapControl::removeItemListener( const Reference< XItemListener >& l ) throw (RuntimeException)
{
    if( getPeer().is() && maItemListeners.getLength() == 1 )
    {
        Reference < XItemEventBroadcaster >  xRoadmap( getPeer(), UNO_QUERY );
        xRoadmap->removeItemListener( this );
    }

    maItemListeners.removeInterface( l );
}


void SAL_CALL UnoRoadmapControl::propertyChange( const PropertyChangeEvent& evt ) throw (RuntimeException)
{
    Reference< XPropertyChangeListener >  xPeer(getPeer(), UNO_QUERY);
    if ( xPeer.is() )
        xPeer->propertyChange( evt );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
