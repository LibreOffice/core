/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/dataaccessdescriptor.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <tools/urlobj.hxx>

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::ucb;
    using namespace ::comphelper;

#define CONST_CHAR( propname ) propname, sizeof(propname) - 1

    //====================================================================
    //= ODADescriptorImpl
    //====================================================================
    class ODADescriptorImpl
    {
    protected:
        sal_Bool                    m_bSetOutOfDate         : 1;
        sal_Bool                    m_bSequenceOutOfDate    : 1;

    public:
        typedef ::std::map< DataAccessDescriptorProperty, Any >     DescriptorValues;
        DescriptorValues            m_aValues;
        Sequence< PropertyValue >   m_aAsSequence;
        Reference< XPropertySet >   m_xAsSet;

        typedef ::std::map< ::rtl::OUString, PropertyMapEntry* >    MapString2PropertyEntry;

    public:
        ODADescriptorImpl();
        ODADescriptorImpl(const ODADescriptorImpl& _rSource);

        void invalidateExternRepresentations();

        void updateSequence();
        void updateSet();

        /** builds the descriptor from a property value sequence
            @return <TRUE/>
                if and only if the sequence contained valid properties only
        */
        sal_Bool buildFrom( const Sequence< PropertyValue >& _rValues );

        /** builds the descriptor from a property set
            @return <TRUE/>
                if and only if the set contained valid properties only
        */
        sal_Bool buildFrom( const Reference< XPropertySet >& _rValues );

    protected:
        static PropertyValue                    buildPropertyValue( const DescriptorValues::const_iterator& _rPos );
        static const MapString2PropertyEntry&   getPropertyMap( );
        static PropertyMapEntry*                getPropertyMapEntry( const DescriptorValues::const_iterator& _rPos );
    };

    //--------------------------------------------------------------------
    ODADescriptorImpl::ODADescriptorImpl()
        :m_bSetOutOfDate(sal_True)
        ,m_bSequenceOutOfDate(sal_True)
    {
    }

    //--------------------------------------------------------------------
    ODADescriptorImpl::ODADescriptorImpl(const ODADescriptorImpl& _rSource)
        :m_bSetOutOfDate( _rSource.m_bSetOutOfDate )
        ,m_bSequenceOutOfDate( _rSource.m_bSequenceOutOfDate )
        ,m_aValues( _rSource.m_aValues )
    {
        if (!m_bSetOutOfDate)
            m_xAsSet = _rSource.m_xAsSet;
        if (!m_bSequenceOutOfDate)
            m_aAsSequence = _rSource.m_aAsSequence;
    }

    //--------------------------------------------------------------------
    sal_Bool ODADescriptorImpl::buildFrom( const Sequence< PropertyValue >& _rValues )
    {
        const MapString2PropertyEntry& rProperties = getPropertyMap();

        sal_Bool bValidPropsOnly = sal_True;

        // loop through the sequence, and fill our m_aValues
        const PropertyValue* pValues = _rValues.getConstArray();
        const PropertyValue* pValuesEnd = pValues + _rValues.getLength();
        for (;pValues != pValuesEnd; ++pValues)
        {
            MapString2PropertyEntry::const_iterator aPropPos = rProperties.find( pValues->Name );
            if ( aPropPos != rProperties.end() )
            {
                DataAccessDescriptorProperty eProperty = (DataAccessDescriptorProperty)aPropPos->second->mnHandle;
                m_aValues[eProperty] = pValues->Value;
            }
            else
                // unknown property
                bValidPropsOnly = sal_False;
        }

        if (bValidPropsOnly)
        {
            m_aAsSequence = _rValues;
            m_bSequenceOutOfDate = sal_False;
        }
        else
            m_bSequenceOutOfDate = sal_True;

        return bValidPropsOnly;
    }

    //--------------------------------------------------------------------
    sal_Bool ODADescriptorImpl::buildFrom( const Reference< XPropertySet >& _rxValues )
    {
        Reference< XPropertySetInfo > xPropInfo;
        if (_rxValues.is())
            xPropInfo = _rxValues->getPropertySetInfo();
        if (!xPropInfo.is())
        {
            OSL_ENSURE(sal_False, "ODADescriptorImpl::buildFrom: invalid property set!");
            return sal_False;
        }

        // build a PropertyValue sequence with the current values
        Sequence< Property > aProperties = xPropInfo->getProperties();
        const Property* pProperty = aProperties.getConstArray();
        const Property* pPropertyEnd = pProperty + aProperties.getLength();

        Sequence< PropertyValue > aValues(aProperties.getLength());
        PropertyValue* pValues = aValues.getArray();

        for (;pProperty != pPropertyEnd; ++pProperty, ++pValues)
        {
            pValues->Name = pProperty->Name;
            pValues->Value = _rxValues->getPropertyValue(pProperty->Name);
        }

        sal_Bool bValidPropsOnly = buildFrom(aValues);
        if (bValidPropsOnly)
        {
            m_xAsSet = _rxValues;
            m_bSetOutOfDate = sal_False;
        }
        else
            m_bSetOutOfDate = sal_True;

        return bValidPropsOnly;
    }

    //--------------------------------------------------------------------
    void ODADescriptorImpl::invalidateExternRepresentations()
    {
        m_bSetOutOfDate = sal_True;
        m_bSequenceOutOfDate = sal_True;
    }

    //--------------------------------------------------------------------
    const ODADescriptorImpl::MapString2PropertyEntry& ODADescriptorImpl::getPropertyMap( )
    {
        // the properties we know
        static MapString2PropertyEntry s_aProperties;
        if ( s_aProperties.empty() )
        {
            static PropertyMapEntry s_aDesriptorProperties[] =
            {
                { CONST_CHAR("ActiveConnection"),   daConnection,           &::getCppuType( static_cast< Reference< XConnection >* >(NULL) ),   PropertyAttribute::TRANSIENT, 0 },
                { CONST_CHAR("BookmarkSelection"),  daBookmarkSelection,    &::getBooleanCppuType( ),                                           PropertyAttribute::TRANSIENT, 0 },
                { CONST_CHAR("Column"),             daColumnObject,         &::getCppuType( static_cast< Reference< XPropertySet >* >(NULL) ),  PropertyAttribute::TRANSIENT, 0 },
                { CONST_CHAR("ColumnName"),         daColumnName,           &::getCppuType( static_cast< ::rtl::OUString* >(NULL) ),            PropertyAttribute::TRANSIENT, 0 },
                { CONST_CHAR("Command"),            daCommand,              &::getCppuType( static_cast< ::rtl::OUString* >(NULL) ),            PropertyAttribute::TRANSIENT, 0 },
                { CONST_CHAR("CommandType"),        daCommandType,          &::getCppuType( static_cast< sal_Int32* >(NULL) ),                  PropertyAttribute::TRANSIENT, 0 },
                { CONST_CHAR("Component"),          daComponent,            &::getCppuType( static_cast< Reference< XContent >* >(NULL) ),      PropertyAttribute::TRANSIENT, 0 },
                { CONST_CHAR("ConnectionResource"), daConnectionResource,   &::getCppuType( static_cast< ::rtl::OUString* >(NULL) ),            PropertyAttribute::TRANSIENT, 0 },
                { CONST_CHAR("Cursor"),             daCursor,               &::getCppuType( static_cast< Reference< XResultSet>* >(NULL) ),     PropertyAttribute::TRANSIENT, 0 },
                { CONST_CHAR("DataSourceName"),     daDataSource,           &::getCppuType( static_cast< ::rtl::OUString* >(NULL) ),            PropertyAttribute::TRANSIENT, 0 },
                { CONST_CHAR("DatabaseLocation"),   daDatabaseLocation,     &::getCppuType( static_cast< ::rtl::OUString* >(NULL) ),            PropertyAttribute::TRANSIENT, 0 },
                { CONST_CHAR("EscapeProcessing"),   daEscapeProcessing,     &::getBooleanCppuType( ),                                           PropertyAttribute::TRANSIENT, 0 },
                { CONST_CHAR("Filter"),             daFilter,               &::getCppuType( static_cast< ::rtl::OUString* >(NULL) ),            PropertyAttribute::TRANSIENT, 0 },
                { CONST_CHAR("Selection"),          daSelection,            &::getCppuType( static_cast< Sequence< Any >* >(NULL) ),            PropertyAttribute::TRANSIENT, 0 },
                { NULL, 0, 0, NULL, 0, 0 }
            };

            PropertyMapEntry* pEntry = s_aDesriptorProperties;
            while ( pEntry->mpName )
            {
                s_aProperties[ ::rtl::OUString::createFromAscii( pEntry->mpName ) ] = pEntry;
                ++pEntry;
            }
        }

        return s_aProperties;
    }

    //--------------------------------------------------------------------
    PropertyMapEntry* ODADescriptorImpl::getPropertyMapEntry( const DescriptorValues::const_iterator& _rPos )
    {
        const MapString2PropertyEntry& rProperties = getPropertyMap();

        sal_Int32 nNeededHandle = (sal_Int32)(_rPos->first);

        for ( MapString2PropertyEntry::const_iterator loop = rProperties.begin();
              loop != rProperties.end();
              ++loop
            )
        {
            if ( nNeededHandle == loop->second->mnHandle )
                return loop->second;
        }
        throw RuntimeException();
    }

    //--------------------------------------------------------------------
    PropertyValue ODADescriptorImpl::buildPropertyValue( const DescriptorValues::const_iterator& _rPos )
    {
        // the map entry
        PropertyMapEntry* pProperty = getPropertyMapEntry( _rPos );

        // build the property value
        PropertyValue aReturn;
        aReturn.Name    = ::rtl::OUString( pProperty->mpName, pProperty->mnNameLen, RTL_TEXTENCODING_ASCII_US );
        aReturn.Handle  = pProperty->mnHandle;
        aReturn.Value   = _rPos->second;
        aReturn.State   = PropertyState_DIRECT_VALUE;

        // outta here
        return aReturn;
    }

    //--------------------------------------------------------------------
    void ODADescriptorImpl::updateSequence()
    {
        if (!m_bSequenceOutOfDate)
            return;

        m_aAsSequence.realloc(m_aValues.size());
        PropertyValue* pValue = m_aAsSequence.getArray();

        // loop through all our values
        for (   DescriptorValues::const_iterator aLoop = m_aValues.begin();
                aLoop != m_aValues.end();
                ++aLoop, ++pValue
            )
        {
            *pValue = buildPropertyValue(aLoop);
        }

        // don't need to rebuild next time
        m_bSequenceOutOfDate = sal_False;
    }

    //--------------------------------------------------------------------
    void ODADescriptorImpl::updateSet()
    {
        if (!m_bSetOutOfDate)
            return;

        // will be the current values
        Sequence< PropertyValue > aValuesToSet(m_aValues.size());
        PropertyValue* pValuesToSet = aValuesToSet.getArray();

        // build a new property set info
        PropertySetInfo* pPropSetInfo = new PropertySetInfo;

        // loop through all our values
        for (   DescriptorValues::const_iterator aLoop = m_aValues.begin();
                aLoop != m_aValues.end();
                ++aLoop, ++pValuesToSet
            )
        {
            PropertyMapEntry* pMapEntry = getPropertyMapEntry( aLoop );
            pPropSetInfo->add( pMapEntry, 1 );

            *pValuesToSet = buildPropertyValue(aLoop);
        }

        // create the generic set
        m_xAsSet = GenericPropertySet_CreateInstance( pPropSetInfo );

        // no we have the set, still need to set the current values
        const PropertyValue* pSetValues = aValuesToSet.getConstArray();
        const PropertyValue* pSetValuesEnd = pSetValues + aValuesToSet.getLength();
        for (; pSetValues != pSetValuesEnd; ++pSetValues)
            m_xAsSet->setPropertyValue(pSetValues->Name, pSetValues->Value);

        // don't need to rebuild next time
        m_bSetOutOfDate = sal_True;
    }

    //====================================================================
    //= ODataAccessDescriptor
    //====================================================================
    //--------------------------------------------------------------------
    ODataAccessDescriptor::ODataAccessDescriptor()
        :m_pImpl(new ODADescriptorImpl)
    {
    }

    //--------------------------------------------------------------------
    ODataAccessDescriptor::ODataAccessDescriptor( const ODataAccessDescriptor& _rSource )
        :m_pImpl(new ODADescriptorImpl(*_rSource.m_pImpl))
    {
    }

    //--------------------------------------------------------------------
    const ODataAccessDescriptor& ODataAccessDescriptor::operator=(const ODataAccessDescriptor& _rSource)
    {
        delete m_pImpl;
        m_pImpl = new ODADescriptorImpl(*_rSource.m_pImpl);
        return *this;
    }

    //--------------------------------------------------------------------
    ODataAccessDescriptor::ODataAccessDescriptor( const Reference< XPropertySet >& _rValues )
        :m_pImpl(new ODADescriptorImpl)
    {
        m_pImpl->buildFrom(_rValues);
    }

    //--------------------------------------------------------------------
    ODataAccessDescriptor::ODataAccessDescriptor( const Any& _rValues )
        :m_pImpl(new ODADescriptorImpl)
    {
        // check if we know the format in the Any
        Sequence< PropertyValue > aValues;
        Reference< XPropertySet > xValues;
        if ( _rValues >>= aValues )
            m_pImpl->buildFrom( aValues );
        else if ( _rValues >>= xValues )
            m_pImpl->buildFrom( xValues );
    }

    //--------------------------------------------------------------------
    ODataAccessDescriptor::ODataAccessDescriptor( const Sequence< PropertyValue >& _rValues )
        :m_pImpl(new ODADescriptorImpl)
    {
        m_pImpl->buildFrom(_rValues);
    }

    //--------------------------------------------------------------------
    ODataAccessDescriptor::~ODataAccessDescriptor()
    {
        delete m_pImpl;
    }

    //--------------------------------------------------------------------
    void ODataAccessDescriptor::clear()
    {
        m_pImpl->m_aValues.clear();
    }

    //--------------------------------------------------------------------
    void ODataAccessDescriptor::erase(DataAccessDescriptorProperty _eWhich)
    {
        OSL_ENSURE(has(_eWhich), "ODataAccessDescriptor::erase: invalid call!");
        if (has(_eWhich))
            m_pImpl->m_aValues.erase(_eWhich);
    }

    //--------------------------------------------------------------------
    sal_Bool ODataAccessDescriptor::has(DataAccessDescriptorProperty _eWhich) const
    {
        return m_pImpl->m_aValues.find(_eWhich) != m_pImpl->m_aValues.end();
    }

    //--------------------------------------------------------------------
    const Any& ODataAccessDescriptor::operator [] ( DataAccessDescriptorProperty _eWhich ) const
    {
        if (!has(_eWhich))
        {
            OSL_ENSURE(sal_False, "ODataAccessDescriptor::operator[]: invalid acessor!");
            static const Any aDummy;
            return aDummy;
        }

        return m_pImpl->m_aValues[_eWhich];
    }

    //--------------------------------------------------------------------
    Any& ODataAccessDescriptor::operator[] ( DataAccessDescriptorProperty _eWhich )
    {
        m_pImpl->invalidateExternRepresentations();
        return m_pImpl->m_aValues[_eWhich];
    }

    //--------------------------------------------------------------------
    void ODataAccessDescriptor::initializeFrom(const Reference< XPropertySet >& _rxValues, sal_Bool _bClear)
    {
        if (_bClear)
            clear();
        m_pImpl->buildFrom(_rxValues);
    }

    //--------------------------------------------------------------------
    void ODataAccessDescriptor::initializeFrom(const Sequence< PropertyValue >& _rValues, sal_Bool _bClear)
    {
        if (_bClear)
            clear();
        m_pImpl->buildFrom(_rValues);
    }

    //--------------------------------------------------------------------
    Sequence< PropertyValue > ODataAccessDescriptor::createPropertyValueSequence()
    {
        m_pImpl->updateSequence();
        return m_pImpl->m_aAsSequence;
    }
    //--------------------------------------------------------------------
    Sequence< Any > ODataAccessDescriptor::createAnySequence()
    {
        m_pImpl->updateSequence();
        Sequence< Any > aRet(m_pImpl->m_aAsSequence.getLength());
        const PropertyValue* pBegin = m_pImpl->m_aAsSequence.getConstArray();
        const PropertyValue* pEnd     = pBegin + m_pImpl->m_aAsSequence.getLength();
        for(sal_Int32 i=0;pBegin != pEnd;++pBegin,++i)
            aRet[i] <<= *pBegin;
        return aRet;
    }

    //--------------------------------------------------------------------
    Reference< XPropertySet > ODataAccessDescriptor::createPropertySet()
    {
        m_pImpl->updateSet();
        return m_pImpl->m_xAsSet;
    }
    //--------------------------------------------------------------------
    ::rtl::OUString ODataAccessDescriptor::getDataSource() const
    {
        ::rtl::OUString sDataSourceName;
        if ( has(daDataSource) )
            (*this)[daDataSource] >>= sDataSourceName;
        else if ( has(daDatabaseLocation) )
            (*this)[daDatabaseLocation] >>= sDataSourceName;
        return sDataSourceName;
    }
    //--------------------------------------------------------------------
    void ODataAccessDescriptor::setDataSource(const ::rtl::OUString& _sDataSourceNameOrLocation)
    {
        if ( _sDataSourceNameOrLocation.getLength() )
        {
            INetURLObject aURL(_sDataSourceNameOrLocation);
            (*this)[ (( aURL.GetProtocol() == INET_PROT_FILE ) ? daDatabaseLocation : daDataSource)] <<= _sDataSourceNameOrLocation;
        }
        else
            (*this)[ daDataSource ] <<= ::rtl::OUString();
    }

//........................................................................
}   // namespace svx
//........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
