/*************************************************************************
 *
 *  $RCSfile: dataaccessdescriptor.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:44:28 $
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

#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include "dataaccessdescriptor.hxx"
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_PROPERTSETINFO_HXX_
#include <comphelper/propertysetinfo.hxx>
#endif
#ifndef _COMPHELPER_GENERICPROPERTYSET_HXX_
#include <comphelper/genericpropertyset.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::ucb;
    using namespace ::comphelper;

    //====================================================================
    //=
    //====================================================================
#define CONST_CHAR( propname ) propname, sizeof(propname) - 1

#ifndef SVX_LIGHT
    //====================================================================
    //= ODADescriptorImpl
    //====================================================================
    class ODADescriptorImpl
    {
    protected:
        sal_Bool                    m_bSetOutOfDate         : 1;
        sal_Bool                    m_bSequenceOutOfDate    : 1;

    public:
        DECLARE_STL_STDKEY_MAP( DataAccessDescriptorProperty, Any, DescriptorValues );
        DescriptorValues            m_aValues;
        Sequence< PropertyValue >   m_aAsSequence;
        Reference< XPropertySet >   m_xAsSet;

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
        PropertyValue       buildPropertyValue( const ConstDescriptorValuesIterator& _rPos );
        PropertyMapEntry*   getPropertyMap( ) const;
        PropertyMapEntry*   getPropertyMapEntry( const ConstDescriptorValuesIterator& _rPos ) const;
    };

    //--------------------------------------------------------------------
    ODADescriptorImpl::ODADescriptorImpl()
        :m_bSetOutOfDate(sal_True)
        ,m_bSequenceOutOfDate(sal_True)
    {
    }

    //--------------------------------------------------------------------
    ODADescriptorImpl::ODADescriptorImpl(const ODADescriptorImpl& _rSource)
        :m_aValues( _rSource.m_aValues )
        ,m_bSetOutOfDate( _rSource.m_bSetOutOfDate )
        ,m_bSequenceOutOfDate( _rSource.m_bSequenceOutOfDate )
    {
        if (!m_bSetOutOfDate)
            m_xAsSet = _rSource.m_xAsSet;
        if (!m_bSequenceOutOfDate)
            m_aAsSequence = _rSource.m_aAsSequence;
    }

    //--------------------------------------------------------------------
    // Vergleichen von Strings
    static int
    #if defined( WNT )
     __cdecl
    #endif
    #if defined( ICC ) && defined( OS2 )
    _Optlink
    #endif
        PropertySearch(const void* pFirst, const void* pSecond)
    {
        return static_cast<const PropertyValue*>(pFirst)->Name.compareToAscii(
            static_cast<const PropertyMapEntry*>(pSecond)->mpName);
    }

    //--------------------------------------------------------------------
    sal_Bool ODADescriptorImpl::buildFrom( const Sequence< PropertyValue >& _rValues )
    {
        PropertyMapEntry* pPropertyMap = getPropertyMap();
        // determine the size of the map
        sal_Int32 nMapSize = 0;
        PropertyMapEntry* pMapLoop = pPropertyMap;
        while (pMapLoop->mpName)
        {
            ++nMapSize; ++pMapLoop;
        }

        sal_Bool bValidPropsOnly = sal_True;

        // loop through the sequence, and fill our m_aValues
        const PropertyValue* pValues = _rValues.getConstArray();
        const PropertyValue* pValuesEnd = pValues + _rValues.getLength();
        for (;pValues != pValuesEnd; ++pValues)
        {
            PropertyMapEntry* pPropPos = static_cast<PropertyMapEntry*>(bsearch(pValues, pPropertyMap, nMapSize, sizeof(*pPropertyMap), PropertySearch));
            if (pPropPos)
            {
                DataAccessDescriptorProperty eProperty = (DataAccessDescriptorProperty)pPropPos->mnHandle;
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
    PropertyMapEntry* ODADescriptorImpl::getPropertyMap( ) const
    {
        // the properties we know
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
        // MUST be sorted !!

#if OSL_DEBUG_LEVEL > 0
        PropertyMapEntry* pLoop = s_aDesriptorProperties;
        if (pLoop->mpName)
        {
            ::rtl::OUString sLeft = ::rtl::OUString::createFromAscii(pLoop->mpName);
            ::rtl::OUString sRight;
            while ((++pLoop)->mpName)
            {
                sRight = ::rtl::OUString::createFromAscii(pLoop->mpName);
                OSL_ENSURE(sLeft < sRight, "ODADescriptorImpl::getPropertyMap: property map not sorted!");
                sLeft = sRight;
            }
        }
#endif

        return s_aDesriptorProperties;
    }

    //--------------------------------------------------------------------
    PropertyMapEntry* ODADescriptorImpl::getPropertyMapEntry( const ConstDescriptorValuesIterator& _rPos ) const
    {
        PropertyMapEntry* pMap = getPropertyMap();

        // the index in the map above (depends on the property requested)
        sal_Int32 nNeededHandle = (sal_Int32)(_rPos->first);

        PropertyMapEntry* pSearchHandle = pMap;
        while (pSearchHandle->mpName)
        {
            if (nNeededHandle == pSearchHandle->mnHandle)
                return pSearchHandle;

            ++pSearchHandle;
        }
        OSL_ENSURE(sal_False, "ODADescriptorImpl::getPropertyMapEntry: could not find the property!");
        return NULL;
    }

    //--------------------------------------------------------------------
    PropertyValue ODADescriptorImpl::buildPropertyValue( const ConstDescriptorValuesIterator& _rPos )
    {
        // the map entry
        PropertyMapEntry* pProp = getPropertyMapEntry(_rPos);

        // build the property value
        PropertyValue aReturn;
        aReturn.Name    = ::rtl::OUString::createFromAscii(pProp->mpName);
        aReturn.Handle  = pProp->mnHandle;
        aReturn.Value   = _rPos->second;
        aReturn.State   = PropertyState_DIRECT_VALUE;

#if OSL_DEBUG_LEVEL > 0
        // check for type consistency
        if (TypeClass_INTERFACE == pProp->mpType->getTypeClass())
        {
            Reference< XInterface > xCurrentValue;
            _rPos->second >>= xCurrentValue;
            Any aRequestedIFace;
            if (xCurrentValue.is())
                aRequestedIFace = xCurrentValue->queryInterface(*pProp->mpType);
            OSL_ENSURE(aRequestedIFace.hasValue(), "ODADescriptorImpl::buildPropertyValue: invalid property value type (missing the requested interface)!");
        }
        else
            OSL_ENSURE(pProp->mpType->equals(_rPos->second.getValueType()), "ODADescriptorImpl::buildPropertyValue: invalid property value type!");
#endif

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
        for (   ConstDescriptorValuesIterator aLoop = m_aValues.begin();
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
        for (   ConstDescriptorValuesIterator aLoop = m_aValues.begin();
                aLoop != m_aValues.end();
                ++aLoop, ++pValuesToSet
            )
        {
            PropertyMapEntry* pMapEntry = getPropertyMapEntry(aLoop);
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
#endif

    //====================================================================
    //= ODataAccessDescriptor
    //====================================================================
    //--------------------------------------------------------------------
    ODataAccessDescriptor::ODataAccessDescriptor()
#ifndef SVX_LIGHT
        :m_pImpl(new ODADescriptorImpl)
#else
        :m_pImpl(NULL)
#endif
    {
    }

    //--------------------------------------------------------------------
    ODataAccessDescriptor::ODataAccessDescriptor( const ODataAccessDescriptor& _rSource )
#ifndef SVX_LIGHT
        :m_pImpl(new ODADescriptorImpl(*_rSource.m_pImpl))
#else
        :m_pImpl(NULL)
#endif
    {
    }

    //--------------------------------------------------------------------
    const ODataAccessDescriptor& ODataAccessDescriptor::operator=(const ODataAccessDescriptor& _rSource)
    {
#ifndef SVX_LIGHT
        delete m_pImpl;
        m_pImpl = new ODADescriptorImpl(*_rSource.m_pImpl);
#else
        OSL_ENSURE(sal_False, "ODataAccessDescriptor::operator=: not available in the SVX_LIGHT version!");
#endif
        return *this;
    }

    //--------------------------------------------------------------------
    ODataAccessDescriptor::ODataAccessDescriptor( const Reference< XPropertySet >& _rValues )
#ifndef SVX_LIGHT
        :m_pImpl(new ODADescriptorImpl)
#else
        :m_pImpl(NULL)
#endif
    {
#ifndef SVX_LIGHT
        m_pImpl->buildFrom(_rValues);
#else
        OSL_ENSURE(sal_False, "ODataAccessDescriptor::ODataAccessDescriptor: not available in the SVX_LIGHT version!");
#endif
    }

    //--------------------------------------------------------------------
    ODataAccessDescriptor::ODataAccessDescriptor( const Any& _rValues )
#ifndef SVX_LIGHT
        :m_pImpl(new ODADescriptorImpl)
#else
        :m_pImpl(NULL)
#endif
    {
#ifndef SVX_LIGHT
        // check if we know the format in the Any
        Sequence< PropertyValue > aValues;
        Reference< XPropertySet > xValues;
        if ( _rValues >>= aValues )
            m_pImpl->buildFrom( aValues );
        else if ( _rValues >>= xValues )
            m_pImpl->buildFrom( xValues );
#else
        OSL_ENSURE(sal_False, "ODataAccessDescriptor::ODataAccessDescriptor: not available in the SVX_LIGHT version!");
#endif
    }

    //--------------------------------------------------------------------
    ODataAccessDescriptor::ODataAccessDescriptor( const Sequence< PropertyValue >& _rValues )
#ifndef SVX_LIGHT
        :m_pImpl(new ODADescriptorImpl)
#else
        :m_pImpl(NULL)
#endif
    {
#ifndef SVX_LIGHT
        m_pImpl->buildFrom(_rValues);
#else
        OSL_ENSURE(sal_False, "ODataAccessDescriptor::ODataAccessDescriptor: not available in the SVX_LIGHT version!");
#endif
    }

    //--------------------------------------------------------------------
    ODataAccessDescriptor::~ODataAccessDescriptor()
    {
        delete m_pImpl;
    }

    //--------------------------------------------------------------------
    void ODataAccessDescriptor::clear()
    {
#ifndef SVX_LIGHT
        m_pImpl->m_aValues.clear();
#endif
    }

    //--------------------------------------------------------------------
    void ODataAccessDescriptor::erase(DataAccessDescriptorProperty _eWhich)
    {
#ifndef SVX_LIGHT
        OSL_ENSURE(has(_eWhich), "ODataAccessDescriptor::erase: invalid call!");
        if (has(_eWhich))
            m_pImpl->m_aValues.erase(_eWhich);
#endif
    }

    //--------------------------------------------------------------------
    sal_Bool ODataAccessDescriptor::has(DataAccessDescriptorProperty _eWhich) const
    {
#ifndef SVX_LIGHT
        return m_pImpl->m_aValues.find(_eWhich) != m_pImpl->m_aValues.end();
#else
        return sal_False;
#endif
    }

    //--------------------------------------------------------------------
    const Any& ODataAccessDescriptor::operator [] ( DataAccessDescriptorProperty _eWhich ) const
    {
#ifndef SVX_LIGHT
        if (!has(_eWhich))
        {
            OSL_ENSURE(sal_False, "ODataAccessDescriptor::operator[]: invalid acessor!");
            static const Any aDummy;
            return aDummy;
        }

        return m_pImpl->m_aValues[_eWhich];
#else
        static const Any aDummy;
        return aDummy;
#endif
    }

    //--------------------------------------------------------------------
    Any& ODataAccessDescriptor::operator[] ( DataAccessDescriptorProperty _eWhich )
    {
#ifndef SVX_LIGHT
        m_pImpl->invalidateExternRepresentations();
        return m_pImpl->m_aValues[_eWhich];
#else
        static const Any aDummy;
        return aDummy;
#endif
    }

    //--------------------------------------------------------------------
    void ODataAccessDescriptor::initializeFrom(const Reference< XPropertySet >& _rxValues, sal_Bool _bClear)
    {
#ifndef SVX_LIGHT
        if (_bClear)
            clear();
        m_pImpl->buildFrom(_rxValues);
#endif
    }

    //--------------------------------------------------------------------
    void ODataAccessDescriptor::initializeFrom(const Sequence< PropertyValue >& _rValues, sal_Bool _bClear)
    {
#ifndef SVX_LIGHT
        if (_bClear)
            clear();
        m_pImpl->buildFrom(_rValues);
#endif
    }

    //--------------------------------------------------------------------
    Sequence< PropertyValue > ODataAccessDescriptor::createPropertyValueSequence()
    {
#ifndef SVX_LIGHT
        m_pImpl->updateSequence();
        return m_pImpl->m_aAsSequence;
#else
        return Sequence< PropertyValue >();
#endif
    }
    //--------------------------------------------------------------------
    Sequence< Any > ODataAccessDescriptor::createAnySequence()
    {
#ifndef SVX_LIGHT
        m_pImpl->updateSequence();
        Sequence< Any > aRet(m_pImpl->m_aAsSequence.getLength());
        const PropertyValue* pBegin = m_pImpl->m_aAsSequence.getConstArray();
        const PropertyValue* pEnd     = pBegin + m_pImpl->m_aAsSequence.getLength();
        for(sal_Int32 i=0;pBegin != pEnd;++pBegin,++i)
            aRet[i] <<= *pBegin;
        return aRet;
#else
        return Sequence< createAnySequence >();
#endif
    }

    //--------------------------------------------------------------------
    Reference< XPropertySet > ODataAccessDescriptor::createPropertySet()
    {
#ifndef SVX_LIGHT
        m_pImpl->updateSet();
        return m_pImpl->m_xAsSet;
#else
        return Reference< XPropertySet >();
#endif
    }
    //--------------------------------------------------------------------
    ::rtl::OUString ODataAccessDescriptor::getDataSource() const
    {
#ifndef SVX_LIGHT
        ::rtl::OUString sDataSourceName;
        if ( has(daDataSource) )
            (*this)[daDataSource] >>= sDataSourceName;
        else if ( has(daDatabaseLocation) )
            (*this)[daDatabaseLocation] >>= sDataSourceName;
        return sDataSourceName;
#else
        return ::rtl::OUString();
#endif
    }
    //--------------------------------------------------------------------
    void ODataAccessDescriptor::setDataSource(const ::rtl::OUString& _sDataSourceNameOrLocation)
    {
#ifndef SVX_LIGHT
        if ( _sDataSourceNameOrLocation.getLength() )
        {
            INetURLObject aURL(_sDataSourceNameOrLocation);
            (*this)[ (( aURL.GetProtocol() == INET_PROT_FILE ) ? daDatabaseLocation : daDataSource)] <<= _sDataSourceNameOrLocation;
        }
        else
            (*this)[ daDataSource ] <<= ::rtl::OUString();
#endif
    }

//........................................................................
}   // namespace svx
//........................................................................


