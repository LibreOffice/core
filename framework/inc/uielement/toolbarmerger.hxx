/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: toolbarmerger.hxx,v $
 * $Revision: 1.4 $
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
    ::rtl::OUString aImageId;
    ::rtl::OUString aTarget;
    ::rtl::OUString aControlType;
};

struct AddonToolbarItem
{
    ::rtl::OUString aCommandURL;
    ::rtl::OUString aLabel;
    ::rtl::OUString aImageIdentifier;
    ::rtl::OUString aTarget;
    ::rtl::OUString aContext;
    ::rtl::OUString aControlType;
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
        static bool       IsCorrectContext( const ::rtl::OUString& aContext, const ::rtl::OUString& aModuleIdentifier );

        static bool       ConvertSeqSeqToVector( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > rSequence,
                                                 AddonToolbarItemContainer& rContainer );

        static void       ConvertSequenceToValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > rSequence,
                                                   ::rtl::OUString& rCommandURL,
                                                   ::rtl::OUString& rLabel,
                                                   ::rtl::OUString& rImageIdentifier,
                                                   ::rtl::OUString& rTarget,
                                                   ::rtl::OUString& rContext,
                                                   ::rtl::OUString& rControlType,
                                                   sal_uInt16&      rWidth );

        static ReferenceToolbarPathInfo FindReferencePoint( ToolBox*               pToolbar,
                                                            const ::rtl::OUString& rReferencePoint );

        static bool       ProcessMergeOperation( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                                 ToolBox*                         pToolbar,
                                                 sal_uInt16                       nPos,
                                                 sal_uInt16&                      rItemId,
                                                 CommandToInfoMap&                rCommandMap,
                                                 const ::rtl::OUString&           rModuleIdentifier,
                                                 const ::rtl::OUString&           rMergeCommand,
                                                 const ::rtl::OUString&           rMergeCommandParameter,
                                                 const AddonToolbarItemContainer& rItems );

        static bool       ProcessMergeFallback( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                                ToolBox*                         pToolbar,
                                                sal_uInt16                       nPos,
                                                sal_uInt16&                      rItemId,
                                                CommandToInfoMap&                rCommandMap,
                                                const ::rtl::OUString&           rModuleIdentifier,
                                                const ::rtl::OUString&           rMergeCommand,
                                                const ::rtl::OUString&           rMergeFallback,
                                                const AddonToolbarItemContainer& rItems );

        static bool       MergeItems( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                      ToolBox*                  pToolbar,
                                      sal_uInt16                nPos,
                                      sal_uInt16                nModIndex,
                                      sal_uInt16&               rItemId,
                                      CommandToInfoMap&         rCommandMap,
                                      const ::rtl::OUString&    rModuleIdentifier,
                                      const AddonToolbarItemContainer& rAddonToolbarItems );

        static bool       ReplaceItem( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                       ToolBox*                  pToolbar,
                                       sal_uInt16                nPos,
                                       sal_uInt16&               rItemId,
                                       CommandToInfoMap&         rCommandMap,
                                       const ::rtl::OUString&    rModuleIdentifier,
                                       const AddonToolbarItemContainer& rAddonToolbarItems );

        static bool       RemoveItems( ToolBox*                  pToolbar,
                                       sal_uInt16                nPos,
                                       const ::rtl::OUString&    rMergeCommandParameter );

        static ::cppu::OWeakObject* CreateController(
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMGR,
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame,
            ToolBox*               pToolbar,
            const ::rtl::OUString& rCommandURL,
            sal_uInt16             nId,
            sal_uInt16             nWidth,
            const ::rtl::OUString& rControlType );

        static void CreateToolbarItem( ToolBox* pToolbox,
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
