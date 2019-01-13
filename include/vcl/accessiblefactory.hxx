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

#ifndef INCLUDED_VCL_ACCESSIBLEFACTORY_HXX
#define INCLUDED_VCL_ACCESSIBLEFACTORY_HXX

#include <vcl/AccessibleBrowseBoxObjType.hxx>

#include <com/sun/star/uno/Reference.hxx>

#include <salhelper/simplereferenceobject.hxx>
#include <tools/gen.hxx>

namespace com { namespace sun { namespace star {
    namespace accessibility {
        class XAccessible;
        class XAccessibleContext;
    }
    namespace awt {
        class XWindow;
    }
} } }

namespace vcl { class IAccessibleBrowseBox; }
namespace vcl { class IAccessibleTabListBox; }
namespace vcl { class IAccessibleTableProvider; }

class SvHeaderTabListBox;
class SvtIconChoiceCtrl;
class TabBar;
class SvTreeListBox;
class VCLXWindow;
class TextEngine;
class TextView;

namespace vcl
{
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
    typedef void* (* GetSvtAccessibilityComponentFactory)( );


    //= IAccessibleFactory

    class IAccessibleFactory : public virtual ::salhelper::SimpleReferenceObject
    {
    public:
        virtual vcl::IAccessibleTabListBox*
            createAccessibleTabListBox(
                const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
                SvHeaderTabListBox& rBox
            ) const = 0;
        virtual css::uno::Reference< css::accessibility::XAccessible >
            createAccessibleTreeListBox(
                SvTreeListBox& _rListBox,
                const css::uno::Reference< css::accessibility::XAccessible >& _xParent
            ) const = 0;
        virtual vcl::IAccessibleBrowseBox*
            createAccessibleBrowseBox(
                const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                vcl::IAccessibleTableProvider& _rBrowseBox
            ) const = 0;
        virtual table::IAccessibleTableControl*
            createAccessibleTableControl(
                const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                table::IAccessibleTable& _rTable
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessible >
            createAccessibleIconChoiceCtrl(
                SvtIconChoiceCtrl& _rIconCtrl,
                const css::uno::Reference< css::accessibility::XAccessible >& _xParent
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessible >
            createAccessibleTabBar(
                TabBar& _rTabBar
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
            createAccessibleTextWindowContext(
                VCLXWindow* pVclXWindow, TextEngine& rEngine, TextView& rView
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessible >
            createAccessibleBrowseBoxHeaderBar(
                const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
                vcl::IAccessibleTableProvider& _rOwningTable,
                vcl::AccessibleBrowseBoxObjType _eObjType
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessible >
            createAccessibleBrowseBoxTableCell(
                const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                vcl::IAccessibleTableProvider& _rBrowseBox,
                const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
                sal_Int32 _nRowId,
                sal_uInt16 _nColId,
                sal_Int32 _nOffset
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessible >
            createAccessibleBrowseBoxHeaderCell(
                sal_Int32 _nColumnRowId,
                const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
                vcl::IAccessibleTableProvider& _rBrowseBox,
                const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
                vcl::AccessibleBrowseBoxObjType  _eObjType
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessible >
            createAccessibleCheckBoxCell(
                const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                vcl::IAccessibleTableProvider& _rBrowseBox,
                const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
                sal_Int32 _nRowPos,
                sal_uInt16 _nColPos,
                const TriState& _eState,
                bool _bIsTriState
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessible >
            createEditBrowseBoxTableCellAccess(
                const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                const css::uno::Reference< css::accessibility::XAccessible >& _rxControlAccessible,
                const css::uno::Reference< css::awt::XWindow >& _rxFocusWindow,
                vcl::IAccessibleTableProvider& _rBrowseBox,
                sal_Int32 _nRowPos,
                sal_uInt16 _nColPos
            ) const = 0;

    protected:
        virtual ~IAccessibleFactory() override {}
    };

}   // namespace vcl


#endif // INCLUDED_VCL_ACCESSIBLEFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
