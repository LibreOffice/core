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

#include <svx/dataaccessdescriptor.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <tools/urlobj.hxx>
#include <map>

namespace svx
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::ucb;

    typedef std::pair<OUString const, DataAccessDescriptorProperty> PropertyMapEntry;

    class ODADescriptorImpl
    {
    protected:
        bool                    m_bSetOutOfDate         : 1;
        bool                    m_bSequenceOutOfDate    : 1;

    public:
        typedef ::std::map< DataAccessDescriptorProperty, Any >     DescriptorValues;
        DescriptorValues            m_aValues;
        Sequence< PropertyValue >   m_aAsSequence;

        typedef ::std::map< OUString, DataAccessDescriptorProperty >    MapString2PropertyEntry;

    public:
        ODADescriptorImpl();
        ODADescriptorImpl(const ODADescriptorImpl& _rSource);

        void invalidateExternRepresentations();

        void updateSequence();

        /** builds the descriptor from a property value sequence
            @return <TRUE/>
                if and only if the sequence contained valid properties only
        */
        bool buildFrom( const Sequence< PropertyValue >& _rValues );

        /** builds the descriptor from a property set
            @return <TRUE/>
                if and only if the set contained valid properties only
        */
        bool buildFrom( const Reference< XPropertySet >& _rValues );

    protected:
        static PropertyValue                    buildPropertyValue( const DescriptorValues::const_iterator& _rPos );
        static const MapString2PropertyEntry&   getPropertyMap( );
        static PropertyMapEntry const *         getPropertyMapEntry( const DescriptorValues::const_iterator& _rPos );
    };

    ODADescriptorImpl::ODADescriptorImpl()
        :m_bSetOutOfDate(true)
        ,m_bSequenceOutOfDate(true)
    {
    }

    ODADescriptorImpl::ODADescriptorImpl(const ODADescriptorImpl& _rSource)
        :m_bSetOutOfDate( _rSource.m_bSetOutOfDate )
        ,m_bSequenceOutOfDate( _rSource.m_bSequenceOutOfDate )
        ,m_aValues( _rSource.m_aValues )
    {
        if (!m_bSequenceOutOfDate)
            m_aAsSequence = _rSource.m_aAsSequence;
    }

    bool ODADescriptorImpl::buildFrom( const Sequence< PropertyValue >& _rValues )
    {
        const MapString2PropertyEntry& rProperties = getPropertyMap();

        bool bValidPropsOnly = true;

        // loop through the sequence, and fill our m_aValues
        const PropertyValue* pValues = _rValues.getConstArray();
        const PropertyValue* pValuesEnd = pValues + _rValues.getLength();
        for (;pValues != pValuesEnd; ++pValues)
        {
            MapString2PropertyEntry::const_iterator aPropPos = rProperties.find( pValues->Name );
            if ( aPropPos != rProperties.end() )
            {
                DataAccessDescriptorProperty eProperty = aPropPos->second;
                m_aValues[eProperty] = pValues->Value;
            }
            else
                // unknown property
                bValidPropsOnly = false;
        }

        if (bValidPropsOnly)
        {
            m_aAsSequence = _rValues;
            m_bSequenceOutOfDate = false;
        }
        else
            m_bSequenceOutOfDate = true;

        return bValidPropsOnly;
    }

    bool ODADescriptorImpl::buildFrom( const Reference< XPropertySet >& _rxValues )
    {
        Reference< XPropertySetInfo > xPropInfo;
        if (_rxValues.is())
            xPropInfo = _rxValues->getPropertySetInfo();
        if (!xPropInfo.is())
        {
            OSL_FAIL("ODADescriptorImpl::buildFrom: invalid property set!");
            return false;
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

        bool bValidPropsOnly = buildFrom(aValues);
        m_bSetOutOfDate = !bValidPropsOnly;

        return bValidPropsOnly;
    }

    void ODADescriptorImpl::invalidateExternRepresentations()
    {
        m_bSetOutOfDate = true;
        m_bSequenceOutOfDate = true;
    }

    const ODADescriptorImpl::MapString2PropertyEntry& ODADescriptorImpl::getPropertyMap( )
    {
        // the properties we know
        static MapString2PropertyEntry s_aProperties
            {
                { OUString("ActiveConnection"),   DataAccessDescriptorProperty::Connection,            },
                { OUString("BookmarkSelection"),  DataAccessDescriptorProperty::BookmarkSelection,     },
                { OUString("Column"),             DataAccessDescriptorProperty::ColumnObject,          },
                { OUString("ColumnName"),         DataAccessDescriptorProperty::ColumnName,            },
                { OUString("Command"),            DataAccessDescriptorProperty::Command,               },
                { OUString("CommandType"),        DataAccessDescriptorProperty::CommandType,           },
                { OUString("Component"),          DataAccessDescriptorProperty::Component,             },
                { OUString("ConnectionResource"), DataAccessDescriptorProperty::ConnectionResource,    },
                { OUString("Cursor"),             DataAccessDescriptorProperty::Cursor,                },
                { OUString("DataSourceName"),     DataAccessDescriptorProperty::DataSource,            },
                { OUString("DatabaseLocation"),   DataAccessDescriptorProperty::DatabaseLocation,      },
                { OUString("EscapeProcessing"),   DataAccessDescriptorProperty::EscapeProcessing,      },
                { OUString("Filter"),             DataAccessDescriptorProperty::Filter,                },
                { OUString("Selection"),          DataAccessDescriptorProperty::Selection,             }
            };

        return s_aProperties;
    }

    PropertyMapEntry const * ODADescriptorImpl::getPropertyMapEntry( const DescriptorValues::const_iterator& _rPos )
    {
        const MapString2PropertyEntry& rProperties = getPropertyMap();

        DataAccessDescriptorProperty nNeededHandle = _rPos->first;

        auto loop = std::find_if(rProperties.begin(), rProperties.end(),
            [&nNeededHandle](const MapString2PropertyEntry::value_type& rProp) { return nNeededHandle == rProp.second; });
        if (loop != rProperties.end())
            return &*loop;
        throw RuntimeException();
    }

    PropertyValue ODADescriptorImpl::buildPropertyValue( const DescriptorValues::const_iterator& _rPos )
    {
        // the map entry
        PropertyMapEntry const * pProperty = getPropertyMapEntry( _rPos );

        // build the property value
        PropertyValue aReturn;
        aReturn.Name    = pProperty->first;
        aReturn.Handle  = static_cast<sal_Int32>(pProperty->second);
        aReturn.Value   = _rPos->second;
        aReturn.State   = PropertyState_DIRECT_VALUE;

        // outta here
        return aReturn;
    }

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
        m_bSequenceOutOfDate = false;
    }

    ODataAccessDescriptor::ODataAccessDescriptor()
        :m_pImpl(new ODADescriptorImpl)
    {
    }

    ODataAccessDescriptor::ODataAccessDescriptor( const ODataAccessDescriptor& _rSource )
        :m_pImpl(new ODADescriptorImpl(*_rSource.m_pImpl))
    {
    }

    ODataAccessDescriptor::ODataAccessDescriptor( ODataAccessDescriptor&& _rSource )
        :m_pImpl(std::move(_rSource.m_pImpl))
    {
    }

    ODataAccessDescriptor& ODataAccessDescriptor::operator=(const ODataAccessDescriptor& _rSource)
    {
        if (this != &_rSource)
            m_pImpl.reset(new ODADescriptorImpl(*_rSource.m_pImpl));
        return *this;
    }

    ODataAccessDescriptor& ODataAccessDescriptor::operator=(ODataAccessDescriptor&& _rSource)
    {
        m_pImpl = std::move(_rSource.m_pImpl);
        return *this;
    }

    ODataAccessDescriptor::ODataAccessDescriptor( const Reference< XPropertySet >& _rValues )
        :m_pImpl(new ODADescriptorImpl)
    {
        m_pImpl->buildFrom(_rValues);
    }

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

    ODataAccessDescriptor::ODataAccessDescriptor( const Sequence< PropertyValue >& _rValues )
        :m_pImpl(new ODADescriptorImpl)
    {
        m_pImpl->buildFrom(_rValues);
    }

    ODataAccessDescriptor::~ODataAccessDescriptor()
    {
    }

    void ODataAccessDescriptor::clear()
    {
        m_pImpl->m_aValues.clear();
    }

    void ODataAccessDescriptor::erase(DataAccessDescriptorProperty _eWhich)
    {
        OSL_ENSURE(has(_eWhich), "ODataAccessDescriptor::erase: invalid call!");
        if (has(_eWhich))
            m_pImpl->m_aValues.erase(_eWhich);
    }

    bool ODataAccessDescriptor::has(DataAccessDescriptorProperty _eWhich) const
    {
        return m_pImpl->m_aValues.find(_eWhich) != m_pImpl->m_aValues.end();
    }

    const Any& ODataAccessDescriptor::operator [] ( DataAccessDescriptorProperty _eWhich ) const
    {
        if (!has(_eWhich))
        {
            OSL_FAIL("ODataAccessDescriptor::operator[]: invalid accessor!");
            static const Any aDummy;
            return aDummy;
        }

        return m_pImpl->m_aValues[_eWhich];
    }

    Any& ODataAccessDescriptor::operator[] ( DataAccessDescriptorProperty _eWhich )
    {
        m_pImpl->invalidateExternRepresentations();
        return m_pImpl->m_aValues[_eWhich];
    }

    void ODataAccessDescriptor::initializeFrom(const Sequence< PropertyValue >& _rValues)
    {
        clear();
        m_pImpl->buildFrom(_rValues);
    }

    Sequence< PropertyValue > const & ODataAccessDescriptor::createPropertyValueSequence()
    {
        m_pImpl->updateSequence();
        return m_pImpl->m_aAsSequence;
    }

    OUString ODataAccessDescriptor::getDataSource() const
    {
        OUString sDataSourceName;
        if ( has(DataAccessDescriptorProperty::DataSource) )
            (*this)[DataAccessDescriptorProperty::DataSource] >>= sDataSourceName;
        else if ( has(DataAccessDescriptorProperty::DatabaseLocation) )
            (*this)[DataAccessDescriptorProperty::DatabaseLocation] >>= sDataSourceName;
        return sDataSourceName;
    }

    void ODataAccessDescriptor::setDataSource(const OUString& _sDataSourceNameOrLocation)
    {
        if ( !_sDataSourceNameOrLocation.isEmpty() )
        {
            INetURLObject aURL(_sDataSourceNameOrLocation);
            (*this)[ (( aURL.GetProtocol() == INetProtocol::File ) ? DataAccessDescriptorProperty::DatabaseLocation : DataAccessDescriptorProperty::DataSource)] <<= _sDataSourceNameOrLocation;
        }
        else
            (*this)[ DataAccessDescriptorProperty::DataSource ] <<= OUString();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
