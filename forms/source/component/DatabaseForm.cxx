/*************************************************************************
 *
 *  $RCSfile: DatabaseForm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-09-29 15:31:36 $
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

#include <stdio.h>

#ifndef _FRM_DATABASEFORM_HXX_
#include "DatabaseForm.hxx"
#endif
#ifndef _FRM_EVENT_THREAD_HXX_
#include "EventThread.hxx"
#endif
#ifndef _FORMS_LISTBOX_HXX_
#include "ListBox.hxx"
#endif
#ifndef _FRM_RESOURCE_HXX_
#include "frm_resource.hxx"
#endif
#ifndef _FRM_RESOURCE_HRC_
#include "frm_resource.hrc"
#endif

#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif
#ifndef _COM_SUN_STAR_DATA_DATABASECURSORTYPE_HPP_
#include <com/sun/star/data/DatabaseCursorType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_DATASELECTIONTYPE_HPP_
#include <com/sun/star/form/DataSelectionType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_ROWSETVETOEXCEPTION_HPP_
#include <com/sun/star/sdb/RowSetVetoException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XPARAMETERSSUPPLIER_HPP_
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_TABULATORCYCLE_HPP_
#include <com/sun/star/form/TabulatorCycle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef _FRM_GROUPMANAGER_HXX_
#include "GroupManager.hxx"
#endif

#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_PROPERTY_HXX_
#include "property.hxx"
#endif
#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif
#ifndef _FRM_IDS_HXX_
#include "ids.hxx"
#endif

#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _TOOLS_INETMSG_HXX
#include <tools/inetmsg.hxx>
#endif
#ifndef _INETSTRM_HXX //autogen
#include <svtools/inetstrm.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _UTL_STLTYPES_HXX_
#include <unotools/stl_types.hxx>
#endif
#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
#endif
#ifndef _UTL_STLTYPES_HXX_
#include <unotools/stl_types.hxx>
#endif
#ifndef _UTL_UNO3_HXX_
#include <unotools/uno3.hxx>
#endif
#ifndef _UNOTOOLS_ENUMHELPER_HXX_
#include <unotools/enumhelper.hxx>
#endif
#ifndef _UTL_CONTAINER_HXX_
#include <unotools/container.hxx>
#endif
#ifndef _UTL_UNO3_DB_TOOLS_HXX_
#include <unotools/dbtools.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _TOOLS_SOLMATH_HXX
#include <tools/solmath.hxx>
#endif
#ifndef _DATE_HXX
#include <tools/Date.hxx>
#endif
#ifndef _TOOLS_TIME_HXX
#include <tools/Time.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/DateTime.hxx>
#endif
#ifndef _INETTYPE_HXX
#include <svtools/inettype.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX   // because of the solar mutex
#include <vcl/svapp.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#define DATABASEFORM_IMPLEMENTATION_NAME    ::rtl::OUString::createFromAscii("com.sun.star.form.component.ODatabaseForm")

//.........................................................................
namespace frm
{
//.........................................................................

    namespace starframe = ::com::sun::star::frame;
    namespace stardata  = ::com::sun::star::data;

//------------------------------------------------------------------
staruno::Reference< starframe::XModel> getXModel(const staruno::Reference< staruno::XInterface>& xIface)
{
    staruno::Reference< starframe::XModel> xModel(xIface, staruno::UNO_QUERY);
    if (xModel.is())
        return xModel;
    else
    {
        staruno::Reference< starcontainer::XChild> xChild(xIface, staruno::UNO_QUERY);
        if (xChild.is())
        {
            staruno::Reference< staruno::XInterface> xParent( xChild->getParent());
            return getXModel(xParent);
        }
        else
            return NULL;
    }
}

//==================================================================
//= OParameterWrapper
//=-----------------------------------------------------------------
//= wraps a parameter property set got from an starsdb::SQLQueryComposer
//= so it has an additional property "Value", which is forwarded to
//= an starsdbc::XParameters interface
//==================================================================

class OParameterWrapper
        :public ::cppu::OWeakObject
        ,public ::cppu::OPropertySetHelper
        ,public ::utl::OAggregationArrayUsageHelper<OParameterWrapper>
{
    staruno::Any                m_aValue;
    ::osl::Mutex                m_aMutex;
    ::cppu::OBroadcastHelper    m_aBroadcastHelper;
    OImplementationIdsRef       m_aHoldIdHelper;

    staruno::Reference<starbeans::XPropertySet>     m_xPseudoAggregate;
    staruno::Reference<starsdbc::XParameters>       m_xValueDestination;
    sal_Int32               m_nIndex;


    virtual ~OParameterWrapper();
public:
    OParameterWrapper(const staruno::Reference<starbeans::XPropertySet>& _rxColumn, const staruno::Reference<starsdbc::XParameters>& _rxAllParameters, sal_Int32 _nIndex);

    // UNO
    DECLARE_UNO3_DEFAULTS(OParameterWrapper, OWeakObject);
    virtual staruno::Any    SAL_CALL queryInterface(const staruno::Type& _rType) throw (staruno::RuntimeException);
    virtual staruno::Sequence<sal_Int8>         SAL_CALL getImplementationId() throw(staruno::RuntimeException);
    virtual staruno::Sequence<staruno::Type>    SAL_CALL getTypes() throw(staruno::RuntimeException);

    // starbeans::XPropertySet
    virtual staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw( staruno::RuntimeException );
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // OPropertySetHelper
    virtual sal_Bool SAL_CALL convertFastPropertyValue( staruno::Any& rConvertedValue, staruno::Any& rOldValue, sal_Int32 nHandle, const staruno::Any& rValue) throw( starlang::IllegalArgumentException );
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const staruno::Any& rValue ) throw( staruno::Exception );
    virtual void SAL_CALL getFastPropertyValue( staruno::Any& rValue, sal_Int32 nHandle ) const;

    // OAggregationArrayUsageHelper
    virtual void fillProperties(
        staruno::Sequence< starbeans::Property >& /* [out] */ _rProps,
        staruno::Sequence< starbeans::Property >& /* [out] */ _rAggregateProps
        ) const;

protected:
    ::rtl::OUString getPseudoAggregatePropertyName(sal_Int32 _nHandle) const;
};

DBG_NAME(OParameterWrapper)
//------------------------------------------------------------------------------
OParameterWrapper::OParameterWrapper(const staruno::Reference<starbeans::XPropertySet>& _rxColumn, const staruno::Reference<starsdbc::XParameters>& _rxAllParameters, sal_Int32 _nIndex)
    :OPropertySetHelper(m_aBroadcastHelper)
    ,m_aBroadcastHelper(m_aMutex)
    ,m_xPseudoAggregate(_rxColumn)
    ,m_xValueDestination(_rxAllParameters)
    ,m_nIndex(_nIndex)
{
    DBG_CTOR(OParameterWrapper, NULL);
}

