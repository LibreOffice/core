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
#include <com/sun/star/beans/PropertyValue.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/menu.hxx>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/util/Color.hpp>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/menuoptions.hxx>
#include <osl/mutex.hxx>
#include <svx/svxids.hrc>
#define ITEMID_FONT         3
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/tbcontrl.hxx>
#include <editeng/colritem.hxx>

#include <cppuhelper/supportsservice.hxx>

namespace rptui
{
    using namespace svt;
    using namespace com::sun::star;
    using namespace com::sun::star::uno;
    using namespace com::sun::star::beans;
    using namespace com::sun::star::lang;
    using namespace frame;
    using namespace util;
    using namespace ui;

OUString SAL_CALL OToolboxController::getImplementationName() throw( RuntimeException, std::exception )
{
    return getImplementationName_Static();
}


OUString OToolboxController::getImplementationName_Static() throw( RuntimeException )
{
    return OUString("com.sun.star.report.comp.ReportToolboxController");
}

Sequence< OUString> OToolboxController::getSupportedServiceNames_Static() throw( RuntimeException )
{
    Sequence<OUString> aSupported { "com.sun.star.report.ReportToolboxController" };
    return aSupported;
}

sal_Bool SAL_CALL OToolboxController::supportsService( const OUString& ServiceName ) throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString> SAL_CALL OToolboxController::getSupportedServiceNames() throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

Reference< XInterface > OToolboxController::create(Reference< XComponentContext > const & xContext)
{
    return * new OToolboxController(xContext);
}

OToolboxController::OToolboxController(const Reference< XComponentContext >& _rxORB)
    : m_pToolbarController(nullptr)
    ,m_nToolBoxId(1)
    ,m_nSlotId(0)
{
    osl_atomic_increment(&m_refCount);
    m_xContext = _rxORB;
    osl_atomic_decrement(&m_refCount);

}

OToolboxController::~OToolboxController()
{
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

    VclPtr< ToolBox > pToolBox = static_cast<ToolBox*>(VCLUnoHelper::GetWindow(getParent()).get());
    if ( pToolBox )
    {
        const sal_uInt16 nCount = pToolBox->GetItemCount();
        for (sal_uInt16 nPos = 0; nPos < nCount; ++nPos)
        {
            const sal_uInt16 nItemId = pToolBox->GetItemId(nPos);
            if ( pToolBox->GetItemCommand(nItemId) == m_aCommandURL )
            {
                m_nToolBoxId = nItemId;
                break;
            }
        }
        if ( m_aCommandURL == ".uno:CharFontName" )
        {
            m_aStates.insert(TCommandState::value_type(OUString(".uno:CharFontName"),sal_True));
            m_pToolbarController = new SvxFontNameToolBoxControl/*SvxStyleToolBoxControl*/(m_nSlotId = SID_ATTR_CHAR_FONT,m_nToolBoxId,*pToolBox);
        }
        else if ( m_aCommandURL == ".uno:FontColor" || m_aCommandURL == ".uno:Color" )
        {
            m_aStates.insert(TCommandState::value_type(OUString(".uno:FontColor"),sal_True));
            m_aStates.insert(TCommandState::value_type(OUString(".uno:Color"),sal_True));
            m_pToolbarController = new SvxColorToolBoxControl(m_nSlotId = SID_ATTR_CHAR_COLOR2,m_nToolBoxId,*pToolBox);
        }
        else
        {
            m_aStates.insert(TCommandState::value_type(OUString(".uno:BackgroundColor"),sal_True));
            m_pToolbarController = new SvxColorToolBoxControl(m_nSlotId = SID_BACKGROUND_COLOR,m_nToolBoxId,*pToolBox);
        }

        TCommandState::iterator aIter = m_aStates.begin();
        for (; aIter != m_aStates.end(); ++aIter)
            addStatusListener(aIter->first);

        if ( m_pToolbarController.is() )
            m_pToolbarController->initialize(_rArguments);
        // check if paste special is allowed, when not don't add DROPDOWN
        pToolBox->SetItemBits(m_nToolBoxId,pToolBox->GetItemBits(m_nToolBoxId) | ToolBoxItemBits::DROPDOWN);
    }
}

void SAL_CALL OToolboxController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    TCommandState::iterator aFind = m_aStates.find( Event.FeatureURL.Complete );
    if ( aFind != m_aStates.end() )
    {
        aFind->second = Event.IsEnabled;
        if ( m_pToolbarController.is() )
        {
            // All other status events will be processed here
            ToolBox& rTb = m_pToolbarController->GetToolBox();
            for ( sal_uInt16 i = 0; i < rTb.GetItemCount(); i++ )
            {
                sal_uInt16 nId = rTb.GetItemId( i );
                if ( nId == 0 )
                    continue;

                OUString aCmd = rTb.GetItemCommand( nId );
                if ( aCmd == Event.FeatureURL.Complete )
                {
                    // Enable/disable item
                    rTb.EnableItem( nId, Event.IsEnabled );
                }
            }

            switch(m_nSlotId)
            {
                case SID_ATTR_CHAR_COLOR2:
                case SID_BACKGROUND_COLOR:
                    {
                        util::Color nColor(COL_TRANSPARENT);
                        Event.State >>= nColor;
                        SvxColorItem aColorItem(::Color(nColor), 1);
                        static_cast<SvxColorToolBoxControl*>(m_pToolbarController.get())->StateChanged(m_nSlotId,Event.IsEnabled ? SfxItemState::SET : SfxItemState::DISABLED,&aColorItem);
                    }
                    break;
                case SID_ATTR_CHAR_FONT:
                    {
                        SvxFontItem aItem(ITEMID_FONT);
                        aItem.PutValue(Event.State, 0);
                        static_cast<SvxFontNameToolBoxControl*>(m_pToolbarController.get())->StateChanged(m_nSlotId,Event.IsEnabled ? SfxItemState::DEFAULT : SfxItemState::DISABLED,&aItem);
                    }
                    break;
            }
        }
    }
}

