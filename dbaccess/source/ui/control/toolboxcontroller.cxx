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

#include "toolboxcontroller.hxx"
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/menu.hxx>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/ModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include "dbu_resource.hrc"
#include <svtools/miscopt.hxx>
#include <unotools/moduleoptions.hxx>
#include <tools/diagnose_ex.h>
#include <svtools/menuoptions.hxx>
#include <osl/mutex.hxx>
#include "dbu_reghelper.hxx"
#include "UITools.hxx"
#include <comphelper/componentcontext.hxx>


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
        void lcl_copy(Menu* _pMenu,sal_uInt16 _nMenuId,sal_uInt16 _nMenuPos,ToolBox* _pToolBox,sal_uInt16 _nToolId,const ::rtl::OUString& _sCommand)
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
        SolarMutexGuard aSolarMutexGuard;
        ::osl::MutexGuard aGuard(m_aMutex);

        if ( m_aCommandURL == ".uno:DBNewForm" )
        {
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewForm"))           ,sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewView"))           ,sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewViewSQL"))        ,sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewQuery"))          ,sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewQuerySql"))       ,sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewReport"))         ,sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewReportAutoPilot")),sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBNewTable"))          ,sal_True));
        }
        else
        {
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Refresh"))         ,sal_True));
            m_aStates.insert(TCommandState::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DBRebuildData"))   ,sal_True));
        }

        TCommandState::iterator aIter = m_aStates.begin();
        TCommandState::iterator aEnd = m_aStates.end();
        for (; aIter != aEnd; ++aIter)
            addStatusListener(aIter->first);

        ToolBox*    pToolBox = static_cast<ToolBox*>(VCLUnoHelper::GetWindow(getParent()));
        if ( pToolBox )
        {
            sal_uInt16 nCount = pToolBox->GetItemCount();
            for (sal_uInt16 nPos = 0; nPos < nCount; ++nPos)
            {
                sal_uInt16 nItemId = pToolBox->GetItemId(nPos);
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
        SolarMutexGuard aSolarMutexGuard;
        ::osl::MutexGuard aGuard(m_aMutex);
        TCommandState::iterator aFind = m_aStates.find( Event.FeatureURL.Complete );
        if ( aFind != m_aStates.end() )
        {
            aFind->second = Event.IsEnabled;
            if ( m_aCommandURL == aFind->first && !Event.IsEnabled )
            {
                SAL_WNODEPRECATED_DECLARATIONS_PUSH
                ::std::auto_ptr<PopupMenu> pMenu = getMenu();
                SAL_WNODEPRECATED_DECLARATIONS_POP
                sal_uInt16 nCount = pMenu->GetItemCount();
                for (sal_uInt16 i = 0; i < nCount; ++i)
                {
                    sal_uInt16 nItemId = pMenu->GetItemId(i);
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
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<PopupMenu> OToolboxController::getMenu()
    {
        ::std::auto_ptr<PopupMenu> pMenu;
        if ( m_aStates.size() > 2 )
        {
            pMenu.reset( new PopupMenu( ModuleRes( RID_MENU_APP_NEW ) ) );

            try
            {
                Reference<XModuleUIConfigurationManagerSupplier> xModuleCfgMgrSupplier(ModuleUIConfigurationManagerSupplier::create(comphelper::ComponentContext(getServiceManager()).getUNOContext()));
                Reference<XUIConfigurationManager> xUIConfigMgr = xModuleCfgMgrSupplier->getUIConfigurationManager(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.OfficeDatabaseDocument")));
                Reference<XImageManager> xImageMgr(xUIConfigMgr->getImageManager(),UNO_QUERY);


                short nImageType = hasBigImages() ? ImageType::SIZE_LARGE : ImageType::SIZE_DEFAULT;

                Sequence< ::rtl::OUString> aSeq(1);
                sal_uInt16 nCount = pMenu->GetItemCount();
                for (sal_uInt16 nPos = 0; nPos < nCount; ++nPos)
                {
                    if ( pMenu->GetItemType( nPos ) == MENUITEM_SEPARATOR )
                        continue;

                    sal_uInt16 nItemId = pMenu->GetItemId(nPos);
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
            catch(const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        else
        {
            pMenu.reset( new PopupMenu( ModuleRes( RID_MENU_REFRESH_DATA ) ) );
        }
        return pMenu;
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP
    // -----------------------------------------------------------------------------
    Reference< ::com::sun::star::awt::XWindow > SAL_CALL OToolboxController::createPopupWindow() throw (RuntimeException)
    {
        // execute the menu
        SolarMutexGuard aSolarMutexGuard;
        ::osl::MutexGuard aGuard(m_aMutex);

        ToolBox* pToolBox = static_cast<ToolBox*>(VCLUnoHelper::GetWindow(getParent()));
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<PopupMenu> pMenu = getMenu();
        SAL_WNODEPRECATED_DECLARATIONS_POP

        sal_uInt16 nSelected = pMenu->Execute(pToolBox, pToolBox->GetItemRect( m_nToolBoxId ),POPUPMENU_EXECUTE_DOWN);
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
                OSL_ENSURE(!aUrl.Complete.isEmpty(),"Command is empty!");
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
