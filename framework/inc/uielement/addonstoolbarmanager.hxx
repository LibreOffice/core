/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: addonstoolbarmanager.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-23 07:10:00 $
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

#ifndef __FRAMEWORK_UIELEMENT_ADDONSTOOLBARMANAGER_HXX_
#define __FRAMEWORK_UIELEMENT_ADDONSTOOLBARMANAGER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_UILEMENT_TOOLBARMANAGER_HXX_
#include <uielement/toolbarmanager.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#include <vcl/toolbox.hxx>


namespace framework
{

class ToolBar;
class AddonsToolBarManager : public ToolBarManager
{
    public:
        AddonsToolBarManager( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServicveManager,
                              const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                              const rtl::OUString& rResourceName,
                              ToolBar* pToolBar );
        virtual ~AddonsToolBarManager();

        // XComponent
        void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        virtual void RefreshImages();
        using ToolBarManager::FillToolbar;
        void FillToolbar( const com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > >& rAddonToolbar );

    protected:
        DECL_LINK( Click, ToolBox * );
        DECL_LINK( DoubleClick, ToolBox * );
        DECL_LINK( Command, CommandEvent * );
        DECL_LINK( Select, ToolBox * );
        DECL_LINK( Highlight, ToolBox * );
        DECL_LINK( Activate, ToolBox * );
        DECL_LINK( Deactivate, ToolBox * );
        DECL_LINK( StateChanged, StateChangedType* );
        DECL_LINK( DataChanged, DataChangedEvent* );

        virtual bool MenuItemAllowed( sal_uInt16 ) const;

    private:
        struct AddonsParams
        {
            rtl::OUString aImageId;
            rtl::OUString aTarget;
        };
};

}

#endif // __FRAMEWORK_UIELEMENT_ADDONSTOOLBARMANAGER_HXX_
