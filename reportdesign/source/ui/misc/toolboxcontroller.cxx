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
#include <svx/tbcontrl.hxx>

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

OUString SAL_CALL OToolboxController::getImplementationName()
{
    return getImplementationName_Static();
}


OUString OToolboxController::getImplementationName_Static()
{
    return OUString("com.sun.star.report.comp.ReportToolboxController");
}

Sequence< OUString> OToolboxController::getSupportedServiceNames_Static()
{
    Sequence<OUString> aSupported { "com.sun.star.report.ReportToolboxController" };
    return aSupported;
}

sal_Bool SAL_CALL OToolboxController::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString> SAL_CALL OToolboxController::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

Reference< XInterface > OToolboxController::create(Reference< XComponentContext > const & xContext)
{
    return * new OToolboxController(xContext);
}

OToolboxController::OToolboxController(const Reference< XComponentContext >& _rxORB)
    : m_pToolbarController(nullptr)
{
    osl_atomic_increment(&m_refCount);
    m_xContext = _rxORB;
    osl_atomic_decrement(&m_refCount);

}

OToolboxController::~OToolboxController()
{
}

// XInterface
Any SAL_CALL OToolboxController::queryInterface( const Type& _rType )
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

void SAL_CALL OToolboxController::initialize( const Sequence< Any >& _rArguments )
{
    ToolboxController::initialize(_rArguments);
    SolarMutexGuard aSolarMutexGuard;
    ::osl::MutexGuard aGuard(m_aMutex);
    sal_uInt16      nToolBoxId = 1;

    VclPtr< ToolBox > pToolBox = static_cast<ToolBox*>(VCLUnoHelper::GetWindow(getParent()).get());
    if ( pToolBox )
    {
        const ToolBox::ImplToolItems::size_type nCount = pToolBox->GetItemCount();
        for (ToolBox::ImplToolItems::size_type nPos = 0; nPos < nCount; ++nPos)
        {
            const sal_uInt16 nItemId = pToolBox->GetItemId(nPos);
            if ( pToolBox->GetItemCommand(nItemId) == m_aCommandURL )
            {
                nToolBoxId = nItemId;
                break;
            }
        }
        if ( m_aCommandURL == ".uno:FontColor" || m_aCommandURL == ".uno:Color" )
        {
            m_aStates.emplace(OUString(".uno:FontColor"),true);
            m_aStates.emplace(OUString(".uno:Color"),true);
            m_pToolbarController = new SvxColorToolBoxControl(SID_ATTR_CHAR_COLOR2,nToolBoxId,*pToolBox);
        }
        else
        {
            m_aStates.emplace(OUString(".uno:BackgroundColor"),true);
            m_pToolbarController = new SvxColorToolBoxControl(SID_BACKGROUND_COLOR,nToolBoxId,*pToolBox);
        }

        TCommandState::const_iterator aIter = m_aStates.begin();
        for (; aIter != m_aStates.end(); ++aIter)
            addStatusListener(aIter->first);

        if ( m_pToolbarController.is() )
            m_pToolbarController->initialize(_rArguments);
        // check if paste special is allowed, when not don't add DROPDOWN
        pToolBox->SetItemBits(nToolBoxId,pToolBox->GetItemBits(nToolBoxId) | ToolBoxItemBits::DROPDOWN);
    }
}

void SAL_CALL OToolboxController::statusChanged( const FeatureStateEvent& Event )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    TCommandState::iterator aFind = m_aStates.find( Event.FeatureURL.Complete );
    if ( aFind != m_aStates.end() && m_pToolbarController.is() )
        m_pToolbarController->statusChanged( Event );
}

Reference< awt::XWindow > SAL_CALL OToolboxController::createPopupWindow()
{
    // execute the menu
    SolarMutexGuard aSolarMutexGuard;
    ::osl::MutexGuard aGuard(m_aMutex);

    Reference< awt::XWindow > xRet;
    if ( m_pToolbarController.is() )
        xRet = m_pToolbarController->createPopupWindow();

    return xRet;
}

void SAL_CALL OToolboxController::execute( sal_Int16 KeyModifier )
{
    if ( m_pToolbarController.is() )
        m_pToolbarController->execute( KeyModifier );
}

sal_Bool SAL_CALL OToolboxController::opensSubToolbar()
{
    return true;
}

OUString SAL_CALL OToolboxController::getSubToolbarName()
{
    return OUString();
}

void SAL_CALL OToolboxController::functionSelected( const OUString& /*rCommand*/ )
{
}

void SAL_CALL OToolboxController::updateImage(  )
{
    SolarMutexGuard aSolarMutexGuard;
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_pToolbarController.is() )
        m_pToolbarController->updateImage();
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
