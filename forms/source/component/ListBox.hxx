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
#include <cppuhelper/implbase.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>

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


namespace frm
{

typedef ::std::vector< ::connectivity::ORowSetValue >   ValueList;

class OListBoxModel :public OBoundControlModel
                    ,public OEntryListHelper
                    ,public OErrorBroadcaster
{

    CachedRowSet                                m_aListRowSet;          // the row set to fill the list
    ::connectivity::ORowSetValue                m_aSaveValue;

    // <properties>
    css::form::ListSourceType                   m_eListSourceType;      // type der list source
    css::uno::Any                               m_aBoundColumn;
    ValueList                                   m_aListSourceValues;
    ValueList                                   m_aBoundValues;         // do not write directly; use setBoundValues()
    mutable ValueList                           m_aConvertedBoundValues;
    mutable sal_Int32                           m_nConvertedBoundValuesType;
    css::uno::Sequence<sal_Int16>               m_aDefaultSelectSeq;    // DefaultSelected
    // </properties>

    mutable sal_Int16                           m_nNULLPos;             // position of the NULL value in our list
    sal_Int32                                   m_nBoundColumnType;

private:
    ::connectivity::ORowSetValue getFirstSelectedValue() const;

    virtual css::uno::Sequence< css::uno::Type>   _getTypes() override;

public:
    DECLARE_DEFAULT_LEAF_XTOR( OListBoxModel );

// XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("com.sun.star.form.OListBoxModel"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

// UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OListBoxModel, OBoundControlModel)
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) throw (css::uno::RuntimeException, std::exception) override;

// OComponentHelper
    virtual void SAL_CALL disposing() override;

// OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle) const override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue )
                throw (css::uno::Exception, std::exception) override;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, sal_Int32 _nHandle, const css::uno::Any& _rValue )
                throw (css::lang::IllegalArgumentException) override;

protected:
    static const ::connectivity::ORowSetValue s_aEmptyValue;
    static const ::connectivity::ORowSetValue s_aEmptyStringValue;

    // XMultiPropertySet
    virtual void SAL_CALL   setPropertyValues(const css::uno::Sequence< OUString >& PropertyNames, const css::uno::Sequence< css::uno::Any >& Values) throw(css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XPersistObject
    virtual OUString SAL_CALL    getServiceName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL
        write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL
        read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const override;
    virtual void describeAggregateProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rAggregateProps
    ) const override;

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw (css::uno::RuntimeException, std::exception) override;

    // OPropertyChangeListener
    virtual void    _propertyChanged( const css::beans::PropertyChangeEvent& _rEvt ) throw ( css::uno::RuntimeException ) override;

    // prevent method hiding
    using OBoundControlModel::getFastPropertyValue;
    using OBoundControlModel::setPropertyValues;

protected:
    // OBoundControlModel overridables
    virtual css::uno::Any   translateDbColumnToControlValue( ) override;
    virtual css::uno::Sequence< css::uno::Type >
                            getSupportedBindingTypes() override;
    virtual css::uno::Any   translateExternalValueToControlValue( const css::uno::Any& _rExternalValue ) const override;
    virtual css::uno::Any   translateControlValueToExternalValue( ) const override;
    virtual css::uno::Any   translateControlValueToValidatableValue( ) const override;
    virtual bool            commitControlValueToDbColumn( bool _bPostReset ) override;

    virtual void            onConnectedDbColumn( const css::uno::Reference< css::uno::XInterface >& _rxForm ) override;
    virtual void            onDisconnectedDbColumn() override;

    virtual css::uno::Any   getDefaultForReset() const override;
    virtual void            resetNoBroadcast() override;

    virtual css::uno::Any   getCurrentFormComponentValue() const override;

    // OEntryListHelper overridables
    virtual void    stringItemListChanged( ControlModelLock& _rInstanceLock ) override;
    virtual void    connectedExternalListSource( ) override;
    virtual void    disconnectedExternalListSource( ) override;
    virtual void    refreshInternalEntryList() override;

