/*************************************************************************
 *
 *  $RCSfile: statusbarmanager.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-17 14:57:15 $
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

#ifndef __FRAMEWORK_UIELEMENT_STATUSBARMANAGER_HXX_
#define __FRAMEWORK_UIELEMENT_STATUSBARMANAGER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

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
#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
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
#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XUICONTROLLERREGISTRATION_HPP_
#include <drafts/com/sun/star/frame/XUIControllerRegistration.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATION_HPP_
#include <drafts/com/sun/star/ui/XUIConfiguration.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <drafts/com/sun/star/frame/XModuleManager.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
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
#include <vcl/status.hxx>

namespace framework
{

class FrameworkStatusBar;
class StatusBarManager : public ::com::sun::star::frame::XFrameActionListener         ,
                         public ::com::sun::star::lang::XComponent                    ,
                         public ::com::sun::star::lang::XTypeProvider                 ,
                         public ::drafts::com::sun::star::ui::XUIConfigurationListener,
                         public ThreadHelpBase                                        ,
                         public ::cppu::OWeakObject
{
    friend class FrameworkStatusBar;

    public:
        StatusBarManager( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServicveManager,
                          const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                          const rtl::OUString& rResourceName,
                          StatusBar* pStatusBar );
        virtual ~StatusBarManager();

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        StatusBar* GetStatusBar() const;

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const com::sun::star::frame::FrameActionEvent& Action ) throw ( ::com::sun::star::uno::RuntimeException );

        // XEventListener
        virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

        // XUIConfigurationListener
        virtual void SAL_CALL elementInserted( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );
        void SAL_CALL addEventListener( const com::sun::star::uno::Reference< XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
        void SAL_CALL removeEventListener( const com::sun::star::uno::Reference< XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException );

        void FillStatusBar( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rStatusBarData );

    protected:
        void StateChanged( StateChangedType nType );
        void DataChanged( const DataChangedEvent& rDCEvt );
        void UserDraw( const UserDrawEvent& rUDEvt );
        void Command( const CommandEvent& rEvt );
        DECL_LINK( Click, StatusBar* );
        DECL_LINK( DoubleClick, StatusBar* );

        void RemoveControllers();
        rtl::OUString RetrieveLabelFromCommand( const rtl::OUString& aCmdURL );
        void CreateControllers();
        void UpdateControllers();
        void AddFrameActionListener();

    protected:
        typedef std::vector< ::com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener > > StatusBarControllerVector;

        sal_Bool                                                                                        m_bDisposed : 1,
                                                                                                        m_bModuleIdentified : 1,
                                                                                                        m_bFrameActionRegistered : 1,
                                                                                                        m_bUpdateControllers : 1;
        StatusBar*                                                                                      m_pStatusBar;
        rtl::OUString                                                                                   m_aModuleIdentifier;
        rtl::OUString                                                                                   m_aResourceName;
        com::sun::star::uno::Reference< com::sun::star::frame::XFrame >                                 m_xFrame;
        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >                        m_xUICommandLabels;
        StatusBarControllerVector                                                                       m_aControllerVector;
        ::cppu::OMultiTypeInterfaceContainerHelper                                                      m_aListenerContainer;   /// container for ALL Listener
        ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >                  m_xServiceManager;
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::frame::XUIControllerRegistration >  m_xStatusbarControllerRegistration;
};

}

#endif // __FRAMEWORK_UIELEMENT_STATUSBARMANAGER_HXX_