//------------------------------------------------------------------------------
OParameterWrapper::~OParameterWrapper()
{
    DBG_DTOR(OParameterWrapper, NULL);
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OParameterWrapper::queryInterface(const staruno::Type& _rType) throw (staruno::RuntimeException)
{
    staruno::Any aReturn;
    aReturn = OWeakObject::queryInterface(_rType);

    if (!aReturn.hasValue())
        OPropertySetHelper::queryInterface(_rType);

    return aReturn;
}

//------------------------------------------------------------------------------
staruno::Sequence< staruno::Type > SAL_CALL OParameterWrapper::getTypes(  ) throw(staruno::RuntimeException)
{
    staruno::Sequence< staruno::Type > aWeakTypes(1);
    aWeakTypes.getArray()[0] = ::getCppuType(static_cast<staruno::Reference<staruno::XWeak>*>(NULL));

    staruno::Sequence< staruno::Type > aPropertyTypes(3);
    aPropertyTypes.getArray()[0] = ::getCppuType(static_cast<staruno::Reference<starbeans::XPropertySet>*>(NULL));
    aPropertyTypes.getArray()[1] = ::getCppuType(static_cast<staruno::Reference<starbeans::XFastPropertySet>*>(NULL));
    aPropertyTypes.getArray()[2] = ::getCppuType(static_cast<staruno::Reference<starbeans::XMultiPropertySet>*>(NULL));

    return concatSequences(aWeakTypes, aPropertyTypes);
}

//------------------------------------------------------------------------------
staruno::Sequence<sal_Int8> SAL_CALL OParameterWrapper::getImplementationId() throw(staruno::RuntimeException)
{
    staruno::Reference<starlang::XTypeProvider> xMyTpes;
    query_interface(static_cast<staruno::XWeak*>(this), xMyTpes);
    return OImplementationIds::getImplementationId(xMyTpes);
}

//------------------------------------------------------------------------------
::rtl::OUString OParameterWrapper::getPseudoAggregatePropertyName(sal_Int32 _nHandle) const
{
    staruno::Reference<starbeans::XPropertySetInfo>  xInfo = const_cast<OParameterWrapper*>(this)->getPropertySetInfo();
    staruno::Sequence<starbeans::Property> aProperties = xInfo->getProperties();
    const starbeans::Property* pProperties = aProperties.getConstArray();
    for (sal_Int32 i=0; i<aProperties.getLength(); ++i, ++pProperties)
    {
        if (pProperties->Handle == _nHandle)
            return pProperties->Name;
    }

    DBG_ERROR("OParameterWrapper::getPseudoAggregatePropertyName : invalid argument !");
    return ::rtl::OUString();
}

//------------------------------------------------------------------------------
staruno::Reference<starbeans::XPropertySetInfo>  OParameterWrapper::getPropertySetInfo() throw( staruno::RuntimeException )
{
    staruno::Reference<starbeans::XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
void OParameterWrapper::fillProperties(
        staruno::Sequence< starbeans::Property >& _rProps,
        staruno::Sequence< starbeans::Property >& _rAggregateProps ) const
{
    BEGIN_AGGREGATION_PROPERTY_HELPER(1, m_xPseudoAggregate)
        DECL_PROP2(VALUE,       ::rtl::OUString, TRANSIENT, MAYBEVOID);
    END_AGGREGATION_PROPERTY_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OParameterWrapper::getInfoHelper()
{
    return *const_cast<OParameterWrapper*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
sal_Bool OParameterWrapper::convertFastPropertyValue(staruno::Any& rConvertedValue, staruno::Any& rOldValue, sal_Int32 nHandle, const staruno::Any& rValue) throw( starlang::IllegalArgumentException )
{
    DBG_ASSERT(PROPERTY_ID_VALUE == nHandle, "OParameterWrapper::convertFastPropertyValue : the only non-readonly prop should be our PROPERTY_VALUE");
    // we're lazy here ...
    rOldValue = m_aValue;
    rConvertedValue = rValue;
    return sal_True;    // assume "modified" ...
}

//------------------------------------------------------------------------------
void OParameterWrapper::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const staruno::Any& rValue ) throw( staruno::Exception )
{
    if (nHandle == PROPERTY_ID_VALUE)
    {
        // get the type of the param
        staruno::Any aParamType = m_xPseudoAggregate->getPropertyValue(PROPERTY_FIELDTYPE);
        DBG_ASSERT(aParamType.getValueType().getTypeClass() == staruno::TypeClass_LONG, "ODatabaseForm::setPropertyValue : invalid parameter field !");
        sal_Int32 nScale = 0;
        if (hasProperty(PROPERTY_SCALE, m_xPseudoAggregate))
        {
            staruno::Any aScale = m_xPseudoAggregate->getPropertyValue(PROPERTY_SCALE);
            DBG_ASSERT(aScale.getValueType().getTypeClass() == staruno::TypeClass_LONG, "ODatabaseForm::setPropertyValue : invalid parameter field !");
            nScale = getINT32(aScale);
        }
        // TODO : aParamType & nScale can be obtained within the constructor ....

        try
        {
            m_xValueDestination->setObjectWithInfo(m_nIndex + 1, rValue, getINT32(aParamType), nScale);
                // the index of the parameters is one-based
            m_aValue = rValue;
        }
        catch(starsdbc::SQLException& e)
        {
            starlang::WrappedTargetException aExceptionWrapper;
            aExceptionWrapper.Context = e.Context;
            aExceptionWrapper.Message = e.Message;
            aExceptionWrapper.TargetException <<= e;
            throw starlang::WrappedTargetException(aExceptionWrapper);
        }
    }
    else
    {
        ::rtl::OUString aName = getPseudoAggregatePropertyName(nHandle);
        m_xPseudoAggregate->setPropertyValue(aName, rValue);
    }
}

//------------------------------------------------------------------------------
void OParameterWrapper::getFastPropertyValue( staruno::Any& rValue, sal_Int32 nHandle ) const
{
    if (nHandle == PROPERTY_ID_VALUE)
    {
        rValue = m_aValue;
    }
    else
    {
        ::rtl::OUString aName = getPseudoAggregatePropertyName(nHandle);
        rValue = m_xPseudoAggregate->getPropertyValue(aName);
    }
}

//==================================================================
//= OParametersImpl
//=-----------------------------------------------------------------
//= class for the parameter event see approveParameter
//==================================================================

typedef ::cppu::WeakImplHelper2<starcontainer::XIndexAccess, starcontainer::XEnumerationAccess> OParametersImplBase;
class OParametersImpl : public OParametersImplBase
{
public:
    typedef ::std::vector<staruno::Reference<starbeans::XPropertySet> > Parameters;
    typedef Parameters::iterator                    ParametersIterator;

private:
    Parameters      m_aParameters;

public:
    // UNO
    DECLARE_UNO3_AGG_DEFAULTS(OParametersImpl, OParametersImplBase);

    // XElementAccess
    virtual staruno::Type SAL_CALL getElementType() throw( staruno::RuntimeException );
    virtual sal_Bool SAL_CALL hasElements() throw( staruno::RuntimeException );

    // XEnumerationAccess
    virtual staruno::Reference<starcontainer::XEnumeration> SAL_CALL createEnumeration() throw( staruno::RuntimeException );

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw( staruno::RuntimeException );
    virtual staruno::Any SAL_CALL getByIndex(sal_Int32 _rIndex) throw( starlang::IndexOutOfBoundsException, starlang::WrappedTargetException, staruno::RuntimeException );

    Parameters& getParameters() { return m_aParameters; }
};

// starcontainer::XElementAccess
//------------------------------------------------------------------------------
staruno::Type SAL_CALL OParametersImpl::getElementType() throw( staruno::RuntimeException )
{
    return ::getCppuType(static_cast<staruno::Reference<starbeans::XPropertySet>*>(NULL));
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OParametersImpl::hasElements() throw( staruno::RuntimeException )
{
    return m_aParameters.size() != 0;
}

// starcontainer::XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OParametersImpl::getCount() throw( staruno::RuntimeException )
{
    return m_aParameters.size();
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OParametersImpl::getByIndex(sal_Int32 _nIndex) throw( starlang::IndexOutOfBoundsException, starlang::WrappedTargetException, staruno::RuntimeException )
{
    if (_nIndex < 0 ||
        _nIndex >= m_aParameters.size())
        throw starlang::IndexOutOfBoundsException();

    return staruno::makeAny(m_aParameters[_nIndex]);
}

// starcontainer::XEnumerationAccess
//------------------------------------------------------------------------------
staruno::Reference<starcontainer::XEnumeration>  OParametersImpl::createEnumeration() throw( staruno::RuntimeException )
{
    return new OEnumerationByIndex(reinterpret_cast<starcontainer::XIndexAccess*>(this));
}

//==================================================================
//= OParameterInfoImpl
//=-----------------------------------------------------------------
//= class which collects all information for parameter filling
//==================================================================
DECLARE_STL_USTRINGACCESS_MAP(sal_Int32, MapUString2INT32);

struct OParameterInfoImpl
{
    sal_Int32                   nCount;                 //  Number of Parameters
    staruno::Reference<starsdb::XSQLQueryComposer>  xComposer;
    staruno::Reference<starcontainer::XNameAccess>          xParamsAsNames;
    OParametersImpl*        pParameters;
    MapUString2INT32        aParamMapping;

    OParameterInfoImpl():nCount(0),pParameters(NULL){}
    ~OParameterInfoImpl()
    {
        if (pParameters)
            pParameters->release();
    }
};

//==================================================================
//= OFormSubmitResetThread
//=-----------------------------------------------------------------
//= submitting and resetting html-forms asynchronously
//==================================================================

//------------------------------------------------------------------
class OFormSubmitResetThread: public OComponentEventThread
{
protected:

    // duplicate an event with respect to it's type
    virtual starlang::EventObject *cloneEvent( const starlang::EventObject *pEvt ) const;

    // process an event. while processing the mutex isn't locked, and pCompImpl
    // is made sure to remain valid
    virtual void processEvent( ::cppu::OComponentHelper* _pCompImpl,
                               const starlang::EventObject* _pEvt,
                               const staruno::Reference<starawt::XControl>& _rControl,
                               sal_Bool _bSubmit);

public:

    OFormSubmitResetThread(ODatabaseForm* pControl) : OComponentEventThread(pControl) { }
};

//------------------------------------------------------------------
starlang::EventObject* OFormSubmitResetThread::cloneEvent(
        const starlang::EventObject *pEvt ) const
{
    return new starawt::MouseEvent( *(starawt::MouseEvent *)pEvt );
}

//------------------------------------------------------------------
void OFormSubmitResetThread::processEvent(
        ::cppu::OComponentHelper* pCompImpl,
        const starlang::EventObject *_pEvt,
        const staruno::Reference<starawt::XControl>& _rControl,
        sal_Bool _bSubmit)
{
    if (_bSubmit)
        ((ODatabaseForm *)pCompImpl)->submit_impl(_rControl, *reinterpret_cast<const starawt::MouseEvent*>(_pEvt), true);
    else
        ((ODatabaseForm *)pCompImpl)->reset_impl(true);
}

//==================================================================
//= ODatabaseForm
//==================================================================

//------------------------------------------------------------------
InterfaceRef SAL_CALL ODatabaseForm_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    return *(new ODatabaseForm(_rxFactory));
}

//------------------------------------------------------------------------------
staruno::Sequence<sal_Int8> SAL_CALL ODatabaseForm::getImplementationId() throw(staruno::RuntimeException)
{
    return OImplementationIds::getImplementationId(getTypes());
}

//------------------------------------------------------------------
staruno::Sequence<staruno::Type> SAL_CALL ODatabaseForm::getTypes() throw(staruno::RuntimeException)
{
    // ask the aggregate
    staruno::Sequence<staruno::Type> aAggregateTypes;
    staruno::Reference<starlang::XTypeProvider> xAggregateTypes;
    if (query_aggregation(m_xAggregate, xAggregateTypes))
        aAggregateTypes = xAggregateTypes->getTypes();

    staruno::Sequence<staruno::Type> aOwnTypes(23);
    aOwnTypes.getArray()[0] = ::getCppuType(static_cast<staruno::Reference<starbeans::XFastPropertySet>*>(NULL));
    aOwnTypes.getArray()[1] = ::getCppuType(static_cast<staruno::Reference<starbeans::XMultiPropertySet>*>(NULL));
    aOwnTypes.getArray()[2] = ::getCppuType(static_cast<staruno::Reference<starbeans::XPropertySet>*>(NULL));
    aOwnTypes.getArray()[3] = ::getCppuType(static_cast<staruno::Reference<starbeans::XPropertyState>*>(NULL));
    aOwnTypes.getArray()[4] = ::getCppuType(static_cast<staruno::Reference<starsdb::XSQLErrorBroadcaster>*>(NULL));
    aOwnTypes.getArray()[5] = ::getCppuType(static_cast<staruno::Reference<starform::XForm>*>(NULL));
    aOwnTypes.getArray()[6] = ::getCppuType(static_cast<staruno::Reference<starawt::XTabControllerModel>*>(NULL));
    aOwnTypes.getArray()[7] = ::getCppuType(static_cast<staruno::Reference<starform::XReset>*>(NULL));
    aOwnTypes.getArray()[8] = ::getCppuType(static_cast<staruno::Reference<starform::XSubmit>*>(NULL));
    aOwnTypes.getArray()[9] = ::getCppuType(static_cast<staruno::Reference<starform::XLoadListener>*>(NULL));
    aOwnTypes.getArray()[10] = ::getCppuType(static_cast<staruno::Reference<starform::XLoadable>*>(NULL));
    aOwnTypes.getArray()[11] = ::getCppuType(static_cast<staruno::Reference<starsdbc::XCloseable>*>(NULL));
    aOwnTypes.getArray()[12] = ::getCppuType(static_cast<staruno::Reference<starsdbc::XRowSet>*>(NULL));
    aOwnTypes.getArray()[13] = ::getCppuType(static_cast<staruno::Reference<starsdbc::XRowSetListener>*>(NULL));
    aOwnTypes.getArray()[14] = ::getCppuType(static_cast<staruno::Reference<starsdb::XRowSetApproveListener>*>(NULL));
    aOwnTypes.getArray()[15] = ::getCppuType(static_cast<staruno::Reference<starsdb::XRowSetApproveBroadcaster>*>(NULL));
    aOwnTypes.getArray()[16] = ::getCppuType(static_cast<staruno::Reference<starsdbc::XResultSetUpdate>*>(NULL));
    aOwnTypes.getArray()[17] = ::getCppuType(static_cast<staruno::Reference<starsdbcx::XDeleteRows>*>(NULL));
    aOwnTypes.getArray()[18] = ::getCppuType(static_cast<staruno::Reference<starsdbc::XParameters>*>(NULL));
    aOwnTypes.getArray()[19] = ::getCppuType(static_cast<staruno::Reference<starform::XDatabaseParameterBroadcaster>*>(NULL));
    aOwnTypes.getArray()[20] = ::getCppuType(static_cast<staruno::Reference<starsdb::XSQLErrorListener>*>(NULL));
    aOwnTypes.getArray()[21] = ::getCppuType(static_cast<staruno::Reference<starcontainer::XNamed>*>(NULL));
    aOwnTypes.getArray()[22] = ::getCppuType(static_cast<staruno::Reference<starlang::XServiceInfo>*>(NULL));

    return concatSequences(aAggregateTypes, aOwnTypes, OFormComponents::getTypes());
}

//------------------------------------------------------------------
staruno::Any SAL_CALL ODatabaseForm::queryAggregation(const staruno::Type& _rType) throw(staruno::RuntimeException)
{
    staruno::Any aReturn;
    // our own interfaces
    aReturn = ::cppu::queryInterface(_rType,
        static_cast<starform::XForm*>(this),
        static_cast<starawt::XTabControllerModel*>(this),
        static_cast<starform::XLoadListener*>(this),
        static_cast<starform::XLoadable*>(this),
        static_cast<starsdbc::XRowSetListener*>(this),
        static_cast<starsdb::XRowSetApproveListener*>(this),
        static_cast<starform::XDatabaseParameterBroadcaster*>(this),
        static_cast<starsdb::XSQLErrorListener*>(this),
        static_cast<starsdb::XSQLErrorBroadcaster*>(this),
        static_cast<stario::XPersistObject*>(this),
        static_cast<starform::XReset*>(this),
        static_cast<starform::XSubmit*>(this)
    );
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast<starcontainer::XNamed*>(this),
            static_cast<starlang::XServiceInfo*>(this),
            static_cast<starlang::XEventListener*>(static_cast<starform::XLoadListener*>(this))
        );
    // interfaces already present in the aggregate which we want to reroute
    // only available if we could create the aggregate
    if (!aReturn.hasValue() && m_xAggregateAsRowSet.is())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast<starsdb::XRowSetApproveBroadcaster*>(this),
            static_cast<starsdbc::XRowSet*>(this),
            static_cast<starsdbc::XResultSetUpdate*>(this),
            static_cast<starsdbc::XResultSet*>(this),
            static_cast<starsdbc::XCloseable*>(this),
            static_cast<starsdbcx::XDeleteRows*>(this),
            static_cast<starsdbc::XParameters*>(this)
        );

    // property set related interfaces
    if (!aReturn.hasValue())
        aReturn = OPropertySetAggregationHelper::queryInterface(_rType);

    // form component collection related interfaces
    if (!aReturn.hasValue())
        aReturn = OFormComponents::queryAggregation(_rType);

    // aggregate interfaces
    // (ask the aggregated object _after_ the OComponentHelper (base of OFormComponents),
    // so calls to the starlang::XComponent interface reach us and not the aggreagtion)
    if (!aReturn.hasValue() && m_xAggregate.is())
        aReturn = m_xAggregate->queryAggregation(_rType);

    return aReturn;
}

DBG_NAME(ODatabaseForm);
//------------------------------------------------------------------
ODatabaseForm::ODatabaseForm(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
        :OFormComponents(_rxFactory)
        ,OPropertySetAggregationHelper(OComponentHelper::rBHelper)
        ,OPropertyChangeListener(m_aMutex)
        ,m_aLoadListeners(m_aMutex)
        ,m_aRowSetApproveListeners(m_aMutex)
        ,m_aRowSetListeners(m_aMutex)
        ,m_aParameterListeners(m_aMutex)
        ,m_aResetListeners(m_aMutex)
        ,m_aSubmitListeners(m_aMutex)
        ,m_aErrorListeners(m_aMutex)
        ,m_bLoaded(sal_False)
        ,m_bSubForm(sal_False)
        ,m_eNavigation(starform::NavigationBarMode_CURRENT)
        ,m_nPrivileges(0)
        ,m_pParameterInfo(NULL)
        ,m_pThread(NULL)
        ,m_eSubmitMethod(starform::FormSubmitMethod_GET)
        ,m_eSubmitEncoding(starform::FormSubmitEncoding_URL)
        ,m_bAllowDelete(sal_True)
        ,m_bAllowUpdate(sal_True)
        ,m_bAllowInsert(sal_True)
        ,m_pLoadTimer(NULL)
        ,m_nResetsPending(0)
{
    DBG_CTOR(ODatabaseForm,NULL);

    // aggregate a row set
    increment(m_refCount);

    {
        m_xAggregate = staruno::Reference<staruno::XAggregation>(m_xServiceFactory->createInstance(SRV_SDB_ROWSET), staruno::UNO_QUERY);
        //  m_xAggregate = staruno::Reference<staruno::XAggregation>(m_xServiceFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.sdb.dbaccess.ORowSet")), staruno::UNO_QUERY);
        DBG_ASSERT(m_xAggregate.is(), "ODatabaseForm::ODatabaseForm : could not instantiate an SDB rowset !");
        m_xAggregateAsRowSet = staruno::Reference<starsdbc::XRowSet> (m_xAggregate,staruno::UNO_QUERY);
        setAggregation(m_xAggregate);
    }

    if (m_xAggregate.is())
    {
        m_xAggregate->setDelegator(static_cast<staruno::XWeak*>(this));
    }

    // listen for the properties, important for Parameters
    if (m_xAggregateSet.is())
    {
        OPropertyChangeMultiplexer* pMultiplexer = new OPropertyChangeMultiplexer(this, m_xAggregateSet);
        pMultiplexer->addProperty(PROPERTY_COMMAND);
        pMultiplexer->addProperty(PROPERTY_FILTER_CRITERIA);
        pMultiplexer->addProperty(PROPERTY_APPLYFILTER);
    }

    decrement(m_refCount);

    m_pGroupManager = new OGroupManager();
    m_pGroupManager->acquire();
    addContainerListener(m_pGroupManager);
}

//------------------------------------------------------------------
ODatabaseForm::~ODatabaseForm()
{
    DBG_DTOR(ODatabaseForm,NULL);

    m_pGroupManager->release();

    if (m_xAggregate.is())
        m_xAggregate->setDelegator(InterfaceRef());
}

//==============================================================================
// html tools
//------------------------------------------------------------------------
::rtl::OUString ODatabaseForm::GetDataURLEncoded(const staruno::Reference<starawt::XControl>& SubmitButton, const starawt::MouseEvent& MouseEvt)
{

    // Liste von successful Controls fuellen
    HtmlSuccessfulObjList aSuccObjList;
    FillSuccessfulList( aSuccObjList, SubmitButton, MouseEvt );


    // Liste zu ::rtl::OUString zusammensetzen
    ::rtl::OUString aResult;
    ::rtl::OUString aName;
    ::rtl::OUString aValue;

    for (   HtmlSuccessfulObjListIterator pSuccObj = aSuccObjList.begin();
            pSuccObj < aSuccObjList.end();
            ++pSuccObj
        )
    {
        aName = pSuccObj->aName;
        aValue = pSuccObj->aValue;
        if( pSuccObj->nRepresentation == SUCCESSFUL_REPRESENT_FILE && aValue.getLength() )
        {
            // Bei File-URLs wird der Dateiname und keine starutil::URL uebertragen,
            // weil Netscape dies so macht.
            INetURLObject aURL;
            aURL.SetSmartProtocol(INET_PROT_FILE);
            aURL.SetSmartURL(aValue);
            if( INET_PROT_FILE == aURL.GetProtocol() )
                aValue = INetURLObject::decode(aURL.PathToFileName(), '%', INetURLObject::DECODE_UNAMBIGUOUS);
        }
        Encode( aName );
        Encode( aValue );
        aResult += aName;
        aResult += UniString('=');
        aResult += aValue;
        if (pSuccObj < aSuccObjList.end() - 1)
            aResult += UniString('&');
    }


    aSuccObjList.clear();

    return aResult;
}

//==============================================================================
// html tools
//------------------------------------------------------------------------
::rtl::OUString ODatabaseForm::GetDataTextEncoded(const staruno::Reference<starawt::XControl>& SubmitButton, const starawt::MouseEvent& MouseEvt)
{

    // Liste von successful Controls fuellen
    HtmlSuccessfulObjList aSuccObjList;
    FillSuccessfulList( aSuccObjList, SubmitButton, MouseEvt );
    // Liste zu ::rtl::OUString zusammensetzen
    ::rtl::OUString aResult;
    ::rtl::OUString aName;
    ::rtl::OUString aValue;

    for (   HtmlSuccessfulObjListIterator pSuccObj = aSuccObjList.begin();
            pSuccObj < aSuccObjList.end();
            ++pSuccObj
        )
    {
        aName = pSuccObj->aName;
        aValue = pSuccObj->aValue;
        if (pSuccObj->nRepresentation == SUCCESSFUL_REPRESENT_FILE && aValue.getLength())
        {
            // Bei File-URLs wird der Dateiname und keine starutil::URL uebertragen,
            // weil Netscape dies so macht.
            INetURLObject aURL;
            aURL.SetSmartProtocol(INET_PROT_FILE);
            aURL.SetSmartURL(aValue);
            if( INET_PROT_FILE == aURL.GetProtocol() )
                aValue = INetURLObject::decode(aURL.PathToFileName(), '%', INetURLObject::DECODE_UNAMBIGUOUS);
        }
        Encode( aName );
        Encode( aValue );
        aResult += pSuccObj->aName;
        aResult += UniString('=');
        aResult += pSuccObj->aValue;
        if (pSuccObj < aSuccObjList.end() - 1)
            aResult += ::rtl::OUString::createFromAscii("\r\n");
    }


    // Liste loeschen
    aSuccObjList.clear();

    return aResult;
}

//------------------------------------------------------------------------
staruno::Sequence<sal_Int8> ODatabaseForm::GetDataMultiPartEncoded(const staruno::Reference<starawt::XControl>& SubmitButton, const starawt::MouseEvent& MouseEvt, ::rtl::OUString& rContentType)
{

    // Parent erzeugen
    INetMIMEMessage aParent;
    aParent.EnableAttachChild( INETMSG_MULTIPART_FORM_DATA );


    // Liste von successful Controls fuellen
    HtmlSuccessfulObjList aSuccObjList;
    FillSuccessfulList( aSuccObjList, SubmitButton, MouseEvt );


    // Liste zu ::rtl::OUString zusammensetzen
    ::rtl::OUString aResult;
    for (   HtmlSuccessfulObjListIterator pSuccObj = aSuccObjList.begin();
            pSuccObj < aSuccObjList.end();
            ++pSuccObj
        )
    {
        if( pSuccObj->nRepresentation == SUCCESSFUL_REPRESENT_TEXT )
            InsertTextPart( aParent, pSuccObj->aName, pSuccObj->aValue );
        else if( pSuccObj->nRepresentation == SUCCESSFUL_REPRESENT_FILE )
            InsertFilePart( aParent, pSuccObj->aName, pSuccObj->aValue );
    }


    // Liste loeschen
    aSuccObjList.clear();

    // Fuer Parent MessageStream erzeugen
    INetMIMEMessageStream aMessStream;
    aMessStream.SetSourceMessage( &aParent );
    aMessStream.GenerateHeader( sal_False );

    // MessageStream in SvStream kopieren
    SvMemoryStream aMemStream;
    char* pBuf = new char[1025];
    int nRead;
    while( (nRead = aMessStream.Read(pBuf, 1024)) > 0 )
        aMemStream.Write( pBuf, nRead );
    delete[] pBuf;

    aMemStream.Flush();
    aMemStream.Seek( 0 );
    void* pData = (void*)aMemStream.GetData();
    sal_Int32 nLen = aMemStream.Seek(STREAM_SEEK_TO_END);

    rContentType = UniString(aParent.GetContentType());
    return staruno::Sequence<sal_Int8>((sal_Int8*)pData, nLen);
}

//------------------------------------------------------------------------
void ODatabaseForm::AppendComponent(HtmlSuccessfulObjList& rList, const staruno::Reference<starbeans::XPropertySet>& xComponentSet, const ::rtl::OUString& rNamePrefix,
                     const staruno::Reference<starawt::XControl>& rxSubmitButton, const starawt::MouseEvent& MouseEvt)
{
    if (!xComponentSet.is())
        return;

    // MIB 25.6.98: Geschachtelte Formulare abfangen ... oder muesste
    // man sie submitten?
    if (!hasProperty(PROPERTY_CLASSID, xComponentSet))
        return;

    // Namen ermitteln
    if (!hasProperty(PROPERTY_NAME, xComponentSet))
        return;

    sal_Int16 nClassId;
    xComponentSet->getPropertyValue(PROPERTY_CLASSID) >>= nClassId;
    ::rtl::OUString aName;
    xComponentSet->getPropertyValue( PROPERTY_NAME ) >>= aName;
    if( !aName.getLength() && nClassId != starform::FormComponentType::IMAGEBUTTON)
        return;
    else    // Name um den Prefix erweitern
        aName = rNamePrefix + aName;

    switch( nClassId )
    {
        // Buttons
        case starform::FormComponentType::COMMANDBUTTON:
        {
            // Es wird nur der gedrueckte Submit-Button ausgewertet
            // MIB: Sofern ueberhaupt einer uebergeben wurde
            if( rxSubmitButton.is() )
            {
                staruno::Reference<starbeans::XPropertySet>  xSubmitButtonComponent(rxSubmitButton->getModel(), staruno::UNO_QUERY);
                if (xSubmitButtonComponent == xComponentSet && hasProperty(PROPERTY_LABEL, xComponentSet))
                {
                    // <name>=<label>
                    ::rtl::OUString aLabel;
                    xComponentSet->getPropertyValue( PROPERTY_LABEL ) >>= aLabel;
                    rList.push_back( HtmlSuccessfulObj(aName, aLabel) );
                }
            }
        } break;

        // ImageButtons
        case starform::FormComponentType::IMAGEBUTTON:
        {
            // Es wird nur der gedrueckte Submit-Button ausgewertet
            // MIB: Sofern ueberhaupt einer uebergeben wurde
            if( rxSubmitButton.is() )
            {
                staruno::Reference<starbeans::XPropertySet>  xSubmitButtonComponent(rxSubmitButton->getModel(), staruno::UNO_QUERY);
                if (xSubmitButtonComponent == xComponentSet)
                {
                    // <name>.x=<pos.X>&<name>.y=<pos.Y>
                    ::rtl::OUString aLhs = aName;
                    ::rtl::OUString aRhs = ::rtl::OUString( MouseEvt.X );

                    // nur wenn ein Name vorhanden ist, kann ein name.x
                    aLhs += aName.getLength() ? UniString::CreateFromAscii(".x") : UniString::CreateFromAscii("x");
                    rList.push_back( HtmlSuccessfulObj(aLhs, aRhs) );

                    aLhs = aName;
                    aRhs = ::rtl::OUString( MouseEvt.Y );
                    aLhs += aName.getLength() ? UniString::CreateFromAscii(".y") : UniString::CreateFromAscii("y");
                    rList.push_back( HtmlSuccessfulObj(aLhs, aRhs) );

                }
            }
        } break;

        // CheckBoxen / RadioButtons
        case starform::FormComponentType::CHECKBOX:
        case starform::FormComponentType::RADIOBUTTON:
        {
            // <name>=<refValue>
            if( !hasProperty(PROPERTY_STATE, xComponentSet) )
                break;
            sal_Int16 nChecked;
            xComponentSet->getPropertyValue( PROPERTY_STATE ) >>= nChecked;
            if( nChecked != 1 )
                break;

            ::rtl::OUString aStrValue;
            if( hasProperty(PROPERTY_REFVALUE, xComponentSet) )
                xComponentSet->getPropertyValue( PROPERTY_REFVALUE ) >>= aStrValue;

            rList.push_back( HtmlSuccessfulObj(aName, aStrValue) );
        } break;

        // Edit
        case starform::FormComponentType::TEXTFIELD:
        {
            // <name>=<text>
            if( !hasProperty(PROPERTY_TEXT, xComponentSet) )
                break;

            // MIB: Spezial-Behandlung fuer Multiline-Edit nur dann, wenn
            // es auch ein Control dazu gibt.
            staruno::Any aTmp = xComponentSet->getPropertyValue( PROPERTY_MULTILINE );
            sal_Bool bMulti =   rxSubmitButton.is()
                            && (aTmp.getValueType().getTypeClass() == staruno::TypeClass_BOOLEAN)
                            && getBOOL(aTmp);
            ::rtl::OUString sText;
            if ( bMulti )   // Bei MultiLineEdit Text am Control abholen
            {

                staruno::Reference<starawt::XControlContainer>  xControlContainer(rxSubmitButton->getContext(), staruno::UNO_QUERY);
                if( !xControlContainer.is() ) break;

                staruno::Sequence<staruno::Reference<starawt::XControl> > aControlSeq = xControlContainer->getControls();
                staruno::Reference<starawt::XControl>  xControl;
                staruno::Reference<starform::XFormComponent>  xControlComponent;

                // Richtiges Control suchen
                sal_Int32 i;
                for( i=0; i<aControlSeq.getLength(); i++ )
                {
                    xControl = aControlSeq.getConstArray()[i];
                    staruno::Reference<starbeans::XPropertySet>  xModel(xControl->getModel(), staruno::UNO_QUERY);
                    if (xModel == xComponentSet)
                    {
                        staruno::Reference<starawt::XTextComponent>  xTextComponent(xControl, staruno::UNO_QUERY);
                        if( xTextComponent.is() )
                            sText = xTextComponent->getText();
                        break;
                    }
                }
                // Control nicht gefunden oder nicht existent, (Edit im Grid)
                if (i == aControlSeq.getLength())
                    xComponentSet->getPropertyValue( PROPERTY_TEXT ) >>= sText;
            }
            else
                xComponentSet->getPropertyValue( PROPERTY_TEXT ) >>= sText;

            rList.push_back( HtmlSuccessfulObj(aName, sText) );
        } break;

        // ComboBox, Patternfield
        case starform::FormComponentType::COMBOBOX:
        case starform::FormComponentType::PATTERNFIELD:
        {
            // <name>=<text>
            if( hasProperty(PROPERTY_TEXT, xComponentSet) )
            {
                ::rtl::OUString aText;
                xComponentSet->getPropertyValue( PROPERTY_TEXT ) >>= aText;
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        } break;
        case starform::FormComponentType::CURRENCYFIELD:
        case starform::FormComponentType::NUMERICFIELD:
        {
            // <name>=<wert> // wert wird als double mit Punkt als Decimaltrenner
                             // kein Wert angegeben (NULL) -> wert leer
            if( hasProperty(PROPERTY_VALUE, xComponentSet) )
            {
                ::rtl::OUString aText;
                staruno::Any aVal  = xComponentSet->getPropertyValue( PROPERTY_VALUE );

                double aDoubleVal;
                if (aVal >>= aDoubleVal)
                {
                    sal_Int16 nScale;
                    xComponentSet->getPropertyValue( PROPERTY_DECIMAL_ACCURACY ) >>= nScale;
                    String aTempText = UniString(aText);
                        // TODO : remove this if SolarMath works with unicode strings
                    SolarMath::DoubleToString(aTempText, aDoubleVal, 'F', nScale, '.', sal_True);
                }
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        }   break;
        case starform::FormComponentType::DATEFIELD:
        {
            // <name>=<wert> // Wert wird als Datum im Format (MM-DD-YYYY)
                             // kein Wert angegeben (NULL) -> wert leer
            if( hasProperty(PROPERTY_DATE, xComponentSet) )
            {
                ::rtl::OUString aText;
                staruno::Any aVal  = xComponentSet->getPropertyValue( PROPERTY_DATE );
                sal_Int32 nInt32Val;
                if (aVal >>= nInt32Val)
                {
                    Date aDate(nInt32Val);
                    char s[11];
                    sprintf(s,"%02d-%02d-%04d",
                    (int)aDate.GetMonth(),
                    (int)aDate.GetDay(),
                    (int)aDate.GetYear());
                    s[10] = 0;
                    aText = ::rtl::OUString::createFromAscii(s);
                }
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        }   break;
        case starform::FormComponentType::TIMEFIELD:
        {
            // <name>=<wert> // Wert wird als Zeit im Format (HH:MM:SS) angegeben
                             // kein Wert angegeben (NULL) -> wert leer
            if( hasProperty(PROPERTY_TIME, xComponentSet) )
            {
                ::rtl::OUString aText;
                staruno::Any aVal  = xComponentSet->getPropertyValue( PROPERTY_TIME );
                sal_Int32 nInt32Val;
                if (aVal >>= nInt32Val)
                {
                    Time aTime(nInt32Val);
                    char s[10];
                    sprintf(s,"%02d:%02d:%02d",
                    (int)aTime.GetHour(),
                    (int)aTime.GetMin(),
                    (int)aTime.GetSec());
                    s[8] = 0;
                    aText = ::rtl::OUString::createFromAscii(s);
                }
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        }   break;

        // starform
        case starform::FormComponentType::HIDDENCONTROL:
        {

            // <name>=<value>
            if( hasProperty(PROPERTY_HIDDEN_VALUE, xComponentSet) )
            {
                ::rtl::OUString aText;
                xComponentSet->getPropertyValue( PROPERTY_HIDDEN_VALUE ) >>= aText;
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        } break;

        // starform
        case starform::FormComponentType::FILECONTROL:
        {
            // <name>=<text>
            if( hasProperty(PROPERTY_TEXT, xComponentSet) )
            {

                ::rtl::OUString aText;
                xComponentSet->getPropertyValue( PROPERTY_TEXT ) >>= aText;
                rList.push_back( HtmlSuccessfulObj(aName, aText, SUCCESSFUL_REPRESENT_FILE) );
            }
        } break;

        // starform
        case starform::FormComponentType::LISTBOX:
        {

            // <name>=<Token0>&<name>=<Token1>&...&<name>=<TokenN> (Mehrfachselektion)
            if (!hasProperty(PROPERTY_SELECT_SEQ, xComponentSet) ||
                !hasProperty(PROPERTY_STRINGITEMLIST, xComponentSet))
                break;

            // angezeigte Werte
            staruno::Sequence< ::rtl::OUString > aVisibleList =
                *(staruno::Sequence< ::rtl::OUString >*)xComponentSet->getPropertyValue( PROPERTY_STRINGITEMLIST ).getValue();
            sal_Int32 nStringCnt = aVisibleList.getLength();
            const ::rtl::OUString* pStrings = aVisibleList.getConstArray();

            // Werte-Liste
            staruno::Sequence< ::rtl::OUString > aValueList =
                *(staruno::Sequence< ::rtl::OUString >*)xComponentSet->getPropertyValue( PROPERTY_VALUE_SEQ ).getValue();
            sal_Int32 nValCnt = aValueList.getLength();
            const ::rtl::OUString* pVals = aValueList.getConstArray();

            // Selektion
            staruno::Sequence<sal_Int16> aSelectList =
                *(staruno::Sequence<sal_Int16>*)xComponentSet->getPropertyValue( PROPERTY_SELECT_SEQ ).getValue();
            sal_Int32 nSelCount = aSelectList.getLength();
            const sal_Int16* pSels = aSelectList.getConstArray();

            // Einfach- oder Mehrfach-Selektion
            // Bei Einfach-Selektionen beruecksichtigt MT nur den ersten Eintrag
            // in der Liste.
            if (nSelCount > 1 && !getBOOL(xComponentSet->getPropertyValue(PROPERTY_MULTISELECTION)))
                nSelCount = 1;

            // Die Indizes in der Selektions-Liste koennen auch ungueltig sein,
            // also muss man die gueltigen erstmal raussuchen um die Laenge
            // der neuen Liste zu bestimmen.
            sal_Int32 nCurCnt = 0;
            sal_Int32 i;
            for( i=0; i<nSelCount; ++i )
            {
                if( pSels[i] < nStringCnt )
                    ++nCurCnt;
            }

            ::rtl::OUString aSubValue;
            for(i=0; i<nCurCnt; ++i )
            {
                sal_Int16  nSelPos = pSels[i];
                // Wenn starsdbcx::Index in WerteListe, Eintrag aus Werteliste holen
                // ansonsten angezeigten Wert nehmen. Ein
                // LISTBOX_EMPTY_VALUE entspricht einem leeren, aber
                // vorhandenem VALUE des Option-Tags.
                if (nSelPos < nValCnt && pVals[nSelPos].getLength())
                {
                    if( pVals[nSelPos] == LISTBOX_EMPTY_VALUE )
                        aSubValue = ::rtl::OUString();
                    else
                        aSubValue = pVals[nSelPos];
                }
                else
                {
                    aSubValue = pStrings[nSelPos];
                }
                rList.push_back( HtmlSuccessfulObj(aName, aSubValue) );
            }
        } break;
        case starform::FormComponentType::GRIDCONTROL:
        {
            // Die einzelnen Spaltenwerte werden verschickt,
            // der Name wird mit dem Prefix des Names des Grids erweitert
            staruno::Reference<starcontainer::XIndexAccess>  xContainer(xComponentSet, staruno::UNO_QUERY);
            if (!xContainer.is())
                break;

            aName += UniString('.');

            sal_Int32 nCount = xContainer->getCount();
            for (sal_Int32 i = 0; i < nCount; ++i)
            {
                staruno::Reference<starbeans::XPropertySet>  xSet(*(staruno::Reference<starbeans::XPropertySet> *)xContainer->getByIndex(i).getValue());
                if (xSet.is())
                    AppendComponent(rList, xSet, aName, rxSubmitButton, MouseEvt);
            }
        }
    }
}

//------------------------------------------------------------------------
void ODatabaseForm::FillSuccessfulList( HtmlSuccessfulObjList& rList,
    const staruno::Reference<starawt::XControl>& rxSubmitButton, const starawt::MouseEvent& MouseEvt )
{

    // Liste loeschen
    rList.clear();


    // Ueber Components iterieren
    InterfaceRef  xIFace;
    staruno::Reference<starform::XFormComponent>    xComponent;
    staruno::Reference<starbeans::XPropertySet>         xComponentSet;
    ::rtl::OUString aPrefix;

    staruno::Any aElement;
    for( sal_Int32 nIndex=0; nIndex < getCount(); nIndex++ )
    {
        aElement = getByIndex( nIndex );
        xComponentSet = staruno::Reference<starbeans::XPropertySet> (*(InterfaceRef *) aElement.getValue(), staruno::UNO_QUERY);
        AppendComponent(rList, xComponentSet, aPrefix, rxSubmitButton, MouseEvt);
    }
}

//------------------------------------------------------------------------
void ODatabaseForm::Encode( ::rtl::OUString& rString ) const
{
    ::rtl::OUString aResult;

    // Immer ANSI #58641
//  rString.Convert(CHARSET_SYSTEM, CHARSET_ANSI);


    // Zeilenendezeichen werden als CR dargestellt
    UniString sConverter = rString;
    sConverter.ConvertLineEnd( LINEEND_CR );
    rString = sConverter;


    // Jeden einzelnen Character ueberpruefen
    sal_uInt16 nStrLen = rString.getLength();
    sal_Unicode nCharCode;
    for( sal_uInt16 nCurPos=0; nCurPos < nStrLen; ++nCurPos )
    {
        nCharCode = rString[nCurPos];

        // Behandlung fuer chars, die kein alphanumerisches Zeichen sind
        // und CharacterCodes > 127
        if( (!isalnum(nCharCode) && nCharCode != (sal_Unicode)' ') || nCharCode > 127 )
        {
            switch( nCharCode )
            {
                case 13:    // CR
                    aResult += ::rtl::OUString::createFromAscii("%0D%0A");  // Hex-Darstellung CR LF
                    break;


                // Netscape Sonderbehandlung
                case 42:    // '*'
                case 45:    // '-'
                case 46:    // '.'
                case 64:    // '@'
                case 95:    // '_'
                    aResult += UniString(nCharCode);
                    break;

                default:
                {
                    // In Hex umrechnen
                    short nHi = ((sal_Int16)nCharCode) / 16;
                    short nLo = ((sal_Int16)nCharCode) - (nHi*16);
                    if( nHi > 9 ) nHi += (int)'A'-10; else nHi += (int)'0';
                    if( nLo > 9 ) nLo += (int)'A'-10; else nLo += (int)'0';
                    aResult += UniString('%');
                    aResult += UniString((sal_Unicode)nHi);
                    aResult += UniString((sal_Unicode)nLo);
                }
            }
        }
        else
            aResult += UniString(nCharCode);
    }


    // Spaces durch '+' ersetzen
    aResult = aResult.replace(' ', '+');

    rString = aResult;
}

//------------------------------------------------------------------------
void ODatabaseForm::InsertTextPart( INetMIMEMessage& rParent, const ::rtl::OUString& rName,
    const ::rtl::OUString& rData )
{

    // Part als Message-Child erzeugen
    INetMIMEMessage* pChild = new INetMIMEMessage();


    // Header
    ::rtl::OUString aContentDisp = ::rtl::OUString::createFromAscii("form-data; name=\"");
    aContentDisp += rName;
    aContentDisp += UniString('\"');
    pChild->SetContentDisposition( aContentDisp );
    pChild->SetContentType( UniString::CreateFromAscii("text/plain") );

    rtl_TextEncoding eSystemEncoding = gsl_getSystemTextEncoding();
    const sal_Char* pBestMatchingEncoding = rtl_getBestMimeCharsetFromTextEncoding( eSystemEncoding );
    UniString aBestMatchingEncoding = UniString::CreateFromAscii( pBestMatchingEncoding );
    pChild->SetContentTransferEncoding(aBestMatchingEncoding);

    // Body
    SvMemoryStream* pStream = new SvMemoryStream;
    pStream->WriteLine( ByteString( UniString(rData), rtl_getTextEncodingFromMimeCharset(pBestMatchingEncoding) ) );
    pStream->Flush();
    pStream->Seek( 0 );
    pChild->SetDocumentLB( new SvLockBytes(pStream, sal_True) );
    rParent.AttachChild( *pChild );
}

//------------------------------------------------------------------------
sal_Bool ODatabaseForm::InsertFilePart( INetMIMEMessage& rParent, const ::rtl::OUString& rName,
    const ::rtl::OUString& rFileName )
{
    UniString aFileName( rFileName );
    UniString aContentType(UniString::CreateFromAscii(CONTENT_TYPE_STR_TEXT_PLAIN));
    SvStream *pStream = 0;

    if( aFileName.Len() )
    {
        // Bisher koennen wir nur File-URLs verarbeiten
        INetURLObject aURL;
        aURL.SetSmartProtocol(INET_PROT_FILE);
        aURL.SetSmartURL(rFileName);
        if( INET_PROT_FILE == aURL.GetProtocol() )
        {
            aFileName = INetURLObject::decode(aURL.PathToFileName(), '%', INetURLObject::DECODE_UNAMBIGUOUS);
            DirEntry aDirEntry( aFileName );
            if( aDirEntry.Exists() )
            {
                pStream = new SvFileStream( aFileName, STREAM_READ );
                if( pStream->GetError() != ERRCODE_NONE )
                {
                    delete pStream;
                    pStream = 0;
                }
            }
            INetContentType eContentType = INetContentTypes::GetContentType4Extension(
                                                                aDirEntry.GetExtension() );
            if (eContentType != CONTENT_TYPE_UNKNOWN)
                aContentType = INetContentTypes::GetContentType(eContentType);
        }
    }

    // Wenn irgendetwas nicht geklappt hat, legen wir einen leeren
    // MemoryStream an
    if( !pStream )
        pStream = new SvMemoryStream;


    // Part als Message-Child erzeugen
    INetMIMEMessage* pChild = new INetMIMEMessage;


    // Header
    ::rtl::OUString aContentDisp = ::rtl::OUString::createFromAscii( "form-data; name=\"" );
    aContentDisp += rName;
    aContentDisp += UniString('\"');
    aContentDisp += ::rtl::OUString::createFromAscii("; filename=\"");
    aContentDisp += aFileName;
    aContentDisp += UniString('\"');
    pChild->SetContentDisposition( aContentDisp );
    pChild->SetContentType( aContentType );
    pChild->SetContentTransferEncoding( UniString(::rtl::OUString::createFromAscii("8bit")) );


    // Body
    pChild->SetDocumentLB( new SvLockBytes(pStream, sal_True) );
    rParent.AttachChild( *pChild );

    return sal_True;
}

//==============================================================================
// internals
//------------------------------------------------------------------------------
void ODatabaseForm::onError(const starsdb::SQLErrorEvent& _rEvent)
{
    NOTIFY_LISTENERS(m_aErrorListeners, starsdb::XSQLErrorListener, errorOccured, _rEvent);
}

//------------------------------------------------------------------------------
void ODatabaseForm::onError(starsdbc::SQLException& _rException, const ::rtl::OUString& _rContextDescription)
{
    if (!m_aErrorListeners.getLength())
        return;

    starsdb::SQLContext aError = prependContextInfo(_rException, static_cast<staruno::XWeak*>(this), _rContextDescription);
    starsdb::SQLErrorEvent aEvent(static_cast<staruno::XWeak*>(this), staruno::makeAny(aError));

    onError(aEvent);
}

//------------------------------------------------------------------------------
OParameterInfoImpl* ODatabaseForm::createParameterInfo() const
{
    OParameterInfoImpl* pParameterInfo = new OParameterInfoImpl();

    // create and fill a composer
    staruno::Reference<starsdb::XSQLQueryComposer>  xComposer = getCurrentSettingsComposer(m_xAggregateSet, m_xServiceFactory);
    staruno::Reference<starsdb::XParametersSupplier>  xSetParameters = staruno::Reference<starsdb::XParametersSupplier> (xComposer, staruno::UNO_QUERY);

    // if there is no parsable statement return
    if (!xSetParameters.is())
        return pParameterInfo;

    staruno::Reference<starcontainer::XIndexAccess>  xParamsAsIndicies = xSetParameters->getParameters();
    staruno::Reference<starcontainer::XNameAccess>   xParamsAsNames(xParamsAsIndicies, staruno::UNO_QUERY);
    sal_Int32 nParamCount = xParamsAsIndicies.is() ? xParamsAsIndicies->getCount() : 0;
    // without parameters return
    if (!xParamsAsNames.is() || (nParamCount == 0))
        return pParameterInfo;

    // now evaluate the parameters
    pParameterInfo->nCount = nParamCount;
    pParameterInfo->xParamsAsNames = xParamsAsNames;
    pParameterInfo->pParameters = new OParametersImpl();
    pParameterInfo->pParameters->acquire();
    OParametersImpl::Parameters& rParams = pParameterInfo->pParameters->getParameters();

    // we need to map the parameter names (which is all we can get from our parent) to indicies (which are
    // needed by the starsdbc::XParameters interface of the row set)
    MapUString2INT32& rParamMapping = pParameterInfo->aParamMapping;
    staruno::Reference<starbeans::XPropertySet> xParam;
    for (sal_Int32 i = 0; i<nParamCount; ++i)
    {
        ::cppu::extractInterface(xParam, xParamsAsIndicies->getByIndex(i));

        // remember the param fields
        rParams.push_back(xParam);
        ::rtl::OUString sName;
        xParam->getPropertyValue(PROPERTY_NAME) >>= sName;
        rParamMapping[sName] = i;
    }

    // check for a matching of my param master fields with the parent's columns
    staruno::Reference<starsdbcx::XColumnsSupplier>  xParentColsSuppl(m_xParent, staruno::UNO_QUERY);
    if (xParentColsSuppl.is())
    {
        staruno::Reference<starcontainer::XNameAccess>  xParentCols = xParentColsSuppl->getColumns();
        if (xParentCols.is())
        {
            sal_Int32 nMasterLen = m_aMasterFields.getLength();
            sal_Int32 nSlaveLen = m_aDetailFields.getLength();
            if (xParentCols->hasElements() && (nMasterLen == nSlaveLen) && (nMasterLen > 0))
            {
                const ::rtl::OUString* pMasterFields = m_aMasterFields.getConstArray();
                const ::rtl::OUString* pDetailFields = m_aDetailFields.getConstArray();

                OParametersImpl::ParametersIterator iter;
                for (sal_Int32 i = 0; i < nMasterLen; i++)
                {
                    staruno::Reference<starbeans::XPropertySet>  xMasterField, xDetailField;

                    if (xParentCols->hasByName(pMasterFields[i]) &&
                        xParamsAsNames->hasByName(pDetailFields[i]))
                    {
                        // parameter defined by master slave definition
                        ::cppu::extractInterface(xDetailField, xParamsAsNames->getByName(pDetailFields[i]));

                        DBG_ASSERT(rParamMapping.find(pDetailFields[i]) != rParamMapping.end(), "ODatabaseForm::fillParameters : invalid starsdb::XParametersSupplier !");
                            // the mapping was build from the starsdb::XParametersSupplier interface of the composer, and the
                            // starcontainer::XNameAccess interface of the composer said hasByName(...)==sal_True ... so what ?

                        // delete the wrapper as the parameter is set
                        iter = find(rParams.begin(), rParams.end(), xDetailField);
                        DBG_ASSERT(iter != rParams.end(), "ODatabaseForm::fillParameters : Parameter not found");
                        if (iter != rParams.end())
                            rParams.erase(iter);
                    }
                }
            }
        }
    }

    // now set the remaining params
    sal_Int32 nParamsLeft = rParams.size();
    if (nParamsLeft)
    {
        staruno::Reference<starsdbc::XParameters>  xExecutionParams(m_xAggregate, staruno::UNO_QUERY);
        ::rtl::OUString sName;
        staruno::Reference<starbeans::XPropertySet>  xParam;
        staruno::Reference<starbeans::XPropertySet>  xWrapper;
        for (sal_Int32 j = nParamsLeft; j; )
        {
            --j;
            xParam = rParams[j];
            xParam->getPropertyValue(PROPERTY_NAME) >>= sName;
            // need a wrapper for this .... the params supplied by xMyParamsAsIndicies don't have a "Value"
            // property, but the parameter listeners expect such a property. So we need an object "aggregating"
            // xParam and supplying an additional property ("Value")
            // (it's no real aggregation of course ...)

            // get the position of the parameter
            sal_Int32 nPos = rParamMapping[sName];
            if (m_aParameterVisited.size() > nPos && m_aParameterVisited[nPos] == true)
            {
                // parameter already set from outside
                OParametersImpl::ParametersIterator aPos = rParams.begin() + j;
                if (aPos != rParams.end())
                    rParams.erase(aPos);
            }
            else
            {
                xWrapper = new OParameterWrapper(xParam, xExecutionParams, rParamMapping[sName]);
                rParams[j] = xWrapper;
            }
        }
    }
    return pParameterInfo;
}

//------------------------------------------------------------------------------
bool ODatabaseForm::hasValidParent() const
{
    // do we have to fill the parameters again?
    if (m_bSubForm)
    {
        staruno::Reference<starsdbc::XResultSet>  xResultSet(m_xParent, staruno::UNO_QUERY);
        if (!xResultSet.is())
        {
            DBG_ERROR("ODatabaseForm::hasValidParent() : no parent resultset !");
            return false;
        }
        try
        {
            staruno::Reference<starbeans::XPropertySet>  xSet(m_xParent, staruno::UNO_QUERY);
            // only if the parent has a command we have to check if the parent is positioned on a valid row
            if (getString(xSet->getPropertyValue(PROPERTY_COMMAND)).getLength() &&
                (xResultSet->isBeforeFirst() || xResultSet->isAfterLast() ||
                 getBOOL(xSet->getPropertyValue(PROPERTY_ISNEW))))
                return false;
        }
        catch(...)
        {
            // parent could be forwardonly?
            return false;
        }
    }
    return true;
}

//------------------------------------------------------------------------------
bool ODatabaseForm::fillParameters(ReusableMutexGuard& _rClearForNotifies)
{
    staruno::Reference<starsdbc::XParameters>  xExecutionParams;
    if (!query_aggregation( m_xAggregate, xExecutionParams))
    {
        DBG_ERROR("ODatabaseForm::fillParameters : invalid row set (doesn't support parameters) !");
        // normally the row set should support parameters ...
        return true;
    }

    // is there a valid parent?
    if (m_bSubForm && !hasValidParent())
        return true;

    // do we have to fill the parameters again?
    if (!m_pParameterInfo)
        m_pParameterInfo = createParameterInfo();

    if (!m_pParameterInfo || m_pParameterInfo->nCount == 0)
        return true;

    // do we have to fill the parent parameters?
    OParametersImpl::Parameters& rParams = m_pParameterInfo->pParameters->getParameters();

    // do we have to fill the parent parameters?
    if (m_pParameterInfo->nCount > rParams.size())
    {
        staruno::Reference<starsdbcx::XColumnsSupplier>  xParentColsSuppl(m_xParent, staruno::UNO_QUERY);
        if (xParentColsSuppl.is())
        {
            staruno::Reference<starcontainer::XNameAccess>  xParentCols = xParentColsSuppl->getColumns();
            sal_Int32 nMasterLen = m_aMasterFields.getLength();
            if (xParentCols->hasElements() && (nMasterLen > 0))
            {
                const ::rtl::OUString* pMasterFields = m_aMasterFields.getConstArray();
                const ::rtl::OUString* pDetailFields = m_aDetailFields.getConstArray();

                staruno::Any aParamType,aScale,aValue;
                for (sal_Int32 i = 0; i < nMasterLen; i++)
                {
                    staruno::Reference<starbeans::XPropertySet>  xMasterField, xDetailField;
                    if (xParentCols->hasByName(pMasterFields[i]) &&
                        m_pParameterInfo->xParamsAsNames->hasByName(pDetailFields[i]))
                    {
                        // parameter defined by master slave definition
                        ::cppu::extractInterface(xMasterField, xParentCols->getByName(pMasterFields[i]));
                        ::cppu::extractInterface(xDetailField, m_pParameterInfo->xParamsAsNames->getByName(pDetailFields[i]));

                        // get the type of the param
                        aParamType = xDetailField->getPropertyValue(PROPERTY_FIELDTYPE);
                        DBG_ASSERT(aParamType.getValueType().getTypeClass() == staruno::TypeClass_LONG, "ODatabaseForm::fillParameters : invalid parameter field !");
                        sal_Int32 nScale = 0;
                        if (hasProperty(PROPERTY_SCALE, xDetailField))
                        {
                            aScale = xDetailField->getPropertyValue(PROPERTY_SCALE);
                            DBG_ASSERT(aScale.getValueType().getTypeClass() == staruno::TypeClass_LONG, "ODatabaseForm::fillParameters : invalid parameter field !");
                            nScale = getINT32(aScale);
                        }
                        // and fill the param value
                        aValue = xMasterField->getPropertyValue(PROPERTY_VALUE);
                        // parameters are based at 1
                        xExecutionParams->setObjectWithInfo(m_pParameterInfo->aParamMapping[pDetailFields[i]] + 1, aValue, getINT32(aParamType), nScale);
                    }
                    else
                        // no column matching so leave the parameter setting
                        return true;
                }
            }
        }
    }

    // now fill the remaining params
    bool bCanceled = false;
    sal_Int32 nParamsLeft = rParams.size();
    if (nParamsLeft)
    {
        ::cppu::OInterfaceIteratorHelper aIter(m_aParameterListeners);
        starform::DatabaseParameterEvent aEvt(static_cast<staruno::XWeak*>(this), m_pParameterInfo->pParameters);

        _rClearForNotifies.clear();
        while (aIter.hasMoreElements() && !bCanceled)
            bCanceled = !((starform::XDatabaseParameterListener*)aIter.next())->approveParameter(aEvt);
        _rClearForNotifies.attach(m_aMutex);
    }
    return !bCanceled;
}

//------------------------------------------------------------------------------
void ODatabaseForm::executeRowSet(ReusableMutexGuard& _rClearForNotifies, sal_Bool bMoveToFirst)
{
    if (!m_xAggregateAsRowSet.is())
        return;

    fillParameters(_rClearForNotifies);
    sal_Bool bInsertOnly = sal_False;

    // ensure the aggregated row set has the correct properties
    sal_Int32 nConcurrency;
    // if we have a parent, who is not positioned on a valid row
    // we can't be updatable!
    if (m_bSubForm && !hasValidParent())
    {
        // don't use any parameters if we don't have a valid parent
        nConcurrency = starsdbc::ResultSetConcurrency::READ_ONLY;
        clearParameters();
    }
    else if (m_bAllowInsert || m_bAllowUpdate || m_bAllowDelete)
        nConcurrency = starsdbc::ResultSetConcurrency::UPDATABLE;
    else
        nConcurrency = starsdbc::ResultSetConcurrency::READ_ONLY;
    m_xAggregateSet->setPropertyValue(PROPERTY_RESULTSET_CONCURRENCY, staruno::makeAny(nConcurrency));

    sal_Int32 nResultSetType = starsdbc::ResultSetType::SCROLL_SENSITIVE;
    m_xAggregateSet->setPropertyValue(PROPERTY_RESULTSET_TYPE, staruno::makeAny(nResultSetType));

    sal_Bool bSuccess = sal_False;
    try
    {
        m_xAggregateAsRowSet->execute();
        bSuccess = sal_True;
    }
    catch(starsdb::RowSetVetoException& eVeto)
    {
        eVeto;
    }
    catch(starsdbc::SQLException& eDb)
    {
        _rClearForNotifies.clear();
        if (m_sCurrentErrorContext.getLength())
            onError(eDb, m_sCurrentErrorContext);
        else
            onError(eDb, FRM_RES_STRING(RID_STR_READERROR));
        _rClearForNotifies.attach(m_aMutex);
    }

    if (bSuccess)
    {
        // adjust the privilege property
        m_nPrivileges;
        m_xAggregateSet->getPropertyValue(PROPERTY_PRIVILEGES) >>= m_nPrivileges;
        if (!m_bAllowInsert)
            m_nPrivileges &= ~starsdbcx::Privilege::INSERT;
        if (!m_bAllowUpdate)
            m_nPrivileges &= ~starsdbcx::Privilege::UPDATE;
        if (!m_bAllowDelete)
            m_nPrivileges &= ~starsdbcx::Privilege::DELETE;

        if (bMoveToFirst)
        {
            // the row set is positioned _before_ the first row (per definitionem), so move the set ...
            try
            {
                // if we have an insert only rowset we move to the insert row
                next();
                if (((m_nPrivileges & starsdbcx::Privilege::INSERT) == starsdbcx::Privilege::INSERT)
                    && isAfterLast())
                {
                    // move on the insert row of set
                    // resetting must be done later, after the load events have been posted
                    // see :moveToInsertRow and load , reload
                    staruno::Reference<starsdbc::XResultSetUpdate>  xUpdate;
                    if (query_aggregation( m_xAggregate, xUpdate))
                        xUpdate->moveToInsertRow();
                }
            }
            catch(starsdbc::SQLException& eDB)
            {
                _rClearForNotifies.clear();
                if (m_sCurrentErrorContext.getLength())
                    onError(eDB, m_sCurrentErrorContext);
                else
                    onError(eDB, FRM_RES_STRING(RID_STR_READERROR));
                _rClearForNotifies.attach(m_aMutex);
            }
        }
    }
}

//------------------------------------------------------------------
void ODatabaseForm::disposing()
{
    if (m_bLoaded)
        unload();

    // cancel the submit/reset-thread
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if (m_pThread)
        {
            m_pThread->release();
            m_pThread = NULL;
        }
    }

    starlang::EventObject aEvt(static_cast<staruno::XWeak*>(this));
    m_aLoadListeners.disposeAndClear(aEvt);
    m_aRowSetApproveListeners.disposeAndClear(aEvt);
    m_aParameterListeners.disposeAndClear(aEvt);
    m_aResetListeners.disposeAndClear(aEvt);
    m_aSubmitListeners.disposeAndClear(aEvt);
    m_aErrorListeners.disposeAndClear(aEvt);

    OFormComponents::disposing();
    OPropertySetAggregationHelper::disposing();

    // stop listening on the aggregate
    m_xAggregateAsRowSet->removeRowSetListener(this);

    staruno::Reference<starlang::XComponent>  xAggregationComponent;
    if (query_aggregation(m_xAggregate, xAggregationComponent))
        xAggregationComponent->dispose();
}

//------------------------------------------------------------------------------
staruno::Reference< starsdbc::XConnection >  ODatabaseForm::getConnection()
{
    staruno::Reference< starsdbc::XConnection >  xReturn;
    try
    {
        ::cppu::extractInterface(xReturn, getPropertyValue(PROPERTY_ACTIVE_CONNECTION));
    }
    catch(...)
    {
    }

    return xReturn;
}

//==============================================================================
// property handling
//------------------------------------------------------------------------------
void ODatabaseForm::fillProperties(
        staruno::Sequence< starbeans::Property >& _rProps,
        staruno::Sequence< starbeans::Property >& _rAggregateProps ) const
{
    BEGIN_AGGREGATION_PROPERTY_HELPER(14, m_xAggregateSet)
        // this property is overwritten by the form
        RemoveProperty(_rAggregateProps, PROPERTY_PRIVILEGES);
        ModifyPropertyAttributes(_rAggregateProps, PROPERTY_DATASOURCE, starbeans::PropertyAttribute::CONSTRAINED, 0);

        DECL_PROP1(NAME,            ::rtl::OUString,                BOUND);
        DECL_PROP1(MASTERFIELDS,    StringSequence,                 BOUND);
        DECL_PROP1(DETAILFIELDS,    StringSequence,                 BOUND);
        DECL_PROP3(CYCLE,           starform::TabulatorCycle,       BOUND, MAYBEVOID, MAYBEDEFAULT);
        DECL_PROP1(NAVIGATION,      starform::NavigationBarMode,    BOUND);
        DECL_BOOL_PROP1(ALLOWADDITIONS,                             BOUND);
        DECL_BOOL_PROP1(ALLOWEDITS,                                 BOUND);
        DECL_BOOL_PROP1(ALLOWDELETIONS,                             BOUND);
        DECL_BOOL_PROP2(THREADSAFE,                                 TRANSIENT, READONLY);
        DECL_PROP2(PRIVILEGES,      sal_Int32,                      TRANSIENT, READONLY);
        DECL_PROP1(TARGET_URL,      ::rtl::OUString,                BOUND);
        DECL_PROP1(TARGET_FRAME,    ::rtl::OUString,                BOUND);
        DECL_PROP1(SUBMIT_METHOD,   starform::FormSubmitMethod,     BOUND);
        DECL_PROP1(SUBMIT_ENCODING, starform::FormSubmitEncoding,   BOUND);
    END_AGGREGATION_PROPERTY_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ODatabaseForm::getInfoHelper()
{
    return *const_cast<ODatabaseForm*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
staruno::Reference<starbeans::XPropertySetInfo>  ODatabaseForm::getPropertySetInfo() throw( staruno::RuntimeException )
{
    staruno::Reference<starbeans::XPropertySetInfo>  xDescriptorInfo( createPropertySetInfo( getInfoHelper() ) );
    return xDescriptorInfo;
}

//------------------------------------------------------------------------------
void ODatabaseForm::fire( sal_Int32* pnHandles, const staruno::Any* pNewValues, const staruno::Any* pOldValues, sal_Int32 nCount, sal_Bool bVetoable )
{
    // same as in getFastPropertyValue(INT32) : if we're resetting currently don't fire any changes of the
    // IsModified property from FALSE to TRUE, as this is only temporary 'til the reset is done
    if (m_nResetsPending > 0)
    {
        // look for the PROPERTY_ID_ISMODIFIED
        sal_Int32 nPos = 0;
        for (nPos=0; nPos<nCount; ++nPos)
            if (pnHandles[nPos] == PROPERTY_ID_ISMODIFIED)
                break;

        if ((nPos < nCount) && (pNewValues[nPos].getValueType().getTypeClass() == staruno::TypeClass_BOOLEAN) && getBOOL(pNewValues[nPos]))
        {   // yeah, we found it, and it changed to TRUE
            if (nPos == 0)
            {   // just cut the first element
                ++pnHandles;
                ++pNewValues;
                ++pOldValues;
                --nCount;
            }
            else if (nPos == nCount - 1)
                // just cut the last element
                --nCount;
            else
            {   // split into two base class calls
                OPropertySetAggregationHelper::fire(pnHandles, pNewValues, pOldValues, nPos, bVetoable);
                ++nPos;
                OPropertySetAggregationHelper::fire(pnHandles + nPos, pNewValues + nPos, pOldValues + nPos, nCount - nPos, bVetoable);
                return;
            }
        }
    }

    OPropertySetAggregationHelper::fire(pnHandles, pNewValues, pOldValues, nCount, bVetoable);
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL ODatabaseForm::getFastPropertyValue( sal_Int32 nHandle )
       throw(starbeans::UnknownPropertyException, starlang::WrappedTargetException, staruno::RuntimeException)
{
    if ((nHandle == PROPERTY_ID_ISMODIFIED) && (m_nResetsPending > 0))
        return staruno::makeAny(sal_Bool(sal_False));
        // don't allow the aggregate which is currently reset to return a (temporary) "yes"
    else
        return OPropertySetAggregationHelper::getFastPropertyValue(nHandle);
}

//------------------------------------------------------------------------------
void ODatabaseForm::getFastPropertyValue( staruno::Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_THREADSAFE:
        {
            try
            {
                staruno::Reference< starsdbc::XConnection >  xActiveConn =
                    calcConnection(staruno::Reference<starsdbc::XRowSet>(m_xAggregate, staruno::UNO_QUERY), m_xServiceFactory);
                // if we're alive, our aggregate has an active connection which will be returned
                // if we aren't and our aggregate has no active connection, the function will calculate one
                // from the current settings and forward it to the row set
                staruno::Reference<starbeans::XPropertySet>  xConnProps(xActiveConn, staruno::UNO_QUERY);
                if (xConnProps.is() && hasProperty(PROPERTY_THREADSAFE, xConnProps))
                    rValue = xConnProps->getPropertyValue(PROPERTY_THREADSAFE);
                else
                {
                    staruno::Reference<starcontainer::XChild>  xConnAsChild(xActiveConn, staruno::UNO_QUERY);
                    if (xConnAsChild.is())
                        xConnProps = staruno::Reference<starbeans::XPropertySet> (xConnAsChild->getParent(), staruno::UNO_QUERY);
                    if (xConnProps.is() && hasProperty(PROPERTY_THREADSAFE, xConnProps))
                        rValue = xConnProps->getPropertyValue(PROPERTY_THREADSAFE);
                    else
                        rValue <<= (sal_Bool)sal_False;
                }
            }
            catch(...)
            {
                rValue <<= (sal_Bool)sal_False;
            }
        }
        break;
        case PROPERTY_ID_TARGET_URL:
            rValue <<= m_aTargetURL;
            break;
        case PROPERTY_ID_TARGET_FRAME:
            rValue <<= m_aTargetFrame;
            break;
        case PROPERTY_ID_SUBMIT_METHOD:
            rValue <<= m_eSubmitMethod;
            break;
        case PROPERTY_ID_SUBMIT_ENCODING:
            rValue <<= m_eSubmitEncoding;
            break;
        case PROPERTY_ID_NAME:
            rValue <<= m_sName;
            break;
        case PROPERTY_ID_MASTERFIELDS:
            rValue <<= m_aMasterFields;
            break;
        case PROPERTY_ID_DETAILFIELDS:
            rValue <<= m_aDetailFields;
            break;
        case PROPERTY_ID_CYCLE:
            rValue = m_aCycle;
            break;
        case PROPERTY_ID_NAVIGATION:
            rValue <<= m_eNavigation;
            break;
        case PROPERTY_ID_ALLOWADDITIONS:
            rValue <<= (sal_Bool)m_bAllowInsert;
            break;
        case PROPERTY_ID_ALLOWEDITS:
            rValue <<= (sal_Bool)m_bAllowUpdate;
            break;
        case PROPERTY_ID_ALLOWDELETIONS:
            rValue <<= (sal_Bool)m_bAllowDelete;
            break;
        case PROPERTY_ID_PRIVILEGES:
            rValue <<= (sal_Int32)m_nPrivileges;
            break;
    }
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseForm::convertFastPropertyValue( staruno::Any& rConvertedValue, staruno::Any& rOldValue,
                                                sal_Int32 nHandle, const staruno::Any& rValue ) throw( starlang::IllegalArgumentException )
{
    sal_Bool bModified(sal_False);
    switch (nHandle)
    {
        case PROPERTY_ID_TARGET_URL:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aTargetURL);
            break;
        case PROPERTY_ID_TARGET_FRAME:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aTargetFrame);
            break;
        case PROPERTY_ID_SUBMIT_METHOD:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_eSubmitMethod);
            break;
        case PROPERTY_ID_SUBMIT_ENCODING:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_eSubmitEncoding);
            break;
        case PROPERTY_ID_NAME:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sName);
            break;
        case PROPERTY_ID_MASTERFIELDS:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aMasterFields);
            break;
        case PROPERTY_ID_DETAILFIELDS:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aDetailFields);
            break;
        case PROPERTY_ID_CYCLE:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aCycle, ::getCppuType(static_cast<const starform::TabulatorCycle*>(NULL)));
            break;
        case PROPERTY_ID_NAVIGATION:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_eNavigation);
            break;
        case PROPERTY_ID_ALLOWADDITIONS:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAllowInsert);
            break;
        case PROPERTY_ID_ALLOWEDITS:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAllowUpdate);
            break;
        case PROPERTY_ID_ALLOWDELETIONS:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAllowDelete);
            break;
        default:
            DBG_ERROR("ODatabaseForm::convertFastPropertyValue : unknown property !");
    }
    return bModified;
}

