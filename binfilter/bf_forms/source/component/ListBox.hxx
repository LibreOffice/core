/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <vcl/timer.hxx>

#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/form/XChangeBroadcaster.hpp>
#include <cppuhelper/implbase1.hxx>
#include "errorbroadcaster.hxx"
namespace binfilter {

//.........................................................................
namespace frm
{

const ::rtl::OUString LISTBOX_EMPTY_VALUE = ::rtl::OUString::createFromAscii("$$$empty$$$");

//==================================================================
//= OListBoxModel
//==================================================================
typedef ::cppu::ImplHelper1 <	::com::sun::star::util::XRefreshable
                            >	OListBoxModel_BASE;
 
class OListBoxModel :public OBoundControlModel
                    ,public OListBoxModel_BASE
                    ,public OErrorBroadcaster
                    ,public ::comphelper::OAggregationArrayUsageHelper< OListBoxModel >
{
    ::com::sun::star::uno::Any					m_aSaveValue;

    // <properties>
    ::com::sun::star::form::ListSourceType		m_eListSourceType;		// type der list source
    ::com::sun::star::uno::Any					m_aBoundColumn;
    StringSequence								m_aListSourceSeq;		//
    StringSequence								m_aValueSeq;			// alle Werte, readonly
    ::com::sun::star::uno::Sequence<sal_Int16>	m_aDefaultSelectSeq;	// DefaultSelected
    // </properties>

    ::cppu::OInterfaceContainerHelper	m_aRefreshListeners;

    static sal_Int32		nSelectHandle;
    // [properties]

    sal_Int16				m_nNULLPos; // Position an der der NULLwert abgelegt wird
    sal_Bool				m_bBoundComponent : 1;

    // Helper functions
    StringSequence GetCurValueSeq() const;

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>	_getTypes();

protected:
    // UNO Anbindung
    virtual void _onValueChanged();

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

// XLoadListener
    virtual void		 _loaded(const ::com::sun::star::lang::EventObject& rEvent);
    virtual void		 _unloaded();

// XBoundComponent
    virtual sal_Bool _commit();

// XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

// XPersistObject
    virtual ::rtl::OUString SAL_CALL	getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// XReset
    virtual void _reset();

// XRefreshable
    virtual void SAL_CALL refresh() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addRefreshListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRefreshListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);

    // OAggregationArrayUsageHelper
    virtual void fillProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

protected:
    void loadData();

    DECLARE_XCLONEABLE();
};

//==================================================================
//= OListBoxControl
//==================================================================
typedef ::cppu::ImplHelper3<	::com::sun::star::awt::XFocusListener,
                                ::com::sun::star::awt::XItemListener,
                                ::com::sun::star::form::XChangeBroadcaster > OListBoxControl_BASE;
 
class OListBoxControl	:public OBoundControl
                        ,public OListBoxControl_BASE
{
    ::cppu::OInterfaceContainerHelper		m_aChangeListeners;

    ::com::sun::star::uno::Any				m_aCurrentSelection;
    Timer									m_aChangeTimer;

protected:
    // UNO Anbindung
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>	_getTypes();

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

private:
    DECL_LINK( OnTimeout, void* );
};

//.........................................................................
}
//.........................................................................

}//end of namespace binfilter
#endif // _FORMS_LISTBOX_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
