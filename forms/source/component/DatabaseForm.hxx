/*************************************************************************
 *
 *  $RCSfile: DatabaseForm.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-23 07:38:32 $
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
#ifndef _CPPUHELPER_IMPLBASE11_HXX_
#include <cppuhelper/implbase11.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE7_HXX_
#include <cppuhelper/implbase7.hxx>
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
typedef ::cppu::ImplHelper11<   ::com::sun::star::form::XForm,
                                ::com::sun::star::awt::XTabControllerModel,
                                ::com::sun::star::form::XLoadListener,
                                ::com::sun::star::sdbc::XRowSetListener,
                                ::com::sun::star::sdb::XRowSetApproveListener,
                                ::com::sun::star::form::XDatabaseParameterBroadcaster,
                                ::com::sun::star::sdb::XSQLErrorListener,
                                ::com::sun::star::sdb::XSQLErrorBroadcaster,
                                ::com::sun::star::form::XReset,
                                ::com::sun::star::form::XSubmit,
                                ::com::sun::star::form::XLoadable > ODatabaseForm_BASE1;


typedef ::cppu::ImplHelper2<    ::com::sun::star::container::XNamed,
                                ::com::sun::star::lang::XServiceInfo> ODatabaseForm_BASE2;

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
                    ,public ::comphelper::OAggregationArrayUsageHelper<ODatabaseForm>
                    ,public ODatabaseForm_BASE1
                    ,public ODatabaseForm_BASE2
                    ,public ODatabaseForm_BASE3
{
    friend class OFormSubmitResetThread;

    OImplementationIdsRef               m_aHoldIdHelper;

        // listener administration
    ::cppu::OInterfaceContainerHelper   m_aLoadListeners;
    ::cppu::OInterfaceContainerHelper   m_aRowSetApproveListeners;
    ::cppu::OInterfaceContainerHelper   m_aRowSetListeners;
    ::cppu::OInterfaceContainerHelper   m_aParameterListeners;
    ::cppu::OInterfaceContainerHelper   m_aResetListeners;
    ::cppu::OInterfaceContainerHelper   m_aSubmitListeners;
    ::cppu::OInterfaceContainerHelper   m_aErrorListeners;
    ::osl::Mutex                        m_aResetSafety;
    ::com::sun::star::uno::Any          m_aCycle;
    StringSequence                      m_aMasterFields;
    StringSequence                      m_aDetailFields;
    ::std::vector<bool>                 m_aParameterVisited;

    // the object doin' most of the work - an SDB-rowset
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation>      m_xAggregate;
    // same object, interface as member because of performance reasons
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>          m_xAggregateAsRowSet;

    // Verwaltung der ControlGruppen
    OGroupManager*              m_pGroupManager;
    OParameterInfoImpl*         m_pParameterInfo;
    Timer*                      m_pLoadTimer;

    OFormSubmitResetThread*     m_pThread;
    ::rtl::OUString             m_sCurrentErrorContext;
                // will be used as additional context information
                // when an exception is catched and forwarded to the listeners

    INT32                       m_nResetsPending;
//  <overwritten_properties>
    sal_Int32                   m_nPrivileges;
//  </overwritten_properties>

//  <properties>
    ::rtl::OUString             m_sName;
    ::rtl::OUString             m_aTargetURL;
    ::rtl::OUString             m_aTargetFrame;
    ::com::sun::star::form::FormSubmitMethod    m_eSubmitMethod;
    ::com::sun::star::form::FormSubmitEncoding  m_eSubmitEncoding;
    ::com::sun::star::form::NavigationBarMode   m_eNavigation;
    sal_Bool                    m_bAllowInsert : 1;
    sal_Bool                    m_bAllowUpdate : 1;
    sal_Bool                    m_bAllowDelete : 1;
//  </properties>
    sal_Bool                    m_bLoaded : 1;
    sal_Bool                    m_bSubForm : 1;
    sal_Bool                    m_bOwnConnection : 1; // is set to false when an activeconnection was set

public:
    ODatabaseForm(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    ~ODatabaseForm();

    // UNO binding
    DECLARE_UNO3_AGG_DEFAULTS(ODatabaseForm, OFormComponents);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XComponent
    virtual void SAL_CALL disposing();

    // property handling
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue);

    ::com::sun::star::uno::Any  SAL_CALL getFastPropertyValue( sal_Int32 nHandle )
       throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    void fire( sal_Int32 * pnHandles, const ::com::sun::star::uno::Any * pNewValues, const ::com::sun::star::uno::Any * pOldValues, sal_Int32 nCount, sal_Bool bVetoable );

    virtual void fillProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;

    // com::sun::star::beans::XPropertyState
    virtual ::com::sun::star::beans::PropertyState getPropertyStateByHandle(sal_Int32 nHandle);
    virtual void setPropertyToDefaultByHandle(sal_Int32 nHandle);
    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle(sal_Int32 nHandle) const;

    // com::sun::star::sdbc::XSQLErrorBroadcaster
    virtual void SAL_CALL addSQLErrorListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSQLErrorListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::form::XForm
    // nothing to implement

    // com::sun::star::form::XReset
    virtual void SAL_CALL reset() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::form::XSubmit
    virtual void SAL_CALL submit(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& aControl, const ::com::sun::star::awt::MouseEvent& aMouseEvt) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addSubmitListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XSubmitListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSubmitListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XSubmitListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::container::XChild
    virtual InterfaceRef SAL_CALL getParent() { return OFormComponents::getParent(); }
    virtual void SAL_CALL setParent(const InterfaceRef& Parent);

    // com::sun::star::container::XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName(const ::rtl::OUString& aName) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::awt::XTabControllerModel
    virtual sal_Bool SAL_CALL getGroupControl() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setGroupControl(sal_Bool _bGroupControl) throw(::com::sun::star::uno::RuntimeException) { }
    virtual void SAL_CALL setControlModels(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& _rControls) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > SAL_CALL getControlModels() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setGroup(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& _rGroup, const ::rtl::OUString& _rGroupName) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getGroupCount() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL getGroup(sal_Int32 _nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& _rxGroup, ::rtl::OUString& _rName) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL getGroupByName(const ::rtl::OUString& _rName, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& _rxGroup) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::form::XLoadListener
    virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unloading(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unloaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reloading(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reloaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::form::XLoadable
    virtual void SAL_CALL load() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unload() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reload() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isLoaded() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addLoadListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeLoadListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::sdbc::XCloseable
    virtual void SAL_CALL close() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // com::sun::star::sdbc::XRowSetListener
    virtual void SAL_CALL cursorMoved(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL rowChanged(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL rowSetChanged(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::sdb::XRowSetApproveListener
    virtual sal_Bool SAL_CALL approveCursorMove(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL approveRowChange(const ::com::sun::star::sdb::RowChangeEvent& event) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL approveRowSetChange(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::sdb::XRowSetApproveBroadcaster
    virtual void SAL_CALL addRowSetApproveListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRowSetApproveListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);

    // com::sun:star::form::XDatabaseParameterBroadcaster
    virtual void SAL_CALL addParameterListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeParameterListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::sdbc::XRowSet
    virtual void SAL_CALL execute() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addRowSetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRowSetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::sdb::XCompletedExecution
    virtual void SAL_CALL executeWithCompletion( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& handler ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // com::sun::star::sdbc::XResultSet
    virtual sal_Bool SAL_CALL next() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isBeforeFirst() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAfterLast() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isFirst() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isLast() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL beforeFirst() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL afterLast() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL first() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL last() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL absolute(sal_Int32 row) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL relative(sal_Int32 rows) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL previous() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL refreshRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL rowUpdated() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL rowInserted() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL rowDeleted() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual InterfaceRef SAL_CALL getStatement() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // com::sun::star::sdbc::XResultSetUpdate
    virtual void SAL_CALL insertRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deleteRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancelRowUpdates() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL moveToInsertRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL moveToCurrentRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // com::sun::star::sdbcx::XDeleteRows
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& rows) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // com::sun::star::lang::XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& ServiceName)  throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getImplementationName()  throw(::com::sun::star::uno::RuntimeException);
    virtual StringSequence SAL_CALL getSupportedServiceNames()  throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::io::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // com::sun::star::sdbc::XSQLErrorListener
    virtual void SAL_CALL errorOccured(const ::com::sun::star::sdb::SQLErrorEvent& aEvent) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::sdbc::XParameters
    virtual void SAL_CALL setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setBoolean(sal_Int32 parameterIndex, sal_Bool x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setByte(sal_Int32 parameterIndex, sal_Int8 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setShort(sal_Int32 parameterIndex, sal_Int16 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setInt(sal_Int32 parameterIndex, sal_Int32 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLong(sal_Int32 parameterIndex, sal_Int64 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFloat(sal_Int32 parameterIndex, float x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDouble(sal_Int32 parameterIndex, double x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setString(sal_Int32 parameterIndex, const ::rtl::OUString& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setBytes(sal_Int32 parameterIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDate(sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTime(sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTimestamp(sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setBinaryStream(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCharacterStream(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setObject(sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setObjectWithInfo(sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRef(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef>& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setBlob(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob>& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setClob(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob>& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setArray(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray>& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clearParameters() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    inline void submitNBC( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& Control, const ::com::sun::star::awt::MouseEvent& MouseEvt );

protected:
    virtual void implInserted(const InterfaceRef& _rxObject);
    virtual void implRemoved(const InterfaceRef& _rxObject);

    // OPropertyChangeListener
    virtual void _propertyChanged( const ::com::sun::star::beans::PropertyChangeEvent& ) throw(::com::sun::star::uno::RuntimeException);

private:
    sal_Bool executeRowSet(ReusableMutexGuard& _rClearForNotifies, sal_Bool bMoveToFirst = sal_True,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxCompletionHandler = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >());
    bool    fillParameters(ReusableMutexGuard& _rClearForNotifies,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxCompletionHandler = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >());
    OParameterInfoImpl* createParameterInfo() const;
    bool    hasValidParent() const;
    // if there are no parameter infos we now that we have a complete new statement to execute
    bool    needStatementRebuild() const {return m_pParameterInfo == NULL;}

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

    // error handling
    void    onError(const ::com::sun::star::sdb::SQLErrorEvent& _rEvent);
    void    onError(::com::sun::star::sdbc::SQLException&, const ::rtl::OUString& _rContextDescription);

    // html tools
    ::rtl::OUString         GetDataURLEncoded(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt);
    ::rtl::OUString         GetDataTextEncoded(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt);
    ::com::sun::star::uno::Sequence<sal_Int8>   GetDataMultiPartEncoded(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt,
                                             ::rtl::OUString& rContentType);

    void AppendComponent(HtmlSuccessfulObjList& rList, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xComponentSet, const ::rtl::OUString& rNamePrefix,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& rxSubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt);

    void FillSuccessfulList(HtmlSuccessfulObjList& rList, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& rxSubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt);

    void InsertTextPart(INetMIMEMessage& rParent, const ::rtl::OUString& rName, const ::rtl::OUString& rData);
    sal_Bool InsertFilePart(INetMIMEMessage& rParent, const ::rtl::OUString& rName, const ::rtl::OUString& rFileName);
    void Encode(::rtl::OUString& rString) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >  getConnection();

    DECL_LINK( OnTimeout, void* );
};

inline void ODatabaseForm::submitNBC(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& Control, const ::com::sun::star::awt::MouseEvent& MouseEvt)
{
    submit_impl(Control, MouseEvt, sal_False);
}

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FRM_DATABASEFORM_HXX_


