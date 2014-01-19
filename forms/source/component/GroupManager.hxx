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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_GROUPMANAGER_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_GROUPMANAGER_HXX

#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <cppuhelper/implbase2.hxx>
#include <comphelper/types.hxx>

#include <algorithm>
#include <map>
#include <vector>

using namespace comphelper;

/*========================================================================
Funktionsweise GroupManager:

Der GroupManager horcht an der starform, ob FormComponents eingefuegt oder entfernt
werden. Zusaetzlich horcht er bei den FormComponents an den Properties
"Name" und "TabIndex". Mit diesen Infos aktualisiert er seine Gruppen.

Der GroupManager verwaltet eine Gruppe, in der alle Controls nach TabIndex
geordnet sind, und ein Array von Gruppen, in dem jede FormComponent noch
einmal einer Gruppe dem Namen nach zugeordnet wird.
Die einzelnen Gruppen werden ueber eine Map aktiviert, wenn sie mehr als
ein Element besitzen.

Die Gruppen verwalten intern die FormComponents in zwei Arrays. In dem
GroupCompArray werden die Components nach TabIndex und Einfuegepostion
sortiert. Da auf dieses Array ueber die FormComponent zugegriffen
wird, gibt es noch das GroupCompAccessArray, in dem die FormComponents
nach ihrer Speicheradresse sortiert sind. Jedes Element des
GroupCompAccessArrays ist mit einem Element des GroupCompArrays verpointert.

========================================================================*/

//.........................................................................
namespace frm
{
//.........................................................................

//========================================================================
    template <class ELEMENT, class LESS_COMPARE>
    sal_Int32 insert_sorted(::std::vector<ELEMENT>& _rArray, const ELEMENT& _rNewElement, const LESS_COMPARE& _rCompareOp)
    {
        typename ::std::vector<ELEMENT>::iterator aInsertPos = ::std::lower_bound(
            _rArray.begin(),
            _rArray.end(),
            _rNewElement,
            _rCompareOp
        );
        aInsertPos = _rArray.insert(aInsertPos, _rNewElement);
        return aInsertPos - _rArray.begin();
    }

    template <class ELEMENT, class LESS_COMPARE>
    sal_Bool seek_entry(const ::std::vector<ELEMENT>& _rArray, const ELEMENT& _rNewElement, sal_Int32& nPos, const LESS_COMPARE& _rCompareOp)
    {
        typename ::std::vector<ELEMENT>::const_iterator aExistentPos = ::std::lower_bound(
            _rArray.begin(),
            _rArray.end(),
            _rNewElement,
            _rCompareOp
        );
        if ((aExistentPos != _rArray.end()) && (*aExistentPos == _rNewElement))
        {   // we have a valid "lower or equal" element and it's really "equal"
            nPos = aExistentPos - _rArray.begin();
            return sal_True;
        }
        nPos = -1;
        return sal_False;
    }

//========================================================================
class OGroupComp
{
    OUString m_aName;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>    m_xComponent;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>     m_xControlModel;
    sal_Int32   m_nPos;
    sal_Int16   m_nTabIndex;

    friend class OGroupCompLess;

public:
    OGroupComp(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement, sal_Int32 nInsertPos );
    OGroupComp(const OGroupComp& _rSource);
    OGroupComp();

    sal_Bool operator==( const OGroupComp& rComp ) const;

    inline const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& GetComponent() const { return m_xComponent; }
    inline const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>&   GetControlModel() const { return m_xControlModel; }

    sal_Int32   GetPos() const { return m_nPos; }
    sal_Int16   GetTabIndex() const { return m_nTabIndex; }
    OUString GetName() const { return m_aName; }
};

typedef std::vector<OGroupComp> OGroupCompArr;

//========================================================================
class OGroupComp;
class OGroupCompAcc
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>    m_xComponent;

    OGroupComp                                      m_aGroupComp;

    friend class OGroupCompAccLess;

public:
    OGroupCompAcc(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement, const OGroupComp& _rGroupComp );

    sal_Bool operator==( const OGroupCompAcc& rCompAcc ) const;

    inline const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>&  GetComponent() const { return m_xComponent; }
    const OGroupComp&   GetGroupComponent() const { return m_aGroupComp; }
};

typedef std::vector<OGroupCompAcc> OGroupCompAccArr;

//========================================================================
class OGroup
{
    OGroupCompArr       m_aCompArray;
    OGroupCompAccArr    m_aCompAccArray;

    OUString m_aGroupName;
    sal_uInt16  m_nInsertPos;               // Die Einfugeposition der GroupComps wird von der Gruppe bestimmt.

    friend class OGroupLess;

public:
    OGroup( const OUString& rGroupName );
    virtual ~OGroup();

    sal_Bool operator==( const OGroup& rGroup ) const;

    OUString GetGroupName() const { return m_aGroupName; }
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>  > GetControlModels() const;

    void InsertComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement );
    void RemoveComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement );
    sal_uInt16 Count() const { return sal::static_int_cast< sal_uInt16 >(m_aCompArray.size()); }
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& GetObject( sal_uInt16 nP ) const
        { return m_aCompArray[nP].GetComponent(); }
};

typedef std::map<OUString, OGroup> OGroupArr;
typedef std::vector<OGroupArr::iterator> OActiveGroups;

//========================================================================
class OGroupManager : public ::cppu::WeakImplHelper2< ::com::sun::star::beans::XPropertyChangeListener, ::com::sun::star::container::XContainerListener>
{
    OGroup*         m_pCompGroup;           // Alle Components nach TabIndices sortiert
    OGroupArr       m_aGroupArr;            // Alle Components nach Gruppen sortiert
    OActiveGroups   m_aActiveGroupMap;      // In dieser Map werden die Indices aller Gruppen gehalten,
                                        // die mehr als 1 Element haben

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >
                    m_xContainer;

    // Helper functions
    void InsertElement( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement );
    void RemoveElement( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement );
    void removeFromGroupMap(const OUString& _sGroupName,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xSet);

public:
    OGroupManager(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >& _rxContainer);
    virtual ~OGroupManager();

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw ( ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XContainerListener
    virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& _rEvent) throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& _rEvent) throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& _rEvent) throw ( ::com::sun::star::uno::RuntimeException);

// Other functions
    sal_Int32 getGroupCount();
    void getGroup(sal_Int32 nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> >& _rGroup, OUString& Name);
    void getGroupByName(const OUString& Name, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> >& _rGroup);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> > getControlModels();

    static OUString GetGroupName( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xComponent );
};


//.........................................................................
}   // namespace frm
//.........................................................................

#endif // INCLUDED_FORMS_SOURCE_COMPONENT_GROUPMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
