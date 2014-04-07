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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_DATABASEFORM_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_DATABASEFORM_HXX

#include <sal/config.h>

#include <vector>

#include "propertybaghelper.hxx"
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdb/XSQLErrorListener.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#include <com/sun/star/form/FormSubmitMethod.hpp>
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#include <com/sun/star/form/XSubmit.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/form/XDatabaseParameterBroadcaster2.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdb/XRowSetApproveListener.hpp>
#include <com/sun/star/sdb/XRowSetApproveBroadcaster.hpp>
#include <com/sun/star/form/NavigationBarMode.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/form/XLoadListener.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>


#include <tools/link.hxx>
#include "InterfaceContainer.hxx"

#include <connectivity/parameters.hxx>
#include <connectivity/filtermanager.hxx>
#include <connectivity/warningscontainer.hxx>

#include "listenercontainers.hxx"
#include <comphelper/propmultiplex.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/implbase12.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase7.hxx>

namespace com { namespace sun { namespace star { namespace sdbc {
    class SQLException;
} } } }

class Timer;
class INetMIMEMessage;


namespace frm
{



//= html tools


const sal_uInt16 SUCCESSFUL_REPRESENT_TEXT          = 0x0001;
const sal_uInt16 SUCCESSFUL_REPRESENT_FILE          = 0x0002;


class HtmlSuccessfulObj
{
public:
    OUString     aName;
    OUString     aValue;
    sal_uInt16          nRepresentation;

    HtmlSuccessfulObj( const OUString& _rName, const OUString& _rValue,
        sal_uInt16 _nRepresent = SUCCESSFUL_REPRESENT_TEXT )
        :aName( _rName )
        ,aValue( _rValue )
        ,nRepresentation( _nRepresent )
    {
    }

