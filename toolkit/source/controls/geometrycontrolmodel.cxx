/*************************************************************************
 *
 *  $RCSfile: geometrycontrolmodel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ab $ $Date: 2001-02-21 17:31:20 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLKIT_HELPERS_GEOMETRYCONTROLMODEL_HXX_
#include "toolkit/controls/geometrycontrolmodel.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#include "toolkit/controls/eventcontainer.hxx"


#define GCM_PROPERTY_ID_POS_X       1
#define GCM_PROPERTY_ID_POS_Y       2
#define GCM_PROPERTY_ID_WIDTH       3
#define GCM_PROPERTY_ID_HEIGHT      4

#define GCM_PROPERTY_POS_X  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionX"))
#define GCM_PROPERTY_POS_Y  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionY"))
#define GCM_PROPERTY_WIDTH  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Width"))
#define GCM_PROPERTY_HEIGHT ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Height"))

#define DEFAULT_ATTRIBS()       PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT

//........................................................................
// namespace toolkit
// {
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::comphelper;

    //====================================================================
    //= OGeometryControlModel_Base
    //====================================================================
    //--------------------------------------------------------------------
    OGeometryControlModel_Base::OGeometryControlModel_Base(XAggregation* _pAggregateInstance)
        :OPropertySetAggregationHelper(m_aBHelper)
        ,OPropertyContainer(m_aBHelper)
        ,m_nPosX(0)
        ,m_nPosY(0)
        ,m_nWidth(0)
        ,m_nHeight(0)
    {
        OSL_ENSURE(NULL != _pAggregateInstance, "OGeometryControlModel_Base::OGeometryControlModel_Base: invalid aggregate!");

        // create our aggregate
        increment(m_refCount);
        {
            m_xAggregate = _pAggregateInstance;
            setAggregation(m_xAggregate);
            m_xAggregate->setDelegator(static_cast< XWeak* >(this));
        }
        decrement(m_refCount);

        // register our members for the property handling of the OPropertyContainer
        registerProperty(GCM_PROPERTY_POS_X,    GCM_PROPERTY_ID_POS_X,  DEFAULT_ATTRIBS(), &m_nPosX, ::getCppuType(&m_nPosX));
        registerProperty(GCM_PROPERTY_POS_Y,    GCM_PROPERTY_ID_POS_Y,  DEFAULT_ATTRIBS(), &m_nPosY, ::getCppuType(&m_nPosY));
        registerProperty(GCM_PROPERTY_WIDTH,    GCM_PROPERTY_ID_WIDTH,  DEFAULT_ATTRIBS(), &m_nWidth, ::getCppuType(&m_nWidth));
        registerProperty(GCM_PROPERTY_HEIGHT,   GCM_PROPERTY_ID_HEIGHT, DEFAULT_ATTRIBS(), &m_nHeight, ::getCppuType(&m_nHeight));
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OGeometryControlModel_Base::queryAggregation( const Type& _rType ) throw(RuntimeException)
    {
        Any aReturn = OWeakAggObject::queryAggregation(_rType);
            // the basic interfaces (XInterface, XAggregation etc)

        if (!aReturn.hasValue())
            aReturn = OPropertySetAggregationHelper::queryInterface(_rType);
            // the property set related interfaces

        if (!aReturn.hasValue() && m_xAggregate.is())
            aReturn = m_xAggregate->queryAggregation(_rType);
            // the interfaces our aggregate can provide

        if (!aReturn.hasValue() && m_xAggregate.is())
        {
            aReturn = ::cppu::queryInterface( _rType,
                static_cast< ::com::sun::star::script::XScriptEventsSupplier* >( this ) );
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OGeometryControlModel_Base::queryInterface( const Type& _rType ) throw(RuntimeException)
    {
        return OWeakAggObject::queryInterface(_rType);
    }

    //--------------------------------------------------------------------
    void SAL_CALL OGeometryControlModel_Base::acquire(  ) throw()
    {
        OWeakAggObject::acquire();
    }

    //--------------------------------------------------------------------
    void SAL_CALL OGeometryControlModel_Base::release(  ) throw()
    {
        OWeakAggObject::release();
    }

    //--------------------------------------------------------------------
    OGeometryControlModel_Base::~OGeometryControlModel_Base()
    {
        // release the aggregate (_before_ clearing m_xAggregate)
        if (m_xAggregate.is())
            m_xAggregate->setDelegator(NULL);
        setAggregation(NULL);
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OGeometryControlModel_Base::convertFastPropertyValue(Any& _rConvertedValue, Any& _rOldValue,
            sal_Int32 _nHandle, const Any& _rValue) throw (IllegalArgumentException)
    {
        return OPropertyContainer::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
    }

    //--------------------------------------------------------------------
    void SAL_CALL OGeometryControlModel_Base::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue) throw (Exception)
    {
        OPropertyContainer::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    }

    //--------------------------------------------------------------------
    void SAL_CALL OGeometryControlModel_Base::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
    {
        OPropertyArrayAggregationHelper& rPH = static_cast<OPropertyArrayAggregationHelper&>(const_cast<OGeometryControlModel_Base*>(this)->getInfoHelper());
        ::rtl::OUString sPropName;
        sal_Int32   nOriginalHandle = -1;

        if (rPH.fillAggregatePropertyInfoByHandle(&sPropName, &nOriginalHandle, _nHandle))
            OPropertySetAggregationHelper::getFastPropertyValue(_rValue, _nHandle);
        else
            OPropertyContainer::getFastPropertyValue(_rValue, _nHandle);
    }

    //--------------------------------------------------------------------
    Reference< XPropertySetInfo> SAL_CALL OGeometryControlModel_Base::getPropertySetInfo() throw(RuntimeException)
    {
        return OPropertySetAggregationHelper::createPropertySetInfo(getInfoHelper());
    }

    //--------------------------------------------------------------------
    Reference< XNameContainer > SAL_CALL OGeometryControlModel_Base::getEvents() throw(RuntimeException)
    {
        if( !mxEventContainer.is() )
            mxEventContainer = (XNameContainer*)new ScriptEventContainer();
        return mxEventContainer;
    }

//........................................................................
// }    // namespace toolkit
//........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/01/24 14:55:12  mt
 *  model for dialog controls (weith pos/size)
 *
 *
 *  Revision 1.0 17.01.01 11:35:20  fs
 ************************************************************************/

