/*************************************************************************
 *
 *  $RCSfile: geometrycontrolmodel.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: tbe $ $Date: 2001-03-22 15:34:23 $
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
#include <toolkit/helper/property.hxx>
#include <tools/debug.hxx>


#define GCM_PROPERTY_ID_POS_X       1
#define GCM_PROPERTY_ID_POS_Y       2
#define GCM_PROPERTY_ID_WIDTH       3
#define GCM_PROPERTY_ID_HEIGHT      4
#define GCM_PROPERTY_ID_NAME        5
#define GCM_PROPERTY_ID_TABINDEX    6
#define GCM_PROPERTY_ID_STEP        7
#define GCM_PROPERTY_ID_TAG         8
#define GCM_PROPERTY_ID_HELPTEXT    9

#define GCM_PROPERTY_POS_X      ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionX"))
#define GCM_PROPERTY_POS_Y      ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionY"))
#define GCM_PROPERTY_WIDTH      ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Width"))
#define GCM_PROPERTY_HEIGHT     ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Height"))
#define GCM_PROPERTY_NAME       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name"))
#define GCM_PROPERTY_TABINDEX   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TabIndex"))
#define GCM_PROPERTY_STEP       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Step"))
#define GCM_PROPERTY_TAG        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Tag"))
#define GCM_PROPERTY_HELPTEXT   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HelpText"))

#define DEFAULT_ATTRIBS()       PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT

//........................................................................
// namespace toolkit
// {
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::container;
    using namespace ::comphelper;

    //====================================================================
    //= OGeometryControlModel_Base
    //====================================================================
    //--------------------------------------------------------------------
    OGeometryControlModel_Base::OGeometryControlModel_Base(::com::sun::star::uno::XAggregation* _pAggregateInstance)
        :OPropertySetAggregationHelper(m_aBHelper)
        ,OPropertyContainer(m_aBHelper)
        ,m_nPosX(0)
        ,m_nPosY(0)
        ,m_nWidth(0)
        ,m_nHeight(0)
        ,m_bCloneable(sal_False)
        ,m_aName(::rtl::OUString())
        ,m_nTabIndex(0)
        ,m_nStep(0)
        ,m_aTag(::rtl::OUString())
        ,m_aHelpText(::rtl::OUString())
    {
        OSL_ENSURE(NULL != _pAggregateInstance, "OGeometryControlModel_Base::OGeometryControlModel_Base: invalid aggregate!");

        increment(m_refCount);
        {
            m_xAggregate = _pAggregateInstance;

            {   // check if the aggregat is cloneable
                Reference< XCloneable > xCloneAccess(m_xAggregate, UNO_QUERY);
                m_bCloneable = xCloneAccess.is();
            }

            setAggregation(m_xAggregate);
            m_xAggregate->setDelegator(static_cast< XWeak* >(this));
        }
        decrement(m_refCount);

        registerProperties();
    }

    //--------------------------------------------------------------------
    OGeometryControlModel_Base::OGeometryControlModel_Base(Reference< XCloneable >& _rxAggregateInstance)
        :OPropertySetAggregationHelper(m_aBHelper)
        ,OPropertyContainer(m_aBHelper)
        ,m_nPosX(0)
        ,m_nPosY(0)
        ,m_nWidth(0)
        ,m_nHeight(0)
        ,m_bCloneable(_rxAggregateInstance.is())
    {
        increment(m_refCount);
        {
            m_xAggregate = Reference< XAggregation >(_rxAggregateInstance, UNO_QUERY);
            OSL_ENSURE(m_xAggregate.is(), "OGeometryControlModel_Base::OGeometryControlModel_Base: invalid object given!");

            // now the aggregate has a ref count of 2, but before setting the delegator it must be 1
            _rxAggregateInstance.clear();
            // now it should be the 1 we need here ...

            setAggregation(m_xAggregate);
            m_xAggregate->setDelegator(static_cast< XWeak* >(this));
        }
        decrement(m_refCount);

        registerProperties();
    }

    //--------------------------------------------------------------------
    void OGeometryControlModel_Base::registerProperties()
    {
        // register our members for the property handling of the OPropertyContainer
        registerProperty(GCM_PROPERTY_POS_X,    GCM_PROPERTY_ID_POS_X,      DEFAULT_ATTRIBS(), &m_nPosX, ::getCppuType(&m_nPosX));
        registerProperty(GCM_PROPERTY_POS_Y,    GCM_PROPERTY_ID_POS_Y,      DEFAULT_ATTRIBS(), &m_nPosY, ::getCppuType(&m_nPosY));
        registerProperty(GCM_PROPERTY_WIDTH,    GCM_PROPERTY_ID_WIDTH,      DEFAULT_ATTRIBS(), &m_nWidth, ::getCppuType(&m_nWidth));
        registerProperty(GCM_PROPERTY_HEIGHT,   GCM_PROPERTY_ID_HEIGHT,     DEFAULT_ATTRIBS(), &m_nHeight, ::getCppuType(&m_nHeight));
        registerProperty(GCM_PROPERTY_NAME,     GCM_PROPERTY_ID_NAME,       DEFAULT_ATTRIBS(), &m_aName, ::getCppuType(&m_aName));
        registerProperty(GCM_PROPERTY_TABINDEX, GCM_PROPERTY_ID_TABINDEX,   DEFAULT_ATTRIBS(), &m_nTabIndex, ::getCppuType(&m_nTabIndex));
        registerProperty(GCM_PROPERTY_STEP,     GCM_PROPERTY_ID_STEP,       DEFAULT_ATTRIBS(), &m_nStep, ::getCppuType(&m_nStep));
        registerProperty(GCM_PROPERTY_TAG,      GCM_PROPERTY_ID_TAG,        DEFAULT_ATTRIBS(), &m_aTag, ::getCppuType(&m_aTag));
        registerProperty(GCM_PROPERTY_HELPTEXT, GCM_PROPERTY_ID_HELPTEXT,   DEFAULT_ATTRIBS(), &m_aHelpText, ::getCppuType(&m_aHelpText));
    }

    //--------------------------------------------------------------------
    ::com::sun::star::uno::Any OGeometryControlModel_Base::ImplGetDefaultValueByHandle(sal_Int32 nHandle) const
    {
        ::com::sun::star::uno::Any aDefault;

        switch ( nHandle )
        {
            case GCM_PROPERTY_ID_POS_X:         aDefault <<= (sal_Int32) 0; break;
            case GCM_PROPERTY_ID_POS_Y:         aDefault <<= (sal_Int32) 0; break;
            case GCM_PROPERTY_ID_WIDTH:         aDefault <<= (sal_Int32) 0; break;
            case GCM_PROPERTY_ID_HEIGHT:        aDefault <<= (sal_Int32) 0; break;
            case GCM_PROPERTY_ID_NAME:          aDefault <<= ::rtl::OUString(); break;
            case GCM_PROPERTY_ID_TABINDEX:      aDefault <<= (sal_Int16) 0; break;
            case GCM_PROPERTY_ID_STEP:          aDefault <<= (sal_Int32) 0; break;
            case GCM_PROPERTY_ID_TAG:           aDefault <<= ::rtl::OUString(); break;
            case GCM_PROPERTY_ID_HELPTEXT:      aDefault <<= ::rtl::OUString(); break;
            default:                            DBG_ERROR( "ImplGetDefaultValueByHandle - unknown Property" );
        }

        return aDefault;
    }

    //--------------------------------------------------------------------
    ::com::sun::star::uno::Any OGeometryControlModel_Base::ImplGetPropertyValueByHandle(sal_Int32 nHandle) const
    {
        ::com::sun::star::uno::Any aValue;

        switch ( nHandle )
        {
            case GCM_PROPERTY_ID_POS_X:         aValue <<= m_nPosX; break;
            case GCM_PROPERTY_ID_POS_Y:         aValue <<= m_nPosY; break;
            case GCM_PROPERTY_ID_WIDTH:         aValue <<= m_nWidth; break;
            case GCM_PROPERTY_ID_HEIGHT:        aValue <<= m_nHeight; break;
            case GCM_PROPERTY_ID_NAME:          aValue <<= m_aName; break;
            case GCM_PROPERTY_ID_TABINDEX:      aValue <<= m_nTabIndex; break;
            case GCM_PROPERTY_ID_STEP:          aValue <<= m_nStep; break;
            case GCM_PROPERTY_ID_TAG:           aValue <<= m_aTag; break;
            case GCM_PROPERTY_ID_HELPTEXT:      aValue <<= m_aHelpText; break;
            default:                            DBG_ERROR( "ImplGetPropertyValueByHandle - unknown Property" );
        }

        return aValue;
    }

    //--------------------------------------------------------------------
    void OGeometryControlModel_Base::ImplSetPropertyValueByHandle(sal_Int32 nHandle, const :: com::sun::star::uno::Any& aValue)
    {
        switch ( nHandle )
        {
            case GCM_PROPERTY_ID_POS_X:         aValue >>= m_nPosX; break;
            case GCM_PROPERTY_ID_POS_Y:         aValue >>= m_nPosY; break;
            case GCM_PROPERTY_ID_WIDTH:         aValue >>= m_nWidth; break;
            case GCM_PROPERTY_ID_HEIGHT:        aValue >>= m_nHeight; break;
            case GCM_PROPERTY_ID_NAME:          aValue >>= m_aName; break;
            case GCM_PROPERTY_ID_TABINDEX:      aValue >>= m_nTabIndex; break;
            case GCM_PROPERTY_ID_STEP:          aValue >>= m_nStep; break;
            case GCM_PROPERTY_ID_TAG:           aValue >>= m_aTag; break;
            case GCM_PROPERTY_ID_HELPTEXT:      aValue >>= m_aHelpText; break;
            default:                            DBG_ERROR( "ImplSetPropertyValueByHandle - unknown Property" );
        }
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OGeometryControlModel_Base::queryAggregation( const Type& _rType ) throw(RuntimeException)
    {
        Any aReturn;
        if (_rType.equals(::getCppuType(static_cast< Reference< XCloneable>* >(NULL))) && !m_bCloneable)
            // somebody is asking for the XCloneable interface, but our aggregate does not support it
            // -> outta here
            // (need this extra check, cause OGCM_Base::queryAggregation would return this interface
            // in every case)
            return aReturn;

        aReturn = OGCM_Base::queryAggregation(_rType);
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
        return OGCM_Base::queryInterface(_rType);
    }

    //--------------------------------------------------------------------
    void SAL_CALL OGeometryControlModel_Base::acquire(  ) throw()
    {
        OGCM_Base::acquire();
    }

    //--------------------------------------------------------------------
    void SAL_CALL OGeometryControlModel_Base::release(  ) throw()
    {
        OGCM_Base::release();
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
    ::com::sun::star::beans::PropertyState OGeometryControlModel_Base::getPropertyStateByHandle(sal_Int32 nHandle)
    {
        ::com::sun::star::uno::Any aValue = ImplGetPropertyValueByHandle( nHandle );
        ::com::sun::star::uno::Any aDefault = ImplGetDefaultValueByHandle( nHandle );

        return CompareProperties( aValue, aDefault ) ? ::com::sun::star::beans::PropertyState_DEFAULT_VALUE : ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
    }

    //--------------------------------------------------------------------
    void OGeometryControlModel_Base::setPropertyToDefaultByHandle(sal_Int32 nHandle)
    {
        ImplSetPropertyValueByHandle( nHandle , ImplGetDefaultValueByHandle( nHandle ) );
    }

    //--------------------------------------------------------------------
    ::com::sun::star::uno::Any OGeometryControlModel_Base::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
    {
        return ImplGetDefaultValueByHandle( nHandle );
    }

    //--------------------------------------------------------------------
    Reference< XPropertySetInfo> SAL_CALL OGeometryControlModel_Base::getPropertySetInfo() throw(RuntimeException)
    {
        return OPropertySetAggregationHelper::createPropertySetInfo(getInfoHelper());
    }

    //--------------------------------------------------------------------
    Reference< XCloneable > SAL_CALL OGeometryControlModel_Base::createClone(  ) throw(RuntimeException)
    {
        OSL_ENSURE(m_bCloneable, "OGeometryControlModel_Base::createClone: invalid call!");
        if (!m_bCloneable)
            return Reference< XCloneable >();

        // let the aggregate create it's own clone
        // the interface
        Reference< XCloneable > xCloneAccess;
        m_xAggregate->queryAggregation(::getCppuType(&xCloneAccess)) >>= xCloneAccess;
        OSL_ENSURE(xCloneAccess.is(), "OGeometryControlModel_Base::createClone: suspicious aggregate!");
        if (!xCloneAccess.is())
            return Reference< XCloneable >();
        // the aggregate's clone
        Reference< XCloneable > xAggregateClone = xCloneAccess->createClone();
        OSL_ENSURE(xAggregateClone.is(), "OGeometryControlModel_Base::createClone: suspicious return of the aggregate!");

        // create a new wrapper aggregating this return value
        OGeometryControlModel_Base* pOwnClone = createClone_Impl(xAggregateClone);
        OSL_ENSURE(pOwnClone, "OGeometryControlModel_Base::createClone: invalid derivee behaviour!");
        OSL_ENSURE(!xAggregateClone.is(), "OGeometryControlModel_Base::createClone: invalid ctor behaviour!");
            // should have been reset

        // set properties
        pOwnClone->m_nPosX      = m_nPosX;
        pOwnClone->m_nPosY      = m_nPosY;
        pOwnClone->m_nWidth     = m_nWidth;
        pOwnClone->m_nHeight    = m_nHeight;
        pOwnClone->m_aName      = m_aName;
        pOwnClone->m_nTabIndex  = m_nTabIndex;
        pOwnClone->m_nStep      = m_nStep;
        pOwnClone->m_aTag       = m_aTag;
        pOwnClone->m_aHelpText  = m_aHelpText;

        return pOwnClone;
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
 *  Revision 1.7  2001/03/08 16:44:03  tbe
 *  OPropertyStateHelper overridables
 *
 *  Revision 1.6  2001/03/07 14:27:23  tbe
 *  added step and tag property
 *
 *  Revision 1.5  2001/03/02 12:33:21  tbe
 *  clone geometry control model
 *
 *  Revision 1.4  2001/03/01 14:26:33  tbe
 *  removed ClassId from geometry control model
 *
 *  Revision 1.3  2001/02/28 10:49:53  tbe
 *  added additional properties to geometry model
 *
 *  Revision 1.2  2001/02/21 17:31:20  ab
 *  Support for XScriptEventsSupplier added
 *
 *  Revision 1.1  2001/01/24 14:55:12  mt
 *  model for dialog controls (weith pos/size)
 *
 *
 *  Revision 1.0 17.01.01 11:35:20  fs
 ************************************************************************/