    HtmlSuccessfulObj()
    {
    }
};

typedef std::vector<HtmlSuccessfulObj> HtmlSuccessfulObjList;


class OGroupManager;
class OFormSubmitResetThread;
typedef ::cppu::ImplHelper12    <   ::com::sun::star::form::XForm
                                ,   ::com::sun::star::awt::XTabControllerModel
                                ,   ::com::sun::star::form::XLoadListener
                                ,   ::com::sun::star::sdbc::XRowSetListener
                                ,   ::com::sun::star::sdb::XRowSetApproveListener
                                ,   ::com::sun::star::form::XDatabaseParameterBroadcaster2
                                ,   ::com::sun::star::sdb::XSQLErrorListener
                                ,   ::com::sun::star::sdb::XSQLErrorBroadcaster
                                ,   ::com::sun::star::form::XReset
                                ,   ::com::sun::star::form::XSubmit
                                ,   ::com::sun::star::form::XLoadable
                                ,   ::com::sun::star::container::XNamed
                                >   ODatabaseForm_BASE1;


typedef ::cppu::ImplHelper4 <   ::com::sun::star::lang::XServiceInfo
                            ,   ::com::sun::star::beans::XPropertyContainer
                            ,   ::com::sun::star::beans::XPropertyAccess
                            ,   ::com::sun::star::sdbc::XWarningsSupplier
                            >   ODatabaseForm_BASE2;

typedef ::cppu::ImplHelper7<    ::com::sun::star::sdbc::XCloseable,
                                ::com::sun::star::sdbc::XRowSet,
                                ::com::sun::star::sdb::XCompletedExecution,
                                ::com::sun::star::sdb::XRowSetApproveBroadcaster,
                                ::com::sun::star::sdbc::XResultSetUpdate,
                                ::com::sun::star::sdbcx::XDeleteRows,
                                ::com::sun::star::sdbc::XParameters > ODatabaseForm_BASE3;


class ODatabaseForm :public OFormComponents
                    ,public OPropertySetAggregationHelper
                    ,public OPropertyChangeListener
                    ,public ODatabaseForm_BASE1
                    ,public ODatabaseForm_BASE2
                    ,public ODatabaseForm_BASE3
                    ,public IPropertyBagHelperContext
{
    friend class OFormSubmitResetThread;

        // listener administration
    ::cppu::OInterfaceContainerHelper   m_aLoadListeners;
    ::cppu::OInterfaceContainerHelper   m_aRowSetApproveListeners;
    ::cppu::OInterfaceContainerHelper   m_aRowSetListeners;
    ::cppu::OInterfaceContainerHelper   m_aSubmitListeners;
    ::cppu::OInterfaceContainerHelper   m_aErrorListeners;
    ResetListeners                      m_aResetListeners;
    ::osl::Mutex                        m_aResetSafety;
    ::com::sun::star::uno::Any          m_aCycle;
    ::com::sun::star::uno::Any          m_aIgnoreResult; // set when we are a subform and our master form positioned on a new row
    ::com::sun::star::uno::Sequence< OUString >                      m_aMasterFields;
    ::com::sun::star::uno::Sequence< OUString >                      m_aDetailFields;

    // the object doin' most of the work - an SDB-rowset
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation>      m_xAggregate;
    // same object, interface as member because of performance reasons
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>          m_xAggregateAsRowSet;

    PropertyBagHelper           m_aPropertyBagHelper;
    ::dbtools::WarningsContainer    m_aWarnings;
    OPropertyChangeMultiplexer* m_pAggregatePropertyMultiplexer;
    // Verwaltung der ControlGruppen
    OGroupManager*              m_pGroupManager;
    ::dbtools::ParameterManager m_aParameterManager;
    ::dbtools::FilterManager    m_aFilterManager;
    Timer*                      m_pLoadTimer;

    OFormSubmitResetThread*     m_pThread;
    OUString             m_sCurrentErrorContext;
                // will be used as additional context information
                // when an exception is catched and forwarded to the listeners

    sal_Int32                       m_nResetsPending;
//  <overwritten_properties>
    sal_Int32                   m_nPrivileges;
    sal_Bool                    m_bInsertOnly;
//  </overwritten_properties>

//  <properties>
    ::com::sun::star::uno::Any  m_aControlBorderColorFocus;
    ::com::sun::star::uno::Any  m_aControlBorderColorMouse;
    ::com::sun::star::uno::Any  m_aControlBorderColorInvalid;
    ::com::sun::star::uno::Any  m_aDynamicControlBorder;
    OUString             m_sName;
    OUString             m_aTargetURL;
    OUString             m_aTargetFrame;
    ::com::sun::star::form::FormSubmitMethod    m_eSubmitMethod;
    ::com::sun::star::form::FormSubmitEncoding  m_eSubmitEncoding;
    ::com::sun::star::form::NavigationBarMode   m_eNavigation;
    sal_Bool                    m_bAllowInsert : 1;
    sal_Bool                    m_bAllowUpdate : 1;
    sal_Bool                    m_bAllowDelete : 1;
//  </properties>
    sal_Bool                    m_bLoaded : 1;
    sal_Bool                    m_bSubForm : 1;
    sal_Bool                    m_bForwardingConnection : 1;    // sal_True if we're setting the ActiveConnection on the aggregate
    sal_Bool                    m_bSharingConnection : 1;       // sal_True if the connection we're using is shared with out parent

public:
    ODatabaseForm(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory);
    ODatabaseForm( const ODatabaseForm& _cloneSource );
    virtual ~ODatabaseForm();

    // UNO binding
    DECLARE_UNO3_AGG_DEFAULTS(ODatabaseForm, OFormComponents)
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XComponent
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // property handling
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception, std::exception ) SAL_OVERRIDE;

    ::com::sun::star::uno::Any  SAL_CALL getFastPropertyValue( sal_Int32 nHandle )
       throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void fire( sal_Int32 * pnHandles, const ::com::sun::star::uno::Any * pNewValues, const ::com::sun::star::uno::Any * pOldValues, sal_Int32 nCount, sal_Bool bVetoable );

