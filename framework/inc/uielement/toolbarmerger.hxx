/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: toolbarmerger.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 17:24:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
