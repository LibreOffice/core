/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
