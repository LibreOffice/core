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
#include <toolkit/helper/property.hxx>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/diagnose.h>

#include "helper/unopropertyarrayhelper.hxx"

namespace toolkit
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;


//  helper


static void lcl_throwIllegalArgumentException( )
{   // throwing is expensive (in terms of code size), thus we hope the compiler does not inline this ....
    throw IllegalArgumentException();
}

static void lcl_throwIndexOutOfBoundsException( )
{   // throwing is expensive (in terms of code size), thus we hope the compiler does not inline this ....
    throw IndexOutOfBoundsException();
}


    // = UnoControlRoadmapModel


    UnoControlRoadmapModel::UnoControlRoadmapModel( const Reference< XComponentContext >& i_factory )
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


    OUString UnoControlRoadmapModel::getServiceName() throw(RuntimeException, std::exception)
    {
        return OUString::createFromAscii( szServiceName_UnoControlRoadmapModel );
    }

    OUString UnoControlRoadmapModel::getImplementationName()
        throw (css::uno::RuntimeException, std::exception)
    {
        return OUString("stardiv.Toolkit.UnoControlRoadmapModel");
    }

    css::uno::Sequence<OUString>
    UnoControlRoadmapModel::getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception)
    {
        auto s(UnoControlRoadmapModel_Base::getSupportedServiceNames());
        s.realloc(s.getLength() + 2);
        s[s.getLength() - 2] = "com.sun.star.awt.UnoControlRoadmapModel";
        s[s.getLength() - 1] = "stardiv.vcl.controlmodel.Roadmap";
        return s;
    }

    Any UnoControlRoadmapModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        Any aReturn;
        switch (nPropId)
        {
                case BASEPROPERTY_COMPLETE:
                    aReturn <<= true;
                    break;
                case BASEPROPERTY_ACTIVATED:
                    aReturn <<= true;
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
                    aReturn <<= OUString( OUString::createFromAscii( szServiceName_UnoControlRoadmap ) );
                    break;
            default : aReturn = UnoControlRoadmapModel_Base::ImplGetDefaultValue( nPropId ); break;
        }

        return aReturn;
    }


    Reference< XInterface > SAL_CALL UnoControlRoadmapModel::createInstance(  ) throw (Exception, css::uno::RuntimeException, std::exception)
    {
        ORoadmapEntry* pRoadmapItem = new ORoadmapEntry();
        Reference< XInterface > xNewRoadmapItem = static_cast<cppu::OWeakObject*>(pRoadmapItem);
        return xNewRoadmapItem;
    }


    Reference< XInterface > SAL_CALL UnoControlRoadmapModel::createInstanceWithArguments( const Sequence< Any >& /*aArguments*/ ) throw (Exception, RuntimeException, std::exception)
    {
        // Todo: implementation of the arguments handling
        ORoadmapEntry* pRoadmapItem = new ORoadmapEntry();
        Reference< XInterface > xNewRoadmapItem = static_cast<cppu::OWeakObject*>(pRoadmapItem);
        return xNewRoadmapItem;
    }


 IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoControlRoadmapModel, UnoControlRoadmapModel_Base, UnoControlRoadmapModel_IBase )


    css::uno::Any  SAL_CALL UnoControlRoadmapModel::queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception)
    {
        Any aRet = UnoControlRoadmapModel_Base::queryAggregation( rType );
        if ( !aRet.hasValue() )
            aRet = UnoControlRoadmapModel_IBase::queryInterface( rType );
         return aRet;
    }


    ::cppu::IPropertyArrayHelper& UnoControlRoadmapModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper* pHelper = nullptr;
        if ( !pHelper )
        {
            Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }


    // beans::XMultiPropertySet

    Reference< XPropertySetInfo > UnoControlRoadmapModel::getPropertySetInfo(  ) throw(RuntimeException, std::exception)
    {
        static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    sal_Int32 SAL_CALL UnoControlRoadmapModel::getCount() throw(RuntimeException, std::exception)
    {
        return maRoadmapItems.size();
    }

    Any SAL_CALL UnoControlRoadmapModel::getByIndex( sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception )
    {
        if (( Index >= (sal_Int32)maRoadmapItems.size()) || (Index < 0))
            lcl_throwIndexOutOfBoundsException( );
        Any aAny;
        aAny = makeAny( maRoadmapItems.at( Index ));
        return aAny;
    }


    void UnoControlRoadmapModel::MakeRMItemValidation( sal_Int32 Index, const Reference< XInterface >& xRoadmapItem )
    {
        if ((Index > (sal_Int32)maRoadmapItems.size()) || ( Index < 0 ) )
            lcl_throwIndexOutOfBoundsException( );
        if ( !xRoadmapItem.is() )
            lcl_throwIllegalArgumentException();
        Reference< XServiceInfo > xServiceInfo( xRoadmapItem, UNO_QUERY );
        bool bIsRoadmapItem = xServiceInfo->supportsService("com.sun.star.awt.RoadmapItem");
        if ( !bIsRoadmapItem )
            lcl_throwIllegalArgumentException();
    }


    void UnoControlRoadmapModel::SetRMItemDefaultProperties( const sal_Int32 , const Reference< XInterface >& xRoadmapItem)
    {
        Reference< XPropertySet > xPropertySet( xRoadmapItem, UNO_QUERY );
        Reference< XPropertySet > xProps( xRoadmapItem, UNO_QUERY );
        if ( xProps.is() )
        {
            sal_Int32 LocID = 0;
            Any aValue = xPropertySet->getPropertyValue("ID");
            aValue >>= LocID;
            if (LocID < 0)              // index may not be smaller than zero
            {
                xPropertySet->setPropertyValue("ID", Any(GetUniqueID()) );
            }
        }
    }


// The performance of this method could certainly be improved.
// As long as only vectors with up to 10 elements are
// involved it should be sufficient
       sal_Int32 UnoControlRoadmapModel::GetUniqueID()
      {
          Any aAny;
          bool bIncrement = true;
          sal_Int32 CurID = 0;
          sal_Int32 n_CurItemID = 0;
          Reference< XInterface > CurRoadmapItem;
          while ( bIncrement )
          {
              bIncrement = false;
              for ( RoadmapItemHolderList::iterator i = maRoadmapItems.begin(); i < maRoadmapItems.end(); ++i )
              {
                CurRoadmapItem = *i;
                Reference< XPropertySet > xPropertySet( CurRoadmapItem, UNO_QUERY );
                aAny = xPropertySet->getPropertyValue("ID");
                aAny >>= n_CurItemID;
                if (n_CurItemID == CurID)
                {
                    bIncrement = true;
                    CurID++;
                    break;
                }
            }
        }
        return CurID;
    }


    ContainerEvent UnoControlRoadmapModel::GetContainerEvent(sal_Int32 Index, const Reference< XInterface >& xRoadmapItem)
    {
        ContainerEvent aEvent;
        aEvent.Source = *this;
        aEvent.Element <<= xRoadmapItem;
        aEvent.Accessor = makeAny(Index);
        return aEvent;
    }


    sal_Int16 UnoControlRoadmapModel::GetCurrentItemID( const Reference< XPropertySet >& xPropertySet )
    {
        Any aAny = xPropertySet->getPropertyValue( GetPropertyName( BASEPROPERTY_CURRENTITEMID ) );
        sal_Int16 n_CurrentItemID = 0;
        aAny >>= n_CurrentItemID;
        return n_CurrentItemID;
    }


    void SAL_CALL UnoControlRoadmapModel::insertByIndex( const sal_Int32 Index, const Any& Element)
                                    throw (IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception )
    {
        if ( ( Index >= ( (sal_Int32)maRoadmapItems.size() + 1 ) ) || (Index < 0))
            lcl_throwIndexOutOfBoundsException( );
        Reference< XInterface > xRoadmapItem;
        Element >>= xRoadmapItem;
        MakeRMItemValidation( Index, xRoadmapItem);
        SetRMItemDefaultProperties( Index, xRoadmapItem );
        maRoadmapItems.insert( maRoadmapItems.begin() + Index, xRoadmapItem);
        ContainerEvent aEvent = GetContainerEvent(Index, xRoadmapItem);
        maContainerListeners.elementInserted( aEvent );
        Reference< XPropertySet > xPropertySet( static_cast<XAggregation*>(static_cast<cppu::OWeakAggObject*>(this)), UNO_QUERY );
        sal_Int16 n_CurrentItemID = GetCurrentItemID( xPropertySet );
        if ( Index <= n_CurrentItemID )
        {
            Any aAny(( sal_Int16 ) ( n_CurrentItemID + 1 ) );
            xPropertySet->setPropertyValue( GetPropertyName( BASEPROPERTY_CURRENTITEMID ), aAny );
        }
    }


    void SAL_CALL UnoControlRoadmapModel::removeByIndex( sal_Int32 Index)
                                                throw    (IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception )
    {
        if (( Index > (sal_Int32)maRoadmapItems.size()) || (Index < 0))
            lcl_throwIndexOutOfBoundsException( );
        Reference< XInterface > xRoadmapItem;
        maRoadmapItems.erase( maRoadmapItems.begin() + Index );
        ContainerEvent aEvent = GetContainerEvent(Index, xRoadmapItem);
        maContainerListeners.elementRemoved( aEvent );
        Reference< XPropertySet > xPropertySet( static_cast<XAggregation*>(static_cast<cppu::OWeakAggObject*>(this)), UNO_QUERY );
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


    void SAL_CALL UnoControlRoadmapModel::replaceByIndex( const sal_Int32 Index, const Any& Element)
                                throw (IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception )
    {
        Reference< XInterface > xRoadmapItem;
        Element >>= xRoadmapItem;
        MakeRMItemValidation( Index, xRoadmapItem);
        SetRMItemDefaultProperties( Index, xRoadmapItem );
        maRoadmapItems.erase( maRoadmapItems.begin() + Index );
        maRoadmapItems.insert( maRoadmapItems.begin() + Index, xRoadmapItem);        //push_back( xRoadmapItem );
        ContainerEvent aEvent = GetContainerEvent(Index, xRoadmapItem);
        maContainerListeners.elementReplaced( aEvent );
    }


    Type SAL_CALL UnoControlRoadmapModel::getElementType() throw(RuntimeException, std::exception)
    {
        Type aType = cppu::UnoType<XPropertySet>::get();
        return aType;
    }


    sal_Bool SAL_CALL UnoControlRoadmapModel::hasElements() throw(RuntimeException, std::exception)
    {
        return !maRoadmapItems.empty();
    }


    void SAL_CALL UnoControlRoadmapModel::addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw (css::uno::RuntimeException, std::exception)
    {
        maContainerListeners.addInterface( xListener );
    }

    void SAL_CALL UnoControlRoadmapModel::removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw (css::uno::RuntimeException, std::exception)
    {
        maContainerListeners.removeInterface( xListener );
    }


    // = UnoRoadmapControl


    UnoRoadmapControl::UnoRoadmapControl()
        :UnoControlRoadmap_Base()
        ,maItemListeners( *this )
    {
    }

IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoRoadmapControl, UnoControlRoadmap_Base, UnoControlRoadmap_IBase )
IMPLEMENT_FORWARD_XINTERFACE2( UnoRoadmapControl, UnoControlRoadmap_Base, UnoControlRoadmap_IBase )


sal_Bool SAL_CALL UnoRoadmapControl::setModel(const Reference< XControlModel >& _rModel) throw ( RuntimeException, std::exception )
    {


           Reference< XContainer > xC( getModel(), UNO_QUERY );
        if ( xC.is() )
            xC->removeContainerListener( this );

        bool bReturn = UnoControlBase::setModel( _rModel );

        xC.set(getModel(), css::uno::UNO_QUERY);
        if ( xC.is() )
            xC->addContainerListener( this );

        return bReturn;
    }


    OUString UnoRoadmapControl::GetComponentServiceName()
    {
        return OUString("Roadmap");
    }


    void UnoRoadmapControl::dispose() throw(RuntimeException, std::exception)
    {
        EventObject aEvt;
        aEvt.Source = static_cast<cppu::OWeakObject*>(this);
        maItemListeners.disposeAndClear( aEvt );
        UnoControl::dispose();
    }


void UnoRoadmapControl::elementInserted( const ContainerEvent& rEvent )throw(RuntimeException, std::exception)
{
    Reference< XInterface > xRoadmapItem;
    rEvent.Element >>= xRoadmapItem;
    Reference< XPropertySet > xRoadmapPropertySet( xRoadmapItem, UNO_QUERY );
    if ( xRoadmapPropertySet.is() )
        xRoadmapPropertySet->addPropertyChangeListener( OUString(), this );

    Reference< XContainerListener >  xPeer(getPeer(), UNO_QUERY);
    if ( xPeer.is() )
    {
        xPeer->elementInserted( rEvent );
        Reference < XPropertySet > xPropertySet( xPeer, UNO_QUERY );
        if ( xPropertySet.is() )
            xPropertySet->addPropertyChangeListener( OUString(), this );
    }
}


void UnoRoadmapControl::elementRemoved( const ContainerEvent& rEvent )throw(RuntimeException, std::exception)
{
    Reference< XContainerListener >  xPeer(getPeer(), UNO_QUERY);
    if ( xPeer.is() )
        xPeer->elementRemoved( rEvent );
    Reference< XInterface > xRoadmapItem;
    rEvent.Element >>= xRoadmapItem;
    Reference< XPropertySet > xPropertySet( xRoadmapItem, UNO_QUERY );
    if ( xPropertySet.is() )
        xPropertySet->removePropertyChangeListener( OUString(), this );
}


void UnoRoadmapControl::elementReplaced( const ContainerEvent& rEvent )throw(RuntimeException, std::exception)
{
    Reference< XContainerListener >  xPeer(getPeer(), UNO_QUERY);
    if ( xPeer.is() )
        xPeer->elementReplaced( rEvent );
}


void SAL_CALL UnoRoadmapControl::itemStateChanged( const ItemEvent& rEvent ) throw (RuntimeException, std::exception)
{
    sal_Int16 CurItemIndex = sal::static_int_cast< sal_Int16 >(rEvent.ItemId);
    Reference< XControlModel > xModel( getModel( ), UNO_QUERY );
    Reference< XPropertySet > xPropertySet( xModel, UNO_QUERY );
    xPropertySet->setPropertyValue( GetPropertyName( BASEPROPERTY_CURRENTITEMID ), Any(CurItemIndex) );
    if ( maItemListeners.getLength() )
        maItemListeners.itemStateChanged( rEvent );
}


void SAL_CALL UnoRoadmapControl::addItemListener( const Reference< XItemListener >& l ) throw (RuntimeException, std::exception)
{
    maItemListeners.addInterface( l );
    if( getPeer().is() && maItemListeners.getLength() == 1 )
    {
        Reference < XItemEventBroadcaster > xRoadmap( getPeer(), UNO_QUERY );
        xRoadmap->addItemListener( this );
    }
}


void SAL_CALL UnoRoadmapControl::removeItemListener( const Reference< XItemListener >& l ) throw (RuntimeException, std::exception)
{
    if( getPeer().is() && maItemListeners.getLength() == 1 )
    {
        Reference < XItemEventBroadcaster >  xRoadmap( getPeer(), UNO_QUERY );
        xRoadmap->removeItemListener( this );
    }

    maItemListeners.removeInterface( l );
}


void SAL_CALL UnoRoadmapControl::propertyChange( const PropertyChangeEvent& evt ) throw (RuntimeException, std::exception)
{
    Reference< XPropertyChangeListener >  xPeer(getPeer(), UNO_QUERY);
    if ( xPeer.is() )
        xPeer->propertyChange( evt );
}

OUString UnoRoadmapControl::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("stardiv.Toolkit.UnoRoadmapControl");
}

css::uno::Sequence<OUString> UnoRoadmapControl::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    auto s(UnoControlBase::getSupportedServiceNames());
    s.realloc(s.getLength() + 2);
    s[s.getLength() - 2] = "com.sun.star.awt.UnoControlRoadmap";
    s[s.getLength() - 1] = "stardiv.vcl.control.Roadmap";
    return s;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
stardiv_Toolkit_UnoControlRoadmapModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new toolkit::UnoControlRoadmapModel(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
stardiv_Toolkit_UnoRoadmapControl_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new toolkit::UnoRoadmapControl());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
