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

#pragma once

#include <sal/config.h>

#include <string_view>
#include <vector>

#include <propertybaghelper.hxx>
#include <com/sun/star/sdb/XSQLErrorListener.hpp>
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
#include <com/sun/star/sdbc/XRowSetListener.hpp>
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
#include <InterfaceContainer.hxx>

#include <connectivity/parameters.hxx>
#include <connectivity/filtermanager.hxx>
#include <connectivity/warningscontainer.hxx>

#include <listenercontainers.hxx>
#include <comphelper/propmultiplex.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase12.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase7.hxx>
#include <rtl/ref.hxx>

namespace com::sun::star::sdbc { class SQLException; }

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
};

typedef std::vector<HtmlSuccessfulObj> HtmlSuccessfulObjList;


class OGroupManager;
class OFormSubmitResetThread;
typedef ::cppu::ImplHelper12    <   css::form::XForm
                                ,   css::awt::XTabControllerModel
                                ,   css::form::XLoadListener
                                ,   css::sdbc::XRowSetListener
                                ,   css::sdb::XRowSetApproveListener
                                ,   css::form::XDatabaseParameterBroadcaster2
                                ,   css::sdb::XSQLErrorListener
                                ,   css::sdb::XSQLErrorBroadcaster
                                ,   css::form::XReset
                                ,   css::form::XSubmit
                                ,   css::form::XLoadable
                                ,   css::container::XNamed
                                >   ODatabaseForm_BASE1;


typedef ::cppu::ImplHelper4 <   css::lang::XServiceInfo
                            ,   css::beans::XPropertyContainer
                            ,   css::beans::XPropertyAccess
                            ,   css::sdbc::XWarningsSupplier
                            >   ODatabaseForm_BASE2;

typedef ::cppu::ImplHelper7<    css::sdbc::XCloseable,
                                css::sdbc::XRowSet,
                                css::sdb::XCompletedExecution,
                                css::sdb::XRowSetApproveBroadcaster,
                                css::sdbc::XResultSetUpdate,
                                css::sdbcx::XDeleteRows,
                                css::sdbc::XParameters > ODatabaseForm_BASE3;


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
    ::comphelper::OInterfaceContainerHelper2   m_aLoadListeners;
    ::comphelper::OInterfaceContainerHelper2   m_aRowSetApproveListeners;
    ::comphelper::OInterfaceContainerHelper2   m_aSubmitListeners;
    ::comphelper::OInterfaceContainerHelper2   m_aErrorListeners;
    ResetListeners                      m_aResetListeners;
    ::osl::Mutex                        m_aResetSafety;
    css::uno::Any                       m_aCycle;
    css::uno::Any                       m_aIgnoreResult; // set when we are a subform and our master form positioned on a new row
    css::uno::Sequence< OUString >      m_aMasterFields;
    css::uno::Sequence< OUString >      m_aDetailFields;

    // the object doin' most of the work - an SDB-rowset
    css::uno::Reference< css::uno::XAggregation>      m_xAggregate;
    // same object, interface as member because of performance reasons
    css::uno::Reference< css::sdbc::XRowSet>          m_xAggregateAsRowSet;

    PropertyBagHelper           m_aPropertyBagHelper;
    ::dbtools::WarningsContainer    m_aWarnings;
    rtl::Reference<OPropertyChangeMultiplexer> m_xAggregatePropertyMultiplexer;
    // Management of the Control Groups
    rtl::Reference<OGroupManager>   m_pGroupManager;
    ::dbtools::ParameterManager m_aParameterManager;
    ::dbtools::FilterManager    m_aFilterManager;
    std::unique_ptr<Timer>      m_pLoadTimer;

    rtl::Reference<OFormSubmitResetThread>  m_pThread;
    OUString                    m_sCurrentErrorContext;
    // will be used as additional context information
    // when an exception is caught and forwarded to the listeners

    sal_Int32                   m_nResetsPending;
//  <overwritten_properties>
    sal_Int32                   m_nPrivileges;
    bool                        m_bInsertOnly;
//  </overwritten_properties>

