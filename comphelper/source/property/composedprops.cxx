/*************************************************************************
 *
 *  $RCSfile: composedprops.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 15:58:38 $
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

#ifndef _COMPHELPER_COMPOSEDPROPS_HXX_
#include <comphelper/composedprops.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;

    //=====================================================================
    //= OComposedPropertySetInfo
    //=====================================================================
    class OComposedPropertySetInfo : public ::cppu::WeakImplHelper1< XPropertySetInfo >
    {
    private:
        Sequence< Property>     m_aProperties;

    public:
        OComposedPropertySetInfo(const Sequence< Property>& _rProperties);

        virtual Sequence< Property > SAL_CALL getProperties(  ) throw(RuntimeException);
        virtual Property SAL_CALL getPropertyByName( const ::rtl::OUString& _rName ) throw(UnknownPropertyException, RuntimeException);
        virtual sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& _rName ) throw(RuntimeException);
    };

    //=====================================================================
    //= OComposedPropertySet
    //=====================================================================
    //---------------------------------------------------------------------
    OComposedPropertySet::OComposedPropertySet(
            const Sequence< Reference< XPropertySet> > & _rElements,
            const IPropertySetComposerCallback* _pPropertyMetaData)
        :m_pInfo(NULL)
    {
        // copy the sequence
        sal_Int32 nSingleSets = _rElements.getLength();
        if (nSingleSets)
        {
            m_aSingleSets.resize(nSingleSets);
            const Reference< XPropertySet >* pSingleSets = _rElements.getConstArray();
            ::std::copy(pSingleSets, pSingleSets + nSingleSets, m_aSingleSets.begin());
        }

        // impl ctor
        compose(_pPropertyMetaData);
    }

    //---------------------------------------------------------------------
    OComposedPropertySet::~OComposedPropertySet()
    {
        if (m_pInfo)
            m_pInfo->release();
    }

    //---------------------------------------------------------------------
    void OComposedPropertySet::compose(const IPropertySetComposerCallback* _pMetaData)
    {
        sal_Int32 nSingleSets = m_aSingleSets.size();

        if (nSingleSets>0)
        {
            // get the properties of the first set
            Reference< XPropertySet > xMasterSet = m_aSingleSets[0];
            Sequence< Property> aMasterProps;
            if (xMasterSet.is())
                aMasterProps = xMasterSet->getPropertySetInfo()->getProperties();
            sal_Int32 nMasterPropsCount = aMasterProps.getLength();
            const Property* pMasterProps = aMasterProps.getConstArray();

            // check which of the master properties should be included
            Sequence<sal_Bool> aInclusionFlags(nMasterPropsCount);
            sal_Bool* pInclusionFlags = aInclusionFlags.getArray();

            // the states of all these properties
            Sequence< PropertyState > aPropertyStates(nMasterPropsCount);

            for (sal_Int32 i=0; i<nMasterPropsCount; ++i)
                pInclusionFlags[i] = sal_True;

            Reference< XPropertySet >  xSecondarySet;
            sal_Int32 nSecondaryPropertyCount;
            Sequence< Property > aSecondaryProperties;
            const Property* pPrimaryProperty = aMasterProps.getConstArray();
            for (sal_Int32 nPrimary=0; nPrimary<nMasterPropsCount; ++nPrimary, ++pPrimaryProperty)
            {
                if (_pMetaData && !_pMetaData->isComposeable(pPrimaryProperty->Name))
                    // do not include this property
                    pInclusionFlags[nPrimary] = sal_False;
                else
                {
                    // search the property in all secondary sets
                    for (sal_Int32 i=1; i<nSingleSets; ++i)
                    {
                        xSecondarySet = m_aSingleSets[i];
                        aSecondaryProperties = xSecondarySet->getPropertySetInfo()->getProperties();
                        nSecondaryPropertyCount = aSecondaryProperties.getLength();
                        const Property* pSecondaryProperties = aSecondaryProperties.getConstArray();

                        // search the current primary property in the secondary property sequence
                        sal_Int32 k=0;
                        while (k<nSecondaryPropertyCount && (pSecondaryProperties[k].Name != pPrimaryProperty->Name))
                            ++k;

                        if (k >= nSecondaryPropertyCount)
                            // not found -> do not include
                            pInclusionFlags[nPrimary] = sal_False;
                    }
                }
            }

            // count what's left ....
            sal_Int32 nOverallProperties = 0;
            for (sal_Int32 nCounter=0; nCounter<nMasterPropsCount; ++nCounter)
            {
                if (pInclusionFlags[nCounter])
                    ++nOverallProperties;
            }

            // and finally construct our sequence
            m_aProperties = Sequence< Property >(nOverallProperties);
            Property* pProperties = m_aProperties.getArray();
            const Property* pMasterProperties = pMasterProps;
            sal_Int32 nOwnProperties = 0;
            for (sal_Int32 nCopy = 0; nCopy < nMasterPropsCount; ++nCopy, ++pMasterProperties)
            {
                if (pInclusionFlags[nCopy])
                    pProperties[nOwnProperties++] = *pMasterProperties;
            }
        }
    }

    //------------------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL OComposedPropertySet::getPropertySetInfo(  ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if (!m_pInfo)
        {
            m_pInfo = new OComposedPropertySetInfo(m_aProperties);
            m_pInfo->acquire();
        }
        return m_pInfo;
    }

    //------------------------------------------------------------------------------
    PropertyState SAL_CALL OComposedPropertySet::getPropertyState( const ::rtl::OUString& _rPropertyName ) throw(UnknownPropertyException, RuntimeException)
    {
        // assume DIRECT for the moment
        PropertyState eState = PropertyState_DIRECT_VALUE;

        sal_Int32 nSingleSets = m_aSingleSets.size();
        if (nSingleSets>0)
        {
            // check the master state
            Reference< XPropertySet >  xMasterSet(m_aSingleSets[0]);
            Any aPrimaryValue;
            if (xMasterSet.is())
            {
                Reference< XPropertyState >  xMasterState(xMasterSet,UNO_QUERY);
                aPrimaryValue = xMasterSet->getPropertyValue(_rPropertyName);

                if (xMasterState.is())
                    eState = xMasterState->getPropertyState(_rPropertyName);
            }

            // loop through the secondary sets
            PropertyState eSecondaryState;
            for (sal_Int32 i=1; i<nSingleSets; ++i)
            {
                Reference< XPropertySet >   xSecondary(m_aSingleSets[i]);
                Reference< XPropertyState > xSecondaryState(xSecondary, UNO_QUERY);

                // the secondary state
                eSecondaryState = PropertyState_DIRECT_VALUE;
                if(xSecondaryState.is())
                    eSecondaryState = xSecondaryState->getPropertyState(_rPropertyName);

                // the secondary value
                Any aSecondaryValue(xSecondary->getPropertyValue(_rPropertyName));

                if  (   (PropertyState_AMBIGUOUS_VALUE == eSecondaryState)      // secondary is ambiguous
                    ||  !::comphelper::compare(aPrimaryValue, aSecondaryValue)  // unequal values
                    )
                {
                    eState = PropertyState_AMBIGUOUS_VALUE;
                    break;
                }
            }
        }
        else
        {
            throw UnknownPropertyException(::rtl::OUString(), static_cast< XPropertySet* >(this));
        }

        return eState;
    }

    //---------------------------------------------------------------------
    Sequence< PropertyState > SAL_CALL OComposedPropertySet::getPropertyStates( const Sequence< ::rtl::OUString >& _rPropertyName ) throw(UnknownPropertyException, RuntimeException)
    {
        sal_Int32 nCount = _rPropertyName.getLength();
        Sequence< PropertyState > aReturn(nCount);
        const ::rtl::OUString* pNames = _rPropertyName.getConstArray();
        PropertyState* pStates = aReturn.getArray();
        for (sal_Int32 i=0; i<nCount; ++i, ++pNames, ++pStates)
            *pStates = getPropertyState(*pNames);
        return aReturn;
    }

    //---------------------------------------------------------------------
    void SAL_CALL OComposedPropertySet::setPropertyToDefault( const ::rtl::OUString& _rPropertyName ) throw(UnknownPropertyException, RuntimeException)
    {
        sal_Int32 nSingleSets = m_aSingleSets.size();
        for (sal_Int32 i=0; i<nSingleSets; ++i)
        {
            Reference< XPropertyState > xState(m_aSingleSets[i], UNO_QUERY);
            if(xState.is())
                xState->setPropertyToDefault(_rPropertyName);
        }
    }

    //---------------------------------------------------------------------
    Any SAL_CALL OComposedPropertySet::getPropertyDefault( const ::rtl::OUString& _rPropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
    {
        return Any();
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OComposedPropertySet::setPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
    {
        sal_Int32 nSingleSets = m_aSingleSets.size();
        for (sal_Int32 i=0; i<nSingleSets; ++i)
        {
            if (m_aSingleSets[i].is())
                m_aSingleSets[i]->setPropertyValue(_rPropertyName, _rValue);
        }
    }

    //------------------------------------------------------------------------------
    Any SAL_CALL OComposedPropertySet::getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
    {
        sal_Int32 nSingleSets = m_aSingleSets.size();

        if ((nSingleSets>0) && (m_aSingleSets[0].is()))
            return m_aSingleSets[0]->getPropertyValue(_rPropertyName);
        return Any();
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OComposedPropertySet::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
    {
        // TODO:
        // hold the single property sets weak
        // be a property change listener on all single property sets (for all composed properties)
        // upon property change
        //   determine the new state/value of the composed property
        //   broadcast the new composed property value
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OComposedPropertySet::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
    {
        // TODO
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OComposedPropertySet::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
    {
        OSL_ENSURE(sal_False, "OComposedPropertySet::addVetoableChangeListener: no implemented (yet)!");
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OComposedPropertySet::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
    {
        OSL_ENSURE(sal_False, "OComposedPropertySet::removeVetoableChangeListener: no implemented (yet)!");
    }

    //------------------------------------------------------------------------------
    OComposedPropertySetInfo::OComposedPropertySetInfo(const Sequence< Property>& rSeq)
        :m_aProperties(rSeq)
    {
    }

    //------------------------------------------------------------------------------
    Sequence< Property> SAL_CALL OComposedPropertySetInfo::getProperties() throw(RuntimeException)
    {
        return m_aProperties;
    }

    //------------------------------------------------------------------------------
    Property SAL_CALL OComposedPropertySetInfo::getPropertyByName( const ::rtl::OUString& _rName ) throw(UnknownPropertyException, RuntimeException)
    {
        sal_Int32 nLength = m_aProperties.getLength();
        const Property* pProps = m_aProperties.getConstArray();
        // TODO TODO TODO: this O(n) search really sucks ...
        for (sal_Int32 i=0; i<nLength; ++i, ++pProps)
        {
            if (pProps->Name == _rName)
                return *pProps;
        }

        throw UnknownPropertyException(::rtl::OUString(), static_cast< XPropertySetInfo* >(this));
    }

    //------------------------------------------------------------------------------
    sal_Bool SAL_CALL OComposedPropertySetInfo::hasPropertyByName( const ::rtl::OUString& _rName ) throw(RuntimeException)
    {
        sal_Int32 nLength = m_aProperties.getLength();
        const Property* pProps = m_aProperties.getConstArray();
        // TODO TODO TODO: this O(n) search really sucks ...
        for( sal_Int32 i=0; i<nLength; ++i,++pProps )
        {
            if(pProps->Name == _rName)
                return sal_True;
        }

        return sal_False;
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................