    // IPropertyBagHelperContext
    virtual ::osl::Mutex&   getMutex() SAL_OVERRIDE;
    virtual void            describeFixedAndAggregateProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& _out_rFixedProperties,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& _out_rAggregateProperties
    ) const SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet >
                            getPropertiesInterface() SAL_OVERRIDE;

    // com::sun::star::beans::XPropertyState
    virtual ::com::sun::star::beans::PropertyState getPropertyStateByHandle(sal_Int32 nHandle) SAL_OVERRIDE;
    virtual void setPropertyToDefaultByHandle(sal_Int32 nHandle) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle(sal_Int32 nHandle) const SAL_OVERRIDE;

    // com::sun::star::sdbc::XSQLErrorBroadcaster
    virtual void SAL_CALL addSQLErrorListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeSQLErrorListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::form::XForm
    // nothing to implement

    // com::sun::star::form::XReset
    virtual void SAL_CALL reset() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::form::XSubmit
    virtual void SAL_CALL submit(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& aControl, const ::com::sun::star::awt::MouseEvent& aMouseEvt) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addSubmitListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XSubmitListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeSubmitListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XSubmitListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::container::XChild
    virtual InterfaceRef SAL_CALL getParent() throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return OFormComponents::getParent(); }
    virtual void SAL_CALL setParent(const InterfaceRef& Parent) throw ( :: com::sun::star::lang::NoSupportException , ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::container::XNamed
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setName(const OUString& aName) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::awt::XTabControllerModel
    virtual sal_Bool SAL_CALL getGroupControl() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setGroupControl(sal_Bool /*_bGroupControl*/) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { }
    virtual void SAL_CALL setControlModels(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& _rControls) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > SAL_CALL getControlModels() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setGroup(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& _rGroup, const OUString& _rGroupName) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getGroupCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL getGroup(sal_Int32 _nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& _rxGroup, OUString& _rName) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL getGroupByName(const OUString& _rName, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& _rxGroup) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::form::XLoadListener
    virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL unloading(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL unloaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL reloading(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL reloaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::form::XLoadable
    virtual void SAL_CALL load() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL unload() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL reload() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isLoaded() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addLoadListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeLoadListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdbc::XCloseable
    virtual void SAL_CALL close() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdbc::XRowSetListener
    virtual void SAL_CALL cursorMoved(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL rowChanged(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL rowSetChanged(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdb::XRowSetApproveListener
    virtual sal_Bool SAL_CALL approveCursorMove(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL approveRowChange(const ::com::sun::star::sdb::RowChangeEvent& event) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL approveRowSetChange(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdb::XRowSetApproveBroadcaster
    virtual void SAL_CALL addRowSetApproveListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeRowSetApproveListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun:star::form::XDatabaseParameterBroadcaster2
    virtual void SAL_CALL addDatabaseParameterListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeDatabaseParameterListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun:star::form::XDatabaseParameterBroadcaster
    virtual void SAL_CALL addParameterListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeParameterListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdbc::XRowSet
    virtual void SAL_CALL execute() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addRowSetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeRowSetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdb::XCompletedExecution
    virtual void SAL_CALL executeWithCompletion( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& handler ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdbc::XResultSet
    virtual sal_Bool SAL_CALL next() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isBeforeFirst() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isAfterLast() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isFirst() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isLast() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL beforeFirst() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL afterLast() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL first() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL last() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL absolute(sal_Int32 row) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL relative(sal_Int32 rows) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL previous() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL refreshRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL rowUpdated() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL rowInserted() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL rowDeleted() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual InterfaceRef SAL_CALL getStatement() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdbc::XResultSetUpdate
    virtual void SAL_CALL insertRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL updateRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL deleteRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL cancelRowUpdates() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL moveToInsertRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL moveToCurrentRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdbcx::XDeleteRows
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& rows) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::lang::XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)  throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getImplementationName()  throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::lang::XServiceInfo - static version
    static  OUString SAL_CALL getImplementationName_Static();
    static  ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static();
    static  ::com::sun::star::uno::Sequence< OUString > SAL_CALL getCurrentServiceNames_Static();
    static  ::com::sun::star::uno::Sequence< OUString > SAL_CALL getCompatibleServiceNames_Static();
    static  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

    // com::sun::star::io::XPersistObject
    virtual OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdbc::XSQLErrorListener
    virtual void SAL_CALL errorOccured(const ::com::sun::star::sdb::SQLErrorEvent& aEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdbc::XParameters
    virtual void SAL_CALL setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setBoolean(sal_Int32 parameterIndex, sal_Bool x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setByte(sal_Int32 parameterIndex, sal_Int8 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setShort(sal_Int32 parameterIndex, sal_Int16 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setInt(sal_Int32 parameterIndex, sal_Int32 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setLong(sal_Int32 parameterIndex, sal_Int64 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setFloat(sal_Int32 parameterIndex, float x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDouble(sal_Int32 parameterIndex, double x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setString(sal_Int32 parameterIndex, const OUString& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setBytes(sal_Int32 parameterIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDate(sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setTime(sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setTimestamp(sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setBinaryStream(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCharacterStream(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setObject(sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setObjectWithInfo(sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRef(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef>& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setBlob(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob>& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setClob(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob>& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setArray(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray>& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL clearParameters() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertyContainer
    virtual void SAL_CALL addProperty( const OUString& Name, ::sal_Int16 Attributes, const ::com::sun::star::uno::Any& DefaultValue ) throw (::com::sun::star::beans::PropertyExistException, ::com::sun::star::beans::IllegalTypeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeProperty( const OUString& Name ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::NotRemoveableException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertyAccess
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPropertyValues(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    using OPropertySetAggregationHelper::setPropertyValues;

    // XWarningsSupplier
    virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL clearWarnings(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    inline void submitNBC( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& Control, const ::com::sun::star::awt::MouseEvent& MouseEvt );

protected:
    // OPropertySetAggregationHelper overridables
    virtual void forwardingPropertyValue( sal_Int32 _nHandle ) SAL_OVERRIDE;
    virtual void forwardedPropertyValue( sal_Int32 _nHandle ) SAL_OVERRIDE;

    // OInterfaceContainer overridables
    virtual void implInserted( const ElementDescription* _pElement ) SAL_OVERRIDE;
    virtual void implRemoved(const InterfaceRef& _rxObject) SAL_OVERRIDE;

    // OPropertyChangeListener
    virtual void _propertyChanged( const ::com::sun::star::beans::PropertyChangeEvent& ) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

private:
    sal_Bool executeRowSet(::osl::ResettableMutexGuard& _rClearForNotifies, sal_Bool bMoveToFirst = sal_True,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxCompletionHandler = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >());
    bool    fillParameters(::osl::ResettableMutexGuard& _rClearForNotifies,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxCompletionHandler = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >());
    void    updateParameterInfo();
    bool    hasValidParent() const;

    // impl methods
    void    load_impl(sal_Bool bCausedByParentForm, sal_Bool bMoveToFirst = sal_True,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxCompletionHandler = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >())
        throw(::com::sun::star::uno::RuntimeException);
    void    reload_impl(sal_Bool bMoveToFirst,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxCompletionHandler = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >())
        throw(::com::sun::star::uno::RuntimeException);
    void    submit_impl(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& Control, const ::com::sun::star::awt::MouseEvent& MouseEvt, bool _bAproveByListeners);
    void    reset_impl(bool _bAproveByListeners);

    sal_Bool    implEnsureConnection();

    // connection sharing

    /// checks if we can re-use (aka share) the connection of the given parent
    sal_Bool    canShareConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxParentProps );

    /// starts sharing the connection with the parent
    void        doShareConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxParentProps );

    /// stops sharing the connection with the parent
    void        stopSharingConnection( );

    /// called when the connection which we share with our parent is beeing disposed
    void        disposingSharedConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn );

    /// checks if we currently share our connection with our parent
    sal_Bool    isSharingConnection( ) const { return m_bSharingConnection; }

    /** calls our row set approval listeners

        @param _rEvent
            the event to notify
        @param _bAllowSQLException
            <TRUE/> if SQLExceptions are allowed to leave the method
        @param _rGuard
            the guard to be cleared before actually calling into the listeners, but after making
            a copy of the listeners array to operate on.
        @return
            <TRUE/> if and only if the execution has been approved
    */
    bool    impl_approveRowChange_throw(
        const ::com::sun::star::lang::EventObject& _rEvent,
        const bool _bAllowSQLException,
        ::osl::ClearableMutexGuard& _rGuard
    );

    /// invalidate all our parameter-related stuff
    void        invlidateParameters();

    void        saveInsertOnlyState( );
    void        restoreInsertOnlyState( );

    // error handling
    void    onError(const ::com::sun::star::sdb::SQLErrorEvent& _rEvent);
    void    onError(const ::com::sun::star::sdbc::SQLException&, const OUString& _rContextDescription);

    // html tools
    OUString         GetDataEncoded(bool _bURLEncoded,const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt);
    OUString         GetDataURLEncoded(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt);
    OUString         GetDataTextEncoded(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt);
    ::com::sun::star::uno::Sequence<sal_Int8>   GetDataMultiPartEncoded(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt,
                                             OUString& rContentType);

    void AppendComponent(HtmlSuccessfulObjList& rList, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xComponentSet, const OUString& rNamePrefix,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& rxSubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt);

    void FillSuccessfulList(HtmlSuccessfulObjList& rList, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& rxSubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt);

    void InsertTextPart(INetMIMEMessage& rParent, const OUString& rName, const OUString& rData);
    sal_Bool InsertFilePart(INetMIMEMessage& rParent, const OUString& rName, const OUString& rFileName);
    void Encode(OUString& rString) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > getConnection();

    void    impl_createLoadTimer();

    void    impl_construct();

    DECL_LINK( OnTimeout, void* );
protected:
    using OPropertySetHelper::getPropertyValues;
};

inline void ODatabaseForm::submitNBC(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& Control, const ::com::sun::star::awt::MouseEvent& MouseEvt)
{
    submit_impl(Control, MouseEvt, false);
}


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_DATABASEFORM_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
