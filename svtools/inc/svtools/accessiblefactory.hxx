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

#ifndef SVTOOLS_ACCESSIBLE_FACTORY_HXX
#define SVTOOLS_ACCESSIBLE_FACTORY_HXX

#include "AccessibleBrowseBoxObjType.hxx"
#include "accessibletableprovider.hxx"

#include <com/sun/star/uno/Reference.hxx>

#include <rtl/ref.hxx>

namespace com { namespace sun { namespace star {
    namespace accessibility {
        class XAccessible;
        class XAccessibleContext;
    }
    namespace awt {
        class XWindow;
    }
} } }
class SvHeaderTabListBox;
class SvtIconChoiceCtrl;
class TabBar;
class SvTreeListBox;
class VCLXWindow;
class TextEngine;
class TextView;

//........................................................................
namespace svt
{
//........................................................................

    class ToolPanelDeck;
    class IToolPanelDeck;
    class PanelTabBar;
    namespace table
    {
        class IAccessibleTable;
        class IAccessibleTableControl;
    }

    /** a function which is able to create a factory for the standard Accessible/Context
        components needed for standard toolkit controls

        The returned pointer denotes an instance of the IAccessibleFactory, which has been acquired
        <em>once</em>. The caller is responsible for holding this reference as long as it needs the
        factory, and release it afterwards.
    */
    typedef void* (SAL_CALL * GetSvtAccessibilityComponentFactory)( );

    //================================================================
    //= IAccessibleFactory
    //================================================================
    class IAccessibleFactory : public ::rtl::IReference
    {
    public:
        virtual IAccessibleTabListBox*
            createAccessibleTabListBox(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
                SvHeaderTabListBox& rBox
            ) const = 0;

        virtual IAccessibleBrowseBox*
            createAccessibleBrowseBox(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
                IAccessibleTableProvider& _rBrowseBox
            ) const = 0;
        virtual table::IAccessibleTableControl*
            createAccessibleTableControl(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
                table::IAccessibleTable& _rTable
            ) const = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleIconChoiceCtrl(
                SvtIconChoiceCtrl& _rIconCtrl,
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _xParent
            ) const = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleTabBar(
                TabBar& _rTabBar
            ) const = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleTextWindowContext(
                VCLXWindow* pVclXWindow, TextEngine& rEngine, TextView& rView, bool bCompoundControlChild
            ) const = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleTreeListBox(
                SvTreeListBox& _rListBox,
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _xParent
            ) const = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleBrowseBoxHeaderBar(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
                IAccessibleTableProvider& _rOwningTable,
                AccessibleBrowseBoxObjType _eObjType
            ) const = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleBrowseBoxTableCell(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
                IAccessibleTableProvider& _rBrowseBox,
                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
                sal_Int32 _nRowId,
                sal_uInt16 _nColId,
                sal_Int32 _nOffset
            ) const = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleBrowseBoxHeaderCell(
                sal_Int32 _nColumnRowId,
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
                IAccessibleTableProvider& _rBrowseBox,
                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
                AccessibleBrowseBoxObjType  _eObjType
            ) const = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createAccessibleCheckBoxCell(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
                IAccessibleTableProvider& _rBrowseBox,
                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
                sal_Int32 _nRowPos,
                sal_uInt16 _nColPos,
                const TriState& _eState,
                sal_Bool _bIsTriState
            ) const = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            createEditBrowseBoxTableCellAccess(
                const ::com::sun::star::uno::Reference< com::sun::star::accessibility::XAccessible >& _rxParent,
                const ::com::sun::star::uno::Reference< com::sun::star::accessibility::XAccessible >& _rxControlAccessible,
                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _rxFocusWindow,
                IAccessibleTableProvider& _rBrowseBox,
                sal_Int32 _nRowPos,
                sal_uInt16 _nColPos
            ) const = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleToolPanelDeck(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rAccessibleParent,
                ::svt::ToolPanelDeck& i_rPanelDeck
            ) = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            createAccessibleToolPanelTabBar(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rAccessibleParent,
                ::svt::IToolPanelDeck& i_rPanelDeck,
                ::svt::PanelTabBar& i_rTabBar
            ) = 0;

    protected:
        ~IAccessibleFactory() {}
    };

//........................................................................
}   // namespace svt
//........................................................................

#endif // SVTOOLS_ACCESSIBLE_FACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