//------------------------------------------------------------------------------
void ODatabaseForm::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const staruno::Any& rValue )
{
    switch (nHandle)
    {
        case PROPERTY_ID_TARGET_URL:
            rValue >>= m_aTargetURL;
            break;
        case PROPERTY_ID_TARGET_FRAME:
            rValue >>= m_aTargetFrame;
            break;
        case PROPERTY_ID_SUBMIT_METHOD:
            rValue >>= m_eSubmitMethod;
            break;
        case PROPERTY_ID_SUBMIT_ENCODING:
            rValue >>= m_eSubmitEncoding;
            break;
        case PROPERTY_ID_NAME:
            rValue >>= m_sName;
            break;
        case PROPERTY_ID_MASTERFIELDS:
            rValue >>= m_aMasterFields;
            break;
        case PROPERTY_ID_DETAILFIELDS:
            rValue >>= m_aDetailFields;
            break;
        case PROPERTY_ID_CYCLE:
            m_aCycle = rValue;
            break;
        case PROPERTY_ID_NAVIGATION:
            rValue >>= m_eNavigation;
            break;
        case PROPERTY_ID_ALLOWADDITIONS:
            m_bAllowInsert = getBOOL(rValue);
            break;
        case PROPERTY_ID_ALLOWEDITS:
            m_bAllowUpdate = getBOOL(rValue);
            break;
        case PROPERTY_ID_ALLOWDELETIONS:
            m_bAllowDelete = getBOOL(rValue);
            break;
        default:
            DBG_ERROR("ODatabaseForm::setFastPropertyValue_NoBroadcast : unknown property !");
    }
}