Reference< awt::XWindow > SAL_CALL OToolboxController::createPopupWindow() throw (RuntimeException, std::exception)
{
    // execute the menu
    SolarMutexGuard aSolarMutexGuard;
    ::osl::MutexGuard aGuard(m_aMutex);

    Reference< awt::XWindow > xRet;
    if ( m_pToolbarController.is() )
        xRet = m_pToolbarController->createPopupWindow();

    return xRet;
}

void SAL_CALL OToolboxController::execute( sal_Int16 KeyModifier ) throw (uno::RuntimeException, std::exception)
{
    if ( m_pToolbarController.is() )
        m_pToolbarController->execute( KeyModifier );
}

sal_Bool SAL_CALL OToolboxController::opensSubToolbar() throw (uno::RuntimeException, std::exception)
{
    return ( m_nSlotId != SID_ATTR_CHAR_FONT );
}

OUString SAL_CALL OToolboxController::getSubToolbarName() throw (uno::RuntimeException, std::exception)
{
    return OUString();
}

void SAL_CALL OToolboxController::functionSelected( const OUString& /*rCommand*/ ) throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OToolboxController::updateImage(  ) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarMutexGuard;
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_pToolbarController.is() )
        m_pToolbarController->updateImage();
}

uno::Reference< awt::XWindow > SAL_CALL OToolboxController::createItemWindow( const uno::Reference< awt::XWindow >& _xParent)
throw (uno::RuntimeException, std::exception)
{
    uno::Reference< awt::XWindow > xWindow;
    if ( m_pToolbarController.is() )
    {
        switch(m_nSlotId)
        {
            case SID_ATTR_CHAR_FONT:
                xWindow = VCLUnoHelper::GetInterface(static_cast<SvxFontNameToolBoxControl*>(m_pToolbarController.get())->CreateItemWindow(VCLUnoHelper::GetWindow(_xParent)));
                break;
            default:
                ;
        }
    }
    return xWindow;
}


} // rptui



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
