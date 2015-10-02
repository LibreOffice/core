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

#include "GroupManager.hxx"
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <comphelper/property.hxx>
#include <osl/diagnose.h>
#include <tools/solar.h>

#include "property.hrc"

#include <algorithm>

namespace frm
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;

namespace
{
    bool isRadioButton( const Reference< XPropertySet >& _rxComponent )
    {
        bool bIs = false;
        if ( hasProperty( PROPERTY_CLASSID, _rxComponent ) )
        {
            sal_Int16 nClassId = FormComponentType::CONTROL;
            _rxComponent->getPropertyValue( PROPERTY_CLASSID ) >>= nClassId;
            if ( nClassId == FormComponentType::RADIOBUTTON )
                bIs = true;
        }
        return bIs;
    }
}

OGroupCompAcc::OGroupCompAcc(const Reference<XPropertySet>& rxElement, const OGroupComp& _rGroupComp )
               :m_xComponent( rxElement )
               ,m_aGroupComp( _rGroupComp )
{
}

bool OGroupCompAcc::operator==( const OGroupCompAcc& rCompAcc ) const
{
    return (m_xComponent == rCompAcc.GetComponent());
}

class OGroupCompAccLess : public ::std::binary_function<OGroupCompAcc, OGroupCompAcc, sal_Bool>
{
public:
    bool operator() (const OGroupCompAcc& lhs, const OGroupCompAcc& rhs) const
    {
        return
            reinterpret_cast<sal_Int64>(lhs.m_xComponent.get())
        <   reinterpret_cast<sal_Int64>(rhs.m_xComponent.get());
    }
};

OGroupComp::OGroupComp()
    :m_nPos( -1 )
    ,m_nTabIndex( 0 )
{
}

OGroupComp::OGroupComp(const OGroupComp& _rSource)
    :m_aName( _rSource.m_aName )
    ,m_xComponent( _rSource.m_xComponent )
    ,m_xControlModel(_rSource.m_xControlModel)
    ,m_nPos( _rSource.m_nPos )
    ,m_nTabIndex( _rSource.m_nTabIndex )
{
}

OGroupComp::OGroupComp(const Reference<XPropertySet>& rxSet, sal_Int32 nInsertPos )
    : m_aName( OGroupManager::GetGroupName( rxSet ) )
    , m_xComponent( rxSet )
    , m_xControlModel(rxSet,UNO_QUERY)
    , m_nPos( nInsertPos )
    , m_nTabIndex(0)
{
    if (m_xComponent.is())
    {
        if (hasProperty( PROPERTY_TABINDEX, m_xComponent ) )
            // Indices smaller than 0 are treated like 0
            m_nTabIndex = std::max(getINT16(m_xComponent->getPropertyValue( PROPERTY_TABINDEX )) , sal_Int16(0));
    }
}

bool OGroupComp::operator==( const OGroupComp& rComp ) const
{
    return m_nTabIndex == rComp.GetTabIndex() && m_nPos == rComp.GetPos();
}

class OGroupCompLess : public ::std::binary_function<OGroupComp, OGroupComp, sal_Bool>
{
public:
    bool operator() (const OGroupComp& lhs, const OGroupComp& rhs) const
    {
        bool bResult;
        // TabIndex of 0 will be added at the end
        if (lhs.m_nTabIndex == rhs.GetTabIndex())
            bResult = lhs.m_nPos < rhs.GetPos();
        else if (lhs.m_nTabIndex && rhs.GetTabIndex())
            bResult = lhs.m_nTabIndex < rhs.GetTabIndex();
        else
            bResult = lhs.m_nTabIndex != 0;
        return bResult;
    }
};

OGroup::OGroup( const OUString& rGroupName )
        :m_aGroupName( rGroupName )
        ,m_nInsertPos(0)
{
}

OGroup::~OGroup()
{
}

void OGroup::InsertComponent( const Reference<XPropertySet>& xSet )
{
    OGroupComp aNewGroupComp( xSet, m_nInsertPos );
    sal_Int32 nPosInserted = insert_sorted(m_aCompArray, aNewGroupComp, OGroupCompLess());

    OGroupCompAcc aNewGroupCompAcc( xSet, m_aCompArray[nPosInserted] );
    insert_sorted(m_aCompAccArray, aNewGroupCompAcc, OGroupCompAccLess());
    m_nInsertPos++;
}

void OGroup::RemoveComponent( const Reference<XPropertySet>& rxElement )
{
    sal_Int32 nGroupCompAccPos;
    OGroupCompAcc aSearchCompAcc( rxElement, OGroupComp() );
    if ( seek_entry(m_aCompAccArray, aSearchCompAcc, nGroupCompAccPos, OGroupCompAccLess()) )
    {
        OGroupCompAcc& aGroupCompAcc = m_aCompAccArray[nGroupCompAccPos];
        const OGroupComp& aGroupComp = aGroupCompAcc.GetGroupComponent();

        sal_Int32 nGroupCompPos;
        if ( seek_entry(m_aCompArray, aGroupComp, nGroupCompPos, OGroupCompLess()) )
        {
            m_aCompAccArray.erase( m_aCompAccArray.begin() + nGroupCompAccPos );
            m_aCompArray.erase( m_aCompArray.begin() + nGroupCompPos );

            /*
             * By removing the GroupComp the insertion position has become invalid.
             * We do not to change it here, however, because its passed on continuously
             * and ascending distinctively.
             */
        }
        else
        {
            OSL_FAIL( "OGroup::RemoveComponent: Component not in Group" );
        }
    }
    else
    {
        OSL_FAIL( "OGroup::RemoveComponent: Component not in Group" );
    }
}

