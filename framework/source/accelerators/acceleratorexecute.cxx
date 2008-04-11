/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: acceleratorexecute.cxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include "acceleratorexecute.hxx"

//===============================================
// includes

#ifndef __COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif

#ifndef __COM_SUN_STAR_UI_XUICONFIGURATIONMANAGER_HPP_
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#endif

#ifndef __COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif

#ifndef __COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif

#ifndef __COM_SUN_STAR_AWT_KEYMODIFIER_HPP_
#include <com/sun/star/awt/KeyModifier.hpp>
#endif

#ifndef __COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef __COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//===============================================
// namespace

namespace svt
{

namespace css = ::com::sun::star;

//===============================================
// definitions

//-----------------------------------------------
AcceleratorExecute::AcceleratorExecute()
    : TMutexInit      (                                                     )
    , m_aAsyncCallback(LINK(this, AcceleratorExecute, impl_ts_asyncCallback))
{
}

//-----------------------------------------------
AcceleratorExecute::AcceleratorExecute(const AcceleratorExecute& rCopy)
    : TMutexInit      (                                                     )
    , m_aAsyncCallback(LINK(this, AcceleratorExecute, impl_ts_asyncCallback))
{
    // copy construction sint supported in real ...
    // but we need this ctor to init our async callback ...
}

//-----------------------------------------------
AcceleratorExecute::~AcceleratorExecute()
{
    // does nothing real
}

//-----------------------------------------------
AcceleratorExecute* AcceleratorExecute::createAcceleratorHelper()
{
    AcceleratorExecute* pNew = new AcceleratorExecute();
    return pNew;
}

//-----------------------------------------------
void AcceleratorExecute::init(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR,
                              const css::uno::Reference< css::frame::XFrame >&              xEnv )
{
    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // take over the uno service manager
    m_xSMGR = xSMGR;

    // specify our internal dispatch provider
    // frame or desktop?! => document or global config.
    sal_Bool bDesktopIsUsed = sal_False;
             m_xDispatcher  = css::uno::Reference< css::frame::XDispatchProvider >(xEnv, css::uno::UNO_QUERY);
    if (!m_xDispatcher.is())
    {
        aLock.clear();
        // <- SAFE ------------------------------

        css::uno::Reference< css::frame::XDispatchProvider > xDispatcher(
                            xSMGR->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop")),
                            css::uno::UNO_QUERY_THROW);

        // SAFE -> ------------------------------
        aLock.reset();

        m_xDispatcher  = xDispatcher;
        bDesktopIsUsed = sal_True;
    }

    aLock.clear();
    // <- SAFE ----------------------------------

    // open all needed configuration objects
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xGlobalCfg;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xModuleCfg;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xDocCfg   ;

    // global cfg
    xGlobalCfg = AcceleratorExecute::impl_st_openGlobalConfig(xSMGR);
    if (!bDesktopIsUsed)
    {
        // module cfg
        xModuleCfg = AcceleratorExecute::impl_st_openModuleConfig(xSMGR, xEnv);

        // doc cfg
        css::uno::Reference< css::frame::XController > xController;
        css::uno::Reference< css::frame::XModel >      xModel;
        xController = xEnv->getController();
        if (xController.is())
            xModel = xController->getModel();
        if (xModel.is())
            xDocCfg = AcceleratorExecute::impl_st_openDocConfig(xModel);
    }

    // SAFE -> ------------------------------
    aLock.reset();

    m_xGlobalCfg = xGlobalCfg;
    m_xModuleCfg = xModuleCfg;
    m_xDocCfg    = xDocCfg   ;

    aLock.clear();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void AcceleratorExecute::execute(const KeyCode& aVCLKey)
{
    css::awt::KeyEvent aAWTKey = AcceleratorExecute::st_VCLKey2AWTKey(aVCLKey);
    execute(aAWTKey);
}

//-----------------------------------------------
void AcceleratorExecute::execute(const css::awt::KeyEvent& aAWTKey)
{
    ::rtl::OUString sCommand = impl_ts_findCommand(aAWTKey);

    // No Command found? Do nothing! User isnt interested on any error handling .-)
    if (!sCommand.getLength())
        return;

    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    m_lCommandQueue.push_back(sCommand);
    m_aAsyncCallback.Post(0);

    aLock.clear();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
css::awt::KeyEvent AcceleratorExecute::st_VCLKey2AWTKey(const KeyCode& aVCLKey)
{
    css::awt::KeyEvent aAWTKey;
    aAWTKey.Modifiers = 0;
    aAWTKey.KeyCode   = (sal_Int16)aVCLKey.GetCode();

    if (aVCLKey.IsShift())
        aAWTKey.Modifiers |= css::awt::KeyModifier::SHIFT;
    if (aVCLKey.IsMod1())
        aAWTKey.Modifiers |= css::awt::KeyModifier::MOD1;
    if (aVCLKey.IsMod2())
        aAWTKey.Modifiers |= css::awt::KeyModifier::MOD2;

    return aAWTKey;
}

/*
ViewFrame->GetObjectShell
ObjectShell->GetStyleSheetPool
*/

//-----------------------------------------------
KeyCode AcceleratorExecute::st_AWTKey2VCLKey(const css::awt::KeyEvent& aAWTKey)
{
    sal_Bool bShift = ((aAWTKey.Modifiers & css::awt::KeyModifier::SHIFT) == css::awt::KeyModifier::SHIFT );
    sal_Bool bMod1  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD1 ) == css::awt::KeyModifier::MOD1  );
    sal_Bool bMod2  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD2 ) == css::awt::KeyModifier::MOD2  );
    USHORT   nKey   = (USHORT)aAWTKey.KeyCode;

    return KeyCode(nKey, bShift, bMod1, bMod2);
}

