/*************************************************************************
 *
 *  $RCSfile: GroupManager.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-23 09:04:51 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FRM_GROUPMANAGER_HXX_
#include "GroupManager.hxx"
#endif
#ifndef _FRM_DATABASEFORM_HXX_
#include "DatabaseForm.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#include <tools/debug.hxx>

#include "property.hrc"

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
        <   reinterpret_cast<sal_Int64>(rhs.m_xComponent.get());
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
{
}

//------------------------------------------------------------------
OGroupComp::OGroupComp(const Reference<XPropertySet>& rxSet, sal_Int32 nInsertPos )
            :m_xComponent( rxSet )
            ,m_nTabIndex(0)
            ,m_nPos( nInsertPos )
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
        *pModels = Reference<XControlModel> ((*aGroupComps).GetComponent(), UNO_QUERY);
    }
    return aControlModelSeq;
}

DBG_NAME(OGroupManager);
//------------------------------------------------------------------
OGroupManager::OGroupManager()
    :m_pCompGroup(new OGroup(ALL_COMPONENTS_GROUP_NAME))
{
    DBG_CTOR(OGroupManager,NULL);

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
    if (xContainer.is())
    {
        DELETEZ(m_pCompGroup);

        ////////////////////////////////////////////////////////////////
        // Gruppen loeschen
        m_aGroupArr.erase(m_aGroupArr.begin(), m_aGroupArr.end());
    }
}

//------------------------------------------------------------------
void SAL_CALL OGroupManager::propertyChange(const PropertyChangeEvent& evt)
{
    Reference<XPropertySet>  xSet(evt.Source, UNO_QUERY);

    // Component aus CompGroup entfernen
    m_pCompGroup->RemoveComponent( xSet );

    // Component aus Gruppe entfernen
    sal_Int32       nGroupPos;
    ::rtl::OUString     sGroupName;
    if (evt.PropertyName == PROPERTY_NAME)
        evt.OldValue >>= sGroupName;
    else
        xSet->getPropertyValue( PROPERTY_NAME ) >>= sGroupName;

    OGroup      aSearchGroup( sGroupName );

    if (seek_entry(m_aGroupArr, aSearchGroup, nGroupPos, OGroupLess()))
    {
        // Gruppe vorhanden
        OGroup& rFoundGroup = m_aGroupArr[nGroupPos];
        rFoundGroup.RemoveComponent( xSet );

        // Wenn Anzahl der Gruppenelemente == 1 ist, Gruppe deaktivieren
        if( rFoundGroup.Count() == 1 )
        {
            for (   OUInt32ArrIterator aMapSearch = m_aActiveGroupMap.begin();
                    aMapSearch < m_aActiveGroupMap.end();
                    ++aMapSearch
                )
            {
                if( *aMapSearch == nGroupPos )
                {
                    m_aActiveGroupMap.erase(aMapSearch);
                    break;
                }
            }
        }
    }


    // Bei Component als PropertyChangeListener abmelden
    xSet->removePropertyChangeListener( PROPERTY_NAME, this );
    if (hasProperty(PROPERTY_TABINDEX, xSet))
        xSet->removePropertyChangeListener( PROPERTY_TABINDEX, this );

    // Component neu einordnen
    InsertElement( xSet );
}

// XContainerListener
//------------------------------------------------------------------
void SAL_CALL OGroupManager::elementInserted(const ContainerEvent& Event)
{
    Reference<XPropertySet>  xSet(*(InterfaceRef *)Event.Element.getValue(), UNO_QUERY);
    if (xSet.is())
        InsertElement( xSet );
}

//------------------------------------------------------------------
void SAL_CALL OGroupManager::elementRemoved(const ContainerEvent& Event)
{
    Reference<XPropertySet>  xSet(*(InterfaceRef *)Event.Element.getValue(), UNO_QUERY);
    if (xSet.is())
        RemoveElement( xSet );
}

//------------------------------------------------------------------
void SAL_CALL OGroupManager::elementReplaced(const ContainerEvent& Event)
{
    Reference<XPropertySet>  xSet(*(InterfaceRef *)Event.ReplacedElement.getValue(), UNO_QUERY);
    if (xSet.is())
        RemoveElement( xSet );

    xSet = Reference<XPropertySet> (*(InterfaceRef *)Event.Element.getValue(), UNO_QUERY);
    if (xSet.is())
        InsertElement( xSet );
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
    sal_uInt16 nGroupPos= m_aActiveGroupMap[nGroup];
    OGroup& rGroup      = m_aGroupArr[nGroupPos];
    _rName              = rGroup.GetGroupName();
    _rGroup             = rGroup.GetControlModels();
}

//------------------------------------------------------------------
void OGroupManager::getGroupByName(const ::rtl::OUString& _rName, Sequence< Reference<XControlModel>  >& _rGroup)
{
    sal_Int32 nGroupPos;
    OGroup aSearchGroup( _rName );

    if (seek_entry(m_aGroupArr, aSearchGroup, nGroupPos, OGroupLess()))
    {
        OGroup& rGroup = m_aGroupArr[nGroupPos];
        _rGroup = rGroup.GetControlModels();
    }
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
    sal_Int32 nPos;
    ::rtl::OUString sGroupName;
    xSet->getPropertyValue( PROPERTY_NAME ) >>= sGroupName;

    OGroup aNewGroup( sGroupName );
    if (seek_entry(m_aGroupArr, aNewGroup, nPos, OGroupLess()))
    {
        // Gruppe existiert schon ...
    }
    else
    {
        // Gruppe muss neu angelegt werden
        nPos = insert_sorted(m_aGroupArr, aNewGroup, OGroupLess());

        // Indizes der ActiveGroupMap anpassen
        if (seek_entry(m_aGroupArr, aNewGroup, nPos, OGroupLess()))
        {
            for (   OUInt32ArrIterator aMapSearch = m_aActiveGroupMap.begin();
                    aMapSearch < m_aActiveGroupMap.end();
                    ++aMapSearch
                )
            {
                if( (*aMapSearch) >= nPos )
                    ++(*aMapSearch);
            }
        }
        else
            DBG_ERROR("OGroupManager::InsertElement : inserted the element but did not find it afterwards !");
    }

    OGroup& rNewGroup = m_aGroupArr[nPos];
    rNewGroup.InsertComponent( xSet );


    // Wenn Anzahl der Gruppenelemente == 2 ist, Gruppe aktivieren
    if( rNewGroup.Count() == 2 )
    {
        m_aActiveGroupMap.insert( m_aActiveGroupMap.end(), nPos );
    }


    // Bei Component als PropertyChangeListener anmelden
    xSet->addPropertyChangeListener( PROPERTY_NAME, this );

    // Tabindex muß nicht jeder unterstuetzen
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

    // Component aus CompGroup entfernen
    m_pCompGroup->RemoveComponent( xSet );

    // Component aus Gruppe entfernen
    sal_Int32       nGroupPos;
    ::rtl::OUString     sGroupName;
    xSet->getPropertyValue( PROPERTY_NAME ) >>= sGroupName;
    OGroup      aSearchGroup( sGroupName );

    if (seek_entry(m_aGroupArr, aSearchGroup, nGroupPos, OGroupLess()))
    {
        // Gruppe vorhanden
        OGroup& rGroup = m_aGroupArr[nGroupPos];
        rGroup.RemoveComponent( xSet );


        // Wenn Anzahl der Gruppenelemente == 1 ist, Gruppe deaktivieren
        if( rGroup.Count() == 1 )
        {
            for (   OUInt32ArrIterator aMapSearch = m_aActiveGroupMap.begin();
                    aMapSearch < m_aActiveGroupMap.end();
                    ++aMapSearch
                )
            {
                if( *aMapSearch == nGroupPos )
                {
                    m_aActiveGroupMap.erase(aMapSearch);
                    break;
                }
            }
        }
    }

    // Bei Component als PropertyChangeListener abmelden
    xSet->removePropertyChangeListener( PROPERTY_NAME, this );
    if (hasProperty(PROPERTY_TABINDEX, xSet))
        xSet->removePropertyChangeListener( PROPERTY_TABINDEX, this );
}

//.........................................................................
}   // namespace frm
//.........................................................................

