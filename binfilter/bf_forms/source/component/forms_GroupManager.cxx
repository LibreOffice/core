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

#ifndef _FRM_GROUPMANAGER_HXX_
#include "GroupManager.hxx"
#endif
#ifndef _FRM_DATABASEFORM_HXX_
#include "DatabaseForm.hxx"
#endif


#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#include "property.hrc"

#include <algorithm>
namespace binfilter {

//.........................................................................
namespace frm
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

//========================================================================
// class OGroupCompAcc
//========================================================================
//------------------------------------------------------------------
OGroupCompAcc::OGroupCompAcc(const Reference<XPropertySet>& rxElement, const OGroupComp& _rGroupComp )
               :m_xComponent( rxElement )
               ,m_aGroupComp( _rGroupComp )
{
}

//------------------------------------------------------------------
sal_Bool OGroupCompAcc::operator==( const OGroupCompAcc& rCompAcc ) const
{
    return (m_xComponent == rCompAcc.GetComponent());
}

//------------------------------------------------------------------
class OGroupCompAccLess : public ::std::binary_function<OGroupCompAcc, OGroupCompAcc, sal_Bool>
{
public:
    sal_Bool operator() (const OGroupCompAcc& lhs, const OGroupCompAcc& rhs) const
    {
        return
            reinterpret_cast<sal_Int64>(lhs.m_xComponent.get())
        <	reinterpret_cast<sal_Int64>(rhs.m_xComponent.get());
    }
};

//========================================================================
// class OGroupComp
//========================================================================

//------------------------------------------------------------------
OGroupComp::OGroupComp()
        :m_nTabIndex( 0 )
        ,m_nPos( -1 )
{
}

//------------------------------------------------------------------
OGroupComp::OGroupComp(const OGroupComp& _rSource)
        :m_aName( _rSource.m_aName )
        ,m_xComponent( _rSource.m_xComponent )
        ,m_nTabIndex( _rSource.m_nTabIndex )
        ,m_nPos( _rSource.m_nPos )
        ,m_xControlModel(_rSource.m_xControlModel)
{
}

//------------------------------------------------------------------
OGroupComp::OGroupComp(const Reference<XPropertySet>& rxSet, sal_Int32 nInsertPos )
            :m_xComponent( rxSet )
            ,m_nTabIndex(0)
            ,m_nPos( nInsertPos )
            ,m_xControlModel(rxSet,UNO_QUERY)
{
    if (m_xComponent.is())
    {
        if (hasProperty( PROPERTY_TABINDEX, m_xComponent ) )
            // Indices kleiner 0 werden wie 0 behandelt
            m_nTabIndex = Max(getINT16(m_xComponent->getPropertyValue( PROPERTY_TABINDEX )) , sal_Int16(0));

        m_xComponent->getPropertyValue( PROPERTY_NAME ) >>= m_aName;
    }
}

//------------------------------------------------------------------
sal_Bool OGroupComp::operator==( const OGroupComp& rComp ) const
{
    return m_nTabIndex == rComp.GetTabIndex() && m_nPos == rComp.GetPos();
}

//------------------------------------------------------------------
class OGroupCompLess : public ::std::binary_function<OGroupComp, OGroupComp, sal_Bool>
{
public:
    sal_Bool operator() (const OGroupComp& lhs, const OGroupComp& rhs) const
    {
        sal_Bool bResult;
        // TabIndex von 0 wird hinten einsortiert
        if (lhs.m_nTabIndex == rhs.GetTabIndex())
            bResult = lhs.m_nPos < rhs.GetPos();
        else if (lhs.m_nTabIndex && rhs.GetTabIndex())
            bResult = lhs.m_nTabIndex < rhs.GetTabIndex();
        else
            bResult = lhs.m_nTabIndex != 0;
        return bResult;
    }
};

//========================================================================
// class OGroup
//========================================================================

DBG_NAME(OGroup)
//------------------------------------------------------------------
OGroup::OGroup( const ::rtl::OUString& rGroupName )
        :m_aGroupName( rGroupName )
        ,m_nInsertPos(0)
{
    DBG_CTOR(OGroup,NULL);
}

#ifdef DBG_UTIL
//------------------------------------------------------------------
OGroup::OGroup( const OGroup& _rSource )
    :m_aGroupName(_rSource.m_aGroupName)
    ,m_nInsertPos(_rSource.m_nInsertPos)
    ,m_aCompArray(_rSource.m_aCompArray)
    ,m_aCompAccArray(_rSource.m_aCompAccArray)
{
    DBG_CTOR(OGroup,NULL);
}
#endif

//------------------------------------------------------------------
OGroup::~OGroup()
{
    DBG_DTOR(OGroup,NULL);
}

//------------------------------------------------------------------
void OGroup::InsertComponent( const Reference<XPropertySet>& xSet )
{
    OGroupComp aNewGroupComp( xSet, m_nInsertPos );
    sal_Int32 nPosInserted = insert_sorted(m_aCompArray, aNewGroupComp, OGroupCompLess());

    OGroupCompAcc aNewGroupCompAcc( xSet, m_aCompArray[nPosInserted] );
    insert_sorted(m_aCompAccArray, aNewGroupCompAcc, OGroupCompAccLess());
    m_nInsertPos++;
}

//------------------------------------------------------------------
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

