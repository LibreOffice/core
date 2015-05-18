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

/*
 * The GroupManager listens at the StarForm for FormComponent insertion and removal as well as
 * its properties "Name" and "TabIndex" and updates its Group using this information.
 *
 * The GroupManager manages a Group in which all Controls are sorted by TabIndex.
 * It also manages an array of Groups, in which each FormComponent is assigned a
 * Group according to its name.
 * Each Group is activated using a Map, if they contain more than one element.
 *
 * The Groups manage the FormComponents internally using two arrays.
 * In the GroupCompArray the Components are sorted by TabIndex and insertion position.
 * Because this array is accessed via the FormComponent, we also have the GroupCompAccessArray
 * in which the FormComponents are sorted by their storage address.
 * Every element of the GroupCompArray has a pointer to the GroupCompArray.
 */
namespace frm
{


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
    bool seek_entry(const ::std::vector<ELEMENT>& _rArray, const ELEMENT& _rNewElement, sal_Int32& nPos, const LESS_COMPARE& _rCompareOp)
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
            return true;
        }
        nPos = -1;
        return false;
    }


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

    bool operator==( const OGroupComp& rComp ) const;

    inline const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& GetComponent() const { return m_xComponent; }
    inline const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>&   GetControlModel() const { return m_xControlModel; }

    sal_Int32   GetPos() const { return m_nPos; }
    sal_Int16   GetTabIndex() const { return m_nTabIndex; }
    OUString GetName() const { return m_aName; }
};

typedef std::vector<OGroupComp> OGroupCompArr;


class OGroupComp;
class OGroupCompAcc
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>    m_xComponent;

    OGroupComp                                      m_aGroupComp;

    friend class OGroupCompAccLess;

public:
    OGroupCompAcc(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rxElement, const OGroupComp& _rGroupComp );

    bool operator==( const OGroupCompAcc& rCompAcc ) const;

    inline const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>&  GetComponent() const { return m_xComponent; }
    const OGroupComp&   GetGroupComponent() const { return m_aGroupComp; }
};

typedef std::vector<OGroupCompAcc> OGroupCompAccArr;


class OGroup
{
    OGroupCompArr       m_aCompArray;
    OGroupCompAccArr    m_aCompAccArray;

    OUString m_aGroupName;
    sal_uInt16  m_nInsertPos; // The insertion position of the GroupComps is determind by the Group

    friend class OGroupLess;

public:
    OGroup( const OUString& rGroupName );
    virtual ~OGroup();

    bool operator==( const OGroup& rGroup ) const;

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


class OGroupManager : public ::cppu::WeakImplHelper2< ::com::sun::star::beans::XPropertyChangeListener, ::com::sun::star::container::XContainerListener>
{
    OGroup*         m_pCompGroup;           // Sort all Components by TabIndices
    OGroupArr       m_aGroupArr;            // Sort all Components by group
    OActiveGroups   m_aActiveGroupMap;      // This map contains all indices of all groups with more than 1 element

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
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// ::com::sun::star::container::XContainerListener
    virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& _rEvent) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& _rEvent) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& _rEvent) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// Other functions
    sal_Int32 getGroupCount();
    void getGroup(sal_Int32 nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> >& _rGroup, OUString& Name);
    void getGroupByName(const OUString& Name, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> >& _rGroup);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> > getControlModels();

    static OUString GetGroupName( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xComponent );
};



}


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_GROUPMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
