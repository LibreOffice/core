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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_LISTBOX_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_LISTBOX_HXX

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

/** ListBox is a bit confusing / different from other form components,
    so here are a few notes:

    The general design philosophy is that a ListBox is a mechanism
    to translate back and forth between:
     1) *display* values (strings that the user sees and chooses)
     2) *binding* values, which is what the program (for a dialog),
        the database, ... cares about.

    A non-data aware ListBox exposes this mechanism through
    com.sun.star.awt.XItemList (get|set)ItemData.

    In a data-aware ListBox, this is naturally embodied by the
    StringItemList on the one hand, and the ValueList on the other
    hand (where, depending on ListSourceType, the ValueList is
    possibly automatically filled from the BoundColumn of the
    ListSource).

    This source file implements data-aware ListBox, and the rest
    of this comment applies to data-aware ListBox (only).

    In all public APIs of the *model* (OListBoxModel),
    the value of the control is the *binding* value.
    That is what the bound database field gets,
    that is what a validator validates,
    that is what an external value binding
    (com.sun.star.form.binding.XValueBinding)
    exchanges with the control.

    As an *implementation* choice, we keep the current value of the
    ListBox as a sequence of *indices* in the value list, and do the
    lookup on demand:

     - ListBox's content propery (or value property, sorry the
       terminology is not always consistent) is SelectedItems which is
       a sequence of *indices* in the value list.

     - That is used to synchronise with our peer (UnoControlListBoxModel).

    In particular, note that getCurrentValue() is a public API (and
    deals with bound values), but getControlValue and
    (do)setControlValue are *internal* implementation helpers that
    deal with *indices*.

    Note that the *view* (OListBoxControl) presents a different story
    than the model. E.g. the "SelectedItems" property is *display* *values*.
*/

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
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);

// UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OListBoxModel, OBoundControlModel);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException, std::exception);

// OComponentHelper
    virtual void SAL_CALL disposing();

// OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                throw (::com::sun::star::uno::Exception, std::exception);
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
                throw (::com::sun::star::lang::IllegalArgumentException);

protected:
    // XMultiPropertySet
    virtual void SAL_CALL   setPropertyValues(const ::com::sun::star::uno::Sequence< OUString >& PropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XPersistObject
    virtual OUString SAL_CALL    getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL
        write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL
        read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception);

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const;
    virtual void describeAggregateProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
    ) const;

    // XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw (::com::sun::star::uno::RuntimeException, std::exception);

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
    virtual ::com::sun::star::uno::Any
                            translateControlValueToValidatableValue( ) const;
    virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset );

    virtual void            onConnectedDbColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxForm );
    virtual void            onDisconnectedDbColumn();

    virtual ::com::sun::star::uno::Any
                            getDefaultForReset() const;
    virtual void            resetNoBroadcast();

    virtual ::com::sun::star::uno::Any
                            getCurrentFormComponentValue() const;

    // OEntryListHelper overridables
    virtual void    stringItemListChanged( ControlModelLock& _rInstanceLock );
    virtual void    connectedExternalListSource( );
    virtual void    disconnectedExternalListSource( );
    virtual void    refreshInternalEntryList();

protected:
    DECLARE_XCLONEABLE();

    void init();
    ::com::sun::star::uno::Any getCurrentSingleValue() const;
    ::com::sun::star::uno::Any getCurrentMultiValue() const;
    ::com::sun::star::uno::Sequence< sal_Int16 > translateBindingValuesToControlValue(
        const ::com::sun::star::uno::Sequence< const ::com::sun::star::uno::Any > &i_aValues)
        const;
    ::com::sun::star::uno::Sequence< sal_Int16 > translateDbValueToControlValue(
        const ::connectivity::ORowSetValue &aValue)
        const;


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
    OListBoxControl(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory);
    virtual ~OListBoxControl();

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OListBoxControl, OBoundControl);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException, std::exception);

// XServiceInfo
    IMPLEMENTATION_NAME(OListBoxControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);

// XChangeBroadcaster
        virtual void SAL_CALL addChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception);

// XFocusListener
    virtual void SAL_CALL focusGained(const ::com::sun::star::awt::FocusEvent& _rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL focusLost(const ::com::sun::star::awt::FocusEvent& _rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception);

// XItemListener
    virtual void SAL_CALL itemStateChanged(const ::com::sun::star::awt::ItemEvent& _rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception);

// XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw (::com::sun::star::uno::RuntimeException, std::exception);

// OComponentHelper
    virtual void SAL_CALL disposing();

// XListBox
    virtual void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addItem( const OUString& aItem, ::sal_Int16 nPos ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addItems( const ::com::sun::star::uno::Sequence< OUString >& aItems, ::sal_Int16 nPos ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeItems( ::sal_Int16 nPos, ::sal_Int16 nCount ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::sal_Int16 SAL_CALL getItemCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getItem( ::sal_Int16 nPos ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getItems(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::sal_Int16 SAL_CALL getSelectedItemPos(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int16 > SAL_CALL getSelectedItemsPos(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getSelectedItem(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSelectedItems(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL selectItemPos( ::sal_Int16 nPos, ::sal_Bool bSelect ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL selectItemsPos( const ::com::sun::star::uno::Sequence< ::sal_Int16 >& aPositions, ::sal_Bool bSelect ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL selectItem( const OUString& aItem, ::sal_Bool bSelect ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::sal_Bool SAL_CALL isMutipleMode(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setMultipleMode( ::sal_Bool bMulti ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::sal_Int16 SAL_CALL getDropDownLineCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setDropDownLineCount( ::sal_Int16 nLines ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL makeVisible( ::sal_Int16 nEntry ) throw (::com::sun::star::uno::RuntimeException, std::exception);

protected:
    // IEventProcessor
    virtual void processEvent( const ::comphelper::AnyEvent& _rEvent );

private:
    DECL_LINK( OnTimeout, void* );
};

//.........................................................................
}
//.........................................................................

#endif // INCLUDED_FORMS_SOURCE_COMPONENT_LISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