//  <properties>
    css::uno::Any        m_aControlBorderColorFocus;
    css::uno::Any        m_aControlBorderColorMouse;
    css::uno::Any        m_aControlBorderColorInvalid;
    css::uno::Any        m_aDynamicControlBorder;
    OUString             m_sName;
    OUString             m_aTargetURL;
    OUString             m_aTargetFrame;
    css::form::FormSubmitMethod    m_eSubmitMethod;
    css::form::FormSubmitEncoding  m_eSubmitEncoding;
    css::form::NavigationBarMode   m_eNavigation;
    bool                 m_bAllowInsert : 1;
    bool                 m_bAllowUpdate : 1;
    bool                 m_bAllowDelete : 1;
//  </properties>
    bool                 m_bLoaded : 1;
    bool                 m_bSubForm : 1;
    bool                 m_bForwardingConnection : 1;    // true if we're setting the ActiveConnection on the aggregate
    bool                 m_bSharingConnection : 1;       // true if the connection we're using is shared with our parent

public:
    explicit ODatabaseForm(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory);
    ODatabaseForm( const ODatabaseForm& _cloneSource );
    virtual ~ODatabaseForm() override;

    // UNO binding
    DECLARE_UNO3_AGG_DEFAULTS(ODatabaseForm, OFormComponents)
    virtual css::uno::Any SAL_CALL queryAggregation(const css::uno::Type& _rType) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type> SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) override;

    // css::lang::XComponent
    virtual void SAL_CALL disposing() override;

    // property handling
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle ) const override;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(css::uno::Any& rConvertedValue, css::uno::Any& rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue ) override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const css::uno::Any& rValue) override;

    css::uno::Any  SAL_CALL getFastPropertyValue( sal_Int32 nHandle ) override;
    void fire( sal_Int32 * pnHandles, const css::uno::Any * pNewValues, const css::uno::Any * pOldValues, sal_Int32 nCount );

    // IPropertyBagHelperContext
    virtual ::osl::Mutex&   getMutex() override;
    virtual void            describeFixedAndAggregateProperties(
        css::uno::Sequence< css::beans::Property >& _out_rFixedProperties,
        css::uno::Sequence< css::beans::Property >& _out_rAggregateProperties
    ) const override;
    virtual css::uno::Reference< css::beans::XMultiPropertySet >
                            getPropertiesInterface() override;

    // css::beans::XPropertyState
    virtual css::beans::PropertyState getPropertyStateByHandle(sal_Int32 nHandle) override;
    virtual void setPropertyToDefaultByHandle(sal_Int32 nHandle) override;
    virtual css::uno::Any getPropertyDefaultByHandle(sal_Int32 nHandle) const override;

    // css::sdbc::XSQLErrorBroadcaster
    virtual void SAL_CALL addSQLErrorListener(const css::uno::Reference< css::sdb::XSQLErrorListener>& _rxListener) override;
    virtual void SAL_CALL removeSQLErrorListener(const css::uno::Reference< css::sdb::XSQLErrorListener>& _rxListener) override;

    // css::form::XForm
    // nothing to implement

    // css::form::XReset
    virtual void SAL_CALL reset() override;
    virtual void SAL_CALL addResetListener(const css::uno::Reference< css::form::XResetListener>& _rxListener) override;
    virtual void SAL_CALL removeResetListener(const css::uno::Reference< css::form::XResetListener>& _rxListener) override;

    // css::form::XSubmit
    virtual void SAL_CALL submit(const css::uno::Reference< css::awt::XControl>& aControl, const css::awt::MouseEvent& aMouseEvt) override;
    virtual void SAL_CALL addSubmitListener(const css::uno::Reference< css::form::XSubmitListener>& _rxListener) override;
    virtual void SAL_CALL removeSubmitListener(const css::uno::Reference< css::form::XSubmitListener>& _rxListener) override;

    // css::container::XChild
    virtual css::uno::Reference<css::uno::XInterface> SAL_CALL getParent() override { return OFormComponents::getParent(); }
    virtual void SAL_CALL setParent(const css::uno::Reference<css::uno::XInterface>& Parent) override;

    // css::container::XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName(const OUString& aName) override;

    // css::awt::XTabControllerModel
    virtual sal_Bool SAL_CALL getGroupControl() override;
    virtual void SAL_CALL setGroupControl(sal_Bool /*_bGroupControl*/) override { }
    virtual void SAL_CALL setControlModels(const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& _rControls) override;
    virtual css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > SAL_CALL getControlModels() override;
    virtual void SAL_CALL setGroup(const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& _rGroup, const OUString& _rGroupName) override;
    virtual sal_Int32 SAL_CALL getGroupCount() override;
    virtual void SAL_CALL getGroup(sal_Int32 _nGroup, css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& _rxGroup, OUString& _rName) override;
    virtual void SAL_CALL getGroupByName(const OUString& _rName, css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& _rxGroup) override;

    // css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& _rSource) override;

    // css::form::XLoadListener
    virtual void SAL_CALL loaded(const css::lang::EventObject& aEvent) override;
    virtual void SAL_CALL unloading(const css::lang::EventObject& aEvent) override;
    virtual void SAL_CALL unloaded(const css::lang::EventObject& aEvent) override;
    virtual void SAL_CALL reloading(const css::lang::EventObject& aEvent) override;
    virtual void SAL_CALL reloaded(const css::lang::EventObject& aEvent) override;

    // css::form::XLoadable
    virtual void SAL_CALL load() override;
    virtual void SAL_CALL unload() override;
    virtual void SAL_CALL reload() override;
    virtual sal_Bool SAL_CALL isLoaded() override;
    virtual void SAL_CALL addLoadListener(const css::uno::Reference< css::form::XLoadListener>& _rxListener) override;
    virtual void SAL_CALL removeLoadListener(const css::uno::Reference< css::form::XLoadListener>& _rxListener) override;

    // css::sdbc::XCloseable
    virtual void SAL_CALL close() override;

    // css::sdbc::XRowSetListener
    virtual void SAL_CALL cursorMoved(const css::lang::EventObject& event) override;
    virtual void SAL_CALL rowChanged(const css::lang::EventObject& event) override;
    virtual void SAL_CALL rowSetChanged(const css::lang::EventObject& event) override;

    // css::sdb::XRowSetApproveListener
    virtual sal_Bool SAL_CALL approveCursorMove(const css::lang::EventObject& event) override;
    virtual sal_Bool SAL_CALL approveRowChange(const css::sdb::RowChangeEvent& event) override;
    virtual sal_Bool SAL_CALL approveRowSetChange(const css::lang::EventObject& event) override;

    // css::sdb::XRowSetApproveBroadcaster
    virtual void SAL_CALL addRowSetApproveListener(const css::uno::Reference< css::sdb::XRowSetApproveListener>& _rxListener) override;
    virtual void SAL_CALL removeRowSetApproveListener(const css::uno::Reference< css::sdb::XRowSetApproveListener>& _rxListener) override;

    // com::sun:star::form::XDatabaseParameterBroadcaster2
    virtual void SAL_CALL addDatabaseParameterListener(const css::uno::Reference< css::form::XDatabaseParameterListener>& _rxListener) override;
    virtual void SAL_CALL removeDatabaseParameterListener(const css::uno::Reference< css::form::XDatabaseParameterListener>& _rxListener) override;

    // com::sun:star::form::XDatabaseParameterBroadcaster
    virtual void SAL_CALL addParameterListener(const css::uno::Reference< css::form::XDatabaseParameterListener>& _rxListener) override;
    virtual void SAL_CALL removeParameterListener(const css::uno::Reference< css::form::XDatabaseParameterListener>& _rxListener) override;

    // css::sdbc::XRowSet
    virtual void SAL_CALL execute() override;
    virtual void SAL_CALL addRowSetListener(const css::uno::Reference< css::sdbc::XRowSetListener>& _rxListener) override;
    virtual void SAL_CALL removeRowSetListener(const css::uno::Reference< css::sdbc::XRowSetListener>& _rxListener) override;

    // css::sdb::XCompletedExecution
    virtual void SAL_CALL executeWithCompletion( const css::uno::Reference< css::task::XInteractionHandler >& handler ) override;

    // css::sdbc::XResultSet
    virtual sal_Bool SAL_CALL next() override;
    virtual sal_Bool SAL_CALL isBeforeFirst() override;
    virtual sal_Bool SAL_CALL isAfterLast() override;
    virtual sal_Bool SAL_CALL isFirst() override;
    virtual sal_Bool SAL_CALL isLast() override;
    virtual void SAL_CALL beforeFirst() override;
    virtual void SAL_CALL afterLast() override;
    virtual sal_Bool SAL_CALL first() override;
    virtual sal_Bool SAL_CALL last() override;
    virtual sal_Int32 SAL_CALL getRow() override;
    virtual sal_Bool SAL_CALL absolute(sal_Int32 row) override;
    virtual sal_Bool SAL_CALL relative(sal_Int32 rows) override;
    virtual sal_Bool SAL_CALL previous() override;
    virtual void SAL_CALL refreshRow() override;
    virtual sal_Bool SAL_CALL rowUpdated() override;
    virtual sal_Bool SAL_CALL rowInserted() override;
    virtual sal_Bool SAL_CALL rowDeleted() override;
    virtual css::uno::Reference<css::uno::XInterface> SAL_CALL getStatement() override;

    // css::sdbc::XResultSetUpdate
    virtual void SAL_CALL insertRow() override;
    virtual void SAL_CALL updateRow() override;
    virtual void SAL_CALL deleteRow() override;
    virtual void SAL_CALL cancelRowUpdates() override;
    virtual void SAL_CALL moveToInsertRow() override;
    virtual void SAL_CALL moveToCurrentRow() override;

    // css::sdbcx::XDeleteRows
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL deleteRows(const css::uno::Sequence< css::uno::Any>& rows) override;

    // css::lang::XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // css::io::XPersistObject
    virtual OUString SAL_CALL getServiceName() override;
    virtual void SAL_CALL write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) override;
    virtual void SAL_CALL read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) override;

    // css::sdbc::XSQLErrorListener
    virtual void SAL_CALL errorOccured(const css::sdb::SQLErrorEvent& aEvent) override;

    // css::sdbc::XParameters
    virtual void SAL_CALL setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) override;
    virtual void SAL_CALL setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName) override;
    virtual void SAL_CALL setBoolean(sal_Int32 parameterIndex, sal_Bool x) override;
    virtual void SAL_CALL setByte(sal_Int32 parameterIndex, sal_Int8 x) override;
    virtual void SAL_CALL setShort(sal_Int32 parameterIndex, sal_Int16 x) override;
    virtual void SAL_CALL setInt(sal_Int32 parameterIndex, sal_Int32 x) override;
    virtual void SAL_CALL setLong(sal_Int32 parameterIndex, sal_Int64 x) override;
    virtual void SAL_CALL setFloat(sal_Int32 parameterIndex, float x) override;
    virtual void SAL_CALL setDouble(sal_Int32 parameterIndex, double x) override;
    virtual void SAL_CALL setString(sal_Int32 parameterIndex, const OUString& x) override;
    virtual void SAL_CALL setBytes(sal_Int32 parameterIndex, const css::uno::Sequence< sal_Int8 >& x) override;
    virtual void SAL_CALL setDate(sal_Int32 parameterIndex, const css::util::Date& x) override;
    virtual void SAL_CALL setTime(sal_Int32 parameterIndex, const css::util::Time& x) override;
    virtual void SAL_CALL setTimestamp(sal_Int32 parameterIndex, const css::util::DateTime& x) override;
    virtual void SAL_CALL setBinaryStream(sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream>& x, sal_Int32 length) override;
    virtual void SAL_CALL setCharacterStream(sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream>& x, sal_Int32 length) override;
    virtual void SAL_CALL setObject(sal_Int32 parameterIndex, const css::uno::Any& x) override;
    virtual void SAL_CALL setObjectWithInfo(sal_Int32 parameterIndex, const css::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale) override;
    virtual void SAL_CALL setRef(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XRef>& x) override;
    virtual void SAL_CALL setBlob(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XBlob>& x) override;
    virtual void SAL_CALL setClob(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XClob>& x) override;
    virtual void SAL_CALL setArray(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XArray>& x) override;
    virtual void SAL_CALL clearParameters() override;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) override;

    // XPropertyContainer
    virtual void SAL_CALL addProperty( const OUString& Name, ::sal_Int16 Attributes, const css::uno::Any& DefaultValue ) override;
    virtual void SAL_CALL removeProperty( const OUString& Name ) override;

    // XPropertyAccess
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPropertyValues(  ) override;
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& aProps ) override;
    using OPropertySetAggregationHelper::setPropertyValues;

    // XWarningsSupplier
    virtual css::uno::Any SAL_CALL getWarnings(  ) override;
    virtual void SAL_CALL clearWarnings(  ) override;

    // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;

