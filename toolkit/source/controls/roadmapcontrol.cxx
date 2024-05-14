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


#include <controls/roadmapcontrol.hxx>
#include <controls/roadmapentry.hxx>
#include <helper/property.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <helper/unopropertyarrayhelper.hxx>

namespace toolkit
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;


//  helper

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


    OUString UnoControlRoadmapModel::getServiceName()
    {
        return u"stardiv.vcl.controlmodel.Roadmap"_ustr;
    }

    OUString UnoControlRoadmapModel::getImplementationName()
    {
        return u"stardiv.Toolkit.UnoControlRoadmapModel"_ustr;
    }

    css::uno::Sequence<OUString>
    UnoControlRoadmapModel::getSupportedServiceNames()
    {
        auto s(UnoControlRoadmapModel_Base::getSupportedServiceNames());
        s.realloc(s.getLength() + 2);
        auto ps = s.getArray();
        ps[s.getLength() - 2] = "com.sun.star.awt.UnoControlRoadmapModel";
        ps[s.getLength() - 1] = "stardiv.vcl.controlmodel.Roadmap";
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
                    aReturn <<= sal_Int16(-1);
                    break;
                case BASEPROPERTY_TEXT:
                   break;
                case BASEPROPERTY_BORDER:
                    aReturn <<= sal_Int16(2);              // No Border
                    break;
                case BASEPROPERTY_DEFAULTCONTROL:
                    aReturn <<= u"stardiv.vcl.control.Roadmap"_ustr;
                    break;
            default : aReturn = UnoControlRoadmapModel_Base::ImplGetDefaultValue( nPropId ); break;
        }

        return aReturn;
    }


    Reference< XInterface > SAL_CALL UnoControlRoadmapModel::createInstance(  )
    {
        return cppu::getXWeak(new ORoadmapEntry());
    }


    Reference< XInterface > SAL_CALL UnoControlRoadmapModel::createInstanceWithArguments( const Sequence< Any >& /*aArguments*/ )
    {
        // Todo: implementation of the arguments handling
        return cppu::getXWeak(new ORoadmapEntry());
    }


 IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoControlRoadmapModel, UnoControlRoadmapModel_Base, UnoControlRoadmapModel_IBase )


    css::uno::Any  SAL_CALL UnoControlRoadmapModel::queryAggregation( const css::uno::Type & rType )
    {
        Any aRet = UnoControlRoadmapModel_Base::queryAggregation( rType );
        if ( !aRet.hasValue() )
            aRet = UnoControlRoadmapModel_IBase::queryInterface( rType );
        return aRet;
    }


    ::cppu::IPropertyArrayHelper& UnoControlRoadmapModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper aHelper( ImplGetPropertyIds() );
        return aHelper;
    }


    // beans::XMultiPropertySet

    Reference< XPropertySetInfo > UnoControlRoadmapModel::getPropertySetInfo(  )
    {
        static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    sal_Int32 SAL_CALL UnoControlRoadmapModel::getCount()
    {
        return maRoadmapItems.size();
    }

    Any SAL_CALL UnoControlRoadmapModel::getByIndex( sal_Int32 Index )
    {
        if ((Index < 0) || ( o3tl::make_unsigned(Index) >= maRoadmapItems.size()))
            throw IndexOutOfBoundsException();
        Any aAny( maRoadmapItems.at( Index ) );
        return aAny;
    }


    void UnoControlRoadmapModel::MakeRMItemValidation( sal_Int32 Index, const Reference< XInterface >& xRoadmapItem )
    {
        if (( Index < 0 ) || (o3tl::make_unsigned(Index) > maRoadmapItems.size()) )
            throw IndexOutOfBoundsException();
        if ( !xRoadmapItem.is() )
            throw IllegalArgumentException();
        Reference< XServiceInfo > xServiceInfo( xRoadmapItem, UNO_QUERY );
        bool bIsRoadmapItem = xServiceInfo->supportsService(u"com.sun.star.awt.RoadmapItem"_ustr);
        if ( !bIsRoadmapItem )
            throw IllegalArgumentException();
    }


    void UnoControlRoadmapModel::SetRMItemDefaultProperties( const Reference< XInterface >& xRoadmapItem)
    {
        Reference< XPropertySet > xPropertySet( xRoadmapItem, UNO_QUERY );
        Reference< XPropertySet > xProps( xRoadmapItem, UNO_QUERY );
        if ( xProps.is() )
        {
            sal_Int32 LocID = 0;
            Any aValue = xPropertySet->getPropertyValue(u"ID"_ustr);
            aValue >>= LocID;
            if (LocID < 0)              // index may not be smaller than zero
            {
                xPropertySet->setPropertyValue(u"ID"_ustr, Any(GetUniqueID()) );
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
              for ( const auto& rRoadmapItem : maRoadmapItems )
              {
                CurRoadmapItem = rRoadmapItem;
                Reference< XPropertySet > xPropertySet( CurRoadmapItem, UNO_QUERY );
                aAny = xPropertySet->getPropertyValue(u"ID"_ustr);
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
        aEvent.Accessor <<= Index;
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
    {
        if ( ( Index >= ( static_cast<sal_Int32>(maRoadmapItems.size()) + 1 ) ) || (Index < 0))
            throw IndexOutOfBoundsException();
        Reference< XInterface > xRoadmapItem;
        Element >>= xRoadmapItem;
        MakeRMItemValidation( Index, xRoadmapItem);
        SetRMItemDefaultProperties( xRoadmapItem );
        maRoadmapItems.insert( maRoadmapItems.begin() + Index, xRoadmapItem);
        ContainerEvent aEvent = GetContainerEvent(Index, xRoadmapItem);
        maContainerListeners.elementInserted( aEvent );
        Reference< XPropertySet > xPropertySet( this );
        sal_Int16 n_CurrentItemID = GetCurrentItemID( xPropertySet );
        if ( Index <= n_CurrentItemID )
        {
            Any aAny(static_cast<sal_Int16>( n_CurrentItemID + 1 ) );
            xPropertySet->setPropertyValue( GetPropertyName( BASEPROPERTY_CURRENTITEMID ), aAny );
        }
    }


    void SAL_CALL UnoControlRoadmapModel::removeByIndex( sal_Int32 Index)
    {
        if ((Index < 0) || ( o3tl::make_unsigned(Index) > maRoadmapItems.size()))
            throw IndexOutOfBoundsException();
        Reference< XInterface > xRoadmapItem;
        maRoadmapItems.erase( maRoadmapItems.begin() + Index );
        ContainerEvent aEvent = GetContainerEvent(Index, xRoadmapItem);
        maContainerListeners.elementRemoved( aEvent );
        Reference< XPropertySet > xPropertySet( this );
        sal_Int16 n_CurrentItemID = GetCurrentItemID( xPropertySet );
        Any aAny;
        if ( Index > n_CurrentItemID )
            return;

        if ( n_CurrentItemID >= static_cast<sal_Int32>(maRoadmapItems.size()) )
        {
            n_CurrentItemID = sal::static_int_cast< sal_Int16 >(
                maRoadmapItems.size()-1);
            if ( n_CurrentItemID < 0 )
                return;
            aAny <<= n_CurrentItemID;
        }
        else if (Index == n_CurrentItemID)
            aAny <<= sal_Int16(-1);
        else if( Index < n_CurrentItemID)
            aAny <<= static_cast<sal_Int16>( n_CurrentItemID - 1 );
        xPropertySet->setPropertyValue( GetPropertyName( BASEPROPERTY_CURRENTITEMID ), aAny );
    }


    void SAL_CALL UnoControlRoadmapModel::replaceByIndex( const sal_Int32 Index, const Any& Element)
    {
        Reference< XInterface > xRoadmapItem;
        Element >>= xRoadmapItem;
        MakeRMItemValidation( Index, xRoadmapItem);
        SetRMItemDefaultProperties( xRoadmapItem );
        maRoadmapItems.erase( maRoadmapItems.begin() + Index );
        maRoadmapItems.insert( maRoadmapItems.begin() + Index, xRoadmapItem);        //push_back( xRoadmapItem );
        ContainerEvent aEvent = GetContainerEvent(Index, xRoadmapItem);
        maContainerListeners.elementReplaced( aEvent );
    }


    Type SAL_CALL UnoControlRoadmapModel::getElementType()
    {
        Type aType = cppu::UnoType<XPropertySet>::get();
        return aType;
    }


    sal_Bool SAL_CALL UnoControlRoadmapModel::hasElements()
    {
        return !maRoadmapItems.empty();
    }


    void SAL_CALL UnoControlRoadmapModel::addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener )
    {
        maContainerListeners.addInterface( xListener );
    }

    void SAL_CALL UnoControlRoadmapModel::removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener )
    {
        maContainerListeners.removeInterface( xListener );
    }


    // = UnoRoadmapControl


    UnoRoadmapControl::UnoRoadmapControl()
        :maItemListeners( *this )
    {
    }

IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoRoadmapControl, UnoControlRoadmap_Base, UnoControlRoadmap_IBase )

css::uno::Any UnoRoadmapControl::queryAggregation(css::uno::Type const & aType) {
    auto ret = UnoControlRoadmap_Base::queryAggregation(aType);
    if (!ret.hasValue()) {
        ret = UnoControlRoadmap_IBase::queryInterface(aType);
    }
    return ret;
}


sal_Bool SAL_CALL UnoRoadmapControl::setModel(const Reference< XControlModel >& _rModel)
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


    OUString UnoRoadmapControl::GetComponentServiceName() const
    {
        return u"Roadmap"_ustr;
    }


    void UnoRoadmapControl::dispose()
    {
        EventObject aEvt;
        aEvt.Source = getXWeak();
        maItemListeners.disposeAndClear( aEvt );
        UnoControl::dispose();
    }


void UnoRoadmapControl::elementInserted( const ContainerEvent& rEvent )
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


void UnoRoadmapControl::elementRemoved( const ContainerEvent& rEvent )
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


void UnoRoadmapControl::elementReplaced( const ContainerEvent& rEvent )
{
    Reference< XContainerListener >  xPeer(getPeer(), UNO_QUERY);
    if ( xPeer.is() )
        xPeer->elementReplaced( rEvent );
}


void SAL_CALL UnoRoadmapControl::itemStateChanged( const ItemEvent& rEvent )
{
    sal_Int16 CurItemIndex = sal::static_int_cast< sal_Int16 >(rEvent.ItemId);
    Reference< XControlModel > xModel = getModel( );
    Reference< XPropertySet > xPropertySet( xModel, UNO_QUERY );
    xPropertySet->setPropertyValue( GetPropertyName( BASEPROPERTY_CURRENTITEMID ), Any(CurItemIndex) );
    if ( maItemListeners.getLength() )
        maItemListeners.itemStateChanged( rEvent );
}


void SAL_CALL UnoRoadmapControl::addItemListener( const Reference< XItemListener >& l )
{
    maItemListeners.addInterface( l );
    if( getPeer().is() && maItemListeners.getLength() == 1 )
    {
        Reference < XItemEventBroadcaster > xRoadmap( getPeer(), UNO_QUERY );
        xRoadmap->addItemListener( this );
    }
}


void SAL_CALL UnoRoadmapControl::removeItemListener( const Reference< XItemListener >& l )
{
    if( getPeer().is() && maItemListeners.getLength() == 1 )
    {
        Reference < XItemEventBroadcaster >  xRoadmap( getPeer(), UNO_QUERY );
        xRoadmap->removeItemListener( this );
    }

    maItemListeners.removeInterface( l );
}


void SAL_CALL UnoRoadmapControl::propertyChange( const PropertyChangeEvent& evt )
{
    Reference< XPropertyChangeListener >  xPeer(getPeer(), UNO_QUERY);
    if ( xPeer.is() )
        xPeer->propertyChange( evt );
}

OUString UnoRoadmapControl::getImplementationName()
{
    return u"stardiv.Toolkit.UnoRoadmapControl"_ustr;
}

css::uno::Sequence<OUString> UnoRoadmapControl::getSupportedServiceNames()
{
    auto s(UnoControlBase::getSupportedServiceNames());
    s.realloc(s.getLength() + 2);
    auto ps = s.getArray();
    ps[s.getLength() - 2] = "com.sun.star.awt.UnoControlRoadmap";
    ps[s.getLength() - 1] = "stardiv.vcl.control.Roadmap";
    return s;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_UnoControlRoadmapModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new toolkit::UnoControlRoadmapModel(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_UnoRoadmapControl_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new toolkit::UnoRoadmapControl());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
