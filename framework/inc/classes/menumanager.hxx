/*************************************************************************
 *
 *  $RCSfile: menumanager.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cd $ $Date: 2001-04-02 14:06:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_CLASSES_MENUMANAGER_HXX_
#define __FRAMEWORK_CLASSES_MENUMANAGER_HXX_

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif

#ifndef __SGI_STL_VECTOR
#include <stl/vector>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_OMUTEXMEMBER_HXX_
#include <helper/omutexmember.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#define REFERENCE                                       ::com::sun::star::uno::Reference
#define XFRAME                                          ::com::sun::star::frame::XFrame
#define XDISPATCH                                       ::com::sun::star::frame::XDispatch
#define XDISPATCHPROVIDER                               ::com::sun::star::frame::XDispatchProvider
#define XSTATUSLISTENER                                 ::com::sun::star::frame::XStatusListener
#define XEVENTLISTENER                                  ::com::sun::star::lang::XEventListener
#define FEATURSTATEEVENT                                ::com::sun::star::frame::FeatureStateEvent
#define RUNTIMEEXCEPTION                                ::com::sun::star::uno::RuntimeException
#define EVENTOBJECT                                     ::com::sun::star::lang::EventObject

namespace framework
{

class MenuManager : public XSTATUSLISTENER      ,
                    public OMutexMember         ,
                    public ::cppu::OWeakObject
{
    public:
        MenuManager( REFERENCE< XFRAME >& rFrame, Menu* pMenu, sal_Bool bDelete, sal_Bool bDeleteChildren );
        virtual ~MenuManager();

        // XInterface
        virtual void SAL_CALL acquire() throw( RUNTIMEEXCEPTION )
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw( RUNTIMEEXCEPTION )
            { OWeakObject::release(); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type & rType ) throw( RUNTIMEEXCEPTION );

        // XStatusListener
        virtual void SAL_CALL statusChanged( const FEATURSTATEEVENT& Event ) throw ( RUNTIMEEXCEPTION );

        // XEventListener
        virtual void SAL_CALL disposing( const EVENTOBJECT& Source ) throw ( RUNTIMEEXCEPTION );

        DECL_LINK( Select, Menu * );

        Menu*   GetMenu() const { return m_pVCLMenu; }

    protected:
        DECL_LINK( Highlight, Menu * );
        DECL_LINK( Activate, Menu * );
        DECL_LINK( Deactivate, Menu * );

    private:
        struct MenuItemHandler
        {
            MenuItemHandler( USHORT aItemId, MenuManager* pManager, REFERENCE< XDISPATCH >& rDispatch ) :
                nItemId( aItemId ), pSubMenuManager( pManager ), xMenuItemDispatch( rDispatch ) {}

            USHORT                  nItemId;
            ::rtl::OUString         aMenuItemURL;
            MenuManager*            pSubMenuManager;
            REFERENCE< XDISPATCH >  xMenuItemDispatch;
        };

        MenuItemHandler* GetMenuItemHandler( USHORT nItemId );

        sal_Bool                            m_bInitialized;
        sal_Bool                            m_bDeleteMenu;
        sal_Bool                            m_bDeleteChildren;
        sal_Bool                            m_bActive;
        Menu*                               m_pVCLMenu;
        REFERENCE< XFRAME >                 m_xFrame;
        ::std::vector< MenuItemHandler* >   m_aMenuItemHandlerVector;
};

} // namespace

#endif
