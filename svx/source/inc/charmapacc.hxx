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

#ifndef INCLUDED_SVX_SOURCE_INC_CHARMAPACC_HXX
#define INCLUDED_SVX_SOURCE_INC_CHARMAPACC_HXX

#include <vcl/image.hxx>
#include <comphelper/accessibleselectionhelper.hxx>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>

#include <vector>
class SvxShowCharSet;

namespace svx
{
    typedef ::cppu::ImplHelper1 <   css::accessibility::XAccessible
                                >   OAccessibleHelper_Base_2;

    class SvxShowCharSetAcc;
    /** The class SvxShowCharSetVirtualAcc is used as a virtual class which contains the table and the scrollbar.
        In the vcl control, the table and the scrollbar exists in one class. This is not feasible for the accessibility api.
    */
    class SvxShowCharSetVirtualAcc : public ::comphelper::OAccessibleComponentHelper,
                                     public OAccessibleHelper_Base_2
    {
        VclPtr<SvxShowCharSet>             mpParent; // the vcl control
        rtl::Reference<SvxShowCharSetAcc>  m_xTable; // the table, which holds the characters shown by the vcl control
        sal_Int32 getImplAccessibleChildCount() const;
    protected:
        virtual ~SvxShowCharSetVirtualAcc() override;

        virtual void SAL_CALL disposing() override;

        virtual css::awt::Rectangle implGetBounds(  ) override;
    public:
        SvxShowCharSetVirtualAcc( SvxShowCharSet* pParent );

        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleComponent
        virtual void SAL_CALL grabFocus(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
        //OAccessibleContextHelper
        // XAccessibleContext - still waiting to be overwritten
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) override;
        virtual OUString SAL_CALL getAccessibleName(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;

        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override { return this; }
        virtual sal_Int32 SAL_CALL getForeground(  ) override;
        virtual sal_Int32 SAL_CALL getBackground(  ) override;


        // call the fireEvent method from the table when it exists.
        void fireEvent(
                    const sal_Int16 _nEventId,
                    const css::uno::Any& _rOldValue,
                    const css::uno::Any& _rNewValue
                );

        // simple access methods
        SvxShowCharSetAcc*   getTable() const { return m_xTable.get(); }
        SvxShowCharSet*      getCharSetControl() const { return mpParent; }
    };


    class SvxShowCharSetItemAcc;

    // - SvxShowCharSetItem -

    /** Simple struct to hold some information about the single items of the table.
    */
    struct SvxShowCharSetItem
    {
        SvxShowCharSet&             mrParent;
        sal_uInt16                  mnId;
        OUString                    maText;
        tools::Rectangle                   maRect;
        rtl::Reference<SvxShowCharSetItemAcc>  m_xItem;
        SvxShowCharSetAcc*          m_pParent;

        SvxShowCharSetItem( SvxShowCharSet& rParent, SvxShowCharSetAcc* _pParent, sal_uInt16 _nPos );
        ~SvxShowCharSetItem();

        css::uno::Reference< css::accessibility::XAccessible > GetAccessible();
    };


    typedef ::cppu::ImplHelper2 <   css::accessibility::XAccessible,
                                    css::accessibility::XAccessibleTable
                                >   OAccessibleHelper_Base;

    // - SvxShowCharSetAcc -

    /** The table implementation of the vcl control.
    */

    class SvxShowCharSetAcc : public ::comphelper::OAccessibleSelectionHelper,
                              public OAccessibleHelper_Base
    {
        ::std::vector< css::uno::Reference< css::accessibility::XAccessible > > m_aChildren;
        SvxShowCharSetVirtualAcc* m_pParent; // the virtual parent
    protected:
        virtual void SAL_CALL disposing() override;
    public:
        SvxShowCharSetAcc( SvxShowCharSetVirtualAcc* _pParent );

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleComponent
        virtual void SAL_CALL grabFocus(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;

        //OAccessibleContextHelper
        // XAccessibleContext - still waiting to be overwritten
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) override;
        virtual OUString SAL_CALL getAccessibleName(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;

        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override { return this; }
        virtual sal_Int32 SAL_CALL getForeground(  ) override;
        virtual sal_Int32 SAL_CALL getBackground(  ) override;

        // XAccessibleTable
        virtual sal_Int32 SAL_CALL getAccessibleRowCount(  ) override;
        virtual sal_Int32 SAL_CALL getAccessibleColumnCount(  ) override;
        virtual OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow ) override;
        virtual OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn ) override;
        virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
        virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL getAccessibleRowHeaders(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL getAccessibleColumnHeaders(  ) override;
        virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleRows(  ) override;
        virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleColumns(  ) override;
        virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow ) override;
        virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleCaption(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleSummary(  ) override;
        virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) override;
        virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) override;
        virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex ) override;
        virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex ) override;


        void fireEvent(
                    const sal_Int16 _nEventId,
                    const css::uno::Any& _rOldValue,
                    const css::uno::Any& _rNewValue
                )
        {
            NotifyAccessibleEvent(_nEventId,_rOldValue,_rNewValue);
        }
    protected:

        virtual ~SvxShowCharSetAcc() override;

        // OCommonAccessibleSelection
        // return if the specified child is visible => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
        virtual bool
            implIsSelected( sal_Int32 nAccessibleChildIndex ) override;

        // select the specified child => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
        virtual void
            implSelect( sal_Int32 nAccessibleChildIndex, bool bSelect ) override;

        // OCommonAccessibleComponent
        /// implements the calculation of the bounding rectangle - still waiting to be overwritten
        virtual css::awt::Rectangle implGetBounds(  ) override;
    };


    // - SvxShowCharSetItemAcc -

    typedef ::cppu::ImplHelper2 <   css::accessibility::XAccessible,
                                    css::accessibility::XAccessibleAction
                                >   OAccessibleHelper_Base_3;

    /** The child implementation of the table.
    */
    class SvxShowCharSetItemAcc : public ::comphelper::OAccessibleComponentHelper,
                                  public OAccessibleHelper_Base_3
    {
    private:
        SvxShowCharSetItem* mpParent;
    protected:
        virtual ~SvxShowCharSetItemAcc() override;

        // OCommonAccessibleComponent
        /// implements the calculation of the bounding rectangle - still waiting to be overwritten
        virtual css::awt::Rectangle implGetBounds(  ) override;
    public:

        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        SvxShowCharSetItemAcc( SvxShowCharSetItem* pParent );

        void    ParentDestroyed();

        // XAccessibleComponent
        virtual void SAL_CALL grabFocus(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;

        //OAccessibleContextHelper
        // XAccessibleContext - still waiting to be overwritten
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) override;
        virtual OUString SAL_CALL getAccessibleName(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;

        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override { return this; }

        virtual sal_Int32 SAL_CALL getForeground(  ) override { return mpParent->m_pParent->getForeground(); }
        virtual sal_Int32 SAL_CALL getBackground(  ) override { return mpParent->m_pParent->getBackground(); }

        // XAccessibleAction
        virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) override;
        virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) override;
        virtual ::rtl::OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) override;


        void fireEvent(
                    const sal_Int16 _nEventId,
                    const css::uno::Any& _rOldValue,
                    const css::uno::Any& _rNewValue
                )
        {
            NotifyAccessibleEvent(_nEventId,_rOldValue,_rNewValue);
        }
    };
}


#endif // INCLUDED_SVX_SOURCE_INC_CHARMAPACC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
