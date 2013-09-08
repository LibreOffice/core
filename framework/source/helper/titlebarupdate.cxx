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

#include <helper/titlebarupdate.hxx>

#include <pattern/window.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <macros/generic.hxx>
#include <services.h>
#include <properties.h>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <vcl/window.hxx>
#include <vcl/syswin.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/diagnose_ex.h>

namespace framework{

static const ::sal_Int32 INVALID_ICON_ID = -1;
static const ::sal_Int32 DEFAULT_ICON_ID =  0;


//*****************************************************************************************************************
TitleBarUpdate::TitleBarUpdate(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : ThreadHelpBase          (&Application::GetSolarMutex())
    , m_xContext              (xContext                     )
    , m_xFrame                (                             )
{
}

//*****************************************************************************************************************
TitleBarUpdate::~TitleBarUpdate()
{
}

//*****************************************************************************************************************
void SAL_CALL TitleBarUpdate::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    // check arguments
    css::uno::Reference< css::frame::XFrame > xFrame;
    if (lArguments.getLength() < 1)
        throw css::lang::IllegalArgumentException(
                "Empty argument list!",
                static_cast< ::cppu::OWeakObject* >(this),
                1);

    lArguments[0] >>= xFrame;
    if (!xFrame.is())
        throw css::lang::IllegalArgumentException(
                "No valid frame specified!",
                static_cast< ::cppu::OWeakObject* >(this),
                1);

    // SYNCHRONIZED ->
    WriteGuard aWriteLock(m_aLock);
    // hold the frame as weak reference(!) so it can die everytimes :-)
    m_xFrame = xFrame;
    aWriteLock.unlock();
    // <- SYNCHRONIZED

    // start listening
    xFrame->addFrameActionListener(this);

    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xBroadcaster(xFrame, css::uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->addTitleChangeListener (this);
}

//*****************************************************************************************************************
void SAL_CALL TitleBarUpdate::frameAction(const css::frame::FrameActionEvent& aEvent)
    throw(css::uno::RuntimeException)
{
    // we are interested on events only, which must trigger a title bar update
    // because component was changed.
    if (
        (aEvent.Action == css::frame::FrameAction_COMPONENT_ATTACHED  ) ||
        (aEvent.Action == css::frame::FrameAction_COMPONENT_REATTACHED) ||
        (aEvent.Action == css::frame::FrameAction_COMPONENT_DETACHING )
       )
    {
        impl_forceUpdate ();
    }
}

//*****************************************************************************************************************
void SAL_CALL TitleBarUpdate::titleChanged(const css::frame::TitleChangedEvent& /* aEvent */)
    throw (css::uno::RuntimeException)
{
    impl_forceUpdate ();
}

//*****************************************************************************************************************
void SAL_CALL TitleBarUpdate::disposing(const css::lang::EventObject&)
    throw(css::uno::RuntimeException)
{
    // nothing todo here - because we hold the frame as weak reference only
}

//http://live.gnome.org/GnomeShell/ApplicationBased
//See http://msdn.microsoft.com/en-us/library/dd378459(v=VS.85).aspx for future
//Windows 7 equivalent support
void TitleBarUpdate::impl_updateApplicationID(const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    css::uno::Reference< css::awt::XWindow > xWindow = xFrame->getContainerWindow ();
    if ( ! xWindow.is() )
        return;

    OUString sApplicationID;
    try
    {
        // SYNCHRONIZED ->
        ReadGuard aReadLock(m_aLock);
        css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
        aReadLock.unlock();
        // <- SYNCHRONIZED

        css::uno::Reference< css::frame::XModuleManager2 > xModuleManager =
            css::frame::ModuleManager::create( xContext );

        OUString aModuleId = xModuleManager->identify(xFrame);
        OUString sDesktopName;
#if !defined(MACOSX)
        if ( aModuleId == "com.sun.star.text.TextDocument" ||
             aModuleId == "com.sun.star.text.GlobalDocument" ||
             aModuleId == "com.sun.star.text.WebDocument" ||
             aModuleId == "com.sun.star.xforms.XMLFormDocument" )
            sDesktopName = OUString("Writer");
        else if ( aModuleId == "com.sun.star.sheet.SpreadsheetDocument" )
            sDesktopName = OUString("Calc");
        else if ( aModuleId == "com.sun.star.presentation.PresentationDocument" )
            sDesktopName = OUString("Impress");
        else if ( aModuleId == "com.sun.star.drawing.DrawingDocument" )
            sDesktopName = OUString("Draw");
        else if ( aModuleId == "com.sun.star.formula.FormulaProperties" )
            sDesktopName = OUString("Math");
        else if ( aModuleId == "com.sun.star.sdb.DatabaseDocument" ||
                  aModuleId == "com.sun.star.sdb.OfficeDatabaseDocument" ||
                  aModuleId == "com.sun.star.sdb.RelationDesign" ||
                  aModuleId == "com.sun.star.sdb.QueryDesign" ||
                  aModuleId == "com.sun.star.sdb.TableDesign" ||
                  aModuleId == "com.sun.star.sdb.DataSourceBrowser" )
            sDesktopName = OUString("Base");
        else
            sDesktopName = OUString("Startcenter");
#if defined(WNT)
        // We use a hardcoded product name matching the registry keys so applications can be associated with file types
        sApplicationID = "TheDocumentFoundation.LibreOffice.";
        sApplicationID += sDesktopName;
#else
        sApplicationID = utl::ConfigManager::getProductName().toAsciiLowerCase();
        sApplicationID += OUString(sal_Unicode('-'));
        sApplicationID += sDesktopName.toAsciiLowerCase();
#endif
#endif
    }
    catch(const css::uno::Exception&)
    {
    }

    // VCL SYNCHRONIZED ->
    SolarMutexGuard aSolarGuard;

    Window* pWindow = (VCLUnoHelper::GetWindow( xWindow ));
    if (
        ( pWindow                                 ) &&
        ( pWindow->GetType() == WINDOW_WORKWINDOW )
       )
    {
        WorkWindow* pWorkWindow = (WorkWindow*)pWindow;
        pWorkWindow->SetApplicationID( sApplicationID );
    }
    // <- VCL SYNCHRONIZED
}


//*****************************************************************************************************************
::sal_Bool TitleBarUpdate::implst_getModuleInfo(const css::uno::Reference< css::frame::XFrame >& xFrame,
                                                      TModuleInfo&                               rInfo )
{
    if ( ! xFrame.is ())
        return sal_False;

    // SYNCHRONIZED ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    // <- SYNCHRONIZED

    try
    {
        css::uno::Reference< css::frame::XModuleManager2 > xModuleManager =
            css::frame::ModuleManager::create( xContext );

        rInfo.sID = xModuleManager->identify(xFrame);
        ::comphelper::SequenceAsHashMap lProps    = xModuleManager->getByName (rInfo.sID);

        rInfo.sUIName = lProps.getUnpackedValueOrDefault (OFFICEFACTORY_PROPNAME_UINAME, OUString());
        rInfo.nIcon   = lProps.getUnpackedValueOrDefault (OFFICEFACTORY_PROPNAME_ICON  , INVALID_ICON_ID  );

        // Note: If we could retrieve a module id ... everything is OK.
        // UIName and Icon ID are optional values !
        ::sal_Bool bSuccess = !rInfo.sID.isEmpty();
        return bSuccess;
    }
    catch(const css::uno::Exception&)
        {}

    return sal_False;
}

//*****************************************************************************************************************
void TitleBarUpdate::impl_forceUpdate()
{
    // SYNCHRONIZED ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XFrame >              xFrame(m_xFrame.get(), css::uno::UNO_QUERY);
    aReadLock.unlock();
    // <- SYNCHRONIZED

    // frame already gone ? We hold it weak only ...
    if ( ! xFrame.is())
        return;

    // no window -> no chance to set/update title and icon
    css::uno::Reference< css::awt::XWindow > xWindow = xFrame->getContainerWindow();
    if ( ! xWindow.is())
        return;

    impl_updateIcon  (xFrame);
    impl_updateTitle (xFrame);
#if !defined(MACOSX)
    impl_updateApplicationID (xFrame);
#endif
}

//*****************************************************************************************************************
void TitleBarUpdate::impl_updateIcon(const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    css::uno::Reference< css::frame::XController > xController = xFrame->getController      ();
    css::uno::Reference< css::awt::XWindow >       xWindow     = xFrame->getContainerWindow ();

    if (
        ( ! xController.is() ) ||
        ( ! xWindow.is()     )
       )
        return;

    // a) set default value to an invalid one. So we can start further searches for right icon id, if
    //    first steps failed!
    sal_Int32 nIcon = INVALID_ICON_ID;

    // b) try to find information on controller property set directly
    //    Don't forget to catch possible exceptions - because these property is an optional one!
    css::uno::Reference< css::beans::XPropertySet > xSet( xController, css::uno::UNO_QUERY );
    if ( xSet.is() )
    {
        try
        {
            css::uno::Reference< css::beans::XPropertySetInfo > const xPSI( xSet->getPropertySetInfo(), css::uno::UNO_SET_THROW );
            if ( xPSI->hasPropertyByName( "IconId" ) )
                xSet->getPropertyValue( "IconId" ) >>= nIcon;
        }
        catch(const css::uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // c) if b) failed ... identify the used module and retrieve set icon from module config.
    //    Tirck :-) Module was already specified outside and aInfo contains all needed information.
    if ( nIcon == INVALID_ICON_ID )
    {
        TModuleInfo aInfo;
        if (implst_getModuleInfo(xFrame, aInfo))
            nIcon = aInfo.nIcon;
    }

    // d) if all steps failed - use fallback :-)
    //    ... means using the global staroffice icon
    if( nIcon == INVALID_ICON_ID )
        nIcon = DEFAULT_ICON_ID;

    // e) set icon on container window now
    //    Don't forget SolarMutex! We use vcl directly :-(
    //    Check window pointer for right WorkWindow class too!!!

    // VCL SYNCHRONIZED ->
    SolarMutexGuard aSolarGuard;

    Window* pWindow = (VCLUnoHelper::GetWindow( xWindow ));
    if (
        ( pWindow                                 ) &&
        ( pWindow->GetType() == WINDOW_WORKWINDOW )
       )
    {
        WorkWindow* pWorkWindow = (WorkWindow*)pWindow;
        pWorkWindow->SetIcon( (sal_uInt16)nIcon );

        css::uno::Reference< css::frame::XModel > xModel = xController->getModel();
        OUString aURL;
        if( xModel.is() )
            aURL = xModel->getURL();
        pWorkWindow->SetRepresentedURL( aURL );
    }
    // <- VCL SYNCHRONIZED
}

//*****************************************************************************************************************
void TitleBarUpdate::impl_updateTitle(const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    // no window ... no chance to set any title -> return
    css::uno::Reference< css::awt::XWindow > xWindow = xFrame->getContainerWindow ();
    if ( ! xWindow.is() )
        return;

    css::uno::Reference< css::frame::XTitle > xTitle(xFrame, css::uno::UNO_QUERY);
    if ( ! xTitle.is() )
        return;

    const OUString sTitle = xTitle->getTitle ();

    // VCL SYNCHRONIZED ->
    SolarMutexGuard aSolarGuard;

    Window* pWindow = (VCLUnoHelper::GetWindow( xWindow ));
    if (
        ( pWindow                                 ) &&
        ( pWindow->GetType() == WINDOW_WORKWINDOW )
       )
    {
        WorkWindow* pWorkWindow = (WorkWindow*)pWindow;
        pWorkWindow->SetText( sTitle );
    }
    // <- VCL SYNCHRONIZED
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