//==============================================================================
// com::sun::star::beans::XPropertyState
//------------------------------------------------------------------
starbeans::PropertyState ODatabaseForm::getPropertyStateByHandle(sal_Int32 nHandle)
{
    starbeans::PropertyState eState;
    switch (nHandle)
    {
        case PROPERTY_ID_CYCLE:
            if (!m_aCycle.hasValue())
                eState = starbeans::PropertyState_DEFAULT_VALUE;
            else
                eState = starbeans::PropertyState_DIRECT_VALUE;
            break;
        default:
            eState = OPropertySetAggregationHelper::getPropertyStateByHandle(nHandle);
    }
    return eState;
}

//------------------------------------------------------------------
void ODatabaseForm::setPropertyToDefaultByHandle(sal_Int32 nHandle)
{
    switch (nHandle)
    {
        case PROPERTY_ID_CYCLE:
            setFastPropertyValue(nHandle, staruno::Any());
            break;
        default:
            OPropertySetAggregationHelper::setPropertyToDefaultByHandle(nHandle);
    }
}

//------------------------------------------------------------------
staruno::Any ODatabaseForm::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_CYCLE:
            return staruno::Any();
        default:
            return OPropertySetAggregationHelper::getPropertyDefaultByHandle(nHandle);
    }
}