protected:
    // OPropertySetAggregationHelper overridables
    virtual void forwardingPropertyValue( sal_Int32 _nHandle ) override;
    virtual void forwardedPropertyValue( sal_Int32 _nHandle ) override;

    // OInterfaceContainer overridables
    virtual void implInserted( const ElementDescription* _pElement ) override;
    virtual void implRemoved(const css::uno::Reference<css::uno::XInterface>& _rxObject) override;

    // OPropertyChangeListener
    virtual void _propertyChanged( const css::beans::PropertyChangeEvent& ) override;

private:
    bool executeRowSet(::osl::ResettableMutexGuard& _rClearForNotifies, bool bMoveToFirst,
                    const css::uno::Reference< css::task::XInteractionHandler >& _rxCompletionHandler);
    bool    fillParameters(::osl::ResettableMutexGuard& _rClearForNotifies,
                    const css::uno::Reference< css::task::XInteractionHandler >& _rxCompletionHandler);
    void    updateParameterInfo();
    bool    hasValidParent() const;

    // impl methods
    /// @throws css::uno::RuntimeException
    void    load_impl(bool bCausedByParentForm, bool bMoveToFirst = true,
        const css::uno::Reference< css::task::XInteractionHandler >& _rxCompletionHandler = css::uno::Reference< css::task::XInteractionHandler >());
    /// @throws css::uno::RuntimeException
    void    reload_impl(bool bMoveToFirst,
        const css::uno::Reference< css::task::XInteractionHandler >& _rxCompletionHandler = css::uno::Reference< css::task::XInteractionHandler >());
    void    submit_impl(const css::uno::Reference< css::awt::XControl>& Control, const css::awt::MouseEvent& MouseEvt);
    void    reset_impl(bool _bApproveByListeners);

    bool    implEnsureConnection();

    // connection sharing

    /// checks if we can re-use (aka share) the connection of the given parent
    bool    canShareConnection( const css::uno::Reference< css::beans::XPropertySet >& _rxParentProps );

    /// starts sharing the connection with the parent
    void        doShareConnection( const css::uno::Reference< css::beans::XPropertySet >& _rxParentProps );

    /// stops sharing the connection with the parent
    void        stopSharingConnection( );

    /// called when the connection which we share with our parent is being disposed
    void        disposingSharedConnection( const css::uno::Reference< css::sdbc::XConnection >& _rxConn );

    /// checks if we currently share our connection with our parent
    bool    isSharingConnection( ) const { return m_bSharingConnection; }

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
        const css::lang::EventObject& _rEvent,
        const bool _bAllowSQLException,
        ::osl::ClearableMutexGuard& _rGuard
    );

    /// invalidate all our parameter-related stuff
    void        invalidateParameters();

    void        saveInsertOnlyState( );
    void        restoreInsertOnlyState( );

    // error handling
    void    onError(const css::sdb::SQLErrorEvent& _rEvent);
    void    onError(const css::sdbc::SQLException&, const OUString& _rContextDescription);

    // html tools
    OUString         GetDataEncoded(bool _bURLEncoded,const css::uno::Reference< css::awt::XControl>& SubmitButton, const css::awt::MouseEvent& MouseEvt);
    css::uno::Sequence<sal_Int8>   GetDataMultiPartEncoded(const css::uno::Reference< css::awt::XControl>& SubmitButton, const css::awt::MouseEvent& MouseEvt,
                                             OUString& rContentType);

    void AppendComponent(HtmlSuccessfulObjList& rList, const css::uno::Reference< css::beans::XPropertySet>& xComponentSet, std::u16string_view rNamePrefix,
                     const css::uno::Reference< css::awt::XControl>& rxSubmitButton, const css::awt::MouseEvent& MouseEvt);

    void FillSuccessfulList(HtmlSuccessfulObjList& rList, const css::uno::Reference< css::awt::XControl>& rxSubmitButton, const css::awt::MouseEvent& MouseEvt);

    static void InsertTextPart(INetMIMEMessage& rParent, std::u16string_view rName, std::u16string_view rData);
    static void InsertFilePart(INetMIMEMessage& rParent, std::u16string_view rName, const OUString& rFileName);
    static void Encode(OUString& rString);

    css::uno::Reference< css::sdbc::XConnection > getConnection();

    void    impl_createLoadTimer();

    void    impl_construct();

    DECL_LINK( OnTimeout, Timer*, void );
protected:
    using OPropertySetHelper::getPropertyValues;
};


}   // namespace frm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
