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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARMERGER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARMERGER_HXX

#include <sal/config.h>

#include <string_view>

#include <uielement/commandinfo.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <vcl/toolbox.hxx>

namespace framework
{

struct AddonsParams
{
    OUString aControlType;
    sal_uInt16 nWidth;
};

struct AddonToolbarItem
{
    OUString aCommandURL;
    OUString aLabel;
    OUString aTarget;
    OUString aContext;
    OUString aControlType;
    sal_uInt16 nWidth;
};

typedef ::std::vector< AddonToolbarItem > AddonToolbarItemContainer;

struct ReferenceToolbarPathInfo
{
    ToolBox::ImplToolItems::size_type nPos;
    bool               bResult;
};

class ToolBarMerger
{
    public:
        static bool       IsCorrectContext( const OUString& aContext, std::u16string_view aModuleIdentifier );

        static void       ConvertSeqSeqToVector( const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& rSequence,
                                                 AddonToolbarItemContainer& rContainer );

        static void       ConvertSequenceToValues( const css::uno::Sequence< css::beans::PropertyValue >& rSequence,
                                                   OUString& rCommandURL,
                                                   OUString& rLabel,
                                                   OUString& rTarget,
                                                   OUString& rContext,
                                                   OUString& rControlType,
                                                   sal_uInt16& rWidth );

        static ReferenceToolbarPathInfo FindReferencePoint( const ToolBox* pToolbar,
                                                            std::u16string_view rReferencePoint );

        static bool       ProcessMergeOperation( ToolBox*                  pToolbar,
                                                 ToolBox::ImplToolItems::size_type nPos,
                                                 ToolBoxItemId&            rItemId,
                                                 CommandToInfoMap&         rCommandMap,
                                                 std::u16string_view       rModuleIdentifier,
                                                 std::u16string_view       rMergeCommand,
                                                 const OUString&           rMergeCommandParameter,
                                                 const AddonToolbarItemContainer& rItems );

        static bool       ProcessMergeFallback( ToolBox*                         pToolbar,
                                                ToolBoxItemId&                   rItemId,
                                                CommandToInfoMap&                rCommandMap,
                                                std::u16string_view       rModuleIdentifier,
                                                std::u16string_view       rMergeCommand,
                                                std::u16string_view       rMergeFallback,
                                                const AddonToolbarItemContainer& rItems );

        static void       MergeItems( ToolBox*                  pToolbar,
                                      ToolBox::ImplToolItems::size_type nPos,
                                      sal_uInt16                nModIndex,
                                      ToolBoxItemId&            rItemId,
                                      CommandToInfoMap&         rCommandMap,
                                      std::u16string_view       rModuleIdentifier,
                                      const AddonToolbarItemContainer& rAddonToolbarItems );

        static void       ReplaceItem( ToolBox*                  pToolbar,
                                       ToolBox::ImplToolItems::size_type nPos,
                                       ToolBoxItemId&            rItemId,
                                       CommandToInfoMap&         rCommandMap,
                                       std::u16string_view       rModuleIdentifier,
                                       const AddonToolbarItemContainer& rAddonToolbarItems );

        static void       RemoveItems( ToolBox*           pToolbar,
                                       ToolBox::ImplToolItems::size_type nPos,
                                       const OUString&    rMergeCommandParameter );

        static rtl::Reference<::cppu::OWeakObject> CreateController(
            const css::uno::Reference< css::uno::XComponentContext > & rxContext,
            const css::uno::Reference< css::frame::XFrame > & xFrame,
            ToolBox*        pToolbar,
            const OUString& rCommandURL,
            ToolBoxItemId   nId,
            sal_uInt16      nWidth,
            std::u16string_view rControlType );

        static void CreateToolbarItem( ToolBox* pToolbox,
                                       ToolBox::ImplToolItems::size_type nPos,
                                       ToolBoxItemId nItemId,
                                       const AddonToolbarItem& rAddonToolbarItem );

    private:
        ToolBarMerger( const ToolBarMerger& ) = delete;
        ToolBarMerger& operator=( const ToolBarMerger& ) = delete;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARMERGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