protected:
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

    void init();
    css::uno::Any getCurrentSingleValue() const;
    css::uno::Any getCurrentMultiValue() const;
    css::uno::Sequence< sal_Int16 > translateBindingValuesToControlValue(
        const css::uno::Sequence< const css::uno::Any > &i_aValues)
        const;
    css::uno::Sequence< sal_Int16 > translateDbValueToControlValue(
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

    bool        impl_hasBoundComponent() const { return m_nBoundColumnType != css::sdbc::DataType::SQLNULL; }
};


//= OListBoxControl

typedef ::cppu::ImplHelper  <   css::awt::XFocusListener
                            ,   css::awt::XItemListener
                            ,   css::awt::XListBox
                            ,   css::form::XChangeBroadcaster
                            >   OListBoxControl_BASE;

class OListBoxControl   :public OBoundControl
                        ,public OListBoxControl_BASE
                        ,public IEventProcessor
{
private:
    ::cppu::OInterfaceContainerHelper       m_aChangeListeners;
    ::cppu::OInterfaceContainerHelper       m_aItemListeners;

    css::uno::Any                           m_aCurrentSelection;
    Idle                                    m_aChangeIdle;

    css::uno::Reference< css::awt::XListBox >
                                            m_xAggregateListBox;

    ::rtl::Reference< ::comphelper::AsyncEventNotifier >
                                            m_pItemBroadcaster;

protected:
    // UNO Anbindung
    virtual css::uno::Sequence< css::uno::Type>   _getTypes() override;

public:
    explicit OListBoxControl(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory);
    virtual ~OListBoxControl();

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OListBoxControl, OBoundControl)
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) throw (css::uno::RuntimeException, std::exception) override;

// XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("com.sun.star.form.OListBoxControl"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

// XChangeBroadcaster
        virtual void SAL_CALL addChangeListener(const css::uno::Reference< css::form::XChangeListener>& _rxListener) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeChangeListener(const css::uno::Reference< css::form::XChangeListener>& _rxListener) throw(css::uno::RuntimeException, std::exception) override;

// XFocusListener
    virtual void SAL_CALL focusGained(const css::awt::FocusEvent& _rEvent) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL focusLost(const css::awt::FocusEvent& _rEvent) throw(css::uno::RuntimeException, std::exception) override;

// XItemListener
    virtual void SAL_CALL itemStateChanged(const css::awt::ItemEvent& _rEvent) throw(css::uno::RuntimeException, std::exception) override;

// XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw (css::uno::RuntimeException, std::exception) override;

// OComponentHelper
    virtual void SAL_CALL disposing() override;

// XListBox
    virtual void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& l ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addItem( const OUString& aItem, ::sal_Int16 nPos ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addItems( const css::uno::Sequence< OUString >& aItems, ::sal_Int16 nPos ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeItems( ::sal_Int16 nPos, ::sal_Int16 nCount ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL getItemCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getItem( ::sal_Int16 nPos ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getItems(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL getSelectedItemPos(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< ::sal_Int16 > SAL_CALL getSelectedItemsPos(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getSelectedItem(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSelectedItems(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL selectItemPos( ::sal_Int16 nPos, sal_Bool bSelect ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL selectItemsPos( const css::uno::Sequence< ::sal_Int16 >& aPositions, sal_Bool bSelect ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL selectItem( const OUString& aItem, sal_Bool bSelect ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isMutipleMode(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMultipleMode( sal_Bool bMulti ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL getDropDownLineCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDropDownLineCount( ::sal_Int16 nLines ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL makeVisible( ::sal_Int16 nEntry ) throw (css::uno::RuntimeException, std::exception) override;

protected:
    // IEventProcessor
    virtual void processEvent( const ::comphelper::AnyEvent& _rEvent ) override;

private:
    DECL_LINK_TYPED( OnTimeout, Idle*, void );
};


}


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_LISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
