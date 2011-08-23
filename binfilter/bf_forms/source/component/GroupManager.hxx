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

#ifndef _FRM_GROUPMANAGER_HXX_
#define _FRM_GROUPMANAGER_HXX_

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XContainer.hpp>


#include <cppuhelper/implbase2.hxx>

#include <comphelper/stl_types.hxx>
namespace binfilter {
    using namespace ::comphelper;

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
        typename ::std::vector<ELEMENT>::iterator aInsertPos = lower_bound(
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
        typename ::std::vector<ELEMENT>::const_iterator aExistentPos = lower_bound(
            _rArray.begin(),
            _rArray.end(),
            _rNewElement,
            _rCompareOp
        );
        if ((aExistentPos != _rArray.end()) && (*aExistentPos == _rNewElement))
        {	// we have a valid "lower or equal" element and it's really "equal"
            nPos = aExistentPos - _rArray.begin();
            return sal_True;
        }
        nPos = -1;
        return sal_False;
    }

//========================================================================
class OGroupComp
{
    ::rtl::OUString	m_aName;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> 	m_xComponent;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> 	m_xControlModel;
    sal_Int32	m_nPos;
    sal_Int16	m_nTabIndex;

    friend class OGroupCompLess;

public:
    OGroupComp(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement, sal_Int32 nInsertPos );
    OGroupComp(const OGroupComp& _rSource);
    OGroupComp();

    sal_Bool operator==( const OGroupComp& rComp ) const;

    inline const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& GetComponent() const { return m_xComponent; }
    inline const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>&	GetControlModel() const { return m_xControlModel; }

    sal_Int32	GetPos() const { return m_nPos; }
    sal_Int16	GetTabIndex() const { return m_nTabIndex; }
    ::rtl::OUString GetName() const { return m_aName; }
};

DECLARE_STL_VECTOR(OGroupComp, OGroupCompArr);

//========================================================================
class OGroupComp;
class OGroupCompAcc
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> 	m_xComponent;
    
    OGroupComp										m_aGroupComp;

    friend class OGroupCompAccLess;

public:
    OGroupCompAcc(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement, const OGroupComp& _rGroupComp );

    sal_Bool operator==( const OGroupCompAcc& rCompAcc ) const;

    inline const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>&	GetComponent() const { return m_xComponent; }
    const OGroupComp&	GetGroupComponent() const { return m_aGroupComp; }
};

DECLARE_STL_VECTOR(OGroupCompAcc, OGroupCompAccArr);

//========================================================================
class OGroup
{
    OGroupCompArr		m_aCompArray;
    OGroupCompAccArr	m_aCompAccArray;

    ::rtl::OUString m_aGroupName;
    sal_uInt16	m_nInsertPos;				// Die Einfugeposition der GroupComps wird von der Gruppe bestimmt.

    friend class OGroupLess;

public:
    OGroup( const ::rtl::OUString& rGroupName );
#ifdef DBG_UTIL
    OGroup( const OGroup& _rSource );	// just to ensure the DBG_CTOR call
#endif
    virtual ~OGroup();

    sal_Bool operator==( const OGroup& rGroup ) const;

    ::rtl::OUString GetGroupName() const { return m_aGroupName; }
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>  > GetControlModels() const;

    void InsertComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement );
    void RemoveComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement );
    sal_uInt16 Count() const { return m_aCompArray.size(); }
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& GetObject( sal_uInt16 nP ) const
        { return m_aCompArray[nP].GetComponent(); }
};

DECLARE_STL_USTRINGACCESS_MAP(OGroup, OGroupArr);
DECLARE_STL_VECTOR(OGroupArr::iterator, OActiveGroups);

//========================================================================
class OGroupManager	: public ::cppu::WeakImplHelper2< ::com::sun::star::beans::XPropertyChangeListener, ::com::sun::star::container::XContainerListener>
{
    OGroup*			m_pCompGroup;			// Alle Components nach TabIndizes sortiert
    OGroupArr		m_aGroupArr;			// Alle Components nach Gruppen sortiert
    OActiveGroups	m_aActiveGroupMap;		// In dieser Map werden die Indizes aller Gruppen gehalten,
                                        // die mehr als 1 Element haben

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >
                    m_xContainer;

    // Helper functions
    void InsertElement( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement );
    void RemoveElement( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement );
    void removeFromGroupMap(const ::rtl::OUString& _sGroupName,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xSet);

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
    void getGroup(sal_Int32 nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> >& _rGroup, ::rtl::OUString& Name);
    void getGroupByName(const ::rtl::OUString& Name, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> >& _rGroup);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> > getControlModels();
};


//.........................................................................
}	// namespace frm
//.........................................................................

}//end of namespace binfilter
#endif // _FRM_GROUPMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
