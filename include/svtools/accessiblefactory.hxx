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

#ifndef INCLUDED_SVTOOLS_ACCESSIBLEFACTORY_HXX
#define INCLUDED_SVTOOLS_ACCESSIBLEFACTORY_HXX

#include <svtools/AccessibleBrowseBoxObjType.hxx>
#include <svtools/accessibletableprovider.hxx>

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


namespace svt
{


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


    //= IAccessibleFactory

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
                VCLXWindow* pVclXWindow, TextEngine& rEngine, TextView& rView
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


}   // namespace svt


#endif // INCLUDED_SVTOOLS_ACCESSIBLEFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
