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

#include <toolkit/controls/geometrycontrolmodel.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <osl/diagnose.h>
#include <rtl/instance.hxx>
#include <comphelper/sequence.hxx>
#include <toolkit/controls/eventcontainer.hxx>
#include <toolkit/helper/property.hxx>
#include <algorithm>
#include <functional>


#define GCM_PROPERTY_ID_POS_X               1
#define GCM_PROPERTY_ID_POS_Y               2
#define GCM_PROPERTY_ID_WIDTH               3
#define GCM_PROPERTY_ID_HEIGHT              4
#define GCM_PROPERTY_ID_NAME                5
#define GCM_PROPERTY_ID_TABINDEX            6
#define GCM_PROPERTY_ID_STEP                7
#define GCM_PROPERTY_ID_TAG                 8
#define GCM_PROPERTY_ID_RESOURCERESOLVER    9

#define GCM_PROPERTY_POS_X              "PositionX"
#define GCM_PROPERTY_POS_Y              "PositionY"
#define GCM_PROPERTY_WIDTH              "Width"
#define GCM_PROPERTY_HEIGHT             "Height"
#define GCM_PROPERTY_NAME               "Name"
#define GCM_PROPERTY_TABINDEX           "TabIndex"
#define GCM_PROPERTY_STEP               "Step"
#define GCM_PROPERTY_TAG                "Tag"
#define GCM_PROPERTY_RESOURCERESOLVER   "ResourceResolver"

#define DEFAULT_ATTRIBS()       PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT


