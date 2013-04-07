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

#ifndef __FRAMEWORK_UIELEMENT_TOOLBARMERGER_HXX_
#define __FRAMEWORK_UIELEMENT_TOOLBARMERGER_HXX_

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
    sal_uInt16      nWidth;
};

typedef ::std::vector< AddonToolbarItem > AddonToolbarItemContainer;

struct ReferenceToolbarPathInfo
{
    ToolBox*           pToolbar;
    sal_uInt16         nPos;
    bool               bResult;
};

class ToolBarMerger
{
    public:
        static bool       IsCorrectContext( const OUString& aContext, const OUString& aModuleIdentifier );

        static bool       ConvertSeqSeqToVector( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > rSequence,
                                                 AddonToolbarItemContainer& rContainer );

        static void       ConvertSequenceToValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > rSequence,
                                                   OUString& rCommandURL,
                                                   OUString& rLabel,
                                                   OUString& rImageIdentifier,
                                                   OUString& rTarget,
                                                   OUString& rContext,
                                                   OUString& rControlType,
                                                   sal_uInt16&      rWidth );

        static ReferenceToolbarPathInfo FindReferencePoint( ToolBox*               pToolbar,
                                                            const OUString& rReferencePoint );

        static bool       ProcessMergeOperation( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                                 ToolBox*                         pToolbar,
                                                 sal_uInt16                       nPos,
                                                 sal_uInt16&                      rItemId,
                                                 CommandToInfoMap&                rCommandMap,
                                                 const OUString&           rModuleIdentifier,
                                                 const OUString&           rMergeCommand,
                                                 const OUString&           rMergeCommandParameter,
                                                 const AddonToolbarItemContainer& rItems );

        static bool       ProcessMergeFallback( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                                ToolBox*                         pToolbar,
                                                sal_uInt16                       nPos,
                                                sal_uInt16&                      rItemId,
                                                CommandToInfoMap&                rCommandMap,
                                                const OUString&           rModuleIdentifier,
                                                const OUString&           rMergeCommand,
                                                const OUString&           rMergeFallback,
                                                const AddonToolbarItemContainer& rItems );

        static bool       MergeItems( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                      ToolBox*                  pToolbar,
                                      sal_uInt16                nPos,
                                      sal_uInt16                nModIndex,
                                      sal_uInt16&               rItemId,
                                      CommandToInfoMap&         rCommandMap,
                                      const OUString&    rModuleIdentifier,
                                      const AddonToolbarItemContainer& rAddonToolbarItems );

        static bool       ReplaceItem( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                       ToolBox*                  pToolbar,
                                       sal_uInt16                nPos,
                                       sal_uInt16&               rItemId,
                                       CommandToInfoMap&         rCommandMap,
                                       const OUString&    rModuleIdentifier,
                                       const AddonToolbarItemContainer& rAddonToolbarItems );

        static bool       RemoveItems( ToolBox*                  pToolbar,
                                       sal_uInt16                nPos,
                                       const OUString&    rMergeCommandParameter );

        static ::cppu::OWeakObject* CreateController(
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMGR,
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame,
            ToolBox*               pToolbar,
            const OUString& rCommandURL,
            sal_uInt16             nId,
            sal_uInt16             nWidth,
            const OUString& rControlType );

        static void CreateToolbarItem( ToolBox* pToolbox,
                                       CommandToInfoMap& rCommandMap,
                                       sal_uInt16 nPos,
                                       sal_uInt16 nItemId,
                                       const AddonToolbarItem& rAddonToolbarItem );

    private:
        ToolBarMerger();
        ToolBarMerger( const ToolBarMerger& );
        ToolBarMerger& operator=( const ToolBarMerger& );
};

} // namespace framework

#endif // __FRAMEWORK_UIELEMENT_TOOLBARMERGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
