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
#include <com/sun/star/form/FormComponentType.hpp>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>

#include <frm_strings.hxx>

#include <algorithm>
#include <utility>

namespace frm
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::comphelper;

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

OGroupCompAcc::OGroupCompAcc(const Reference<XPropertySet>& rxElement, OGroupComp _aGroupComp )
               :m_xComponent( rxElement )
               ,m_aGroupComp(std::move( _aGroupComp ))
{
}

bool OGroupCompAcc::operator==( const OGroupCompAcc& rCompAcc ) const
{
    return m_xComponent == rCompAcc.m_xComponent;
}

class OGroupCompAccLess
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

OGroupComp::OGroupComp(const Reference<XPropertySet>& rxSet, sal_Int32 nInsertPos )
    : m_xComponent( rxSet )
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

class OGroupCompLess
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

OGroup::OGroup( OUString sGroupName )
        :m_aGroupName(std::move( sGroupName ))
        ,m_nInsertPos(0)
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
             * We do not to change it here, however, because it's passed on continuously
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

Sequence< Reference<XControlModel>  > OGroup::GetControlModels() const
{
    Sequence<Reference<XControlModel> > aControlModelSeq( m_aCompArray.size() );
    Reference<XControlModel>* pModels = aControlModelSeq.getArray();

    for (auto const& rGroupComp : m_aCompArray)
    {
        *pModels++ = rGroupComp.GetControlModel();
    }
    return aControlModelSeq;
}

OGroupManager::OGroupManager(const Reference< XContainer >& _rxContainer)
    :m_pCompGroup( new OGroup( u"AllComponentGroup"_ustr ) )
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
}

// XPropertyChangeListener
void OGroupManager::disposing(const EventObject& evt)
{
    Reference<XContainer>  xContainer(evt.Source, UNO_QUERY);
    if (xContainer.get() == m_xContainer.get())
    {
        m_pCompGroup.reset();

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

void SAL_CALL OGroupManager::propertyChange(const PropertyChangeEvent& evt)
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
            // No prior GroupName; fallback to Name
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
void SAL_CALL OGroupManager::elementInserted(const ContainerEvent& Event)
{
    Reference< XPropertySet > xProps;
    Event.Element >>= xProps;
    if ( xProps.is() )
        InsertElement( xProps );
}

void SAL_CALL OGroupManager::elementRemoved(const ContainerEvent& Event)
{
    Reference<XPropertySet> xProps;
    Event.Element >>= xProps;
    if ( xProps.is() )
        RemoveElement( xProps );
}

void SAL_CALL OGroupManager::elementReplaced(const ContainerEvent& Event)
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
Sequence<Reference<XControlModel> > OGroupManager::getControlModels() const
{
    return m_pCompGroup->GetControlModels();
}

sal_Int32 OGroupManager::getGroupCount() const
{
    return m_aActiveGroupMap.size();
}

void OGroupManager::getGroup(sal_Int32 nGroup, Sequence< Reference<XControlModel> >& _rGroup, OUString& _rName)
{
    OSL_ENSURE(nGroup >= 0 && o3tl::make_unsigned(nGroup) < m_aActiveGroupMap.size(),"OGroupManager::getGroup: Invalid group index!");
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
        aFind = m_aGroupArr.emplace(sGroupName,OGroup(sGroupName)).first;
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

OUString OGroupManager::GetGroupName( const css::uno::Reference< css::beans::XPropertySet>& xComponent )
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