//==============================================================================
// com::sun::star::form::XReset
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::reset() throw( staruno::RuntimeException )
{
    ReusableMutexGuard aGuard(m_aMutex);

    if (isLoaded())
    {
        ::osl::MutexGuard aResetGuard(m_aResetSafety);
        ++m_nResetsPending;
        reset_impl(true);
        return;
    }

    if (m_aResetListeners.getLength())
    {
        ::osl::MutexGuard aResetGuard(m_aResetSafety);
        ++m_nResetsPending;
        // create an own thread if we have (approve-)reset-listeners (so the listeners can't do that much damage
        // to this thread which is probably the main one)
        if (!m_pThread)
        {
            m_pThread = new OFormSubmitResetThread(this);
            m_pThread->acquire();
            m_pThread->create();
        }
        starlang::EventObject aEvt;
        m_pThread->addEvent(&aEvt, sal_False);
    }
    else
    {
        // direct call without any approving by the listeners
        aGuard.clear();

        ::osl::MutexGuard aResetGuard(m_aResetSafety);
        ++m_nResetsPending;
        reset_impl(false);
    }
}

//-----------------------------------------------------------------------------
void ODatabaseForm::reset_impl(bool _bAproveByListeners)
{
    if (_bAproveByListeners)
    {
        bool bCanceled = false;
        ::cppu::OInterfaceIteratorHelper aIter(m_aResetListeners);
        starlang::EventObject aEvt(static_cast<staruno::XWeak*>(this));

        while (aIter.hasMoreElements() && !bCanceled)
            if (!((starform::XResetListener*)aIter.next())->approveReset(aEvt))
                bCanceled = true;
        if (bCanceled)
            return;
    }

    ReusableMutexGuard aResetGuard(m_aResetSafety);
    // do we have a database connected form and stay on the insert row
    sal_Bool bInsertRow = sal_False;
    if (m_xAggregateSet.is())
        bInsertRow = getBOOL(m_xAggregateSet->getPropertyValue(PROPERTY_ISNEW));
    if (bInsertRow && m_bSubForm)
    {
        // Iterate through all columns and set the default value
        staruno::Reference<starsdbcx::XColumnsSupplier>  xColsSuppl(m_xAggregateSet, staruno::UNO_QUERY);

        // now set the values on which a subform depends
        staruno::Reference<starsdbcx::XColumnsSupplier>  xParentColsSuppl(m_xParent, staruno::UNO_QUERY);
        staruno::Reference<starcontainer::XNameAccess>  xParentCols = xParentColsSuppl->getColumns();
        sal_Int32 nMasterLen = m_aMasterFields.getLength();
        if (xParentCols->hasElements() && (nMasterLen > 0))
        {
            staruno::Reference<starcontainer::XNameAccess>  xCols(xColsSuppl->getColumns(), staruno::UNO_QUERY);
            const ::rtl::OUString* pMasterFields = m_aMasterFields.getConstArray();
            const ::rtl::OUString* pDetailFields = m_aDetailFields.getConstArray();

            for (sal_Int32 i = 0; i < nMasterLen; ++i)
            {
                staruno::Reference<starbeans::XPropertySet>  xMasterField, xField;
                if (xParentCols->hasByName(pMasterFields[i]) &&
                    xCols->hasByName(pDetailFields[i]))
                {
                    ::cppu::extractInterface(xMasterField, xParentCols->getByName(pMasterFields[i]));
                    ::cppu::extractInterface(xField, xCols->getByName(pDetailFields[i]));
                    xField->setPropertyValue(PROPERTY_VALUE, xMasterField->getPropertyValue(PROPERTY_VALUE));
                }
            }
        }
    }

    aResetGuard.clear();
    // iterate through all components. don't use an starcontainer::XIndexAccess as this will cause massive
    // problems with the count.
    staruno::Reference<starcontainer::XEnumeration>  xIter = createEnumeration();
    while (xIter->hasMoreElements())
    {
        staruno::Any aElement(xIter->nextElement());
        staruno::Reference<starform::XReset> xReset;
        ::cppu::extractInterface(xReset, aElement);
        if (xReset.is())
        {
            // TODO : all reset-methods have to be thread-safe
            xReset->reset();
        }
    }

    aResetGuard.attach(m_aResetSafety);
    // ensure that the row isn't modified
    // (do this _before_ the listeners are notified ! their reaction (maybe asynchronous) may depend
    // on the modified state of the row
    // 21.02.00 - 73265 - FS)
    if (bInsertRow)
        m_xAggregateSet->setPropertyValue(PROPERTY_ISMODIFIED, staruno::makeAny(sal_Bool(sal_False)));

    aResetGuard.clear();
    {
        starlang::EventObject aEvt(static_cast<staruno::XWeak*>(this));
        NOTIFY_LISTENERS(m_aResetListeners, starform::XResetListener, resetted, aEvt);
    }

    aResetGuard.attach(m_aResetSafety);
    // and again : ensure the row isn't modified
    // we already did this after we (and maybe our dependents) resetted the values, but the listeners may have changed the row, too
    if (bInsertRow)
        m_xAggregateSet->setPropertyValue(PROPERTY_ISMODIFIED, staruno::makeAny(sal_Bool(sal_False)));

    --m_nResetsPending;
}