bool OGroup::operator==( const OGroup& rGroup ) const
{
    return m_aGroupName.equals(rGroup.GetGroupName());
}

Sequence< Reference<XControlModel>  > OGroup::GetControlModels() const
{
    sal_Int32 nLen = m_aCompArray.size();
    Sequence<Reference<XControlModel> > aControlModelSeq( nLen );
    Reference<XControlModel>* pModels = aControlModelSeq.getArray();

    OGroupCompArr::const_iterator aGroupComps = m_aCompArray.begin();
    for (sal_Int32 i = 0; i < nLen; ++i, ++pModels, ++aGroupComps)
    {
        *pModels = aGroupComps->GetControlModel();
    }
    return aControlModelSeq;
}

OGroupManager::OGroupManager(const Reference< XContainer >& _rxContainer)
    :m_pCompGroup( new OGroup( OUString("AllComponentGroup") ) )
    ,m_xContainer(_rxContainer)
{
    osl_atomic_increment(&m_refCount);
    {
        _rxContainer->addContainerListener(this);
    }
    osl_atomic_decrement(&m_refCount);
}

OGroupManager::~OGroupManager()
{
    // delete all Components and CompGroup
    delete m_pCompGroup;
}

// XPropertyChangeListener
void OGroupManager::disposing(const EventObject& evt) throw( RuntimeException, std::exception )
{
    Reference<XContainer>  xContainer(evt.Source, UNO_QUERY);
    if (xContainer.get() == m_xContainer.get())
    {
        DELETEZ(m_pCompGroup);

        // delete group
        m_aGroupArr.clear();
        m_xContainer.clear();
    }
}

void OGroupManager::removeFromGroupMap(const OUString& _sGroupName,const Reference<XPropertySet>& _xSet)
{
    // remove Component from CompGroup
    m_pCompGroup->RemoveComponent( _xSet );

    OGroupArr::iterator aFind = m_aGroupArr.find(_sGroupName);

    if ( aFind != m_aGroupArr.end() )
    {
        // group exists
        aFind->second.RemoveComponent( _xSet );

        // If the count of Group elements == 1 -> deactivate Group
        sal_Int32 nCount = aFind->second.Count();
        if ( nCount == 1 || nCount == 0 )
        {
            OActiveGroups::iterator aActiveFind = ::std::find(
                m_aActiveGroupMap.begin(),
                m_aActiveGroupMap.end(),
                aFind
            );
            if ( aActiveFind != m_aActiveGroupMap.end() )
            {
                // the group is active. Deactivate it if the remaining component
                // is *no* radio button
                if ( nCount == 0 || !isRadioButton( aFind->second.GetObject( 0 ) ) )
                    m_aActiveGroupMap.erase( aActiveFind );
            }
        }
    }


    // Deregister as PropertyChangeListener at Component
    _xSet->removePropertyChangeListener( PROPERTY_NAME, this );
    if (hasProperty(PROPERTY_GROUP_NAME, _xSet))
        _xSet->removePropertyChangeListener( PROPERTY_GROUP_NAME, this );
    if (hasProperty(PROPERTY_TABINDEX, _xSet))
        _xSet->removePropertyChangeListener( PROPERTY_TABINDEX, this );
}

void SAL_CALL OGroupManager::propertyChange(const PropertyChangeEvent& evt) throw ( css::uno::RuntimeException, std::exception)
{
    Reference<XPropertySet>  xSet(evt.Source, UNO_QUERY);

    // remove Component from group
    OUString     sGroupName;
    if (hasProperty( PROPERTY_GROUP_NAME, xSet ))
        xSet->getPropertyValue( PROPERTY_GROUP_NAME ) >>= sGroupName;
    if (evt.PropertyName == PROPERTY_NAME) {
        if (!sGroupName.isEmpty())
            return; // group hasn't changed; ignore this name change.
        // no GroupName; use Name as GroupName
        evt.OldValue >>= sGroupName;
    }
    else if (evt.PropertyName == PROPERTY_GROUP_NAME) {
        evt.OldValue >>= sGroupName;
        if (sGroupName.isEmpty()) {
            // No prior GroupName; fallback to Nme
            xSet->getPropertyValue( PROPERTY_NAME ) >>= sGroupName;
        }
    }
    else
        sGroupName = GetGroupName( xSet );

    removeFromGroupMap(sGroupName,xSet);

    // Re-insert Component
    InsertElement( xSet );
}

