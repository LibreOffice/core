/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ListBox.hxx,v $
 * $Revision: 1.22 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _FORMS_LISTBOX_HXX_
#define _FORMS_LISTBOX_HXX_

#include "FormComponent.hxx"
#include "cachedrowset.hxx"
#include "errorbroadcaster.hxx"
#include "entrylisthelper.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/form/XChangeBroadcaster.hpp>
/** === end UNO includes === **/

#include <vcl/timer.hxx>

#include <comphelper/asyncnotification.hxx>

#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase1.hxx>

#include <memory>

//.........................................................................
namespace frm
{

//==================================================================
//= OListBoxModel
//==================================================================
typedef ::cppu::ImplHelper1 <   ::com::sun::star::util::XRefreshable
                            >   OListBoxModel_BASE;

class OListBoxModel :public OBoundControlModel
                    ,public OListBoxModel_BASE
                    ,public OEntryListHelper
                    ,public OErrorBroadcaster
{
    CachedRowSet                                m_aListRowSet;          // the row set to fill the list

    ::com::sun::star::uno::Any                  m_aSaveValue;

    // <properties>
    ::com::sun::star::form::ListSourceType      m_eListSourceType;      // type der list source
    ::com::sun::star::uno::Any                  m_aBoundColumn;
    StringSequence                              m_aListSourceSeq;       //
    StringSequence                              m_aValueSeq;            // alle Werte, readonly
    ::com::sun::star::uno::Sequence<sal_Int16>  m_aDefaultSelectSeq;    // DefaultSelected
    // </properties>

    ::cppu::OInterfaceContainerHelper           m_aRefreshListeners;

    sal_Int16                                   m_nNULLPos;             // position of the NULL value in our list
    sal_Bool                                    m_bBoundComponent : 1;

    /** type how we should transfer our selection to external value bindings
    */
    enum TransferSelection
    {
        tsIndexList,        /// as list of indexes of selected entries
        tsIndex,            /// as index of the selected entry
        tsEntryList,        /// as list of string representations of selected entries
        tsEntry             /// as string representation of the selected entry
    };
    TransferSelection                           m_eTransferSelectionAs;

private:
    // Helper functions
    StringSequence GetCurValueSeq() const;

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
    virtual void SAL_CALL   setPropertyValues(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XPersistObject
    virtual ::rtl::OUString SAL_CALL    getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XRefreshable
    virtual void SAL_CALL refresh() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addRefreshListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRefreshListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const;
    virtual void describeAggregateProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
    ) const;

    // XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw (::com::sun::star::uno::RuntimeException);

    // prevent method hiding
    using OBoundControlModel::getFastPropertyValue;
    using OBoundControlModel::setPropertyValues;

protected:
    // OBoundControlModel overridables
    virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( );
    virtual ::com::sun::star::uno::Any
                            translateExternalValueToControlValue( ) const;
    virtual ::com::sun::star::uno::Any
                            translateControlValueToExternalValue( ) const;
    virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset );

    virtual void            onConnectedDbColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxForm );
    virtual void            onDisconnectedDbColumn();
    virtual void            onConnectedExternalValue( );

    virtual ::com::sun::star::uno::Any
                            getDefaultForReset() const;

    virtual sal_Bool        approveValueBinding( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XValueBinding >& _rxBinding );

    virtual ::com::sun::star::uno::Any
                            getCurrentFormComponentValue() const;

    // OEntryListHelper overriables
    virtual void    stringItemListChanged( );
    virtual void    connectedExternalListSource( );
    virtual void    disconnectedExternalListSource( );

protected:
    DECLARE_XCLONEABLE();

private:
    void        loadData();

    /**
    @precond we don't actually have an external list source
    */
    void        implRefreshListFromDbBinding( );

    StringSequence
                impl_getValues() const { return m_aValueSeq.getLength() ? m_aValueSeq : getStringItemList(); }
};

//==================================================================
//= OListBoxControl
//==================================================================
typedef ::cppu::ImplHelper4 <   ::com::sun::star::awt::XFocusListener
                            ,   ::com::sun::star::awt::XItemListener
                            ,   ::com::sun::star::awt::XListBox
                            ,   ::com::sun::star::form::XChangeBroadcaster
                            >   OListBoxControl_BASE;

class ChangeListeners;

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
    virtual void SAL_CALL addItem( const ::rtl::OUString& aItem, ::sal_Int16 nPos ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addItems( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aItems, ::sal_Int16 nPos ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeItems( ::sal_Int16 nPos, ::sal_Int16 nCount ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getItemCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getItem( ::sal_Int16 nPos ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getItems(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getSelectedItemPos(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int16 > SAL_CALL getSelectedItemsPos(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSelectedItem(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSelectedItems(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL selectItemPos( ::sal_Int16 nPos, ::sal_Bool bSelect ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL selectItemsPos( const ::com::sun::star::uno::Sequence< ::sal_Int16 >& aPositions, ::sal_Bool bSelect ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL selectItem( const ::rtl::OUString& aItem, ::sal_Bool bSelect ) throw (::com::sun::star::uno::RuntimeException);
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