// namespace toolkit
// {


    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::container;
    using namespace ::comphelper;


    //= OGeometryControlModel_Base


    OGeometryControlModel_Base::OGeometryControlModel_Base(css::uno::XAggregation* _pAggregateInstance)
        :OPropertySetAggregationHelper( m_aBHelper )
        ,OPropertyContainer( m_aBHelper )
        ,OGCM_Base( m_aMutex )
        ,m_nPosX(0)
        ,m_nPosY(0)
        ,m_nWidth(0)
        ,m_nHeight(0)
        ,m_nTabIndex(-1)
        ,m_nStep(0)
        ,m_bCloneable(false)
    {
        OSL_ENSURE(nullptr != _pAggregateInstance, "OGeometryControlModel_Base::OGeometryControlModel_Base: invalid aggregate!");

        osl_atomic_increment(&m_refCount);
        {
            m_xAggregate = _pAggregateInstance;

            {   // check if the aggregate is cloneable
                Reference< XCloneable > xCloneAccess(m_xAggregate, UNO_QUERY);
                m_bCloneable = xCloneAccess.is();
            }

            setAggregation(m_xAggregate);
            m_xAggregate->setDelegator(static_cast< XWeak* >(this));
        }
        osl_atomic_decrement(&m_refCount);

        registerProperties();
    }


    OGeometryControlModel_Base::OGeometryControlModel_Base(Reference< XCloneable >& _rxAggregateInstance)
        :OPropertySetAggregationHelper( m_aBHelper )
        ,OPropertyContainer( m_aBHelper )
        ,OGCM_Base( m_aMutex )
        ,m_nPosX(0)
        ,m_nPosY(0)
        ,m_nWidth(0)
        ,m_nHeight(0)
        ,m_nTabIndex(-1)
        ,m_nStep(0)
        ,m_bCloneable(_rxAggregateInstance.is())
    {
        osl_atomic_increment(&m_refCount);
        {
            {
                // ensure that the temporary gets destructed NOW
                m_xAggregate.set(_rxAggregateInstance, UNO_QUERY);
            }
            OSL_ENSURE(m_xAggregate.is(), "OGeometryControlModel_Base::OGeometryControlModel_Base: invalid object given!");

            // now the aggregate has a ref count of 2, but before setting the delegator it must be 1
            _rxAggregateInstance.clear();
            // now it should be the 1 we need here ...

            setAggregation(m_xAggregate);
            m_xAggregate->setDelegator(static_cast< XWeak* >(this));
        }
        osl_atomic_decrement(&m_refCount);

        registerProperties();
    }


    Sequence< Type > SAL_CALL OGeometryControlModel_Base::getTypes(  )
    {
        // our own types
        Sequence< Type > aTypes = ::comphelper::concatSequences(
            OPropertySetAggregationHelper::getTypes(),
            getBaseTypes(),
            OGCM_Base::getTypes()
        );

        if ( m_xAggregate.is() )
        {
            // retrieve the types of the aggregate
            Reference< XTypeProvider > xAggregateTypeProv;
            m_xAggregate->queryAggregation( cppu::UnoType<decltype(xAggregateTypeProv)>::get() ) >>= xAggregateTypeProv;
            OSL_ENSURE( xAggregateTypeProv.is(), "OGeometryControlModel_Base::getTypes: aggregate should be a type provider!" );
            Sequence< Type > aAggTypes;
            if ( xAggregateTypeProv.is() )
                aAggTypes = xAggregateTypeProv->getTypes();

            // concat the sequences
            sal_Int32 nOldSize = aTypes.getLength();
            aTypes.realloc( nOldSize + aAggTypes.getLength() );
            ::std::copy(
                aAggTypes.begin(),
                aAggTypes.end(),
                aTypes.getArray() + nOldSize
            );
        }

        return aTypes;
    }


    void OGeometryControlModel_Base::registerProperties()
    {
        // register our members for the property handling of the OPropertyContainer
        registerProperty(GCM_PROPERTY_POS_X,    GCM_PROPERTY_ID_POS_X,      DEFAULT_ATTRIBS(), &m_nPosX, cppu::UnoType<decltype(m_nPosX)>::get());
        registerProperty(GCM_PROPERTY_POS_Y,    GCM_PROPERTY_ID_POS_Y,      DEFAULT_ATTRIBS(), &m_nPosY, cppu::UnoType<decltype(m_nPosY)>::get());
        registerProperty(GCM_PROPERTY_WIDTH,    GCM_PROPERTY_ID_WIDTH,      DEFAULT_ATTRIBS(), &m_nWidth, cppu::UnoType<decltype(m_nWidth)>::get());
        registerProperty(GCM_PROPERTY_HEIGHT,   GCM_PROPERTY_ID_HEIGHT,     DEFAULT_ATTRIBS(), &m_nHeight, cppu::UnoType<decltype(m_nHeight)>::get());
        registerProperty(GCM_PROPERTY_NAME,     GCM_PROPERTY_ID_NAME,       DEFAULT_ATTRIBS(), &m_aName, cppu::UnoType<decltype(m_aName)>::get());
        registerProperty(GCM_PROPERTY_TABINDEX, GCM_PROPERTY_ID_TABINDEX,   DEFAULT_ATTRIBS(), &m_nTabIndex, cppu::UnoType<decltype(m_nTabIndex)>::get());
        registerProperty(GCM_PROPERTY_STEP,     GCM_PROPERTY_ID_STEP,       DEFAULT_ATTRIBS(), &m_nStep, cppu::UnoType<decltype(m_nStep)>::get());
        registerProperty(GCM_PROPERTY_TAG,      GCM_PROPERTY_ID_TAG,        DEFAULT_ATTRIBS(), &m_aTag, cppu::UnoType<decltype(m_aTag)>::get());
        registerProperty(GCM_PROPERTY_RESOURCERESOLVER, GCM_PROPERTY_ID_RESOURCERESOLVER, DEFAULT_ATTRIBS(), &m_xStrResolver, cppu::UnoType<decltype(m_xStrResolver)>::get());
    }


    css::uno::Any OGeometryControlModel_Base::ImplGetDefaultValueByHandle(sal_Int32 nHandle)
    {
        css::uno::Any aDefault;

        switch ( nHandle )
        {
            case GCM_PROPERTY_ID_POS_X:             aDefault <<= sal_Int32(0); break;
            case GCM_PROPERTY_ID_POS_Y:             aDefault <<= sal_Int32(0); break;
            case GCM_PROPERTY_ID_WIDTH:             aDefault <<= sal_Int32(0); break;
            case GCM_PROPERTY_ID_HEIGHT:            aDefault <<= sal_Int32(0); break;
            case GCM_PROPERTY_ID_NAME:              aDefault <<= OUString(); break;
            case GCM_PROPERTY_ID_TABINDEX:          aDefault <<= sal_Int16(-1); break;
            case GCM_PROPERTY_ID_STEP:              aDefault <<= sal_Int32(0); break;
            case GCM_PROPERTY_ID_TAG:               aDefault <<= OUString(); break;
            case GCM_PROPERTY_ID_RESOURCERESOLVER:  aDefault <<= Reference< resource::XStringResourceResolver >(); break;
            default:                            OSL_FAIL( "ImplGetDefaultValueByHandle - unknown Property" );
        }

        return aDefault;
    }


    css::uno::Any OGeometryControlModel_Base::ImplGetPropertyValueByHandle(sal_Int32 nHandle) const
    {
        css::uno::Any aValue;

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
            case GCM_PROPERTY_ID_RESOURCERESOLVER: aValue <<= m_xStrResolver; break;
            default:                            OSL_FAIL( "ImplGetPropertyValueByHandle - unknown Property" );
        }

        return aValue;
    }


    void OGeometryControlModel_Base::ImplSetPropertyValueByHandle(sal_Int32 nHandle, const css::uno::Any& aValue)
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
            case GCM_PROPERTY_ID_RESOURCERESOLVER: aValue >>= m_xStrResolver; break;
            default:                            OSL_FAIL( "ImplSetPropertyValueByHandle - unknown Property" );
        }
    }


    Any SAL_CALL OGeometryControlModel_Base::queryAggregation( const Type& _rType )
    {
        Any aReturn;
        if (_rType.equals(cppu::UnoType<XCloneable>::get()) && !m_bCloneable)
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

        return aReturn;
    }


    Any SAL_CALL OGeometryControlModel_Base::queryInterface( const Type& _rType )
    {
        return OGCM_Base::queryInterface(_rType);
    }


    void SAL_CALL OGeometryControlModel_Base::acquire(  ) throw()
    {
        OGCM_Base::acquire();
    }


    void SAL_CALL OGeometryControlModel_Base::release(  ) throw()
    {
        OGCM_Base::release();
    }


    void OGeometryControlModel_Base::releaseAggregation()
    {
        // release the aggregate (_before_ clearing m_xAggregate)
        if (m_xAggregate.is())
            m_xAggregate->setDelegator(nullptr);
        setAggregation(nullptr);
    }


    OGeometryControlModel_Base::~OGeometryControlModel_Base()
    {
        releaseAggregation();
    }


    sal_Bool SAL_CALL OGeometryControlModel_Base::convertFastPropertyValue(Any& _rConvertedValue, Any& _rOldValue,
            sal_Int32 _nHandle, const Any& _rValue)
    {
        return OPropertyContainer::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
    }


    void SAL_CALL OGeometryControlModel_Base::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue)
    {
        OPropertyContainer::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    }


    void SAL_CALL OGeometryControlModel_Base::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
    {
        OPropertyArrayAggregationHelper& rPH = static_cast<OPropertyArrayAggregationHelper&>(const_cast<OGeometryControlModel_Base*>(this)->getInfoHelper());
        OUString sPropName;
        sal_Int32   nOriginalHandle = -1;

        if (rPH.fillAggregatePropertyInfoByHandle(&sPropName, &nOriginalHandle, _nHandle))
            OPropertySetAggregationHelper::getFastPropertyValue(_rValue, _nHandle);
        else
            OPropertyContainer::getFastPropertyValue(_rValue, _nHandle);
    }


    css::beans::PropertyState OGeometryControlModel_Base::getPropertyStateByHandle(sal_Int32 nHandle)
    {
        css::uno::Any aValue = ImplGetPropertyValueByHandle( nHandle );
        css::uno::Any aDefault = ImplGetDefaultValueByHandle( nHandle );

        return CompareProperties( aValue, aDefault ) ? css::beans::PropertyState_DEFAULT_VALUE : css::beans::PropertyState_DIRECT_VALUE;
    }


    void OGeometryControlModel_Base::setPropertyToDefaultByHandle(sal_Int32 nHandle)
    {
        ImplSetPropertyValueByHandle( nHandle , ImplGetDefaultValueByHandle( nHandle ) );
    }


    css::uno::Any OGeometryControlModel_Base::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
    {
        return ImplGetDefaultValueByHandle( nHandle );
    }


    Reference< XPropertySetInfo> SAL_CALL OGeometryControlModel_Base::getPropertySetInfo()
    {
        return OPropertySetAggregationHelper::createPropertySetInfo(getInfoHelper());
    }


    Reference< XCloneable > SAL_CALL OGeometryControlModel_Base::createClone(  )
    {
        OSL_ENSURE(m_bCloneable, "OGeometryControlModel_Base::createClone: invalid call!");
        if (!m_bCloneable)
            return Reference< XCloneable >();

        // let the aggregate create its own clone
        // the interface
        Reference< XCloneable > xCloneAccess;
        m_xAggregate->queryAggregation(cppu::UnoType<decltype(xCloneAccess)>::get()) >>= xCloneAccess;
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


        // Clone event container
        Reference< css::script::XScriptEventsSupplier > xEventsSupplier =
            static_cast< css::script::XScriptEventsSupplier* >( this );
        Reference< css::script::XScriptEventsSupplier > xCloneEventsSupplier =
            static_cast< css::script::XScriptEventsSupplier* >( pOwnClone );

        if( xEventsSupplier.is() && xCloneEventsSupplier.is() )
        {
            Reference< XNameContainer > xEventCont = xEventsSupplier->getEvents();
            Reference< XNameContainer > xCloneEventCont = xCloneEventsSupplier->getEvents();

            css::uno::Sequence< OUString > aNames =
                xEventCont->getElementNames();
            const OUString* pNames = aNames.getConstArray();
            sal_Int32 i, nNameCount = aNames.getLength();

            for( i = 0 ; i < nNameCount ; i++ )
            {
                OUString aName = pNames[ i ];
                css::uno::Any aElement = xEventCont->getByName( aName );
                xCloneEventCont->insertByName( aName, aElement );
            }
        }

        return pOwnClone;
    }


    Reference< XNameContainer > SAL_CALL OGeometryControlModel_Base::getEvents()
    {
        if( !mxEventContainer.is() )
            mxEventContainer = new toolkit::ScriptEventContainer();
        return mxEventContainer;
    }


    void SAL_CALL OGeometryControlModel_Base::disposing()
    {
        OGCM_Base::disposing();
        OPropertySetAggregationHelper::disposing();

        Reference<XComponent>  xComp;
        if ( query_aggregation( m_xAggregate, xComp ) )
            xComp->dispose();
    }


    //= OCommonGeometryControlModel


    typedef std::unordered_map< OUString, sal_Int32 > HashMapString2Int;
    typedef std::vector< css::uno::Sequence< css::beans::Property > >   PropSeqArray;
    typedef std::vector< ::std::vector< sal_Int32 > > IntArrayArray;

    // for creating class-unique PropertySetInfo's, we need some info:
    namespace { struct ServiceSpecifierMap : public rtl::Static< HashMapString2Int, ServiceSpecifierMap > {}; }
    // this one maps from a String, which is the service specifier for our
    // aggregate, to a unique id

    namespace { struct AggregateProperties : public rtl::Static< PropSeqArray, AggregateProperties > {}; }
    // this one contains the properties which belong to all the unique ids
    // in ServiceSpecifierMap

    namespace { struct AmbiguousPropertyIds : public rtl::Static< IntArrayArray, AmbiguousPropertyIds > {}; }
    // the ids of the properties which we as well as our aggregate supply
    // For such props, we let our base class handle them, and whenever such
    // a prop is set, we forward this to our aggregate.

    // With this, we can ensure that two instances of this class share the
    // same PropertySetInfo if and only if both aggregates have the same
    // service specifier.


    OCommonGeometryControlModel::OCommonGeometryControlModel( Reference< XCloneable >& _rxAgg, const OUString& _rServiceSpecifier )
        :OGeometryControlModel_Base( _rxAgg )
        ,m_sServiceSpecifier( _rServiceSpecifier )
        ,m_nPropertyMapId( 0 )
    {
        Reference< XPropertySetInfo > xPI;
        if ( m_xAggregateSet.is() )
            xPI = m_xAggregateSet->getPropertySetInfo();
        if ( !xPI.is() )
        {
            releaseAggregation();
            throw IllegalArgumentException();
        }

        HashMapString2Int &rMap = ServiceSpecifierMap::get();
        HashMapString2Int::iterator aPropMapIdPos = rMap.find( m_sServiceSpecifier );
        if ( rMap.end() == aPropMapIdPos )
        {
            PropSeqArray &rAggProperties = AggregateProperties::get();
            m_nPropertyMapId = rAggProperties.size();
            rAggProperties.push_back( xPI->getProperties() );
            AmbiguousPropertyIds::get().emplace_back( );

            rMap[ m_sServiceSpecifier ] = m_nPropertyMapId;
        }
        else
            m_nPropertyMapId = aPropMapIdPos->second;
    }


    struct PropertyNameLess
    {
        bool operator()( const Property& _rLHS, const Property& _rRHS )
        {
            return _rLHS.Name < _rRHS.Name;
        }
    };


    struct PropertyNameEqual
    {
        const OUString&  m_rCompare;
        explicit PropertyNameEqual( const OUString& _rCompare ) : m_rCompare( _rCompare ) { }

        bool operator()( const Property& _rLHS )
        {
            return _rLHS.Name == m_rCompare;
        }
    };


    ::cppu::IPropertyArrayHelper* OCommonGeometryControlModel::createArrayHelper( sal_Int32 _nId ) const
    {
        OSL_ENSURE( _nId == m_nPropertyMapId, "OCommonGeometryControlModel::createArrayHelper: invalid argument!" );
        OSL_ENSURE( _nId < static_cast<sal_Int32>(AggregateProperties::get().size()), "OCommonGeometryControlModel::createArrayHelper: invalid status info (1)!" );
        OSL_ENSURE( _nId < static_cast<sal_Int32>(AmbiguousPropertyIds::get().size()), "OCommonGeometryControlModel::createArrayHelper: invalid status info (2)!" );

        // our own properties
        Sequence< Property > aProps;
        OPropertyContainer::describeProperties( aProps );

        // the aggregate properties
        Sequence< Property > aAggregateProps;
        aAggregateProps = AggregateProperties::get()[ _nId ];

        // look for duplicates, and remember them
        IntArrayArray::value_type& rDuplicateIds = AmbiguousPropertyIds::get()[ _nId ];
        // for this, sort the aggregate properties
        ::std::sort(
            aAggregateProps.begin(),
            aAggregateProps.end(),
            PropertyNameLess()
        );
        const Property* pAggProps = aAggregateProps.getConstArray();
        const Property* pAggPropsEnd = aAggregateProps.getConstArray() + aAggregateProps.getLength();

        // now loop through our own props
        const Property* pProp = aProps.getConstArray();
        const Property* pPropEnd = aProps.getConstArray() + aProps.getLength();
        while ( pProp < pPropEnd )
        {
            // look for the current property in the properties of our aggregate
            const Property* pAggPropPos = ::std::find_if( pAggProps, pAggPropsEnd, PropertyNameEqual( pProp->Name ) );
            if ( pAggPropPos != pAggPropsEnd )
            {   // found a duplicate
                // -> remove from the aggregate property sequence
                ::comphelper::removeElementAt( aAggregateProps, pAggPropPos - pAggProps );
                // which means we have to adjust the pointers
                pAggProps = aAggregateProps.getConstArray();
                pAggPropsEnd = aAggregateProps.getConstArray() + aAggregateProps.getLength();

                // and additionally, remember the id of this property
                rDuplicateIds.push_back( pProp->Handle );
            }

            ++pProp;
        }

        // now, finally, sort the duplicates
        ::std::sort( rDuplicateIds.begin(), rDuplicateIds.end(), ::std::less< sal_Int32 >() );

        return new OPropertyArrayAggregationHelper(aProps, aAggregateProps);
    }


    ::cppu::IPropertyArrayHelper& SAL_CALL OCommonGeometryControlModel::getInfoHelper()
    {
        return *getArrayHelper( m_nPropertyMapId );
    }


    OGeometryControlModel_Base* OCommonGeometryControlModel::createClone_Impl( Reference< XCloneable >& _rxAggregateInstance )
    {
        return new OCommonGeometryControlModel( _rxAggregateInstance, m_sServiceSpecifier );
    }

    Sequence< sal_Int8 > SAL_CALL OCommonGeometryControlModel::getImplementationId(  )
    {
        return css::uno::Sequence<sal_Int8>();
    }


    struct Int32Equal
    {
        sal_Int32 const   m_nCompare;
        explicit Int32Equal( sal_Int32 _nCompare ) : m_nCompare( _nCompare ) { }

        bool operator()( sal_Int32 _nLHS )
        {
            return _nLHS == m_nCompare;
        }
    };


    void SAL_CALL OCommonGeometryControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue )
    {
        OGeometryControlModel_Base::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );

        // look if this id is one we recognized as duplicate
        IntArrayArray::value_type& rDuplicateIds = AmbiguousPropertyIds::get()[ m_nPropertyMapId ];

        if ( std::any_of(rDuplicateIds.begin(), rDuplicateIds.end(), Int32Equal( _nHandle )) )
        {
            // yes, it is such a property
            OUString sPropName;
            sal_Int16 nAttributes(0);
            static_cast< OPropertyArrayAggregationHelper* >( getArrayHelper( m_nPropertyMapId ) )->fillPropertyMembersByHandle( &sPropName, &nAttributes, _nHandle );

            if ( m_xAggregateSet.is() && !sPropName.isEmpty() )
                m_xAggregateSet->setPropertyValue( sPropName, _rValue );
        }
    }


// }    // namespace toolkit


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
