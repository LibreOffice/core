/*************************************************************************
 *
 *  $RCSfile: GroupManager.hxx,v $
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
#define _FRM_GROUPMANAGER_HXX_

#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
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
        ::std::vector<ELEMENT>::iterator aInsertPos = lower_bound(
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
        ::std::vector<ELEMENT>::const_iterator aExistentPos = ::std::lower_bound(
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
    ::rtl::OUString m_aName;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>    m_xComponent;
    sal_Int32   m_nPos;
    sal_Int16   m_nTabIndex;

    friend class OGroupCompLess;

public:
    OGroupComp(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement, sal_Int32 nInsertPos );
    OGroupComp(const OGroupComp& _rSource);
    OGroupComp();

    sal_Bool operator==( const OGroupComp& rComp ) const;

    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& GetComponent() const { return m_xComponent; }
    sal_Int32   GetPos() const { return m_nPos; }
    sal_Int16   GetTabIndex() const { return m_nTabIndex; }
    ::rtl::OUString GetName() const { return m_aName; }
};

DECLARE_STL_VECTOR(OGroupComp, OGroupCompArr);

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

    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& GetComponent() const { return m_xComponent; }
    const OGroupComp&   GetGroupComponent() const { return m_aGroupComp; }
};

DECLARE_STL_VECTOR(OGroupCompAcc, OGroupCompAccArr);

//========================================================================
class OGroup
{
    OGroupCompArr       m_aCompArray;
    OGroupCompAccArr    m_aCompAccArray;

    ::rtl::OUString m_aGroupName;
    sal_uInt16  m_nInsertPos;               // Die Einfugeposition der GroupComps wird von der Gruppe bestimmt.

    friend class OGroupLess;

public:
    OGroup( const ::rtl::OUString& rGroupName );
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

DECLARE_STL_VECTOR(OGroup, OGroupArr);
DECLARE_STL_VECTOR(sal_uInt32, OUInt32Arr);

//========================================================================
class OGroupManager : public ::cppu::WeakImplHelper2< ::com::sun::star::beans::XPropertyChangeListener, ::com::sun::star::container::XContainerListener>
{
    OGroup* m_pCompGroup;           // Alle Components nach TabIndizes sortiert
    OGroupArr   m_aGroupArr;            // Alle Components nach Gruppen sortiert
    OUInt32Arr m_aActiveGroupMap;   // In dieser Map werden die Indizes aller Gruppen gehalten,
                                    // die mehr als 1 Element haben

    // Helper functions
    void InsertElement( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement );
    void RemoveElement( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement );

public:
    OGroupManager();
    virtual ~OGroupManager();

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt);

// ::com::sun::star::container::XContainerListener
    virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& _rEvent);
    virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& _rEvent);
    virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& _rEvent);

// Other functions
    sal_Int32 getGroupCount();
    void getGroup(sal_Int32 nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> >& _rGroup, ::rtl::OUString& Name);
    void getGroupByName(const ::rtl::OUString& Name, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> >& _rGroup);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> > getControlModels();
};


//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FRM_GROUPMANAGER_HXX_

