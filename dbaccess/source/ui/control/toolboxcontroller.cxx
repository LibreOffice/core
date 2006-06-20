/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: toolboxcontroller.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:01:08 $
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
#ifndef DBACCESS_TOOLBOXCONTROLLER_HXX
#include "toolboxcontroller.hxx"
#endif
#ifndef _COM_SUN_STAR_UI_IMAGETYPE_HPP_
#include <com/sun/star/ui/ImageType.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGER_HPP_
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XIMAGEMANAGER_HPP_
#include <com/sun/star/ui/XImageManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_IMAGETYPE_HPP_
#include <com/sun/star/ui/ImageType.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_MISCOPT_HXX
#include <svtools/miscopt.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MENUOPTIONS_HXX
#include <svtools/menuoptions.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _PASTEDLG_HXX
#include <so3/pastedlg.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif


extern "C" void SAL_CALL createRegistryInfo_OToolboxController()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OToolboxController> aAutoRegistration;
}
namespace dbaui
{
    using namespace svt;
    using namespace ::com::sun::star::graphic;
    using namespace com::sun::star::uno;
    using namespace com::sun::star::beans;
    using namespace com::sun::star::lang;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::ui;

    namespace
    {
        void lcl_copy(Menu* _pMenu,USHORT _nMenuId,USHORT _nMenuPos,ToolBox* _pToolBox,USHORT _nToolId,const ::rtl::OUString& _sCommand)
        {
            if ( _pMenu->GetItemType(_nMenuPos) != MENUITEM_STRING )
                _pToolBox->SetItemImage(_nToolId, _pMenu->GetItemImage(_nMenuId));
            _pToolBox->SetItemCommand( _nToolId, _sCommand);
            _pToolBox->SetHelpId(_nToolId, _pMenu->GetHelpId(_nMenuId));
            _pToolBox->SetHelpText(_nToolId, _pMenu->GetHelpText(_nMenuId));
            _pToolBox->SetQuickHelpText(_nToolId, _pMenu->GetTipHelpText(_nMenuId));
            _pToolBox->SetItemText(_nToolId, _pMenu->GetItemText(_nMenuId));
        }
    }

