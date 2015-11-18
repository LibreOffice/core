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
    OUString aTarget;
    OUString aControlType;
};

struct AddonToolbarItem
{
    OUString aCommandURL;
    OUString aLabel;
    OUString aImageIdentifier;
    OUString aTarget;
    OUString aContext;
    OUString aControlType;
};

typedef ::std::vector< AddonToolbarItem > AddonToolbarItemContainer;

struct ReferenceToolbarPathInfo
{
    VclPtr<ToolBox>    pToolbar;
    sal_uInt16         nPos;
    bool               bResult;
};

class ToolBarMerger
{
    public:
        static bool       IsCorrectContext( const OUString& aContext, const OUString& aModuleIdentifier );

        static bool       ConvertSeqSeqToVector( const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& rSequence,
                                                 AddonToolbarItemContainer& rContainer );

        static void       ConvertSequenceToValues( const css::uno::Sequence< css::beans::PropertyValue >& rSequence,
                                                   OUString& rCommandURL,
                                                   OUString& rLabel,
                                                   OUString& rImageIdentifier,
                                                   OUString& rTarget,
                                                   OUString& rContext,
                                                   OUString& rControlType );

        static ReferenceToolbarPathInfo FindReferencePoint( ToolBox* pToolbar,
                                                            const OUString& rReferencePoint );

        static bool       ProcessMergeOperation( const css::uno::Reference< css::frame::XFrame >& xFrame,
                                                 ToolBox*                  pToolbar,
                                                 sal_uInt16                nPos,
                                                 sal_uInt16&               rItemId,
                                                 CommandToInfoMap&         rCommandMap,
                                                 const OUString&           rModuleIdentifier,
                                                 const OUString&           rMergeCommand,
                                                 const OUString&           rMergeCommandParameter,
                                                 const AddonToolbarItemContainer& rItems );

        static bool       ProcessMergeFallback( const css::uno::Reference< css::frame::XFrame >& xFrame,
                                                ToolBox*                         pToolbar,
                                                sal_uInt16                       nPos,
                                                sal_uInt16&                      rItemId,
                                                CommandToInfoMap&                rCommandMap,
                                                const OUString&           rModuleIdentifier,
                                                const OUString&           rMergeCommand,
                                                const OUString&           rMergeFallback,
                                                const AddonToolbarItemContainer& rItems );

        static bool       MergeItems( const css::uno::Reference< css::frame::XFrame >& xFrame,
                                      ToolBox*                  pToolbar,
                                      sal_uInt16                nPos,
                                      sal_uInt16                nModIndex,
                                      sal_uInt16&               rItemId,
                                      CommandToInfoMap&         rCommandMap,
                                      const OUString&           rModuleIdentifier,
                                      const AddonToolbarItemContainer& rAddonToolbarItems );

        static bool       ReplaceItem( const css::uno::Reference< css::frame::XFrame >& xFrame,
                                       ToolBox*                  pToolbar,
                                       sal_uInt16                nPos,
                                       sal_uInt16&               rItemId,
                                       CommandToInfoMap&         rCommandMap,
                                       const OUString&           rModuleIdentifier,
                                       const AddonToolbarItemContainer& rAddonToolbarItems );

        static bool       RemoveItems( ToolBox*           pToolbar,
                                       sal_uInt16         nPos,
                                       const OUString&    rMergeCommandParameter );

        static ::cppu::OWeakObject* CreateController(
            const css::uno::Reference< css::uno::XComponentContext > & rxContext,
            const css::uno::Reference< css::frame::XFrame > & xFrame,
            ToolBox*        pToolbar,
            const OUString& rCommandURL,
            sal_uInt16      nId,
            const OUString& rControlType );

        static void CreateToolbarItem( ToolBox* pToolbox,
                                       sal_uInt16 nPos,
                                       sal_uInt16 nItemId,
                                       const AddonToolbarItem& rAddonToolbarItem );

    private:
        ToolBarMerger( const ToolBarMerger& ) = delete;
        ToolBarMerger& operator=( const ToolBarMerger& ) = delete;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARMERGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
