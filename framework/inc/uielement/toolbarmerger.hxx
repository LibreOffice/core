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

#include <uielement/comboboxtoolbarcontroller.hxx>
#include <uielement/imagebuttontoolbarcontroller.hxx>
#include <uielement/togglebuttontoolbarcontroller.hxx>
#include <uielement/buttontoolbarcontroller.hxx>
#include <uielement/spinfieldtoolbarcontroller.hxx>
#include <uielement/edittoolbarcontroller.hxx>
#include <uielement/dropdownboxtoolbarcontroller.hxx>
#include <uielement/commandinfo.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <svtools/toolboxcontroller.hxx>

#include <rtl/ustring.hxx>
#include <vcl/toolbox.hxx>

namespace framework
{

struct AddonsParams
{
    OUString aImageId;
    OUString aControlType;
    sal_uInt16 nWidth;
};

struct AddonToolbarItem
{
    OUString aCommandURL;
    OUString aLabel;
    OUString aImageIdentifier;
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
        static bool       IsCorrectContext( const OUString& aContext, const OUString& aModuleIdentifier );

        static void       ConvertSeqSeqToVector( const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& rSequence,
                                                 AddonToolbarItemContainer& rContainer );

        static void       ConvertSequenceToValues( const css::uno::Sequence< css::beans::PropertyValue >& rSequence,
                                                   OUString& rCommandURL,
                                                   OUString& rLabel,
                                                   OUString& rImageIdentifier,
                                                   OUString& rTarget,
                                                   OUString& rContext,
                                                   OUString& rControlType,
                                                   sal_uInt16& rWidth );

        static ReferenceToolbarPathInfo FindReferencePoint( ToolBox* pToolbar,
                                                            const OUString& rReferencePoint );

        static bool       ProcessMergeOperation( ToolBox*                  pToolbar,
                                                 ToolBox::ImplToolItems::size_type nPos,
                                                 sal_uInt16&               rItemId,
                                                 CommandToInfoMap&         rCommandMap,
                                                 const OUString&           rModuleIdentifier,
                                                 const OUString&           rMergeCommand,
                                                 const OUString&           rMergeCommandParameter,
                                                 const AddonToolbarItemContainer& rItems );

        static bool       ProcessMergeFallback( ToolBox*                         pToolbar,
                                                sal_uInt16&                      rItemId,
                                                CommandToInfoMap&                rCommandMap,
                                                const OUString&           rModuleIdentifier,
                                                const OUString&           rMergeCommand,
                                                const OUString&           rMergeFallback,
                                                const AddonToolbarItemContainer& rItems );

        static void       MergeItems( ToolBox*                  pToolbar,
                                      ToolBox::ImplToolItems::size_type nPos,
                                      sal_uInt16                nModIndex,
                                      sal_uInt16&               rItemId,
                                      CommandToInfoMap&         rCommandMap,
                                      const OUString&           rModuleIdentifier,
                                      const AddonToolbarItemContainer& rAddonToolbarItems );

        static void       ReplaceItem( ToolBox*                  pToolbar,
                                       ToolBox::ImplToolItems::size_type nPos,
                                       sal_uInt16&               rItemId,
                                       CommandToInfoMap&         rCommandMap,
                                       const OUString&           rModuleIdentifier,
                                       const AddonToolbarItemContainer& rAddonToolbarItems );

        static void       RemoveItems( ToolBox*           pToolbar,
                                       ToolBox::ImplToolItems::size_type nPos,
                                       const OUString&    rMergeCommandParameter );

        static ::cppu::OWeakObject* CreateController(
            const css::uno::Reference< css::uno::XComponentContext > & rxContext,
            const css::uno::Reference< css::frame::XFrame > & xFrame,
            ToolBox*        pToolbar,
            const OUString& rCommandURL,
            sal_uInt16      nId,
            sal_uInt16      nWidth,
            const OUString& rControlType );

        static void CreateToolbarItem( ToolBox* pToolbox,
                                       ToolBox::ImplToolItems::size_type nPos,
                                       sal_uInt16 nItemId,
                                       const AddonToolbarItem& rAddonToolbarItem );

    private:
        ToolBarMerger( const ToolBarMerger& ) = delete;
        ToolBarMerger& operator=( const ToolBarMerger& ) = delete;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARMERGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
