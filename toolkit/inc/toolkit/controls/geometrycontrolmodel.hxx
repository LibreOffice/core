/*************************************************************************
 *
 *  $RCSfile: geometrycontrolmodel.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 17:10:01 $
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
#define _TOOLKIT_HELPERS_GEOMETRYCONTROLMODEL_HXX_

#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_AGGREGATION_HXX_
#include <comphelper/propagg.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _CPPUHELPER_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSCRIPTEVENTSSUPPLIER_HPP_
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef COMPHELPER_IDPROPERTYARRAYUSAGEHELPER_HXX
#include <comphelper/IdPropArrayHelper.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

FORWARD_DECLARE_INTERFACE( lang, XMultiServiceFactory )
FORWARD_DECLARE_INTERFACE( script, XNameContainer )

//........................................................................
// namespace toolkit
// {
//........................................................................

    //====================================================================
    //= OGeometryControlModel_Base
    //====================================================================
    typedef ::cppu::WeakAggComponentImplHelper2 <   ::com::sun::star::util::XCloneable
                                                ,   ::com::sun::star::script::XScriptEventsSupplier
                                                >   OGCM_Base;
    class OGeometryControlModel_Base
        :public ::comphelper::OMutexAndBroadcastHelper
        ,public ::comphelper::OPropertySetAggregationHelper
        ,public ::comphelper::OPropertyContainer
        ,public OGCM_Base
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >
                    m_xAggregate;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                    mxEventContainer;

        // <properties>
        sal_Int32       m_nPosX;
        sal_Int32       m_nPosY;
        sal_Int32       m_nWidth;
        sal_Int32       m_nHeight;
        ::rtl::OUString m_aName;
        sal_Int16       m_nTabIndex;
        sal_Int32       m_nStep;
        ::rtl::OUString m_aTag;
        // </properties>

        sal_Bool        m_bCloneable;

    protected:
        virtual ::com::sun::star::uno::Any          ImplGetDefaultValueByHandle(sal_Int32 nHandle) const;
        virtual ::com::sun::star::uno::Any          ImplGetPropertyValueByHandle(sal_Int32 nHandle) const;
        virtual void                                ImplSetPropertyValueByHandle(sal_Int32 nHandle, const :: com::sun::star::uno::Any& aValue);

    protected:
        /**
            @param _pAggregateInstance
                the object to be aggregated. The refcount of the instance given MUST be 0!
        */
        OGeometryControlModel_Base(::com::sun::star::uno::XAggregation* _pAggregateInstance);

        /**
            @param _rxAggregateInstance
                is the object to be aggregated. Must be aquired excatly once (by the reference object given).<br/>
                Will be reset to NULL upon leaving
        */
        OGeometryControlModel_Base(::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance);

        /** releases the aggregation
            <p>Can be used if in a derived class, an exception has to be thrown after this base class here already
            did the aggregation</p>
        */
        void releaseAggregation();

    protected:
        ~OGeometryControlModel_Base();

        // XAggregation
        ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _aType ) throw(::com::sun::star::uno::RuntimeException);

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire(  ) throw();
        virtual void SAL_CALL release(  ) throw();

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);

        // OPropertySetHelper overridables
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue,
                sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
            throw (::com::sun::star::lang::IllegalArgumentException);

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue)
            throw (::com::sun::star::uno::Exception);

        virtual void SAL_CALL getFastPropertyValue(
            ::com::sun::star::uno::Any& _rValue, sal_Int32 _nHandle) const;

        // OPropertyStateHelper overridables
        virtual ::com::sun::star::beans::PropertyState  getPropertyStateByHandle(sal_Int32 nHandle);
        virtual void                                    setPropertyToDefaultByHandle(sal_Int32 nHandle);
        virtual ::com::sun::star::uno::Any              getPropertyDefaultByHandle(sal_Int32 nHandle) const;

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);

        // OPropertySetAggregationHelper overridables
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() = 0;

        // XCloneable
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw(::com::sun::star::uno::RuntimeException);

        //XScriptEventsSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
            SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException);

        // XCloneable implementation - to be overwritten
        virtual OGeometryControlModel_Base* createClone_Impl(
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance) = 0;

        // XComponent
        virtual void SAL_CALL disposing();

    private:
        void registerProperties();
    };

    //====================================================================
    //= OTemplateInstanceDisambiguation
    //====================================================================
    template <class CONTROLMODEL>
    class OTemplateInstanceDisambiguation
    {
    };

    //====================================================================
    //= OGeometryControlModel
    //====================================================================
    /*  example for usage:
            Reference< XAggregation > xIFace = new ::toolkit::OGeometryControlModel< UnoControlButtonModel > ();
    */
    template <class CONTROLMODEL>
    class OGeometryControlModel
        :public OGeometryControlModel_Base
        ,public ::comphelper::OAggregationArrayUsageHelper< OTemplateInstanceDisambiguation< CONTROLMODEL > >
    {
    public:
        OGeometryControlModel();

    private:
        OGeometryControlModel(::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance);

    protected:
        // OAggregationArrayUsageHelper overridables
        virtual void fillProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& _rProps,
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& _rAggregateProps
            ) const;

        // OPropertySetAggregationHelper overridables
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OGeometryControlModel_Base
        virtual OGeometryControlModel_Base* createClone_Impl(
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance);

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);
    };

    //====================================================================
    //= OCommonGeometryControlModel
    //====================================================================
    /** allows to extend an arbitrary <type scope="com.sun.star.awt">UnoControlModel</type> with geometry
        information.
    */
    class OCommonGeometryControlModel
        :public OGeometryControlModel_Base
        ,public ::comphelper::OIdPropertyArrayUsageHelper< OCommonGeometryControlModel >
    {
    private:
        ::rtl::OUString m_sServiceSpecifier;        // the service specifier of our aggregate
        sal_Int32       m_nPropertyMapId;           // our unique property info id, used to look up in s_aAggregateProperties

    public:
        /** instantiate the model

            @param _rxAgg
                the instance to aggregate. Must support the <type scope="com.sun.star.awt">UnoControlModel</type>
                (this is not checked here)
        */
        OCommonGeometryControlModel(
                    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAgg,
            const   ::rtl::OUString& _rxServiceSpecifier
        );

        // OIdPropertyArrayUsageHelper overridables
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const;

        // OPropertySetAggregationHelper overridables
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OGeometryControlModel_Base
        virtual OGeometryControlModel_Base* createClone_Impl(
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance);

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);

    private:
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue)
            throw (::com::sun::star::uno::Exception);
    };

#include "toolkit/controls/geometrycontrolmodel_impl.hxx"

//........................................................................
// }    // namespace toolkit
//........................................................................

#endif // _TOOLKIT_HELPERS_GEOMETRYCONTROLMODEL_HXX_
