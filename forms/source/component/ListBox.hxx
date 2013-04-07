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

#ifndef _FORMS_LISTBOX_HXX_
#define _FORMS_LISTBOX_HXX_

#include "FormComponent.hxx"
#include "cachedrowset.hxx"
#include "errorbroadcaster.hxx"
#include "entrylisthelper.hxx"

#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/form/XChangeBroadcaster.hpp>
#include <com/sun/star/sdbc/DataType.hpp>

#include <comphelper/asyncnotification.hxx>
#include <connectivity/FValue.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <vcl/timer.hxx>

#include <vector>

//.........................................................................
namespace frm
{

//==================================================================
//= OListBoxModel
//==================================================================
typedef ::std::vector< ::connectivity::ORowSetValue >   ValueList;

class OListBoxModel :public OBoundControlModel
                    ,public OEntryListHelper
                    ,public OErrorBroadcaster
{

    CachedRowSet                                m_aListRowSet;          // the row set to fill the list
    ::connectivity::ORowSetValue                m_aSaveValue;

    // <properties>
    ::com::sun::star::form::ListSourceType      m_eListSourceType;      // type der list source
    ::com::sun::star::uno::Any                  m_aBoundColumn;
    ValueList                                   m_aListSourceValues;
    ValueList                                   m_aBoundValues;         // do not write directly; use setBoundValues()
    mutable ValueList                           m_aConvertedBoundValues;
    mutable sal_Int32                           m_nConvertedBoundValuesType;
    ::com::sun::star::uno::Sequence<sal_Int16>  m_aDefaultSelectSeq;    // DefaultSelected
    // </properties>

    sal_Int16                                   m_nNULLPos;             // position of the NULL value in our list
    sal_Int32                                   m_nBoundColumnType;

private:
    ::connectivity::ORowSetValue getFirstSelectedValue() const;

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>   _getTypes();

public:
    DECLARE_DEFAULT_LEAF_XTOR( OListBoxModel );

// XServiceInfo
    IMPLEMENTATION_NAME(OListBoxModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

// UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OListBoxModel, OBoundControlModel);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                throw (::com::sun::star::uno::Exception);
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
                throw (::com::sun::star::lang::IllegalArgumentException);

protected:
    // XMultiPropertySet
    virtual void SAL_CALL   setPropertyValues(const ::com::sun::star::uno::Sequence< OUString >& PropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XPersistObject
    virtual OUString SAL_CALL    getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const;
    virtual void describeAggregateProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
    ) const;

    // XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw (::com::sun::star::uno::RuntimeException);

    // OPropertyChangeListener
    virtual void    _propertyChanged( const ::com::sun::star::beans::PropertyChangeEvent& _rEvt ) throw ( ::com::sun::star::uno::RuntimeException );

    // prevent method hiding
    using OBoundControlModel::getFastPropertyValue;
    using OBoundControlModel::setPropertyValues;

protected:
    // OBoundControlModel overridables
    virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( );
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
                            getSupportedBindingTypes();
    virtual ::com::sun::star::uno::Any
                            translateExternalValueToControlValue( const ::com::sun::star::uno::Any& _rExternalValue ) const;
    virtual ::com::sun::star::uno::Any
                            translateControlValueToExternalValue( ) const;
    virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset );

    virtual void            onConnectedDbColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxForm );
    virtual void            onDisconnectedDbColumn();

    virtual ::com::sun::star::uno::Any
                            getDefaultForReset() const;
    virtual void            resetNoBroadcast();

    virtual ::com::sun::star::uno::Any
                            getCurrentFormComponentValue() const;

    // OEntryListHelper overriables
    virtual void    stringItemListChanged( ControlModelLock& _rInstanceLock );
    virtual void    connectedExternalListSource( );
    virtual void    disconnectedExternalListSource( );
    virtual void    refreshInternalEntryList();

protected:
    DECLARE_XCLONEABLE();

private:
    void        loadData( bool _bForce );

    /** refreshes the list boxes list data
        @precond we don't actually have an external list source
    */
    void        impl_refreshDbEntryList( bool _bForce );

    void        setBoundValues(const ValueList&);
    void        clearBoundValues();

    ValueList   impl_getValues() const;

    sal_Int32   getValueType() const;

    void        convertBoundValues(sal_Int32 nType) const;

    bool        impl_hasBoundComponent() const { return m_nBoundColumnType != ::com::sun::star::sdbc::DataType::SQLNULL; }
};

//==================================================================
//= OListBoxControl
//==================================================================
typedef ::cppu::ImplHelper4 <   ::com::sun::star::awt::XFocusListener
                            ,   ::com::sun::star::awt::XItemListener
                            ,   ::com::sun::star::awt::XListBox
                            ,   ::com::sun::star::form::XChangeBroadcaster
                            >   OListBoxControl_BASE;

class OListBoxControl   :public OBoundControl
                        ,public OListBoxControl_BASE
                        ,public IEventProcessor
{
private:
    ::cppu::OInterfaceContainerHelper       m_aChangeListeners;
    ::cppu::OInterfaceContainerHelper       m_aItemListeners;

    ::com::sun::star::uno::Any              m_aCurrentSelection;
    Timer                                   m_aChangeTimer;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XListBox >
                                            m_xAggregateListBox;

    ::rtl::Reference< ::comphelper::AsyncEventNotifier >
                                            m_pItemBroadcaster;

protected:
    // UNO Anbindung
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>   _getTypes();

public:
    OListBoxControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    virtual ~OListBoxControl();

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OListBoxControl, OBoundControl);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);

// XServiceInfo
    IMPLEMENTATION_NAME(OListBoxControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

// XChangeBroadcaster
        virtual void SAL_CALL addChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);

// XFocusListener
    virtual void SAL_CALL focusGained(const ::com::sun::star::awt::FocusEvent& _rEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL focusLost(const ::com::sun::star::awt::FocusEvent& _rEvent) throw(::com::sun::star::uno::RuntimeException);

// XItemListener
    virtual void SAL_CALL itemStateChanged(const ::com::sun::star::awt::ItemEvent& _rEvent) throw(::com::sun::star::uno::RuntimeException);

// XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw (::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// XListBox
    virtual void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addItem( const OUString& aItem, ::sal_Int16 nPos ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addItems( const ::com::sun::star::uno::Sequence< OUString >& aItems, ::sal_Int16 nPos ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeItems( ::sal_Int16 nPos, ::sal_Int16 nCount ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getItemCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getItem( ::sal_Int16 nPos ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getItems(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getSelectedItemPos(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int16 > SAL_CALL getSelectedItemsPos(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getSelectedItem(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSelectedItems(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL selectItemPos( ::sal_Int16 nPos, ::sal_Bool bSelect ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL selectItemsPos( const ::com::sun::star::uno::Sequence< ::sal_Int16 >& aPositions, ::sal_Bool bSelect ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL selectItem( const OUString& aItem, ::sal_Bool bSelect ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isMutipleMode(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMultipleMode( ::sal_Bool bMulti ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getDropDownLineCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDropDownLineCount( ::sal_Int16 nLines ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL makeVisible( ::sal_Int16 nEntry ) throw (::com::sun::star::uno::RuntimeException);

protected:
    // IEventProcessor
    virtual void processEvent( const ::comphelper::AnyEvent& _rEvent );

private:
    DECL_LINK( OnTimeout, void* );
};

//.........................................................................
}
//.........................................................................

#endif // _FORMS_LISTBOX_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