//-----------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addResetListener(const staruno::Reference<starform::XResetListener>& _rListener) throw( staruno::RuntimeException )
{
    m_aResetListeners.addInterface(_rListener);
}

//-----------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeResetListener(const staruno::Reference<starform::XResetListener>& _rListener) throw( staruno::RuntimeException )
{
    m_aResetListeners.removeInterface(_rListener);
}

//==============================================================================
// com::sun::star::form::XSubmit
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::submit( const staruno::Reference<starawt::XControl>& Control,
                              const starawt::MouseEvent& MouseEvt ) throw( staruno::RuntimeException )
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        // Sind Controls und eine Submit-starutil::URL vorhanden?
        if( !getCount() || !m_aTargetURL.getLength() )
            return;
    }

    ::osl::ClearableMutexGuard aGuard(m_aMutex);
    if (m_aSubmitListeners.getLength())
    {
        // create an own thread if we have (approve-)submit-listeners (so the listeners can't do that much damage
        // to this thread which is probably the main one)
        if (!m_pThread)
        {
            m_pThread = new OFormSubmitResetThread(this);
            m_pThread->acquire();
            m_pThread->create();
        }
        m_pThread->addEvent(&MouseEvt, Control, sal_True);
    }
    else
    {
        // direct call without any approving by the listeners
        aGuard.clear();
        submit_impl( Control, MouseEvt, true );
    }
}