            /*============================================================
            Durch das Entfernen der GroupComp ist die Einfuegeposition
            ungueltig geworden. Sie braucht hier aber nicht angepasst werden,
            da sie fortlaufend vergeben wird und damit immer
            aufsteigend eindeutig ist.
            ============================================================*/
        }
        else
        {
            DBG_ERROR( "OGroup::RemoveComponent: Component nicht in Gruppe" );
        }
    }
    else
    {
        DBG_ERROR( "OGroup::RemoveComponent: Component nicht in Gruppe" );
    }
}

//------------------------------------------------------------------
sal_Bool OGroup::operator==( const OGroup& rGroup ) const
{
    return m_aGroupName.equals(rGroup.GetGroupName());
}

//------------------------------------------------------------------
class OGroupLess : public ::std::binary_function<OGroup, OGroup, sal_Bool>
{
public:
    sal_Bool operator() (const OGroup& lhs, const OGroup& rhs) const
    {
        return lhs.m_aGroupName < rhs.m_aGroupName;
    }
};

//------------------------------------------------------------------
Sequence< Reference<XControlModel>  > OGroup::GetControlModels() const
{
    sal_Int32 nLen = m_aCompArray.size();
    Sequence<Reference<XControlModel> > aControlModelSeq( nLen );
    Reference<XControlModel>* pModels = aControlModelSeq.getArray();

    ConstOGroupCompArrIterator aGroupComps = m_aCompArray.begin();
    for (sal_Int32 i = 0; i < nLen; ++i, ++pModels, ++aGroupComps)
    {
        *pModels = aGroupComps->GetControlModel();
    }
    return aControlModelSeq;
}

DBG_NAME(OGroupManager)
//------------------------------------------------------------------
OGroupManager::OGroupManager(const Reference< XContainer >& _rxContainer)
    :m_pCompGroup(new OGroup(ALL_COMPONENTS_GROUP_NAME))
    ,m_xContainer(_rxContainer)
{
    DBG_CTOR(OGroupManager,NULL);

    increment(m_refCount);
    {
        _rxContainer->addContainerListener(this);
    }
    decrement(m_refCount);
}

//------------------------------------------------------------------
OGroupManager::~OGroupManager()
{
    DBG_DTOR(OGroupManager,NULL);
    // Alle Components und CompGroup loeschen
    delete m_pCompGroup;
}

// XPropertyChangeListener
//------------------------------------------------------------------
void OGroupManager::disposing(const EventObject& evt) throw( RuntimeException )
{
    Reference<XContainer>  xContainer(evt.Source, UNO_QUERY);
    if (xContainer.get() == m_xContainer.get())
    {
        DELETEZ(m_pCompGroup);

        ////////////////////////////////////////////////////////////////
        // Gruppen loeschen
        m_aGroupArr.clear();
        m_xContainer.clear();
    }
}
// -----------------------------------------------------------------------------
void OGroupManager::removeFromGroupMap(const ::rtl::OUString& _sGroupName,const Reference<XPropertySet>& _xSet)
{
    // Component aus CompGroup entfernen
    m_pCompGroup->RemoveComponent( _xSet );

    OGroupArr::iterator aFind = m_aGroupArr.find(_sGroupName);

    if ( aFind != m_aGroupArr.end() )
    {
        // Gruppe vorhanden
        aFind->second.RemoveComponent( _xSet );

        // Wenn Anzahl der Gruppenelemente == 1 ist, Gruppe deaktivieren
        if ( aFind->second.Count() == 1 )
        {
            OActiveGroups::iterator aActiveFind = ::std::find(m_aActiveGroupMap.begin(),m_aActiveGroupMap.end(),aFind);
            if ( aActiveFind != m_aActiveGroupMap.end() )
                m_aActiveGroupMap.erase(aActiveFind);
        }
    }


    // Bei Component als PropertyChangeListener abmelden
    _xSet->removePropertyChangeListener( PROPERTY_NAME, this );
    if (hasProperty(PROPERTY_TABINDEX, _xSet))
        _xSet->removePropertyChangeListener( PROPERTY_TABINDEX, this );
}
//------------------------------------------------------------------
void SAL_CALL OGroupManager::propertyChange(const PropertyChangeEvent& evt) throw ( ::com::sun::star::uno::RuntimeException)
{
    Reference<XPropertySet>  xSet(evt.Source, UNO_QUERY);

    // Component aus Gruppe entfernen
    ::rtl::OUString		sGroupName;
    if (evt.PropertyName == PROPERTY_NAME)
        evt.OldValue >>= sGroupName;
    else
        xSet->getPropertyValue( PROPERTY_NAME ) >>= sGroupName;

    removeFromGroupMap(sGroupName,xSet);

    // Component neu einordnen
    InsertElement( xSet );
}