// XContainerListener
void SAL_CALL OGroupManager::elementInserted(const ContainerEvent& Event) throw ( css::uno::RuntimeException, std::exception)
{
    Reference< XPropertySet > xProps;
    Event.Element >>= xProps;
    if ( xProps.is() )
        InsertElement( xProps );
}

void SAL_CALL OGroupManager::elementRemoved(const ContainerEvent& Event) throw ( css::uno::RuntimeException, std::exception)
{
    Reference<XPropertySet> xProps;
    Event.Element >>= xProps;
    if ( xProps.is() )
        RemoveElement( xProps );
}

void SAL_CALL OGroupManager::elementReplaced(const ContainerEvent& Event) throw ( css::uno::RuntimeException, std::exception)
{
    Reference<XPropertySet> xProps;
    Event.ReplacedElement >>= xProps;
    if ( xProps.is() )
        RemoveElement( xProps );

    xProps.clear();
    Event.Element >>= xProps;
    if ( xProps.is() )
        InsertElement( xProps );
}

// Other functions
Sequence<Reference<XControlModel> > OGroupManager::getControlModels()
{
    return m_pCompGroup->GetControlModels();
}

sal_Int32 OGroupManager::getGroupCount()
{
    return m_aActiveGroupMap.size();
}

void OGroupManager::getGroup(sal_Int32 nGroup, Sequence< Reference<XControlModel> >& _rGroup, OUString& _rName)
{
    OSL_ENSURE(nGroup >= 0 && (size_t)nGroup < m_aActiveGroupMap.size(),"OGroupManager::getGroup: Invalid group index!");
    OGroupArr::iterator aGroupPos   = m_aActiveGroupMap[nGroup];
    _rName                          = aGroupPos->second.GetGroupName();
    _rGroup                         = aGroupPos->second.GetControlModels();
}

void OGroupManager::getGroupByName(const OUString& _rName, Sequence< Reference<XControlModel>  >& _rGroup)
{
    OGroupArr::iterator aFind = m_aGroupArr.find(_rName);
    if ( aFind != m_aGroupArr.end() )
        _rGroup = aFind->second.GetControlModels();
}

void OGroupManager::InsertElement( const Reference<XPropertySet>& xSet )
{
    // Only ControlModels
    Reference<XControlModel>  xControl(xSet, UNO_QUERY);
    if (!xControl.is() )
        return;

    // Add Component to CompGroup
    m_pCompGroup->InsertComponent( xSet );

    // Add Component to Group
    OUString sGroupName( GetGroupName( xSet ) );

    OGroupArr::iterator aFind = m_aGroupArr.find(sGroupName);

    if ( aFind == m_aGroupArr.end() )
    {
        aFind = m_aGroupArr.insert(OGroupArr::value_type(sGroupName,OGroup(sGroupName))).first;
    }

    aFind->second.InsertComponent( xSet );

    // if we have at least 2 elements in the group, then this is an "active group"
    bool bActivateGroup = aFind->second.Count() == 2;

    // Additionally, if the component is a radio button, then it's group becomes active,
    // too. With this, we ensure that in a container with n radio buttons which all are
    // in different groups the selection still works reliably (means that all radios can be
    // clicked independently)
    if ( aFind->second.Count() == 1 )
    {
        if ( isRadioButton( xSet ) )
            bActivateGroup = true;
    }

    if ( bActivateGroup )
    {
        OActiveGroups::iterator aAlreadyExistent = ::std::find(
            m_aActiveGroupMap.begin(),
            m_aActiveGroupMap.end(),
            aFind
        );
        if ( aAlreadyExistent == m_aActiveGroupMap.end() )
            m_aActiveGroupMap.push_back(  aFind );
    }

    // Register as PropertyChangeListener at Component
    xSet->addPropertyChangeListener( PROPERTY_NAME, this );
    if (hasProperty(PROPERTY_GROUP_NAME, xSet))
        xSet->addPropertyChangeListener( PROPERTY_GROUP_NAME, this );

    // Not everyone needs to support Tabindex
    if (hasProperty(PROPERTY_TABINDEX, xSet))
        xSet->addPropertyChangeListener( PROPERTY_TABINDEX, this );
}

void OGroupManager::RemoveElement( const Reference<XPropertySet>& xSet )
{
    // Only ControlModels
    Reference<XControlModel>  xControl(xSet, UNO_QUERY);
    if (!xControl.is() )
        return;

    // Remove Component from Group
    OUString     sGroupName( GetGroupName( xSet ) );

    removeFromGroupMap(sGroupName,xSet);
}

OUString OGroupManager::GetGroupName( css::uno::Reference< css::beans::XPropertySet> xComponent )
{
    if (!xComponent.is())
        return OUString();
    OUString sGroupName;
    if (hasProperty( PROPERTY_GROUP_NAME, xComponent )) {
        xComponent->getPropertyValue( PROPERTY_GROUP_NAME ) >>= sGroupName;
        if (sGroupName.isEmpty())
            xComponent->getPropertyValue( PROPERTY_NAME ) >>= sGroupName;
    }
    else
        xComponent->getPropertyValue( PROPERTY_NAME ) >>= sGroupName;
    return sGroupName;
}
}   // namespace frm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
