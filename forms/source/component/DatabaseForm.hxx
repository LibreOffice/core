/*************************************************************************
 *
 *  $RCSfile: DatabaseForm.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-31 16:03:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FRM_DATABASEFORM_HXX_
#define _FRM_DATABASEFORM_HXX_

#ifndef _COM_SUN_STAR_SDB_XSQLERRORLISTENER_HPP_
#include <com/sun/star/sdb/XSQLErrorListener.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLERRORBROADCASTER_HPP_
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITMETHOD_HPP_
#include <com/sun/star/form/FormSubmitMethod.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITENCODING_HPP_
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XSUBMIT_HPP_
#include <com/sun/star/form/XSubmit.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XDATABASEPARAMETERBROADCASTER_HPP_
#include <com/sun/star/form/XDatabaseParameterBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDEXECUTION_HPP_
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDELETEROWS_HPP_
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XROWSETAPPROVELISTENER_HPP_
#include <com/sun/star/sdb/XRowSetApproveListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XROWSETAPPROVEBROADCASTER_HPP_
#include <com/sun/star/sdb/XRowSetApproveBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_NAVIGATIONBARMODE_HPP_
#include <com/sun/star/form/NavigationBarMode.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADLISTENER_HPP_
#include <com/sun/star/form/XLoadListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLERMODEL_HPP_
#include <com/sun/star/awt/XTabControllerModel.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSETLISTENER_HPP_
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XRESULTSETACCESS_HPP_
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOSEABLE_HPP_
#include <com/sun/star/sdbc/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPARAMETERS_HPP_
#include <com/sun/star/sdbc/XParameters.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif


#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _FRM_INTERFACE_CONTAINER_HXX_
#include "InterfaceContainer.hxx"
#endif
#ifndef _FRM_IDS_HXX_
#include "ids.hxx"
#endif

#ifndef _COMPHELPER_PROPERTY_AGGREGATION_HXX_
#include <comphelper/propagg.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_GUARDING_HXX_
#include <comphelper/guarding.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif

namespace com { namespace sun { namespace star { namespace sdbc {
    class SQLException;
} } } }

class Timer;
class INetMIMEMessage;

typedef ::comphelper::OReusableGuard< ::osl::Mutex > ReusableMutexGuard;

//.........................................................................
namespace frm
{
//.........................................................................

    namespace starsdb   = ::com::sun::star::sdb;
    namespace starsdbc  = ::com::sun::star::sdbc;
    namespace starsdbcx = ::com::sun::star::sdbcx;
    namespace starform  = ::com::sun::star::form;
    namespace starutil  = ::com::sun::star::util;

//========================================================================
//= html tools
//========================================================================

const sal_uInt16 SUCCESSFUL_REPRESENT_TEXT          = 0x0001;
const sal_uInt16 SUCCESSFUL_REPRESENT_FILE          = 0x0002;

const ::rtl::OUString ALL_COMPONENTS_GROUP_NAME = ::rtl::OUString::createFromAscii("AllComponentGroup");

//------------------------------------------------------------------------------
class HtmlSuccessfulObj
{
public:
    ::rtl::OUString     aName;
    ::rtl::OUString     aValue;
    sal_uInt16          nRepresentation;

    HtmlSuccessfulObj( const ::rtl::OUString& _rName, const ::rtl::OUString& _rValue,
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


DECLARE_STL_VECTOR(HtmlSuccessfulObj, HtmlSuccessfulObjList);

//========================================================================
class OGroupManager;
class OFormSubmitResetThread;
struct OParameterInfoImpl;
class ODatabaseForm :public OFormComponents
                    ,public OPropertySetAggregationHelper
                    ,public OPropertyChangeListener
                    ,public ::comphelper::OAggregationArrayUsageHelper<ODatabaseForm>
                    ,public starsdb::XSQLErrorBroadcaster
                    // service com::sun::star::form::component::Form (supported by com::sun::star::form::component::DataForm)
                        ,public starform::XForm
                        ,public starawt::XTabControllerModel
                        // already present (via OFormComponents) : com::sun::star::script::XEventAttacherManager
                    // service com::sun::star::form::component::HTMLForm
                    ,public starform::XReset
                    ,public starform::XSubmit
                    // other stuff (new or re-routed interfaces)
                    ,public starform::XLoadListener
                    ,public starform::XLoadable
                    ,public starsdbc::XCloseable
                    ,public starsdbc::XRowSet
                    ,public starsdb::XCompletedExecution
                    ,public starsdbc::XRowSetListener
                    ,public starsdb::XRowSetApproveListener
                    ,public starsdb::XRowSetApproveBroadcaster
                    ,public starsdbc::XResultSetUpdate
                    ,public starsdbcx::XDeleteRows
                    ,public starsdbc::XParameters
                    ,public starform::XDatabaseParameterBroadcaster
                    // already preset (via OFormComponents) : stario::XPersistObject
                    ,public starsdb::XSQLErrorListener
                    ,public starcontainer::XNamed
                    ,public starlang::XServiceInfo
{
    friend class OFormSubmitResetThread;

    OImplementationIdsRef       m_aHoldIdHelper;

        // listener administration
    ::cppu::OInterfaceContainerHelper   m_aLoadListeners;
    ::cppu::OInterfaceContainerHelper   m_aRowSetApproveListeners;
    ::cppu::OInterfaceContainerHelper   m_aRowSetListeners;
    ::cppu::OInterfaceContainerHelper   m_aParameterListeners;
    ::cppu::OInterfaceContainerHelper   m_aResetListeners;
    ::cppu::OInterfaceContainerHelper   m_aSubmitListeners;
    ::cppu::OInterfaceContainerHelper   m_aErrorListeners;

    staruno::Any                        m_aCycle;
    StringSequence              m_aMasterFields;
    StringSequence              m_aDetailFields;
    ::std::vector<bool>     m_aParameterVisited;

    // the object doin' most of the work - an SDB-rowset
    staruno::Reference<staruno::XAggregation>       m_xAggregate;
    // same object, interface as member because of performance reasons
    staruno::Reference<starsdbc::XRowSet>           m_xAggregateAsRowSet;

    // Verwaltung der ControlGruppen
    OGroupManager*      m_pGroupManager;
    OParameterInfoImpl* m_pParameterInfo;
    Timer*              m_pLoadTimer;

    OFormSubmitResetThread* m_pThread;
    ::rtl::OUString                     m_sCurrentErrorContext;
                // will be used as additional context information
                // when an exception is catched and forwarded to the listeners

    ::osl::Mutex                        m_aResetSafety;
    INT32                               m_nResetsPending;

//  <overwritten_properties>
    sal_Int32               m_nPrivileges;
//  </overwritten_properties>

//  <properties>
    ::rtl::OUString             m_sName;
    ::rtl::OUString             m_aTargetURL;
    ::rtl::OUString             m_aTargetFrame;
    starform::FormSubmitMethod  m_eSubmitMethod;
    starform::FormSubmitEncoding    m_eSubmitEncoding;
    starform::NavigationBarMode m_eNavigation;
    sal_Bool                    m_bAllowInsert : 1;
    sal_Bool                    m_bAllowUpdate : 1;
    sal_Bool                    m_bAllowDelete : 1;
//  </properties>
    sal_Bool                    m_bLoaded : 1;
    sal_Bool                    m_bSubForm : 1;

public:
    ODatabaseForm(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    ~ODatabaseForm();

    // UNO binding
    DECLARE_UNO3_AGG_DEFAULTS(ODatabaseForm, OFormComponents);
    virtual staruno::Any SAL_CALL queryAggregation(const staruno::Type& _rType) throw(staruno::RuntimeException);

    // XTypeProvider
    virtual staruno::Sequence<staruno::Type> SAL_CALL getTypes(  ) throw(staruno::RuntimeException);
    virtual staruno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(staruno::RuntimeException);

    // starlang::XComponent
    virtual void SAL_CALL disposing();

    // property handling
    virtual staruno::Reference< starbeans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(staruno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual void SAL_CALL getFastPropertyValue(staruno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(staruno::Any& rConvertedValue, staruno::Any& rOldValue, sal_Int32 nHandle, const staruno::Any& rValue ) throw(starlang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const staruno::Any& rValue);

    staruno::Any    SAL_CALL getFastPropertyValue( sal_Int32 nHandle )
       throw(starbeans::UnknownPropertyException, starlang::WrappedTargetException, staruno::RuntimeException);
    void fire( sal_Int32 * pnHandles, const staruno::Any * pNewValues, const staruno::Any * pOldValues, sal_Int32 nCount, sal_Bool bVetoable );

    virtual void fillProperties(
        staruno::Sequence< starbeans::Property >& /* [out] */ _rProps,
        staruno::Sequence< starbeans::Property >& /* [out] */ _rAggregateProps
        ) const;

    // com::sun::star::beans::XPropertyState
    virtual starbeans::PropertyState getPropertyStateByHandle(sal_Int32 nHandle);
    virtual void setPropertyToDefaultByHandle(sal_Int32 nHandle);
    virtual staruno::Any getPropertyDefaultByHandle(sal_Int32 nHandle) const;

    // com::sun::star::sdbc::XSQLErrorBroadcaster
    virtual void SAL_CALL addSQLErrorListener(const staruno::Reference<starsdb::XSQLErrorListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeSQLErrorListener(const staruno::Reference<starsdb::XSQLErrorListener>& _rxListener) throw(staruno::RuntimeException);

    // com::sun::star::form::XForm
    // nothing to implement

    // com::sun::star::form::XReset
    virtual void SAL_CALL reset() throw(staruno::RuntimeException);
    virtual void SAL_CALL addResetListener(const staruno::Reference<starform::XResetListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeResetListener(const staruno::Reference<starform::XResetListener>& _rxListener) throw(staruno::RuntimeException);

    // com::sun::star::form::XSubmit
    virtual void SAL_CALL submit(const staruno::Reference<starawt::XControl>& aControl, const starawt::MouseEvent& aMouseEvt) throw(staruno::RuntimeException);
    virtual void SAL_CALL addSubmitListener(const staruno::Reference<starform::XSubmitListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeSubmitListener(const staruno::Reference<starform::XSubmitListener>& _rxListener) throw(staruno::RuntimeException);

    // com::sun::star::container::XChild
    virtual InterfaceRef SAL_CALL getParent() { return OFormComponents::getParent(); }
    virtual void SAL_CALL setParent(const InterfaceRef& Parent);

    // com::sun::star::container::XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(staruno::RuntimeException);
    virtual void SAL_CALL setName(const ::rtl::OUString& aName) throw(staruno::RuntimeException);

    // com::sun::star::awt::XTabControllerModel
    virtual sal_Bool SAL_CALL getGroupControl() throw(staruno::RuntimeException);
    virtual void SAL_CALL setGroupControl(sal_Bool _bGroupControl) throw(staruno::RuntimeException) { }
    virtual void SAL_CALL setControlModels(const staruno::Sequence< staruno::Reference< starawt::XControlModel > >& _rControls) throw(staruno::RuntimeException);
    virtual staruno::Sequence< staruno::Reference< starawt::XControlModel > > SAL_CALL getControlModels() throw(staruno::RuntimeException);
    virtual void SAL_CALL setGroup(const staruno::Sequence< staruno::Reference< starawt::XControlModel > >& _rGroup, const ::rtl::OUString& _rGroupName) throw(staruno::RuntimeException);
    virtual sal_Int32 SAL_CALL getGroupCount() throw(staruno::RuntimeException);
    virtual void SAL_CALL getGroup(sal_Int32 _nGroup, staruno::Sequence< staruno::Reference< starawt::XControlModel > >& _rxGroup, ::rtl::OUString& _rName) throw(staruno::RuntimeException);
    virtual void SAL_CALL getGroupByName(const ::rtl::OUString& _rName, staruno::Sequence< staruno::Reference< starawt::XControlModel > >& _rxGroup) throw(staruno::RuntimeException);

    // com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const starlang::EventObject& _rSource) throw(staruno::RuntimeException);

    // com::sun::star::form::XLoadListener
    virtual void SAL_CALL loaded(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);
    virtual void SAL_CALL unloading(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);
    virtual void SAL_CALL unloaded(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);
    virtual void SAL_CALL reloading(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);
    virtual void SAL_CALL reloaded(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);

    // com::sun::star::form::XLoadable
    virtual void SAL_CALL load() throw(staruno::RuntimeException);
    virtual void SAL_CALL unload() throw(staruno::RuntimeException);
    virtual void SAL_CALL reload() throw(staruno::RuntimeException);
    virtual sal_Bool SAL_CALL isLoaded() throw(staruno::RuntimeException);
    virtual void SAL_CALL addLoadListener(const staruno::Reference<starform::XLoadListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeLoadListener(const staruno::Reference<starform::XLoadListener>& _rxListener) throw(staruno::RuntimeException);

    // com::sun::star::sdbc::XCloseable
    virtual void SAL_CALL close() throw(starsdbc::SQLException, staruno::RuntimeException);

    // com::sun::star::sdbc::XRowSetListener
    virtual void SAL_CALL cursorMoved(const starlang::EventObject& event) throw(staruno::RuntimeException);
    virtual void SAL_CALL rowChanged(const starlang::EventObject& event) throw(staruno::RuntimeException);
    virtual void SAL_CALL rowSetChanged(const starlang::EventObject& event) throw(staruno::RuntimeException);

    // com::sun::star::sdb::XRowSetApproveListener
    virtual sal_Bool SAL_CALL approveCursorMove(const starlang::EventObject& event) throw(staruno::RuntimeException);
    virtual sal_Bool SAL_CALL approveRowChange(const starsdb::RowChangeEvent& event) throw(staruno::RuntimeException);
    virtual sal_Bool SAL_CALL approveRowSetChange(const starlang::EventObject& event) throw(staruno::RuntimeException);

    // com::sun::star::sdb::XRowSetApproveBroadcaster
    virtual void SAL_CALL addRowSetApproveListener(const staruno::Reference<starsdb::XRowSetApproveListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeRowSetApproveListener(const staruno::Reference<starsdb::XRowSetApproveListener>& _rxListener) throw(staruno::RuntimeException);

    // com::sun:star::form::XDatabaseParameterBroadcaster
    virtual void SAL_CALL addParameterListener(const staruno::Reference<starform::XDatabaseParameterListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeParameterListener(const staruno::Reference<starform::XDatabaseParameterListener>& _rxListener) throw(staruno::RuntimeException);

    // com::sun::star::sdbc::XRowSet
    virtual void SAL_CALL execute() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL addRowSetListener(const staruno::Reference<starsdbc::XRowSetListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeRowSetListener(const staruno::Reference<starsdbc::XRowSetListener>& _rxListener) throw(staruno::RuntimeException);

    // com::sun::star::sdb::XCompletedExecution
    virtual void SAL_CALL executeWithCompletion( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& handler ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // com::sun::star::sdbc::XResultSet
    virtual sal_Bool SAL_CALL next() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual sal_Bool SAL_CALL isBeforeFirst() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual sal_Bool SAL_CALL isAfterLast() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual sal_Bool SAL_CALL isFirst() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual sal_Bool SAL_CALL isLast() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL beforeFirst() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL afterLast() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual sal_Bool SAL_CALL first() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual sal_Bool SAL_CALL last() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual sal_Int32 SAL_CALL getRow() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual sal_Bool SAL_CALL absolute(sal_Int32 row) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual sal_Bool SAL_CALL relative(sal_Int32 rows) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual sal_Bool SAL_CALL previous() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL refreshRow() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual sal_Bool SAL_CALL rowUpdated() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual sal_Bool SAL_CALL rowInserted() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual sal_Bool SAL_CALL rowDeleted() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual InterfaceRef SAL_CALL getStatement() throw(starsdbc::SQLException, staruno::RuntimeException);

    // com::sun::star::sdbc::XResultSetUpdate
    virtual void SAL_CALL insertRow() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL updateRow() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL deleteRow() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL cancelRowUpdates() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL moveToInsertRow() throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL moveToCurrentRow() throw(starsdbc::SQLException, staruno::RuntimeException);

    // com::sun::star::sdbcx::XDeleteRows
    virtual staruno::Sequence< sal_Int32 > SAL_CALL deleteRows(const staruno::Sequence<staruno::Any>& rows) throw(starsdbc::SQLException, staruno::RuntimeException);

    // com::sun::star::lang::XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& ServiceName)  throw(staruno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getImplementationName()  throw(staruno::RuntimeException);
    virtual StringSequence SAL_CALL getSupportedServiceNames()  throw(staruno::RuntimeException);

    // com::sun::star::io::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw(staruno::RuntimeException);
    virtual void SAL_CALL write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream) throw(stario::IOException, staruno::RuntimeException);
    virtual void SAL_CALL read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, staruno::RuntimeException);

    // com::sun::star::sdbc::XSQLErrorListener
    virtual void SAL_CALL errorOccured(const starsdb::SQLErrorEvent& aEvent) throw(staruno::RuntimeException);

    // com::sun::star::sdbc::XParameters
    virtual void SAL_CALL setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setBoolean(sal_Int32 parameterIndex, sal_Bool x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setByte(sal_Int32 parameterIndex, sal_Int8 x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setShort(sal_Int32 parameterIndex, sal_Int16 x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setInt(sal_Int32 parameterIndex, sal_Int32 x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setLong(sal_Int32 parameterIndex, Hyper x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setFloat(sal_Int32 parameterIndex, float x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setDouble(sal_Int32 parameterIndex, double x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setString(sal_Int32 parameterIndex, const ::rtl::OUString& x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setBytes(sal_Int32 parameterIndex, const staruno::Sequence< sal_Int8 >& x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setDate(sal_Int32 parameterIndex, const starutil::Date& x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setTime(sal_Int32 parameterIndex, const starutil::Time& x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setTimestamp(sal_Int32 parameterIndex, const starutil::DateTime& x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setBinaryStream(sal_Int32 parameterIndex, const staruno::Reference<stario::XInputStream>& x, sal_Int32 length) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setCharacterStream(sal_Int32 parameterIndex, const staruno::Reference<stario::XInputStream>& x, sal_Int32 length) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setObject(sal_Int32 parameterIndex, const staruno::Any& x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setObjectWithInfo(sal_Int32 parameterIndex, const staruno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setRef(sal_Int32 parameterIndex, const staruno::Reference<starsdbc::XRef>& x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setBlob(sal_Int32 parameterIndex, const staruno::Reference<starsdbc::XBlob>& x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setClob(sal_Int32 parameterIndex, const staruno::Reference<starsdbc::XClob>& x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL setArray(sal_Int32 parameterIndex, const staruno::Reference<starsdbc::XArray>& x) throw(starsdbc::SQLException, staruno::RuntimeException);
    virtual void SAL_CALL clearParameters() throw(starsdbc::SQLException, staruno::RuntimeException);

    inline void submitNBC( const staruno::Reference<starawt::XControl>& Control, const starawt::MouseEvent& MouseEvt );

protected:
    virtual void implInserted(const InterfaceRef& _rxObject);
    virtual void implRemoved(const InterfaceRef& _rxObject);

    // OPropertyChangeListener
    virtual void _propertyChanged( const starbeans::PropertyChangeEvent& ) throw(staruno::RuntimeException);

private:
    void    executeRowSet(ReusableMutexGuard& _rClearForNotifies, sal_Bool bMoveToFirst = sal_True,
                    const staruno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxCompletionHandler = staruno::Reference< ::com::sun::star::task::XInteractionHandler >());
    bool    fillParameters(ReusableMutexGuard& _rClearForNotifies,
                    const staruno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxCompletionHandler = staruno::Reference< ::com::sun::star::task::XInteractionHandler >());
    OParameterInfoImpl* createParameterInfo() const;
    bool    hasValidParent() const;
    // if there are no parameter infos we now that we have a complete new statement to execute
    bool    needStatementRebuild() const {return m_pParameterInfo == NULL;}

    // impl methods
    void    load_impl(sal_Bool bCausedByParentForm, sal_Bool bMoveToFirst = sal_True,
        const staruno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxCompletionHandler = staruno::Reference< ::com::sun::star::task::XInteractionHandler >())
        throw(staruno::RuntimeException);
    void    reload_impl(sal_Bool bMoveToFirst,
        const staruno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxCompletionHandler = staruno::Reference< ::com::sun::star::task::XInteractionHandler >())
        throw(staruno::RuntimeException);
    void    submit_impl(const staruno::Reference<starawt::XControl>& Control, const starawt::MouseEvent& MouseEvt, bool _bAproveByListeners);
    void    reset_impl(bool _bAproveByListeners);

    sal_Bool    implEnsureConnection();

    // error handling
    void    onError(const starsdb::SQLErrorEvent& _rEvent);
    void    onError(starsdbc::SQLException&, const ::rtl::OUString& _rContextDescription);

    // html tools
    ::rtl::OUString         GetDataURLEncoded(const staruno::Reference<starawt::XControl>& SubmitButton, const starawt::MouseEvent& MouseEvt);
    ::rtl::OUString         GetDataTextEncoded(const staruno::Reference<starawt::XControl>& SubmitButton, const starawt::MouseEvent& MouseEvt);
    staruno::Sequence<sal_Int8> GetDataMultiPartEncoded(const staruno::Reference<starawt::XControl>& SubmitButton, const starawt::MouseEvent& MouseEvt,
                                             ::rtl::OUString& rContentType);

    void AppendComponent(HtmlSuccessfulObjList& rList, const staruno::Reference<starbeans::XPropertySet>& xComponentSet, const ::rtl::OUString& rNamePrefix,
                     const staruno::Reference<starawt::XControl>& rxSubmitButton, const starawt::MouseEvent& MouseEvt);

    void FillSuccessfulList(HtmlSuccessfulObjList& rList, const staruno::Reference<starawt::XControl>& rxSubmitButton, const starawt::MouseEvent& MouseEvt);

    void InsertTextPart(INetMIMEMessage& rParent, const ::rtl::OUString& rName, const ::rtl::OUString& rData);
    sal_Bool InsertFilePart(INetMIMEMessage& rParent, const ::rtl::OUString& rName, const ::rtl::OUString& rFileName);
    void Encode(::rtl::OUString& rString) const;

    staruno::Reference< starsdbc::XConnection >  getConnection();

    DECL_LINK( OnTimeout, void* );
};

inline void ODatabaseForm::submitNBC(const staruno::Reference<starawt::XControl>& Control, const starawt::MouseEvent& MouseEvt)
{
    submit_impl(Control, MouseEvt, sal_False);
}

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FRM_DATABASEFORM_HXX_