    OToolboxController::OToolboxController(const Reference< XMultiServiceFactory >& _rxORB)
        : m_nToolBoxId(1)
    {
        osl_incrementInterlockedCount(&m_refCount);
        m_xServiceManager = _rxORB;
        osl_decrementInterlockedCount(&m_refCount);

    }
    // -----------------------------------------------------------------------------
    IMPLEMENT_SERVICE_INFO1_STATIC(OToolboxController,"com.sun.star.sdb.ApplicationToolboxController","com.sun.star.frame.ToolboxController")
    // -----------------------------------------------------------------------------
    // XInterface
    Any SAL_CALL OToolboxController::queryInterface( const Type& _rType ) throw (RuntimeException)
    {
        Any aReturn = ToolboxController::queryInterface(_rType);
        if (!aReturn.hasValue())
            aReturn = TToolboxController_BASE::queryInterface(_rType);
        return aReturn;
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL OToolboxController::acquire() throw ()
    {
        ToolboxController::acquire();
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL OToolboxController::release() throw ()
    {
        ToolboxController::release();
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL OToolboxController::initialize( const Sequence< Any >& _rArguments ) throw (Exception, RuntimeException)
    {
        ToolboxController::initialize(_rArguments);
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
        ::osl::MutexGuard aGuard(m_aMutex);

        if ( m_aCommandURL.equalsAscii(".uno:DBNewForm") )
        {
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewForm"))           ,sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewView"))           ,sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewViewSQL"))        ,sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewQuery"))          ,sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewQuerySql"))       ,sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewReportAutoPilot")),sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewTable"))          ,sal_True));
        }
        else
        {
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Refresh"))         ,sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBRebuildData"))   ,sal_True));
        }

        TCommandState::iterator aIter = m_aStates.begin();
        for (; aIter != m_aStates.end(); ++aIter)
            addStatusListener(aIter->first);

        ToolBox*    pToolBox = static_cast<ToolBox*>(VCLUnoHelper::GetWindow(getParent()));
        if ( pToolBox )
        {
            USHORT nCount = pToolBox->GetItemCount();
            for (USHORT nPos = 0; nPos < nCount; ++nPos)
            {
                USHORT nItemId = pToolBox->GetItemId(nPos);
                if ( pToolBox->GetItemCommand(nItemId) == String(m_aCommandURL) )
                {
                    m_nToolBoxId = nItemId;
                    break;
                }
            }

            // check if paste special is allowed, when not don't add DROPDOWN
            pToolBox->SetItemBits(m_nToolBoxId,pToolBox->GetItemBits(m_nToolBoxId) | TIB_DROPDOWN);
        }
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL OToolboxController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
    {
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
        ::osl::MutexGuard aGuard(m_aMutex);
        TCommandState::iterator aFind = m_aStates.find( Event.FeatureURL.Complete );
        if ( aFind != m_aStates.end() )
        {
            aFind->second = Event.IsEnabled;
            if ( m_aCommandURL == aFind->first && !Event.IsEnabled )
            {
                ::std::auto_ptr<PopupMenu> pMenu = getMenu();
                USHORT nCount = pMenu->GetItemCount();
                for (USHORT i = 0; i < nCount; ++i)
                {
                    USHORT nItemId = pMenu->GetItemId(i);
                    aFind = m_aStates.find(pMenu->GetItemCommand(nItemId));
                    if ( aFind != m_aStates.end() && aFind->second )
                    {
                        m_aCommandURL = aFind->first;

                        ToolBox* pToolBox = static_cast<ToolBox*>(VCLUnoHelper::GetWindow(getParent()));
                        lcl_copy(pMenu.get(),nItemId,i,pToolBox,m_nToolBoxId, m_aCommandURL);
                        break;
                    }
                }
            }
        }
    }
    // -----------------------------------------------------------------------------
    ::std::auto_ptr<PopupMenu> OToolboxController::getMenu()
    {
        ::std::auto_ptr<PopupMenu> pMenu;
        if ( m_aStates.size() > 2 )
        {
            pMenu.reset( new PopupMenu( ModuleRes( RID_MENU_APP_NEW ) ) );

            sal_Bool bHighContrast = isHighContrast();

            try
            {
                Reference<XModuleUIConfigurationManagerSupplier> xModuleCfgMgrSupplier(getServiceManager()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.ModuleUIConfigurationManagerSupplier"))),UNO_QUERY);
                Reference<XUIConfigurationManager> xUIConfigMgr = xModuleCfgMgrSupplier->getUIConfigurationManager(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.OfficeDatabaseDocument")));
                Reference<XImageManager> xImageMgr(xUIConfigMgr->getImageManager(),UNO_QUERY);


                short nImageType = hasBigImages() ? ImageType::SIZE_LARGE : ImageType::SIZE_DEFAULT;
                if ( bHighContrast )
                    nImageType |= ImageType::COLOR_HIGHCONTRAST;

                Sequence< ::rtl::OUString> aSeq(1);
                USHORT nCount = pMenu->GetItemCount();
                for (USHORT nPos = 0; nPos < nCount; ++nPos)
                {
                    if ( pMenu->GetItemType( nPos ) == MENUITEM_SEPARATOR )
                        continue;

                    USHORT nItemId = pMenu->GetItemId(nPos);
                    aSeq[0] = pMenu->GetItemCommand(nItemId);
                    Sequence< Reference<XGraphic> > aImages = xImageMgr->getImages(nImageType,aSeq);

                    Image aImage(aImages[0]);
                    pMenu->SetItemImage(nItemId,aImage);
                    TCommandState::iterator aFind = m_aStates.find( aSeq[0] );
                    if ( aFind != m_aStates.end() )
                    {
                        pMenu->EnableItem(nItemId,aFind->second);
                    }
                }
            }
            catch(Exception&)
            {
                OSL_ENSURE(0,"Exception catched while filling popup menu!");
            }
        }
        else
        {
            pMenu.reset( new PopupMenu( ModuleRes( RID_MENU_REFRESH_DATA ) ) );
        }
        return pMenu;
    }
    // -----------------------------------------------------------------------------
    Reference< ::com::sun::star::awt::XWindow > SAL_CALL OToolboxController::createPopupWindow() throw (RuntimeException)
    {
        // execute the menu
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
        ::osl::MutexGuard aGuard(m_aMutex);

        ToolBox* pToolBox = static_cast<ToolBox*>(VCLUnoHelper::GetWindow(getParent()));
        ::std::auto_ptr<PopupMenu> pMenu = getMenu();

        USHORT nSelected = pMenu->Execute(pToolBox, pToolBox->GetItemRect( m_nToolBoxId ),POPUPMENU_EXECUTE_DOWN);
        // "cleanup" the toolbox state
        Point aPoint = pToolBox->GetItemRect( m_nToolBoxId ).TopLeft();
        MouseEvent aLeave( aPoint, 0, MOUSE_LEAVEWINDOW | MOUSE_SYNTHETIC );
        pToolBox->MouseMove( aLeave );
        pToolBox->SetItemDown( m_nToolBoxId, sal_False);

        if ( nSelected )
        {
            m_aCommandURL = pMenu->GetItemCommand(nSelected);
            lcl_copy(pMenu.get(),nSelected,pMenu->GetItemPos(nSelected),pToolBox,m_nToolBoxId, m_aCommandURL);

            Reference<XDispatch> xDispatch = m_aListenerMap.find(m_aCommandURL)->second;
            if ( xDispatch.is() )
            {
                URL aUrl;
                Sequence < PropertyValue > aArgs;
                aUrl.Complete = m_aCommandURL;
                OSL_ENSURE(aUrl.Complete.getLength(),"Command is empty!");
                if ( getURLTransformer().is() )
                    getURLTransformer()->parseStrict(aUrl);
                xDispatch->dispatch(aUrl,aArgs);

            }
        }
        return Reference< ::com::sun::star::awt::XWindow >();
    }
    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
//..........................................................................
} // dbaui
//..........................................................................