// XContainerListener
//------------------------------------------------------------------
void SAL_CALL OGroupManager::elementInserted(const ContainerEvent& Event) throw ( ::com::sun::star::uno::RuntimeException)
{
    Reference< XPropertySet > xProps;
    Event.Element >>= xProps;
    if ( xProps.is() )
        InsertElement( xProps );
}

//------------------------------------------------------------------
void SAL_CALL OGroupManager::elementRemoved(const ContainerEvent& Event) throw ( ::com::sun::star::uno::RuntimeException)
{
    Reference<XPropertySet> xProps;
    Event.Element >>= xProps;
    if ( xProps.is() )
        RemoveElement( xProps );
}

//------------------------------------------------------------------
void SAL_CALL OGroupManager::elementReplaced(const ContainerEvent& Event) throw ( ::com::sun::star::uno::RuntimeException)
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
//------------------------------------------------------------------
Sequence<Reference<XControlModel> > OGroupManager::getControlModels()
{
    return m_pCompGroup->GetControlModels();
}

//------------------------------------------------------------------
sal_Int32 OGroupManager::getGroupCount()
{
    return m_aActiveGroupMap.size();
}

//------------------------------------------------------------------
void OGroupManager::getGroup(sal_Int32 nGroup, Sequence< Reference<XControlModel> >& _rGroup, ::rtl::OUString& _rName)
{
    OSL_ENSURE(nGroup >= 0 && nGroup < m_aActiveGroupMap.size(),"OGroupManager::getGroup: Invalid group index!");
    OGroupArr::iterator aGroupPos	= m_aActiveGroupMap[nGroup];
    _rName							= aGroupPos->second.GetGroupName();
    _rGroup							= aGroupPos->second.GetControlModels();
}

//------------------------------------------------------------------
void OGroupManager::getGroupByName(const ::rtl::OUString& _rName, Sequence< Reference<XControlModel>  >& _rGroup)
{
    OGroupArr::iterator aFind = m_aGroupArr.find(_rName);
    if ( aFind != m_aGroupArr.end() )
        _rGroup = aFind->second.GetControlModels();
}

//------------------------------------------------------------------
void OGroupManager::InsertElement( const Reference<XPropertySet>& xSet )
{
    // Nur ControlModels
    Reference<XControlModel>  xControl(xSet, UNO_QUERY);
    if (!xControl.is() )
        return;

    // Component in CompGroup aufnehmen
    m_pCompGroup->InsertComponent( xSet );

    // Component in Gruppe aufnehmen
    ::rtl::OUString sGroupName;
    xSet->getPropertyValue( PROPERTY_NAME ) >>= sGroupName;

    OGroupArr::iterator aFind = m_aGroupArr.find(sGroupName);

    if ( aFind == m_aGroupArr.end() )
    {
        aFind = m_aGroupArr.insert(OGroupArr::value_type(sGroupName,OGroup(sGroupName))).first;
    }

    aFind->second.InsertComponent( xSet );


    // Wenn Anzahl der Gruppenelemente == 2 ist, Gruppe aktivieren
    if ( aFind->second.Count() == 2 )
    {
        m_aActiveGroupMap.push_back(  aFind );
    }


    // Bei Component als PropertyChangeListener anmelden
    xSet->addPropertyChangeListener( PROPERTY_NAME, this );

    // Tabindex muss nicht jeder unterstuetzen
    if (hasProperty(PROPERTY_TABINDEX, xSet))
        xSet->addPropertyChangeListener( PROPERTY_TABINDEX, this );

}

//------------------------------------------------------------------
void OGroupManager::RemoveElement( const Reference<XPropertySet>& xSet )
{
    // Nur ControlModels
    Reference<XControlModel>  xControl(xSet, UNO_QUERY);
    if (!xControl.is() )
        return;

    // Component aus Gruppe entfernen
    ::rtl::OUString		sGroupName;
    xSet->getPropertyValue( PROPERTY_NAME ) >>= sGroupName;

    removeFromGroupMap(sGroupName,xSet);
}

//.........................................................................
}	// namespace frm
//.........................................................................

}