//-----------------------------------------------
::rtl::OUString AcceleratorExecute::impl_ts_findCommand(const css::awt::KeyEvent& aKey)
{
    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    css::uno::Reference< css::ui::XAcceleratorConfiguration > xGlobalCfg = m_xGlobalCfg;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xModuleCfg = m_xModuleCfg;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xDocCfg    = m_xDocCfg   ;

    aLock.clear();
    // <- SAFE ----------------------------------

    ::rtl::OUString sCommand;
    try
    {
        if (xDocCfg.is())
            sCommand = xDocCfg->getCommandByKeyEvent(aKey);
    }
    catch(const css::container::NoSuchElementException&)
    {
        try
        {
            if (xModuleCfg.is())
                sCommand = xModuleCfg->getCommandByKeyEvent(aKey);
        }
        catch(const css::container::NoSuchElementException&)
        {
            try
            {
                if (xGlobalCfg.is())
                    sCommand = xGlobalCfg->getCommandByKeyEvent(aKey);
            }
            catch(const css::container::NoSuchElementException&)
                {}
        }
    }

    return sCommand;
}

//-----------------------------------------------
css::uno::Reference< css::ui::XAcceleratorConfiguration > AcceleratorExecute::impl_st_openGlobalConfig(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xAccCfg(
        xSMGR->createInstance(::rtl::OUString::createFromAscii("com.sun.star.ui.GlobalAcceleratorConfiguration")),
        css::uno::UNO_QUERY_THROW);
    return xAccCfg;
}

//-----------------------------------------------
css::uno::Reference< css::ui::XAcceleratorConfiguration > AcceleratorExecute::impl_st_openModuleConfig(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                                                                                                        const css::uno::Reference< css::frame::XFrame >&              xFrame)
{
    css::uno::Reference< css::frame::XModuleManager > xModuleDetection(
        xSMGR->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.ModuleManager")),
        css::uno::UNO_QUERY_THROW);

    ::rtl::OUString sModule;
    try
    {
        sModule = xModuleDetection->identify(xFrame);
    }
    catch(const css::uno::RuntimeException& exRuntime)
        { throw exRuntime; }
    catch(const css::uno::Exception&)
        { return css::uno::Reference< css::ui::XAcceleratorConfiguration >(); }

    css::uno::Reference< css::ui::XModuleUIConfigurationManagerSupplier > xUISupplier(
        xSMGR->createInstance(::rtl::OUString::createFromAscii("com.sun.star.ui.ModuleUIConfigurationManagerSupplier")),
        css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::ui::XUIConfigurationManager >   xUIManager = xUISupplier->getUIConfigurationManager(sModule);
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xAccCfg    (xUIManager->getShortCutManager(), css::uno::UNO_QUERY_THROW);
    return xAccCfg;
}

//-----------------------------------------------
css::uno::Reference< css::ui::XAcceleratorConfiguration > AcceleratorExecute::impl_st_openDocConfig(const css::uno::Reference< css::frame::XModel >& xModel)
{
    css::uno::Reference< css::ui::XUIConfigurationManagerSupplier > xUISupplier(xModel, css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::ui::XUIConfigurationManager >         xUIManager = xUISupplier->getUIConfigurationManager();
    css::uno::Reference< css::ui::XAcceleratorConfiguration >       xAccCfg    (xUIManager->getShortCutManager(), css::uno::UNO_QUERY_THROW);
    return xAccCfg;
}

//-----------------------------------------------
css::uno::Reference< css::util::XURLTransformer > AcceleratorExecute::impl_ts_getURLParser()
{
    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (m_xURLParser.is())
        return m_xURLParser;
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;

    aLock.clear();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::util::XURLTransformer > xParser(
                xSMGR->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer")),
                css::uno::UNO_QUERY_THROW);

    // SAFE -> ----------------------------------
    aLock.reset();
    m_xURLParser = xParser;
    aLock.clear();
    // <- SAFE ----------------------------------

    return xParser;
}

//-----------------------------------------------
IMPL_LINK(AcceleratorExecute, impl_ts_asyncCallback, void*, pVoid)
{
    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    TCommandQueue::iterator pIt = m_lCommandQueue.begin();
    if (pIt == m_lCommandQueue.end())
        return 0;
    ::rtl::OUString sCommand = *pIt;
    m_lCommandQueue.erase(pIt);

    css::uno::Reference< css::frame::XDispatchProvider > xProvider = m_xDispatcher;

    aLock.clear();
    // <- SAFE ----------------------------------

    // convert command in URL structure
    css::uno::Reference< css::util::XURLTransformer > xParser = impl_ts_getURLParser();
    css::util::URL aURL;
    aURL.Complete = sCommand;
    xParser->parseStrict(aURL);

    // ask for dispatch object
    css::uno::Reference< css::frame::XDispatch > xDispatch = xProvider->queryDispatch(aURL, ::rtl::OUString(), 0);
    if (xDispatch.is())
        xDispatch->dispatch(aURL, css::uno::Sequence< css::beans::PropertyValue >());

    return 0;
}

} // namespace svt
