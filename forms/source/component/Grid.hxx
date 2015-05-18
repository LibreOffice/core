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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_GRID_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_GRID_HXX

#include "errorbroadcaster.hxx"
#include "FormComponent.hxx"
#include "formcontrolfont.hxx"
#include "InterfaceContainer.hxx"

#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/sdb/XRowSetSupplier.hpp>
#include <com/sun/star/sdb/XRowSetChangeBroadcaster.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/implbase7.hxx>


namespace frm
{

class OGridColumn;

    struct ColumnDescription : public ElementDescription
    {
    public:
        OGridColumn*                pColumn; // not owned by this instance! only to prevent duplicate XUnoTunnel usage
    };


// OGridControlModel

typedef ::cppu::ImplHelper7 <   ::com::sun::star::awt::XControlModel
                            ,   ::com::sun::star::form::XGridColumnFactory
                            ,   ::com::sun::star::form::XReset
                            ,   ::com::sun::star::view::XSelectionSupplier
                            ,   ::com::sun::star::sdb::XSQLErrorListener
                            ,   ::com::sun::star::sdb::XRowSetSupplier
                            ,   ::com::sun::star::sdb::XRowSetChangeBroadcaster
                            >   OGridControlModel_BASE;

class OGridControlModel :public OControlModel
                        ,public OInterfaceContainer
                        ,public OErrorBroadcaster
                        ,public FontControlModel
                        ,public OGridControlModel_BASE
{
    ::cppu::OInterfaceContainerHelper       m_aSelectListeners,
                                            m_aResetListeners,
                                            m_aRowSetChangeListeners;

// [properties]
    ::com::sun::star::uno::Any              m_aRowHeight; // Row height
    ::com::sun::star::uno::Any              m_aTabStop;
    ::com::sun::star::uno::Any              m_aBackgroundColor;
    ::com::sun::star::uno::Any              m_aCursorColor; // transient
    ::com::sun::star::uno::Any              m_aBorderColor;
    OUString                         m_aDefaultControl;
    OUString                         m_sHelpText;
// [properties]

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xSelection;

// [properties]
    OUString                m_sHelpURL;                 // URL
    sal_Int16               m_nBorder;
    sal_Int16               m_nWritingMode;
    sal_Int16               m_nContextWritingMode;
    bool                    m_bEnableVisible    : 1;
    bool                    m_bEnable           : 1;
    bool                    m_bNavigation       : 1;
    bool                    m_bRecordMarker     : 1;
    bool                    m_bPrintable        : 1;
    bool                    m_bAlwaysShowCursor : 1;    // transient
    bool                    m_bDisplaySynchron  : 1;    // transient
// [properties]

protected:
    void _reset();

public:
    DECLARE_DEFAULT_LEAF_XTOR( OGridControlModel );

    // UNO Binding
    DECLARE_UNO3_AGG_DEFAULTS(OGridControlModel, OControlModel)
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XChild
    virtual void SAL_CALL setParent(const InterfaceRef& Parent) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return OUString("com.sun.star.form.OGridControlModel"); }

    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XReset
    virtual void SAL_CALL reset() throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XSelectionSupplier
    virtual sal_Bool SAL_CALL select(const ::com::sun::star::uno::Any& aElement) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addSelectionChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeSelectionChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XGridColumnFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> SAL_CALL createColumn(const OUString& ColumnType) throw ( :: com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual StringSequence SAL_CALL getColumnTypes() throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPersistObject
    virtual OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertySet
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                        throw(::com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

    // XPropertyState
    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const SAL_OVERRIDE;

    // XSQLErrorListener
    virtual void SAL_CALL errorOccured( const ::com::sun::star::sdb::SQLErrorEvent& _rEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XRowSetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > SAL_CALL getRowSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRowSet( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xDataSource ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XRowSetChangeBroadcaster
    virtual void SAL_CALL addRowSetChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetChangeListener >& i_Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeRowSetChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetChangeListener >& i_Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const SAL_OVERRIDE;

    // prevent method hiding
    using OControlModel::disposing;
    using OControlModel::getFastPropertyValue;

protected:
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    virtual void approveNewElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject,
            ElementDescription* _pElement
        ) SAL_OVERRIDE;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>  createColumn(sal_Int32 nTypeId) const;

    static OGridColumn* getColumnImplementation(const InterfaceRef& _rxIFace);

    virtual ElementDescription* createElementMetaData( ) SAL_OVERRIDE;

protected:
    virtual void implRemoved(const InterfaceRef& _rxObject) SAL_OVERRIDE;
    virtual void implInserted( const ElementDescription* _pElement ) SAL_OVERRIDE;
    virtual void impl_replacedElement(
                    const ::com::sun::star::container::ContainerEvent& _rEvent,
                    ::osl::ClearableMutexGuard& _rInstanceLock
                ) SAL_OVERRIDE;

    void gotColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxColumn);
    void lostColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxColumn);

    void cloneColumns( const OGridControlModel* _pOriginalContainer );
};


}



#endif // INCLUDED_FORMS_SOURCE_COMPONENT_GRID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
