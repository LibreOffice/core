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
#include "uiservices.hxx"
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/menu.hxx>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
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
#include <comphelper/processfactory.hxx>

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
        void lcl_copy(Menu* _pMenu,sal_uInt16 _nMenuId,sal_uInt16 _nMenuPos,ToolBox* _pToolBox,sal_uInt16 _nToolId,const OUString& _sCommand)
        {
            if ( _pMenu->GetItemType(_nMenuPos) != MenuItemType::STRING )
                _pToolBox->SetItemImage(_nToolId, _pMenu->GetItemImage(_nMenuId));
            _pToolBox->SetItemCommand( _nToolId, _sCommand);
            _pToolBox->SetHelpId(_nToolId, _pMenu->GetHelpId(_nMenuId));
            _pToolBox->SetHelpText(_nToolId, _pMenu->GetHelpText(_nMenuId));
            _pToolBox->SetQuickHelpText(_nToolId, _pMenu->GetTipHelpText(_nMenuId));
            _pToolBox->SetItemText(_nToolId, _pMenu->GetItemText(_nMenuId));
        }
    }

    OToolboxController::OToolboxController(const Reference< XComponentContext >& _rxORB)
        : m_nToolBoxId(1)
    {
        osl_atomic_increment(&m_refCount);
        m_xContext = _rxORB;
        osl_atomic_decrement(&m_refCount);

    }
    IMPLEMENT_SERVICE_INFO_IMPLNAME_STATIC(OToolboxController, "com.sun.star.sdb.ApplicationToolboxController")
    IMPLEMENT_SERVICE_INFO_SUPPORTS(OToolboxController)
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED1_STATIC(OToolboxController, "com.sun.star.frame.ToolboxController")

    Reference< XInterface >
        SAL_CALL OToolboxController::Create(const Reference< XMultiServiceFactory >& _rxORB)
    {
        return static_cast< XServiceInfo* >(new OToolboxController( comphelper::getComponentContext(_rxORB) ));
    }

    // XInterface
    Any SAL_CALL OToolboxController::queryInterface( const Type& _rType ) throw (RuntimeException, std::exception)
    {
        Any aReturn = ToolboxController::queryInterface(_rType);
        if (!aReturn.hasValue())
            aReturn = TToolboxController_BASE::queryInterface(_rType);
        return aReturn;
    }
    void SAL_CALL OToolboxController::acquire() throw ()
    {
        ToolboxController::acquire();
    }
    void SAL_CALL OToolboxController::release() throw ()
    {
        ToolboxController::release();
    }
    void SAL_CALL OToolboxController::initialize( const Sequence< Any >& _rArguments ) throw (Exception, RuntimeException, std::exception)
    {
        ToolboxController::initialize(_rArguments);
        SolarMutexGuard aSolarMutexGuard;
        ::osl::MutexGuard aGuard(m_aMutex);

        if ( m_aCommandURL == ".uno:DBNewForm" )
        {
            m_aStates.insert(TCommandState::value_type(OUString(".uno:DBNewForm")           ,sal_True));
            m_aStates.insert(TCommandState::value_type(OUString(".uno:DBNewView")           ,sal_True));
            m_aStates.insert(TCommandState::value_type(OUString(".uno:DBNewViewSQL")        ,sal_True));
            m_aStates.insert(TCommandState::value_type(OUString(".uno:DBNewQuery")          ,sal_True));
            m_aStates.insert(TCommandState::value_type(OUString(".uno:DBNewQuerySql")       ,sal_True));
            m_aStates.insert(TCommandState::value_type(OUString(".uno:DBNewReport")         ,sal_True));
            m_aStates.insert(TCommandState::value_type(OUString(".uno:DBNewReportAutoPilot"),sal_True));
            m_aStates.insert(TCommandState::value_type(OUString(".uno:DBNewTable")          ,sal_True));
        }
        else
        {
            m_aStates.insert(TCommandState::value_type(OUString(".uno:Refresh")         ,sal_True));
            m_aStates.insert(TCommandState::value_type(OUString(".uno:DBRebuildData")   ,sal_True));
        }

        TCommandState::iterator aIter = m_aStates.begin();
        TCommandState::iterator aEnd = m_aStates.end();
        for (; aIter != aEnd; ++aIter)
            addStatusListener(aIter->first);

        VclPtr< ToolBox > pToolBox = static_cast<ToolBox*>(VCLUnoHelper::GetWindow(getParent()).get());
        if ( pToolBox )
        {
            sal_uInt16 nCount = pToolBox->GetItemCount();
            for (sal_uInt16 nPos = 0; nPos < nCount; ++nPos)
            {
                sal_uInt16 nItemId = pToolBox->GetItemId(nPos);
                if ( pToolBox->GetItemCommand(nItemId) == m_aCommandURL )
                {
                    m_nToolBoxId = nItemId;
                    break;
                }
            }

            // check if paste special is allowed, when not don't add DROPDOWN
            pToolBox->SetItemBits(m_nToolBoxId,pToolBox->GetItemBits(m_nToolBoxId) | ToolBoxItemBits::DROPDOWN);
        }
    }
    void SAL_CALL OToolboxController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException, std::exception )
    {
        SolarMutexGuard aSolarMutexGuard;
        ::osl::MutexGuard aGuard(m_aMutex);
        TCommandState::iterator aFind = m_aStates.find( Event.FeatureURL.Complete );
        if ( aFind != m_aStates.end() )
        {
            aFind->second = Event.IsEnabled;
            if ( m_aCommandURL == aFind->first && !Event.IsEnabled )
            {
                ::std::unique_ptr<PopupMenu> pMenu = getMenu();
                sal_uInt16 nCount = pMenu->GetItemCount();
                for (sal_uInt16 i = 0; i < nCount; ++i)
                {
                    sal_uInt16 nItemId = pMenu->GetItemId(i);
                    aFind = m_aStates.find(pMenu->GetItemCommand(nItemId));
                    if ( aFind != m_aStates.end() && aFind->second )
                    {
                        m_aCommandURL = aFind->first;

                        VclPtr< ToolBox > pToolBox = static_cast<ToolBox*>(VCLUnoHelper::GetWindow(getParent()).get());
                        lcl_copy(pMenu.get(),nItemId,i,pToolBox,m_nToolBoxId, m_aCommandURL);
                        break;
                    }
                }
            }
        }
    }
    ::std::unique_ptr<PopupMenu> OToolboxController::getMenu()
    {
        ::std::unique_ptr<PopupMenu> pMenu;
        if ( m_aStates.size() > 2 )
        {
            pMenu.reset( new PopupMenu( ModuleRes( RID_MENU_APP_NEW ) ) );

            try
            {
                Reference<XModuleUIConfigurationManagerSupplier> xModuleCfgMgrSupplier = theModuleUIConfigurationManagerSupplier::get( getContext() );
                Reference<XUIConfigurationManager> xUIConfigMgr = xModuleCfgMgrSupplier->getUIConfigurationManager( "com.sun.star.sdb.OfficeDatabaseDocument" );
                Reference<XImageManager> xImageMgr(xUIConfigMgr->getImageManager(),UNO_QUERY);

                Sequence< OUString> aSeq(1);
                sal_uInt16 nCount = pMenu->GetItemCount();
                for (sal_uInt16 nPos = 0; nPos < nCount; ++nPos)
                {
                    if ( pMenu->GetItemType( nPos ) == MenuItemType::SEPARATOR )
                        continue;

                    sal_uInt16 nItemId = pMenu->GetItemId(nPos);
                    aSeq[0] = pMenu->GetItemCommand(nItemId);
                    Sequence< Reference<XGraphic> > aImages = xImageMgr->getImages(ImageType::SIZE_DEFAULT, aSeq);

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

    Reference< css::awt::XWindow > SAL_CALL OToolboxController::createPopupWindow() throw (RuntimeException, std::exception)
    {
        // execute the menu
        SolarMutexGuard aSolarMutexGuard;
        ::osl::MutexGuard aGuard(m_aMutex);

        VclPtr< ToolBox > pToolBox = static_cast<ToolBox*>(VCLUnoHelper::GetWindow(getParent()).get());
        ::std::unique_ptr<PopupMenu> pMenu = getMenu();

        sal_uInt16 nSelected = pMenu->Execute(pToolBox, pToolBox->GetItemRect( m_nToolBoxId ),PopupMenuFlags::ExecuteDown);
        // "cleanup" the toolbox state
        Point aPoint = pToolBox->GetItemRect( m_nToolBoxId ).TopLeft();
        MouseEvent aLeave( aPoint, 0, MouseEventModifiers::LEAVEWINDOW | MouseEventModifiers::SYNTHETIC );
        pToolBox->MouseMove( aLeave );
        pToolBox->SetItemDown( m_nToolBoxId, false);

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
        return Reference< css::awt::XWindow >();
    }
} // dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
