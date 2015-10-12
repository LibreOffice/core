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

#include <osl/mutex.hxx>
#include <vcl/image.hxx>
#include <comphelper/accessibleselectionhelper.hxx>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>

#include <vector>
class SvxShowCharSet;

namespace svx
{
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::accessibility::XAccessible
                                >   OAccessibleHelper_Base_2;

    class SvxShowCharSetAcc;
    /** The class SvxShowCharSetVirtualAcc is used as a virtual class which contains the table and the scrollbar.
        In the vcl control, the table and the scrollbar exists in one class. This is not feasible for the accessibility api.
    */
    class SvxShowCharSetVirtualAcc : public ::comphelper::OAccessibleComponentHelper,
                                     public OAccessibleHelper_Base_2
    {
        VclPtr<SvxShowCharSet>     mpParent; // the vcl control
        SvxShowCharSetAcc*  m_pTable; // the table, which holds the characters shown by the vcl control
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > m_xAcc; // the ref to the table
    protected:
        virtual ~SvxShowCharSetVirtualAcc();

        virtual void SAL_CALL disposing() override;

        virtual ::com::sun::star::awt::Rectangle implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException) override;
    public:
        SvxShowCharSetVirtualAcc( SvxShowCharSet* pParent );

        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleComponent
        virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        //OAccessibleContextHelper
        // XAccessibleContext - still waiting to be overwritten
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override { return this; }
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;


        // call the fireEvent method from the table when it exists.
        void SAL_CALL fireEvent(
                    const sal_Int16 _nEventId,
                    const ::com::sun::star::uno::Any& _rOldValue,
                    const ::com::sun::star::uno::Any& _rNewValue
                );

        // simple access methods
        inline SvxShowCharSetAcc*   getTable() const { return m_pTable; }
        inline SvxShowCharSet* getCharSetControl() const { return mpParent; }
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
        Rectangle                   maRect;
        SvxShowCharSetItemAcc*      m_pItem;
        SvxShowCharSetAcc*          m_pParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > m_xAcc;

        SvxShowCharSetItem( SvxShowCharSet& rParent,SvxShowCharSetAcc*  _pParent,sal_uInt16 _nPos );
        ~SvxShowCharSetItem();

        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >    GetAccessible();
         void                                                                                       ClearAccessible();
    };



    typedef ::cppu::ImplHelper2 <   ::com::sun::star::accessibility::XAccessible,
                                    ::com::sun::star::accessibility::XAccessibleTable
                                >   OAccessibleHelper_Base;

    // - SvxShowCharSetAcc -

    /** The table implemtentation of the vcl control.
    */

    class SvxShowCharSetAcc : public ::comphelper::OAccessibleSelectionHelper,
                              public OAccessibleHelper_Base
    {
        ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > > m_aChildren;
        SvxShowCharSetVirtualAcc* m_pParent; // the virtual parent
    protected:
        virtual void SAL_CALL disposing() override;
    public:
        SvxShowCharSetAcc( SvxShowCharSetVirtualAcc* _pParent );

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleComponent
        virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        //OAccessibleContextHelper
        // XAccessibleContext - still waiting to be overwritten
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override { return this; }
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XAccessibleTable
        virtual sal_Int32 SAL_CALL getAccessibleRowCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getAccessibleColumnCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL getAccessibleRowHeaders(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL getAccessibleColumnHeaders(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleRows(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleColumns(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleCaption(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleSummary(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;


        inline void SAL_CALL fireEvent(
                    const sal_Int16 _nEventId,
                    const ::com::sun::star::uno::Any& _rOldValue,
                    const ::com::sun::star::uno::Any& _rNewValue
                )
        {
            NotifyAccessibleEvent(_nEventId,_rOldValue,_rNewValue);
        }
    protected:

        virtual ~SvxShowCharSetAcc();

        // OCommonAccessibleSelection
        // return if the specified child is visible => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
        virtual bool
            implIsSelected( sal_Int32 nAccessibleChildIndex )
            throw (::com::sun::star::uno::RuntimeException) override;

        // select the specified child => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
        virtual void
            implSelect( sal_Int32 nAccessibleChildIndex, bool bSelect )
            throw (css::lang::IndexOutOfBoundsException,
                   css::uno::RuntimeException,
                   std::exception) override;

        // OCommonAccessibleComponent
        /// implements the calculation of the bounding rectangle - still waiting to be overwritten
        virtual ::com::sun::star::awt::Rectangle implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException) override;
    };


    // - SvxShowCharSetItemAcc -

    typedef ::cppu::ImplHelper2 <   ::com::sun::star::accessibility::XAccessible,
                                    ::com::sun::star::accessibility::XAccessibleAction
                                >   OAccessibleHelper_Base_3;

    /** The child implementation of the table.
    */
    class SvxShowCharSetItemAcc : public ::comphelper::OAccessibleComponentHelper,
                                  public OAccessibleHelper_Base_3
    {
    private:
        SvxShowCharSetItem* mpParent;
    protected:
        virtual ~SvxShowCharSetItemAcc();

        // OCommonAccessibleComponent
        /// implements the calculation of the bounding rectangle - still waiting to be overwritten
        virtual ::com::sun::star::awt::Rectangle implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException) override;
    public:

        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        SvxShowCharSetItemAcc( SvxShowCharSetItem* pParent );

        void    ParentDestroyed();

        // XAccessibleComponent
        virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        //OAccessibleContextHelper
        // XAccessibleContext - still waiting to be overwritten
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override { return this; }

        virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override { return mpParent->m_pParent->getForeground(); }
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override { return mpParent->m_pParent->getBackground(); }

        // XAccessibleAction
        virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::rtl::OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;


        inline void SAL_CALL fireEvent(
                    const sal_Int16 _nEventId,
                    const ::com::sun::star::uno::Any& _rOldValue,
                    const ::com::sun::star::uno::Any& _rNewValue
                )
        {
            NotifyAccessibleEvent(_nEventId,_rOldValue,_rNewValue);
        }
    };
}


#endif // INCLUDED_SVX_SOURCE_INC_CHARMAPACC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