//------------------------------------------------------------------------------
void ODatabaseForm::submit_impl(const staruno::Reference<starawt::XControl>& Control, const starawt::MouseEvent& MouseEvt, bool _bAproveByListeners)
{

    if (_bAproveByListeners)
    {
        ::cppu::OInterfaceIteratorHelper aIter(m_aSubmitListeners);
        starlang::EventObject aEvt(static_cast<staruno::XWeak*>(this));
        sal_Bool bCanceled = sal_False;
        while (aIter.hasMoreElements() && !bCanceled)
        {
            if (!((starform::XSubmitListener*)aIter.next())->approveSubmit(aEvt))
                bCanceled = sal_True;
        }

        if (bCanceled)
            return;
    }

    starform::FormSubmitEncoding eSubmitEncoding;
    starform::FormSubmitMethod eSubmitMethod;
    ::rtl::OUString aURLStr;
    ::rtl::OUString aReferer;
    ::rtl::OUString aTargetName;
    staruno::Reference< starframe::XModel >  xModel;
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        // starform->Forms

        staruno::Reference<starcontainer::XChild>  xParent(m_xParent, staruno::UNO_QUERY);

        if (xParent.is())
            xModel = getXModel(xParent->getParent());

        if (xModel.is())
            aReferer = xModel->getURL();

        // TargetItem
        aTargetName = m_aTargetFrame;

        eSubmitEncoding = m_eSubmitEncoding;
        eSubmitMethod = m_eSubmitMethod;
        aURLStr = m_aTargetURL;
    }

    if (!xModel.is())
        return;

    staruno::Reference<starutil::XURLTransformer>
        xTransformer(m_xServiceFactory->createInstance(
            ::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer")), staruno::UNO_QUERY);
    DBG_ASSERT(xTransformer.is(), "ODatabaseForm::submit_impl : could not create an URL transformer !");

    // starutil::URL-Encoding
    if( eSubmitEncoding == starform::FormSubmitEncoding_URL )
    {
        ::rtl::OUString aData;
        {
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            aData = GetDataURLEncoded( Control, MouseEvt );
        }

        staruno::Reference< starframe::XFrame >  xFrame = xModel->getCurrentController()->getFrame();
        if (!xFrame.is())
            return;

        starutil::URL aURL;
        // FormMethod GET
        if( eSubmitMethod == starform::FormSubmitMethod_GET )
        {
            INetURLObject aUrlObj( aURLStr );
            aUrlObj.SetParam( aData );
            ::rtl::OUString aMainURL = INetURLObject::decode(aUrlObj.GetMainURL(), '%', INetURLObject::DECODE_UNAMBIGUOUS);

            aURL.Complete = aMainURL;
            xTransformer->parseStrict(aURL);
            staruno::Reference< starframe::XDispatch >  xDisp = staruno::Reference< starframe::XDispatchProvider > (xFrame,staruno::UNO_QUERY)->queryDispatch(aURL, aTargetName,
                    starframe::FrameSearchFlag::SELF | starframe::FrameSearchFlag::PARENT | starframe::FrameSearchFlag::CHILDREN |
                    starframe::FrameSearchFlag::SIBLINGS | starframe::FrameSearchFlag::CREATE | starframe::FrameSearchFlag::TASKS);

            if (xDisp.is())
            {
                staruno::Sequence<starbeans::PropertyValue> aArgs(1);
                aArgs.getArray()->Name = ::rtl::OUString::createFromAscii("Referer");
                aArgs.getArray()->Value <<= aReferer;
                xDisp->dispatch(aURL, aArgs);
            }
        }
        // FormMethod POST
        else if( eSubmitMethod == starform::FormSubmitMethod_POST )
        {
            aURL.Complete = aURLStr;
            xTransformer->parseStrict(aURL);

            staruno::Reference< starframe::XDispatch >  xDisp = staruno::Reference< starframe::XDispatchProvider > (xFrame,staruno::UNO_QUERY)->queryDispatch(aURL, aTargetName,
                starframe::FrameSearchFlag::SELF | starframe::FrameSearchFlag::PARENT | starframe::FrameSearchFlag::CHILDREN |
                starframe::FrameSearchFlag::SIBLINGS | starframe::FrameSearchFlag::CREATE | starframe::FrameSearchFlag::TASKS);

            if (xDisp.is())
            {
                staruno::Sequence<starbeans::PropertyValue> aArgs(2);
                aArgs.getArray()[0].Name = ::rtl::OUString::createFromAscii("Referer");
                aArgs.getArray()[0].Value <<= aReferer;
                aArgs.getArray()[1].Name = ::rtl::OUString::createFromAscii("PostString");
                aArgs.getArray()[1].Value <<= aData;

                xDisp->dispatch(aURL, aArgs);
            }
        }
    }
    else if( eSubmitEncoding == starform::FormSubmitEncoding_MULTIPART )
    {
        staruno::Reference< starframe::XFrame >  xFrame = xModel->getCurrentController()->getFrame();
        if (!xFrame.is())
            return;

        starutil::URL aURL;
        aURL.Complete = aURLStr;
        xTransformer->parseStrict(aURL);

        staruno::Reference< starframe::XDispatch >  xDisp = staruno::Reference< starframe::XDispatchProvider > (xFrame,staruno::UNO_QUERY)->queryDispatch(aURL, aTargetName,
                starframe::FrameSearchFlag::SELF | starframe::FrameSearchFlag::PARENT | starframe::FrameSearchFlag::CHILDREN |
                starframe::FrameSearchFlag::SIBLINGS | starframe::FrameSearchFlag::CREATE | starframe::FrameSearchFlag::TASKS);

        if (xDisp.is())
        {
            ::rtl::OUString aContentType;
            staruno::Sequence<sal_Int8> aData;
            {
                ::vos::OGuard aGuard( Application::GetSolarMutex() );
                aData = GetDataMultiPartEncoded(Control, MouseEvt, aContentType);
            }
            if (!aData.getLength())
                return;

            staruno::Sequence<starbeans::PropertyValue> aArgs(3);
            aArgs.getArray()[0].Name = ::rtl::OUString::createFromAscii("Referer");
            aArgs.getArray()[0].Value <<= aReferer;
            aArgs.getArray()[1].Name = ::rtl::OUString::createFromAscii("ContentType");
            aArgs.getArray()[1].Value <<= aContentType;
            aArgs.getArray()[2].Name = ::rtl::OUString::createFromAscii("PostData");
            aArgs.getArray()[2].Value <<= aData;

            xDisp->dispatch(aURL, aArgs);
        }
    }
    else if( eSubmitEncoding == starform::FormSubmitEncoding_TEXT )
    {
        ::rtl::OUString aData;
        {
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            aData = GetDataTextEncoded( staruno::Reference<starawt::XControl> (), MouseEvt );
        }

        staruno::Reference< starframe::XFrame >  xFrame = xModel->getCurrentController()->getFrame();
        if (!xFrame.is())
            return;

        starutil::URL aURL;

        aURL.Complete = aURLStr;
        xTransformer->parseStrict(aURL);

        staruno::Reference< starframe::XDispatch >  xDisp = staruno::Reference< starframe::XDispatchProvider > (xFrame,staruno::UNO_QUERY)->queryDispatch(aURL, aTargetName,
            starframe::FrameSearchFlag::SELF | starframe::FrameSearchFlag::PARENT | starframe::FrameSearchFlag::CHILDREN |
            starframe::FrameSearchFlag::SIBLINGS | starframe::FrameSearchFlag::CREATE | starframe::FrameSearchFlag::TASKS);

        if (xDisp.is())
        {
            staruno::Sequence<starbeans::PropertyValue> aArgs(2);
            aArgs.getArray()[0].Name = ::rtl::OUString::createFromAscii("Referer");
            aArgs.getArray()[0].Value <<= aReferer;
            aArgs.getArray()[1].Name = ::rtl::OUString::createFromAscii("PostString");
            aArgs.getArray()[1].Value <<= aData;

            xDisp->dispatch(aURL, aArgs);
        }
    }
    else
        DBG_ERROR("ODatabaseForm::submit_Impl : wrong encoding !");

}

// starform::XSubmit
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addSubmitListener(const staruno::Reference<starform::XSubmitListener>& _rListener) throw( staruno::RuntimeException )
{
    m_aSubmitListeners.addInterface(_rListener);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeSubmitListener(const staruno::Reference<starform::XSubmitListener>& _rListener) throw( staruno::RuntimeException )
{
    m_aSubmitListeners.removeInterface(_rListener);
}

//==============================================================================
// com::sun::star::sdbc::XSQLErrorBroadcaster
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addSQLErrorListener(const staruno::Reference<starsdb::XSQLErrorListener>& _rListener) throw( staruno::RuntimeException )
{
    m_aErrorListeners.addInterface(_rListener);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeSQLErrorListener(const staruno::Reference<starsdb::XSQLErrorListener>& _rListener) throw( staruno::RuntimeException )
{
    m_aErrorListeners.removeInterface(_rListener);
}

//==============================================================================
// OChangeListener
//------------------------------------------------------------------------------
void ODatabaseForm::_propertyChanged(const starbeans::PropertyChangeEvent& evt) throw( staruno::RuntimeException )
{
    // if the statement has changed we have to delete the parameter info
    ::osl::MutexGuard aGuard(m_aMutex);
    DELETEZ(m_pParameterInfo);
    clearParameters();
}

//==============================================================================
// smartstarcontainer::XChild
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setParent(const InterfaceRef& Parent)
{
    ReusableMutexGuard aGuard(m_aMutex);

    staruno::Reference<starform::XForm>  xParentForm(getParent(), staruno::UNO_QUERY);
    if (xParentForm.is())
    {
        staruno::Reference<starsdb::XRowSetApproveBroadcaster>  xParentApprBroadcast(xParentForm, staruno::UNO_QUERY);
        if (xParentApprBroadcast.is())
            xParentApprBroadcast->removeRowSetApproveListener(this);
        staruno::Reference<starform::XLoadable>  xParentLoadable(xParentForm, staruno::UNO_QUERY);
        if (xParentLoadable.is())
            xParentLoadable->removeLoadListener(this);
    }

    OFormComponents::setParent(Parent);

    xParentForm = staruno::Reference<starform::XForm> (getParent(), staruno::UNO_QUERY);
    if (xParentForm.is())
    {
        staruno::Reference<starsdb::XRowSetApproveBroadcaster>  xParentApprBroadcast(xParentForm, staruno::UNO_QUERY);
        if (xParentApprBroadcast.is())
            xParentApprBroadcast->addRowSetApproveListener(this);
        staruno::Reference<starform::XLoadable>  xParentLoadable(xParentForm, staruno::UNO_QUERY);
        if (xParentLoadable.is())
            xParentLoadable->addLoadListener(this);
    }
}

//==============================================================================
// smartstarawt::XTabControllerModel
//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::getGroupControl() throw(com::sun::star::uno::RuntimeException)
{
    ReusableMutexGuard aGuard(m_aMutex);

    // Sollen Controls in einer TabOrder gruppe zusammengefaﬂt werden?
    if (m_aCycle.hasValue())
    {
        sal_Int32 nCycle;
        ::cppu::enum2int(nCycle, m_aCycle);
        return nCycle != starform::TabulatorCycle_PAGE;
    }

    if (isLoaded() && getConnection().is())
        return sal_True;

    return sal_False;
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setControlModels(const staruno::Sequence<staruno::Reference<starawt::XControlModel> >& rControls) throw( staruno::RuntimeException )
{
    ReusableMutexGuard aGuard(m_aMutex);

    // TabIndex in der Reihenfolge der Sequence setzen
    const staruno::Reference<starawt::XControlModel>* pControls = rControls.getConstArray();
    sal_Int16 nTabIndex = 1;
    sal_Int32 nCount = getCount();
    sal_Int32 nNewCount = rControls.getLength();

    // HiddenControls und Formulare werden nicht aufgefuehrt
    if (nNewCount <= nCount)
    {
        staruno::Any aElement;
        for (sal_Int32 i=0; i < nNewCount; ++i, ++pControls)
        {
            staruno::Reference<starform::XFormComponent>  xComp(*pControls, staruno::UNO_QUERY);
            if (xComp.is())
            {
                // suchen der Componente in der Liste
                for (sal_Int32 j = 0; j < nCount; ++j)
                {
                    staruno::Reference<starform::XFormComponent> xElement;
                    ::cppu::extractInterface(xElement, getByIndex(j));
                    if (xComp == xElement)
                    {
                        staruno::Reference<starbeans::XPropertySet>  xSet(xComp, staruno::UNO_QUERY);
                        if (xSet.is() && hasProperty(PROPERTY_TABINDEX, xSet))
                            xSet->setPropertyValue( PROPERTY_TABINDEX, staruno::makeAny(nTabIndex++) );
                        break;
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Reference<starawt::XControlModel> > SAL_CALL ODatabaseForm::getControlModels() throw( staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pGroupManager->getControlModels();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setGroup( const staruno::Sequence<staruno::Reference<starawt::XControlModel> >& _rGroup, const ::rtl::OUString& Name ) throw( staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // Die Controls werden gruppiert, indem ihr Name dem Namen des ersten
    // Controls der Sequenz angepasst wird
    const staruno::Reference<starawt::XControlModel>* pControls = _rGroup.getConstArray();
    staruno::Reference<starbeans::XPropertySet>     xSet;
    ::rtl::OUString sGroupName;

    for( sal_Int32 i=0; i<_rGroup.getLength(); ++i, ++pControls )
    {
        staruno::Reference<starbeans::XPropertySet> xSet(*pControls, staruno::UNO_QUERY);

        if (!sGroupName.getLength())
            xSet->getPropertyValue(PROPERTY_NAME) >>= sGroupName;
        else
            xSet->setPropertyValue(PROPERTY_NAME, staruno::makeAny(sGroupName));
    }
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseForm::getGroupCount() throw( staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pGroupManager->getGroupCount();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::getGroup( sal_Int32 nGroup, staruno::Sequence<staruno::Reference<starawt::XControlModel> >& _rGroup, ::rtl::OUString& Name ) throw( staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_pGroupManager->getGroup( nGroup, _rGroup, Name );
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::getGroupByName(const ::rtl::OUString& Name, staruno::Sequence< staruno::Reference<starawt::XControlModel>  >& _rGroup) throw( staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_pGroupManager->getGroupByName( Name, _rGroup );
}

//==============================================================================
// com::sun::star::lang::XEventListener
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::disposing(const starlang::EventObject& Source) throw( staruno::RuntimeException )
{
    OInterfaceContainer::disposing(Source);
}

//==============================================================================
// com::sun::star::form::XLoadListener
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::loaded(const starlang::EventObject& aEvent) throw( staruno::RuntimeException )
{
    // now start the rowset listening to recover cursor events
    load_impl(sal_True);
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        staruno::Reference<starsdbc::XRowSet>  xParentRowSet(m_xParent, staruno::UNO_QUERY);
        if (xParentRowSet.is())
            xParentRowSet->addRowSetListener(this);

        m_pLoadTimer = new Timer();
        m_pLoadTimer->SetTimeout(100);
        m_pLoadTimer->SetTimeoutHdl(LINK(this,ODatabaseForm,OnTimeout));
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::unloading(const starlang::EventObject& aEvent) throw( staruno::RuntimeException )
{
    {
        // now stop the rowset listening if we are a subform
        ::osl::MutexGuard aGuard(m_aMutex);
        DELETEZ(m_pLoadTimer);

        staruno::Reference<starsdbc::XRowSet>  xParentRowSet(m_xParent, staruno::UNO_QUERY);
        if (xParentRowSet.is())
            xParentRowSet->removeRowSetListener(this);
    }
    unload();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::unloaded(const starlang::EventObject& aEvent) throw( staruno::RuntimeException )
{
    // nothing to do
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::reloading(const starlang::EventObject& aEvent) throw( staruno::RuntimeException )
{
    // now stop the rowset listening if we are a subform
    ::osl::MutexGuard aGuard(m_aMutex);
    staruno::Reference<starsdbc::XRowSet>  xParentRowSet(m_xParent, staruno::UNO_QUERY);
    if (xParentRowSet.is())
        xParentRowSet->removeRowSetListener(this);

    if (m_pLoadTimer && m_pLoadTimer->IsActive())
        m_pLoadTimer->Stop();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::reloaded(const starlang::EventObject& aEvent) throw( staruno::RuntimeException )
{
    reload_impl(sal_True);
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        staruno::Reference<starsdbc::XRowSet>  xParentRowSet(m_xParent, staruno::UNO_QUERY);
        if (xParentRowSet.is())
            xParentRowSet->addRowSetListener(this);
    }
}

//------------------------------------------------------------------------------
IMPL_LINK( ODatabaseForm, OnTimeout, void*, EMPTYARG )
{
    reload_impl(sal_True);
    return 1;
}

//==============================================================================
// com::sun::star::form::XLoadable
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::load() throw( staruno::RuntimeException )
{
    load_impl(sal_False);
}

//------------------------------------------------------------------------------
void ODatabaseForm::load_impl(sal_Bool bCausedByParentForm, sal_Bool bMoveToFirst) throw( staruno::RuntimeException )
{
    ReusableMutexGuard aGuard(m_aMutex);

    // are we already loaded?
    if (isLoaded())
        return;

    m_bSubForm = bCausedByParentForm;

    // if we don't have a connection, we are not intended to be a database form or the aggregate was not able
    // to establish a connection
    sal_Bool bConnected     = sal_False;
    try
    {
        if (m_xAggregateSet.is())
        {
            // do we have a connection in the hierarchy than take that connection
            // this overwrites all the other connnections
            staruno::Reference< starsdbc::XConnection >  xConnection = calcConnection(
                staruno::Reference<starsdbc::XRowSet> (m_xAggregate, staruno::UNO_QUERY),
                m_xServiceFactory
            );      // will set a calculated connection implicitly
            bConnected = xConnection.is();
        }
    }
    catch(starsdbc::SQLException& eDB)
    {
        onError(eDB, FRM_RES_STRING(RID_STR_CONNECTERROR));
    }

    // we don't have to execute if we do not have a command to execute
    sal_Bool bExecute = bConnected && m_xAggregateSet.is() && getString(m_xAggregateSet->getPropertyValue(PROPERTY_COMMAND)).getLength();

    // a database form always uses caching
    // we use starting fetchsize with at least 10 rows
    if (bConnected)
        m_xAggregateSet->setPropertyValue(PROPERTY_FETCHSIZE, staruno::makeAny((sal_Int32)10));

    // if we're loaded as sub form we got a "rowSetChanged" from the parent rowset _before_ we got the "loaded"
    // so we don't need to execute the statement again, this was already done
    // (and there were no relevant changes between these two listener calls, the "load" of a form is quite an
    // atomar operation.)

    if (bExecute)
    {
        m_sCurrentErrorContext = FRM_RES_STRING(RID_ERR_LOADING_FORM);
        executeRowSet(aGuard, bMoveToFirst);
    }

    m_bLoaded = sal_True;
    aGuard.clear();
    starlang::EventObject aEvt(static_cast<staruno::XWeak*>(this));
    NOTIFY_LISTENERS(m_aLoadListeners, starform::XLoadListener, loaded, aEvt);

    // if we are on the insert row, we have to reset all controls
    // to set the default values
    if (bExecute && getBOOL(m_xAggregateSet->getPropertyValue(PROPERTY_ISNEW)))
        reset();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::unload() throw( staruno::RuntimeException )
{
    ReusableMutexGuard aGuard(m_aMutex);
    if (!isLoaded())
        return;

    DELETEZ(m_pLoadTimer);

    aGuard.clear();
    starlang::EventObject aEvt(static_cast<staruno::XWeak*>(this));
    NOTIFY_LISTENERS(m_aLoadListeners, starform::XLoadListener, unloading, aEvt);

    if (m_xAggregateAsRowSet.is())
    {
        // clear the parameters if there are any
        DELETEZ(m_pParameterInfo);
        clearParameters();

        try
        {
            // close the aggregate
            staruno::Reference<starsdbc::XCloseable>  xCloseable;
            query_aggregation( m_xAggregate, xCloseable);
            aGuard.clear();
            if (xCloseable.is())
                xCloseable->close();
        }
        catch(starsdbc::SQLException& eDB)
        {
            eDB;
        }
        aGuard.attach(m_aMutex);
    }

    m_bLoaded = sal_False;

    aGuard.clear();
    NOTIFY_LISTENERS(m_aLoadListeners, starform::XLoadListener, unloaded, aEvt);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::reload() throw( staruno::RuntimeException )
{
    reload_impl(sal_True);
}

//------------------------------------------------------------------------------
void ODatabaseForm::reload_impl(sal_Bool bMoveToFirst) throw( staruno::RuntimeException )
{
    ReusableMutexGuard aGuard(m_aMutex);
    if (!isLoaded())
        return;

    starlang::EventObject aEvent(static_cast<staruno::XWeak*>(this));
    {
        // only if there is no approve listener we can post the event at this time
        // otherwise see approveRowsetChange
        // the aprrovement is done by the aggregate
        if (!m_aRowSetApproveListeners.getLength())
        {
            ::cppu::OInterfaceIteratorHelper aIter(m_aLoadListeners);
            aGuard.clear();

            while (aIter.hasMoreElements())
                ((starform::XLoadListener*)aIter.next())->reloading(aEvent);

            aGuard.attach(m_aMutex);
        }
    }

    try
    {
        m_sCurrentErrorContext = FRM_RES_STRING(RID_ERR_REFRESHING_FORM);
        executeRowSet(aGuard, bMoveToFirst);
    }
    catch(starsdbc::SQLException& e)
    {
        DBG_ERROR("ODatabaseForm::reload_impl : shouldn't executeRowSet catch this exception ?");
        e;
    }

    {
        ::cppu::OInterfaceIteratorHelper aIter(m_aLoadListeners);
        aGuard.clear();
        while (aIter.hasMoreElements())
            ((starform::XLoadListener*)aIter.next())->reloaded(aEvent);

        // if we are on the insert row, we have to reset all controls
        // to set the default values
        if (getBOOL(m_xAggregateSet->getPropertyValue(PROPERTY_ISNEW)))
            reset();
    }
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::isLoaded() throw( staruno::RuntimeException )
{
    return m_bLoaded;
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addLoadListener(const staruno::Reference<starform::XLoadListener>& aListener) throw( staruno::RuntimeException )
{
    m_aLoadListeners.addInterface(aListener);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeLoadListener(const staruno::Reference<starform::XLoadListener>& aListener) throw( staruno::RuntimeException )
{
    m_aLoadListeners.removeInterface(aListener);
}

//==============================================================================
// com::sun::star::sdbc::XCloseable
//==============================================================================
void SAL_CALL ODatabaseForm::close() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    // unload will close the aggregate
    unload();
}

//==============================================================================
// com::sun::star::sdbc::XRowSetListener
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::cursorMoved(const starlang::EventObject& event) throw( staruno::RuntimeException )
{
    // reload the subform with the new parameters of the parent
    // do this handling delayed to provide of execute too many SQL Statements
    ReusableMutexGuard aGuard(m_aMutex);
    if (m_pLoadTimer->IsActive())
        m_pLoadTimer->Stop();

    // and start the timer again
    m_pLoadTimer->Start();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::rowChanged(const starlang::EventObject& event) throw( staruno::RuntimeException )
{
    // ignore it
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::rowSetChanged(const starlang::EventObject& event) throw( staruno::RuntimeException )
{
    // not interested in :
    // if our parent is an ODatabaseForm, too, then after this rowSetChanged we'll get a "reloaded"
    // or a "loaded" event.
    // If somebody gave us another parent which is an starsdbc::XRowSet but doesn't handle an execute as
    // "load" respectivly "reload" ... can't do anything ....
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::approveCursorMove(const starlang::EventObject& event) throw( staruno::RuntimeException )
{
    // is our aggregate calling?
    if (event.Source == InterfaceRef(static_cast<staruno::XWeak*>(this)))
    {
        // Our aggregate doesn't have any ApproveRowSetListeners (expect ourself), as we re-routed the queryInterface
        // for starsdb::XRowSetApproveBroadcaster-interface.
        // So we have to multiplex this approve request.
        ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
        while (aIter.hasMoreElements())
            if (!((starsdb::XRowSetApproveListener*)aIter.next())->approveCursorMove(event))
                return sal_False;
    }
    else
    {
        // this is a call from our parent ...
        // a parent's cursor move will result in a re-execute of our own row-set, so we have to
        // ask our own RowSetChangesListeners, too
        ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
        while (aIter.hasMoreElements())
            if (!((starsdb::XRowSetApproveListener*)aIter.next())->approveRowSetChange(event))
                return sal_False;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::approveRowChange(const starsdb::RowChangeEvent& event) throw( staruno::RuntimeException )
{
    // is our aggregate calling?
    if (event.Source == InterfaceRef(static_cast<staruno::XWeak*>(this)))
    {
        // Our aggregate doesn't have any ApproveRowSetListeners (expect ourself), as we re-routed the queryInterface
        // for starsdb::XRowSetApproveBroadcaster-interface.
        // So we have to multiplex this approve request.
        ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
        while (aIter.hasMoreElements())
            if (!((starsdb::XRowSetApproveListener*)aIter.next())->approveRowChange(event))
                return sal_False;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::approveRowSetChange(const starlang::EventObject& event) throw( staruno::RuntimeException )
{
    if (event.Source == InterfaceRef(static_cast<staruno::XWeak*>(this)))   // ignore our aggregate as we handle this approve ourself
    {
        ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
        while (aIter.hasMoreElements())
        if (!((starsdb::XRowSetApproveListener*)aIter.next())->approveRowSetChange(event))
            return sal_False;

        if (isLoaded())
        {
            ::cppu::OInterfaceIteratorHelper aLoadIter(m_aLoadListeners);
            while (aLoadIter.hasMoreElements())
                ((starform::XLoadListener*)aLoadIter.next())->reloading(event);
        }
    }
    else
    {
        // this is a call from our parent ...
        // a parent's cursor move will result in a re-execute of our own row-set, so we have to
        // ask our own RowSetChangesListeners, too
        ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
        while (aIter.hasMoreElements())
            if (!((starsdb::XRowSetApproveListener*)aIter.next())->approveRowSetChange(event))
                return sal_False;
    }
    return sal_True;
}

//==============================================================================
// com::sun::star::sdb::XRowSetApproveBroadcaster
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addRowSetApproveListener(const staruno::Reference<starsdb::XRowSetApproveListener>& _rListener) throw( staruno::RuntimeException )
{
    ReusableMutexGuard aGuard(m_aMutex);
    m_aRowSetApproveListeners.addInterface(_rListener);

    // do we have to multiplex ?
    if (m_aRowSetApproveListeners.getLength() == 1)
    {
        staruno::Reference<starsdb::XRowSetApproveBroadcaster>  xBroadcaster;
        if (query_aggregation( m_xAggregate, xBroadcaster))
        {
            staruno::Reference<starsdb::XRowSetApproveListener>  xListener((starsdb::XRowSetApproveListener*)this);
            xBroadcaster->addRowSetApproveListener(xListener);
        }
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeRowSetApproveListener(const staruno::Reference<starsdb::XRowSetApproveListener>& _rListener) throw( staruno::RuntimeException )
{
    ReusableMutexGuard aGuard(m_aMutex);
    // do we have to remove the multiplex ?
    if (m_aRowSetApproveListeners.getLength() == 1)
    {
        staruno::Reference<starsdb::XRowSetApproveBroadcaster>  xBroadcaster;
        if (query_aggregation( m_xAggregate, xBroadcaster))
        {
            staruno::Reference<starsdb::XRowSetApproveListener>  xListener((starsdb::XRowSetApproveListener*)this);
            xBroadcaster->removeRowSetApproveListener(xListener);
        }
    }
    m_aRowSetApproveListeners.removeInterface(_rListener);
}

//==============================================================================
// com::sun:star::form::XDatabaseParameterBroadcaster
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addParameterListener(const staruno::Reference<starform::XDatabaseParameterListener>& _rListener) throw( staruno::RuntimeException )
{
    m_aParameterListeners.addInterface(_rListener);
}
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeParameterListener(const staruno::Reference<starform::XDatabaseParameterListener>& _rListener) throw( staruno::RuntimeException )
{
    m_aParameterListeners.removeInterface(_rListener);
}

//==============================================================================
// com::sun::star::sdbc::XRowSet
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::execute() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    ReusableMutexGuard aGuard(m_aMutex);
    // This methods is called only from outside, we ourself don't use our own starsdbc::XRowSet interface but the one of our
    // aggregate when calling an execute.
    // DBG_ASSERT(!isExecuting(), "ODatabaseForm::execute : oops ... our own erxecute ?");
        // This thread shouldn't be within executeRowSet (as stated above) and any other thread should be blocked
        // because of our mutex ....

    // So if somebody calls an execute and we're not loaded we reroute this call to our load method.

    // the difference between execute and load is, that we position on the first row in case of load
    // after execute we remain before the first row
    if (!isLoaded())
    {
        aGuard.clear();
        load_impl(sal_False, sal_False);
    }
    else
    {
        starlang::EventObject event(static_cast< staruno::XWeak* >(this));
        ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
        aGuard.clear();

        while (aIter.hasMoreElements())
            if (!((starsdb::XRowSetApproveListener*)aIter.next())->approveRowSetChange(event))
                return;

        // we're loaded and somebody want's to execute ourself -> this means a reload
        reload_impl(sal_False);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addRowSetListener(const staruno::Reference<starsdbc::XRowSetListener>& _rListener) throw( staruno::RuntimeException )
{
    if (m_xAggregateAsRowSet.is())
        m_xAggregateAsRowSet->addRowSetListener(_rListener);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeRowSetListener(const staruno::Reference<starsdbc::XRowSetListener>& _rListener) throw( staruno::RuntimeException )
{
    if (m_xAggregateAsRowSet.is())
        m_xAggregateAsRowSet->removeRowSetListener(_rListener);
}

//==============================================================================
// com::sun::star::sdbc::XResultSet
//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::next() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->next();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::isBeforeFirst() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->isBeforeFirst();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::isAfterLast() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->isAfterLast();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::isFirst() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->isFirst();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::isLast() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->isLast();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::beforeFirst() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    m_xAggregateAsRowSet->beforeFirst();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::afterLast() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    m_xAggregateAsRowSet->afterLast();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::first() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->first();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::last() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->last();
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseForm::getRow() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->getRow();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::absolute(sal_Int32 row) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->absolute(row);
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::relative(sal_Int32 rows) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->relative(rows);
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::previous() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->previous();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::refreshRow() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    m_xAggregateAsRowSet->refreshRow();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::rowUpdated() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->rowUpdated();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::rowInserted() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->rowInserted();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::rowDeleted() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->rowDeleted();
}

//------------------------------------------------------------------------------
InterfaceRef SAL_CALL ODatabaseForm::getStatement() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    return m_xAggregateAsRowSet->getStatement();
}

// com::sun::star::sdbc::XResultSetUpdate
// exceptions during insert update and delete will be forwarded to the errorlistener
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::insertRow() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    try
    {
        staruno::Reference<starsdbc::XResultSetUpdate>  xUpdate;
        if (query_aggregation( m_xAggregate, xUpdate))
            xUpdate->insertRow();
    }
    catch(starsdb::RowSetVetoException& eVeto)
    {
        eVeto;
        throw;
    }
    catch(starsdbc::SQLException& eDb)
    {
        onError(eDb, FRM_RES_STRING(RID_STR_ERR_INSERTRECORD));
        throw;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::updateRow() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    try
    {
        staruno::Reference<starsdbc::XResultSetUpdate>  xUpdate;
        if (query_aggregation( m_xAggregate, xUpdate))
            xUpdate->updateRow();
    }
    catch(starsdb::RowSetVetoException& eVeto)
    {
        eVeto;
        throw;
    }
    catch(starsdbc::SQLException& eDb)
    {
        onError(eDb, FRM_RES_STRING(RID_STR_ERR_UPDATERECORD));
        throw;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::deleteRow() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    try
    {
        staruno::Reference<starsdbc::XResultSetUpdate>  xUpdate;
        if (query_aggregation( m_xAggregate, xUpdate))
            xUpdate->deleteRow();
    }
    catch(starsdb::RowSetVetoException& eVeto)
    {
        eVeto;
        throw;
    }
    catch(starsdbc::SQLException& eDb)
    {
        onError(eDb, FRM_RES_STRING(RID_STR_ERR_DELETERECORD));
        throw;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::cancelRowUpdates() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    staruno::Reference<starsdbc::XResultSetUpdate>  xUpdate;
    if (query_aggregation( m_xAggregate, xUpdate))
        xUpdate->cancelRowUpdates();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::moveToInsertRow() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    staruno::Reference<starsdbc::XResultSetUpdate>  xUpdate;
    if (query_aggregation( m_xAggregate, xUpdate))
    {
        // do we go on the insert row?
        if (!getBOOL(m_xAggregateSet->getPropertyValue(PROPERTY_ISNEW)))
            xUpdate->moveToInsertRow();

        // then set the default values and the parameters given from the parent
        reset();
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::moveToCurrentRow() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    staruno::Reference<starsdbc::XResultSetUpdate>  xUpdate;
    if (query_aggregation( m_xAggregate, xUpdate))
        xUpdate->moveToCurrentRow();
}

// com::sun::star::sdbcx::XDeleteRows
//------------------------------------------------------------------------------
staruno::Sequence<sal_Int32> SAL_CALL ODatabaseForm::deleteRows(const staruno::Sequence<staruno::Any>& rows) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    try
    {
        staruno::Reference<starsdbcx::XDeleteRows>  xDelete;
        if (query_aggregation( m_xAggregate, xDelete))
            return xDelete->deleteRows(rows);
    }
    catch(starsdb::RowSetVetoException& eVeto)
    {
        eVeto; // make compiler happy
        throw;
    }
    catch(starsdbc::SQLException& eDb)
    {
        onError(eDb, FRM_RES_STRING(RID_STR_ERR_DELETERECORDS));
        throw;
    }

    return staruno::Sequence< sal_Int32 >();
}

// com::sun::star::sdbc::XParameters
//------------------------------------------------------------------------------
#define PARAMETER_VISITED(method)                                       \
    ::osl::MutexGuard aGuard(m_aMutex);                                 \
    staruno::Reference<starsdbc::XParameters>  xParameters;             \
    if (query_aggregation( m_xAggregate, xParameters))                  \
        xParameters->method;                                            \
                                                                        \
    if (m_aParameterVisited.size() < parameterIndex)                    \
    {                                                                   \
        for (sal_Int32 i = 0; i < parameterIndex; i++)                  \
            m_aParameterVisited.push_back(false);                       \
    }                                                                   \
    m_aParameterVisited[parameterIndex - 1] = true

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setNull(parameterIndex, sqlType));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setObjectNull(parameterIndex, sqlType, typeName));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setBoolean(sal_Int32 parameterIndex, sal_Bool x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setBoolean(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setByte(sal_Int32 parameterIndex, sal_Int8 x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setByte(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setShort(sal_Int32 parameterIndex, sal_Int16 x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setShort(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setInt(sal_Int32 parameterIndex, sal_Int32 x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setInt(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setLong(sal_Int32 parameterIndex, Hyper x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setLong(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setFloat(sal_Int32 parameterIndex, float x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setFloat(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setDouble(sal_Int32 parameterIndex, double x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setDouble(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setString(sal_Int32 parameterIndex, const ::rtl::OUString& x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setString(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setBytes(sal_Int32 parameterIndex, const staruno::Sequence< sal_Int8 >& x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setBytes(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setDate(sal_Int32 parameterIndex, const starutil::Date& x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setDate(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setTime(sal_Int32 parameterIndex, const starutil::Time& x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setTime(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setTimestamp(sal_Int32 parameterIndex, const starutil::DateTime& x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setTimestamp(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setBinaryStream(sal_Int32 parameterIndex, const staruno::Reference<stario::XInputStream>& x, sal_Int32 length) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setBinaryStream(parameterIndex, x, length));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setCharacterStream(sal_Int32 parameterIndex, const staruno::Reference<stario::XInputStream>& x, sal_Int32 length) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setCharacterStream(parameterIndex, x, length));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setObjectWithInfo(sal_Int32 parameterIndex, const staruno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setObjectWithInfo(parameterIndex, x, targetSqlType, scale));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setObject(sal_Int32 parameterIndex, const staruno::Any& x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setObject(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setRef(sal_Int32 parameterIndex, const staruno::Reference<starsdbc::XRef>& x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setRef(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setBlob(sal_Int32 parameterIndex, const staruno::Reference<starsdbc::XBlob>& x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setBlob(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setClob(sal_Int32 parameterIndex, const staruno::Reference<starsdbc::XClob>& x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setClob(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setArray(sal_Int32 parameterIndex, const staruno::Reference<starsdbc::XArray>& x) throw( starsdbc::SQLException, staruno::RuntimeException )
{
    PARAMETER_VISITED(setArray(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::clearParameters() throw( starsdbc::SQLException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    staruno::Reference<starsdbc::XParameters>  xParameters;
    if (query_aggregation(m_xAggregate, xParameters))
        xParameters->clearParameters();
    m_aParameterVisited.clear();
}

// com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseForm::getImplementationName() throw( staruno::RuntimeException )
{
    return DATABASEFORM_IMPLEMENTATION_NAME;
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL ODatabaseForm::getSupportedServiceNames() throw( staruno::RuntimeException )
{
    StringSequence aServices;
    aServices.realloc(5);
    aServices.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.form.FormComponent");
    aServices.getArray()[1] = ::rtl::OUString::createFromAscii("com.sun.star.form.FormComponents");
    aServices.getArray()[2] = FRM_SUN_COMPONENT_FORM;
    aServices.getArray()[3] = FRM_SUN_COMPONENT_HTMLFORM;
    aServices.getArray()[4] = FRM_SUN_COMPONENT_DATAFORM;
    return aServices;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::supportsService(const ::rtl::OUString& ServiceName) throw( staruno::RuntimeException )
{
    StringSequence aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pArray = aSupported.getConstArray();
    for( sal_Int32 i = 0; i < aSupported.getLength(); ++i, ++pArray )
        if( pArray->equals(ServiceName) )
            return sal_True;
    return sal_False;
}

//==============================================================================
// com::sun::star::io::XPersistObject
//------------------------------------------------------------------------------

const sal_uInt16 CYCLE              = 0x0001;
const sal_uInt16 DONTAPPLYFILTER    = 0x0002;

//------------------------------------------------------------------------------
::rtl::OUString ODatabaseForm::getServiceName() throw( staruno::RuntimeException )
{
    return FRM_COMPONENT_FORM;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream) throw( stario::IOException, staruno::RuntimeException )
{
    DBG_ASSERT(m_xAggregateSet.is(), "ODatabaseForm::write : only to be called if the aggregate exists !");

    // all children
    OFormComponents::write(_rxOutStream);

    // version
    _rxOutStream->writeShort(0x0003);

    // Name
    _rxOutStream << m_sName;

    ::rtl::OUString sDataSource;
    if (m_xAggregateSet.is())
        m_xAggregateSet->getPropertyValue(PROPERTY_DATASOURCE) >>= sDataSource;
    _rxOutStream << sDataSource;

    // former CursorSource
    ::rtl::OUString sCommand;
    if (m_xAggregateSet.is())
        m_xAggregateSet->getPropertyValue(PROPERTY_COMMAND) >>= sCommand;
    _rxOutStream << sCommand;

    // former MasterFields
    _rxOutStream << m_aMasterFields;
    // former DetailFields
    _rxOutStream << m_aDetailFields;

    // former DataSelectionType
    starform::DataSelectionType eTranslated = starform::DataSelectionType_TABLE;
    if (m_xAggregateSet.is())
    {
        sal_Int32 nCommandType;
        m_xAggregateSet->getPropertyValue(PROPERTY_COMMANDTYPE) >>= nCommandType;
        switch (nCommandType)
        {
            case starsdb::CommandType::TABLE : eTranslated = starform::DataSelectionType_TABLE; break;
            case starsdb::CommandType::QUERY : eTranslated = starform::DataSelectionType_QUERY; break;
            case starsdb::CommandType::COMMAND:
            {
                sal_Bool bEscapeProcessing = getBOOL(m_xAggregateSet->getPropertyValue(PROPERTY_ESCAPE_PROCESSING));
                eTranslated = bEscapeProcessing ? starform::DataSelectionType_SQL : starform::DataSelectionType_SQLPASSTHROUGH;
            }
            break;
            default : DBG_ERROR("ODatabaseForm::write : wrong CommandType !");
        }
    }
    _rxOutStream->writeShort((sal_Int16)eTranslated);           // former DataSelectionType

    // very old versions expect a CursorType here
    _rxOutStream->writeShort(stardata::DatabaseCursorType_KEYSET);

    _rxOutStream->writeBoolean(m_eNavigation != starform::NavigationBarMode_NONE);

    // former DataEntry
    if (m_xAggregateSet.is())
        _rxOutStream->writeBoolean(getBOOL(m_xAggregateSet->getPropertyValue(PROPERTY_INSERTONLY)));
    else
        _rxOutStream->writeBoolean(sal_False);

    _rxOutStream->writeBoolean(m_bAllowInsert);
    _rxOutStream->writeBoolean(m_bAllowUpdate);
    _rxOutStream->writeBoolean(m_bAllowDelete);

    // html form stuff
    ::rtl::OUString sTmp = INetURLObject::decode(INetURLObject::AbsToRel( m_aTargetURL ), '%', INetURLObject::DECODE_UNAMBIGUOUS);
    _rxOutStream << sTmp;
    _rxOutStream->writeShort( (sal_Int16)m_eSubmitMethod );
    _rxOutStream->writeShort( (sal_Int16)m_eSubmitEncoding );
    _rxOutStream << m_aTargetFrame;

    // version 2 didn't know some options and the "default" state
    sal_Int32 nCycle = starform::TabulatorCycle_RECORDS;
    if (m_aCycle.hasValue())
    {
        ::cppu::enum2int(nCycle, m_aCycle);
        if (m_aCycle == starform::TabulatorCycle_PAGE)
                // unknown in earlier versions
            nCycle = starform::TabulatorCycle_RECORDS;
    }
    _rxOutStream->writeShort((sal_Int16) nCycle);

    _rxOutStream->writeShort((sal_Int16)m_eNavigation);

    ::rtl::OUString sFilter;
    ::rtl::OUString sOrder;
    if (m_xAggregateSet.is())
    {
        m_xAggregateSet->getPropertyValue(PROPERTY_FILTER_CRITERIA) >>= sFilter;
        m_xAggregateSet->getPropertyValue(PROPERTY_SORT) >>= sOrder;
    }
    _rxOutStream << sFilter;
    _rxOutStream << sOrder;


    // version 3
    sal_uInt16 nAnyMask = 0;
    if (m_aCycle.hasValue())
        nAnyMask |= CYCLE;

    if (m_xAggregateSet.is() && !getBOOL(m_xAggregateSet->getPropertyValue(PROPERTY_APPLYFILTER)))
        nAnyMask |= DONTAPPLYFILTER;

    _rxOutStream->writeShort(nAnyMask);

    if (nAnyMask & CYCLE)
    {
        sal_Int32 nCycle;
        ::cppu::enum2int(nCycle, m_aCycle);
        _rxOutStream->writeShort(nCycle);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream) throw( stario::IOException, staruno::RuntimeException )
{
    DBG_ASSERT(m_xAggregateSet.is(), "ODatabaseForm::read : only to be called if the aggregate exists !");

    OFormComponents::read(_rxInStream);

    // version
    sal_uInt16 nVersion = _rxInStream->readShort();

    _rxInStream >> m_sName;

    ::rtl::OUString sAggregateProp;
    _rxInStream >> sAggregateProp;
    if (m_xAggregateSet.is())
        m_xAggregateSet->setPropertyValue(PROPERTY_DATASOURCE, staruno::makeAny(sAggregateProp));
    _rxInStream >> sAggregateProp;
    if (m_xAggregateSet.is())
        m_xAggregateSet->setPropertyValue(PROPERTY_COMMAND, staruno::makeAny(sAggregateProp));

    _rxInStream >> m_aMasterFields;
    _rxInStream >> m_aDetailFields;

    sal_Int16 nCursorSourceType = _rxInStream->readShort();
    sal_Int32 nCommandType = 0;
    switch ((starform::DataSelectionType)nCursorSourceType)
    {
        case starform::DataSelectionType_TABLE : nCommandType = starsdb::CommandType::TABLE; break;
        case starform::DataSelectionType_QUERY : nCommandType = starsdb::CommandType::QUERY; break;
        case starform::DataSelectionType_SQL:
        case starform::DataSelectionType_SQLPASSTHROUGH:
        {
            nCommandType = starsdb::CommandType::COMMAND;
            sal_Bool bEscapeProcessing = ((starform::DataSelectionType)nCursorSourceType) != starform::DataSelectionType_SQLPASSTHROUGH;
            m_xAggregateSet->setPropertyValue(PROPERTY_ESCAPE_PROCESSING, staruno::makeAny((sal_Bool)bEscapeProcessing));
        }
        break;
        default : DBG_ERROR("ODatabaseForm::read : wrong CommandType !");
    }
    if (m_xAggregateSet.is())
        m_xAggregateSet->setPropertyValue(PROPERTY_COMMANDTYPE, staruno::makeAny(nCommandType));

    // obsolete
    sal_Int16 nDummy = _rxInStream->readShort();

    // navigation mode was a boolean in version 1
    // war in der version 1 ein sal_Bool
    sal_Bool bNavigation = _rxInStream->readBoolean();
    if (nVersion == 1)
        m_eNavigation = bNavigation ? starform::NavigationBarMode_CURRENT : starform::NavigationBarMode_NONE;

    sal_Bool bInsertOnly = _rxInStream->readBoolean();
    if (m_xAggregateSet.is())
        m_xAggregateSet->setPropertyValue(PROPERTY_INSERTONLY, staruno::makeAny(bInsertOnly));

    m_bAllowInsert      = _rxInStream->readBoolean();
    m_bAllowUpdate      = _rxInStream->readBoolean();
    m_bAllowDelete      = _rxInStream->readBoolean();

    // html stuff
    ::rtl::OUString sTmp;
    _rxInStream >> sTmp;
    m_aTargetURL = INetURLObject::decode(INetURLObject::RelToAbs( sTmp ), '%', INetURLObject::DECODE_UNAMBIGUOUS);
    m_eSubmitMethod     = (starform::FormSubmitMethod)_rxInStream->readShort();
    m_eSubmitEncoding       = (starform::FormSubmitEncoding)_rxInStream->readShort();
    _rxInStream >> m_aTargetFrame;

    if (nVersion > 1)
    {
        sal_Int32 nCycle = _rxInStream->readShort();
        m_aCycle = ::cppu::int2enum(nCycle, ::getCppuType(static_cast<const starform::TabulatorCycle*>(NULL)));
        m_eNavigation = (starform::NavigationBarMode)_rxInStream->readShort();

        _rxInStream >> sAggregateProp;
        if (m_xAggregateSet.is())
            m_xAggregateSet->setPropertyValue(PROPERTY_FILTER_CRITERIA, staruno::makeAny(sAggregateProp));
        _rxInStream >> sAggregateProp;
        if (m_xAggregateSet.is())
            m_xAggregateSet->setPropertyValue(PROPERTY_SORT, staruno::makeAny(sAggregateProp));
    }

    sal_uInt16 nAnyMask = 0;
    if (nVersion > 2)
    {
        nAnyMask = _rxInStream->readShort();
        if (nAnyMask & CYCLE)
        {
            sal_Int32 nCycle = _rxInStream->readShort();
            m_aCycle = ::cppu::int2enum(nCycle, ::getCppuType(static_cast<const starform::TabulatorCycle*>(NULL)));
        }
        else
            m_aCycle.clear();
    }
    if (m_xAggregateSet.is())
        m_xAggregateSet->setPropertyValue(PROPERTY_APPLYFILTER, staruno::makeAny((sal_Bool)((nAnyMask & DONTAPPLYFILTER) == 0)));
}

//------------------------------------------------------------------------------
void ODatabaseForm::implInserted(const InterfaceRef& _rxObject)
{
    OFormComponents::implInserted(_rxObject);

    staruno::Reference<starsdb::XSQLErrorBroadcaster>  xBroadcaster(_rxObject, staruno::UNO_QUERY);
    staruno::Reference<starform::XForm>  xForm(_rxObject, staruno::UNO_QUERY);
    if (xBroadcaster.is() && !xForm.is())
    {   // the object is an error broadcaster, but no form itself -> add ourself as listener
        xBroadcaster->addSQLErrorListener(this);
    }
}

//------------------------------------------------------------------------------
void ODatabaseForm::implRemoved(const InterfaceRef& _rxObject)
{
    OFormComponents::implInserted(_rxObject);

    staruno::Reference<starsdb::XSQLErrorBroadcaster>  xBroadcaster(_rxObject, staruno::UNO_QUERY);
    staruno::Reference<starform::XForm>  xForm(_rxObject, staruno::UNO_QUERY);
    if (xBroadcaster.is() && !xForm.is())
    {   // the object is an error broadcaster, but no form itself -> remove ourself as listener
        xBroadcaster->removeSQLErrorListener(this);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::errorOccured(const starsdb::SQLErrorEvent& _rEvent) throw( staruno::RuntimeException )
{
    // give it to my own error listener
    onError(_rEvent);
    // TODO : think about extending the chain with an starsdb::SQLContext object saying
    // "this was an error of one of my children"
}

// com::sun::star::container::XNamed
//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseForm::getName() throw( staruno::RuntimeException )
{
    ::rtl::OUString sReturn;
    OPropertySetHelper::getFastPropertyValue(PROPERTY_ID_NAME) >>= sReturn;
    return sReturn;
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setName(const ::rtl::OUString& aName) throw( staruno::RuntimeException )
{
    setFastPropertyValue(PROPERTY_ID_NAME, staruno::makeAny(aName));
}

//.........................................................................
}   // namespace frm
//.........................................................................

