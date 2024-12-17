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

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <salhelper/simplereferenceobject.hxx>
#include <tools/gen.hxx>
#include <rtl/ref.hxx>

namespace com::sun::star {
    namespace accessibility {
        class XAccessible;
        class XAccessibleContext;
    }
    namespace awt {
        class XWindow;
    }
}

namespace vcl { class IAccessibleBrowseBox; }
namespace vcl { class IAccessibleTabListBox; }
namespace vcl { class IAccessibleTableProvider; }
namespace vcl { class Window; }

class CheckBox;
class ComboBox;
class Edit;
class FixedHyperlink;
class FixedText;
class FormattedField;
class HeaderBar;
class ListBox;
class PushButton;
class RadioButton;
class ScrollBar;
class SvHeaderTabListBox;
class SvtIconChoiceCtrl;
class ToolBox;
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


    /** a function which is able to create a factory for the standard Accessible/Context
        components needed for standard VCL controls

        The returned pointer denotes an instance of the IAccessibleFactory, which has been acquired
        <em>once</em>. The caller is responsible for holding this reference as long as it needs the
        factory, and release it afterwards.
    */
    typedef void* (* GetStandardAccComponentFactory)( );


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
        virtual css::uno::Reference< css::accessibility::XAccessible >
            createAccessibleIconView(
                SvTreeListBox& _rListBox,
                const css::uno::Reference< css::accessibility::XAccessible >& _xParent
            ) const = 0;
        virtual rtl::Reference<vcl::IAccessibleBrowseBox>
            createAccessibleBrowseBox(
                const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                vcl::IAccessibleTableProvider& _rBrowseBox
            ) const = 0;
        virtual rtl::Reference<table::IAccessibleTableControl>
            createAccessibleTableControl(
                const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                table::IAccessibleTable& _rTable
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessible >
            createAccessibleIconChoiceCtrl(
                SvtIconChoiceCtrl& _rIconCtrl,
                const css::uno::Reference< css::accessibility::XAccessible >& _xParent
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
            createAccessibleTextWindowContext(
                vcl::Window* pWindow, TextEngine& rEngine, TextView& rView
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessible >
            createAccessibleBrowseBoxHeaderBar(
                const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
                vcl::IAccessibleTableProvider& _rOwningTable,
                AccessibleBrowseBoxObjType _eObjType
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessible >
            createAccessibleBrowseBoxTableCell(
                const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                vcl::IAccessibleTableProvider& _rBrowseBox,
                sal_Int32 _nRowId,
                sal_uInt16 _nColId,
                sal_Int32 _nOffset
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessible >
            createAccessibleBrowseBoxHeaderCell(
                sal_Int32 _nColumnRowId,
                const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
                vcl::IAccessibleTableProvider& _rBrowseBox,
                AccessibleBrowseBoxObjType  _eObjType
            ) const = 0;

        virtual css::uno::Reference< css::accessibility::XAccessible >
            createAccessibleCheckBoxCell(
                const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                vcl::IAccessibleTableProvider& _rBrowseBox,
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

        /** creates an accessible context for a button window
        */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext(PushButton* pButton) = 0;

        /** creates an accessible context for a checkbox window
        */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext(CheckBox* pCheckBox) = 0;

        /** creates an accessible context for a radio button window
        */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext(RadioButton* pRadioButton) = 0;

        /** creates an accessible context for a listbox window
        */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext(ListBox* pListBox) = 0;

        /** creates an accessible context for a fixed hyperlink window
        */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext(FixedHyperlink* pFixedHyperlink) = 0;

        /** creates an accessible context for a fixed text window
        */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext(FixedText* pFixedText) = 0;

        /** creates an accessible context for a scrollbar window
        */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext(ScrollBar* pScrollBar) = 0;

        /** creates an accessible context for an edit window
        */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext(Edit* pEdit) = 0;

        /** creates an accessible context for a combo box window
        */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext(ComboBox* pComboBox) = 0;

        /** creates an accessible context for a toolbox window
        */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext(ToolBox* pToolBox) = 0;

        /** creates an accessible context for a headerbar window
        */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext(HeaderBar* pHeaderBar) = 0;

        /** creates an accessible context for a numeric field
        */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext(FormattedField* pFormattedField) = 0;

        /** creates an accessible context for a generic window
        */
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext(vcl::Window* pWindow) = 0;

    protected:
        virtual ~IAccessibleFactory() override {}
    };

}   // namespace vcl

class AccessibleFactory : public vcl::IAccessibleFactory
{
public:
    AccessibleFactory();

    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(PushButton* pButton) override;
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(CheckBox* pCheckBox) override;
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(RadioButton* pRadioButton) override;
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(ListBox* pListBox) override;
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(FixedText* pFixedText) override;
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(FixedHyperlink* pFixedHyperlink) override;
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(ScrollBar* pScrollBar) override;
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(Edit* pEdit) override;
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(ComboBox* pComboBox) override;
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(ToolBox* pToolBox) override;
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(HeaderBar* pHeaderBar) override;
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(FormattedField* pFormattedField) override;
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleContext(vcl::Window* pWindow) override;

    virtual vcl::IAccessibleTabListBox*
    createAccessibleTabListBox(const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
                               SvHeaderTabListBox& rBox) const override;

    virtual rtl::Reference<vcl::IAccessibleBrowseBox>
    createAccessibleBrowseBox(const css::uno::Reference<css::accessibility::XAccessible>& _rxParent,
                              vcl::IAccessibleTableProvider& _rBrowseBox) const override;

    virtual rtl::Reference<vcl::table::IAccessibleTableControl> createAccessibleTableControl(
        const css::uno::Reference<css::accessibility::XAccessible>& _rxParent,
        vcl::table::IAccessibleTable& _rTable) const override;

    virtual css::uno::Reference<css::accessibility::XAccessible> createAccessibleIconChoiceCtrl(
        SvtIconChoiceCtrl& _rIconCtrl,
        const css::uno::Reference<css::accessibility::XAccessible>& _xParent) const override;

    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    createAccessibleTextWindowContext(vcl::Window* pWindow, TextEngine& rEngine,
                                      TextView& rView) const override;

    virtual css::uno::Reference<css::accessibility::XAccessible> createAccessibleTreeListBox(
        SvTreeListBox& _rListBox,
        const css::uno::Reference<css::accessibility::XAccessible>& _xParent) const override;

    virtual css::uno::Reference<css::accessibility::XAccessible> createAccessibleIconView(
        SvTreeListBox& _rListBox,
        const css::uno::Reference<css::accessibility::XAccessible>& _xParent) const override;

    virtual css::uno::Reference<css::accessibility::XAccessible> createAccessibleBrowseBoxHeaderBar(
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
        vcl::IAccessibleTableProvider& _rOwningTable,
        AccessibleBrowseBoxObjType _eObjType) const override;

    virtual css::uno::Reference<css::accessibility::XAccessible> createAccessibleBrowseBoxTableCell(
        const css::uno::Reference<css::accessibility::XAccessible>& _rxParent,
        vcl::IAccessibleTableProvider& _rBrowseBox, sal_Int32 _nRowId, sal_uInt16 _nColId,
        sal_Int32 _nOffset) const override;

    virtual css::uno::Reference<css::accessibility::XAccessible>
    createAccessibleBrowseBoxHeaderCell(
        sal_Int32 _nColumnRowId,
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
        vcl::IAccessibleTableProvider& _rBrowseBox,
        AccessibleBrowseBoxObjType _eObjType) const override;

    virtual css::uno::Reference<css::accessibility::XAccessible> createAccessibleCheckBoxCell(
        const css::uno::Reference<css::accessibility::XAccessible>& _rxParent,
        vcl::IAccessibleTableProvider& _rBrowseBox, sal_Int32 _nRowPos, sal_uInt16 _nColPos,
        const TriState& _eState, bool _bIsTriState) const override;

    virtual css::uno::Reference<css::accessibility::XAccessible> createEditBrowseBoxTableCellAccess(
        const css::uno::Reference<css::accessibility::XAccessible>& _rxParent,
        const css::uno::Reference<css::accessibility::XAccessible>& _rxControlAccessible,
        const css::uno::Reference<css::awt::XWindow>& _rxFocusWindow,
        vcl::IAccessibleTableProvider& _rBrowseBox, sal_Int32 _nRowPos,
        sal_uInt16 _nColPos) const override;

protected:
    virtual ~AccessibleFactory() override;
};

#endif // INCLUDED_VCL_ACCESSIBLEFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
