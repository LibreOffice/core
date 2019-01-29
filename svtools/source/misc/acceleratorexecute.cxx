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

#include <memory>
#include <svtools/acceleratorexecute.hxx>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/ui/GlobalAcceleratorConfiguration.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <cppuhelper/implbase.hxx>

#include <vcl/evntpost.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

namespace svt
{

class AsyncAccelExec : public cppu::WeakImplHelper<css::lang::XEventListener>
{
    private:
        css::uno::Reference<css::lang::XComponent> m_xFrame;
        css::uno::Reference< css::frame::XDispatch > m_xDispatch;
        css::util::URL const m_aURL;
        vcl::EventPoster m_aAsyncCallback;
    public:

        /** creates a new instance of this class, which can be used
            one times only!

            This instance can be forced to execute its internal set request
            asynchronous. After that it deletes itself!
         */
        static AsyncAccelExec* createOneShotInstance(const css::uno::Reference<css::lang::XComponent>& xFrame,
                                                    const css::uno::Reference<css::frame::XDispatch>& xDispatch,
                                                    const css::util::URL& rURL);

        void execAsync();
    private:

        virtual void SAL_CALL disposing(const css::lang::EventObject&) override
        {
            m_xFrame->removeEventListener(this);
            m_xFrame.clear();
            m_xDispatch.clear();
        }

        /** @short  allow creation of instances of this class
                    by using our factory only!
         */
        AsyncAccelExec(const css::uno::Reference<css::lang::XComponent>& xFrame,
                                      const css::uno::Reference< css::frame::XDispatch >& xDispatch,
                                      const css::util::URL& rURL);

        DECL_LINK(impl_ts_asyncCallback, LinkParamNone*, void);
};


AcceleratorExecute::AcceleratorExecute()
    : TMutexInit()
{
}

AcceleratorExecute::~AcceleratorExecute()
{
    // does nothing real
}


std::unique_ptr<AcceleratorExecute> AcceleratorExecute::createAcceleratorHelper()
{
    return std::unique_ptr<AcceleratorExecute>(new AcceleratorExecute);
}


void AcceleratorExecute::init(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                              const css::uno::Reference< css::frame::XFrame >&              xEnv )
{
    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // take over the uno service manager
    m_xContext = rxContext;

    // specify our internal dispatch provider
    // frame or desktop?! => document or global config.
    bool bDesktopIsUsed = false;
    m_xDispatcher.set(xEnv, css::uno::UNO_QUERY);
    if (!m_xDispatcher.is())
    {
        aLock.clear();
        // <- SAFE ------------------------------

        css::uno::Reference< css::frame::XDispatchProvider > xDispatcher(css::frame::Desktop::create(rxContext), css::uno::UNO_QUERY_THROW);

        // SAFE -> ------------------------------
        aLock.reset();

        m_xDispatcher  = xDispatcher;
        bDesktopIsUsed = true;
    }

    aLock.clear();
    // <- SAFE ----------------------------------

    // open all needed configuration objects
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xGlobalCfg;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xModuleCfg;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xDocCfg   ;

    // global cfg
    xGlobalCfg = css::ui::GlobalAcceleratorConfiguration::create(rxContext);
    if (!bDesktopIsUsed)
    {
        // module cfg
        xModuleCfg = AcceleratorExecute::st_openModuleConfig(rxContext, xEnv);

        // doc cfg
        css::uno::Reference< css::frame::XController > xController;
        css::uno::Reference< css::frame::XModel >      xModel;
        xController = xEnv->getController();
        if (xController.is())
            xModel = xController->getModel();
        if (xModel.is())
            xDocCfg = AcceleratorExecute::st_openDocConfig(xModel);
    }

    // SAFE -> ------------------------------
    aLock.reset();

    m_xGlobalCfg = xGlobalCfg;
    m_xModuleCfg = xModuleCfg;
    m_xDocCfg    = xDocCfg   ;

    aLock.clear();
    // <- SAFE ----------------------------------
}


bool AcceleratorExecute::execute(const vcl::KeyCode& aVCLKey)
{
    css::awt::KeyEvent aAWTKey = AcceleratorExecute::st_VCLKey2AWTKey(aVCLKey);
    return execute(aAWTKey);
}


bool AcceleratorExecute::execute(const css::awt::KeyEvent& aAWTKey)
{
    OUString sCommand = impl_ts_findCommand(aAWTKey);

    // No Command found? Do nothing! User is not interested on any error handling .-)
    // or for some reason m_xContext is NULL (which would crash impl_ts_getURLParser()
    if (sCommand.isEmpty() || !m_xContext.is())
    {
        return false;
    }

    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    css::uno::Reference< css::frame::XDispatchProvider > xProvider = m_xDispatcher;

    aLock.clear();
    // <- SAFE ----------------------------------

    // convert command in URL structure
    css::uno::Reference< css::util::XURLTransformer > xParser = impl_ts_getURLParser();
    css::util::URL aURL;
    aURL.Complete = sCommand;
    xParser->parseStrict(aURL);

    // ask for dispatch object
    css::uno::Reference< css::frame::XDispatch > xDispatch = xProvider->queryDispatch(aURL, OUString(), 0);
    bool bRet = xDispatch.is();
    if ( bRet )
    {
        // Note: Such instance can be used one times only and destroy itself afterwards .-)
        css::uno::Reference<css::lang::XComponent> xFrame(xProvider, css::uno::UNO_QUERY);
        AsyncAccelExec* pExec = AsyncAccelExec::createOneShotInstance(xFrame, xDispatch, aURL);
        pExec->execAsync();
    }

    return bRet;
}


css::awt::KeyEvent AcceleratorExecute::st_VCLKey2AWTKey(const vcl::KeyCode& aVCLKey)
{
    css::awt::KeyEvent aAWTKey;
    aAWTKey.Modifiers = 0;
    aAWTKey.KeyCode   = static_cast<sal_Int16>(aVCLKey.GetCode());

    if (aVCLKey.IsShift())
        aAWTKey.Modifiers |= css::awt::KeyModifier::SHIFT;
    if (aVCLKey.IsMod1())
        aAWTKey.Modifiers |= css::awt::KeyModifier::MOD1;
    if (aVCLKey.IsMod2())
        aAWTKey.Modifiers |= css::awt::KeyModifier::MOD2;
    if (aVCLKey.IsMod3())
        aAWTKey.Modifiers |= css::awt::KeyModifier::MOD3;
    return aAWTKey;
}


vcl::KeyCode AcceleratorExecute::st_AWTKey2VCLKey(const css::awt::KeyEvent& aAWTKey)
{
    bool bShift = ((aAWTKey.Modifiers & css::awt::KeyModifier::SHIFT) == css::awt::KeyModifier::SHIFT );
    bool bMod1  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD1 ) == css::awt::KeyModifier::MOD1  );
    bool bMod2  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD2 ) == css::awt::KeyModifier::MOD2  );
    bool bMod3  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD3 ) == css::awt::KeyModifier::MOD3  );
    sal_uInt16   nKey   = static_cast<sal_uInt16>(aAWTKey.KeyCode);

    return vcl::KeyCode(nKey, bShift, bMod1, bMod2, bMod3);
}

OUString AcceleratorExecute::findCommand(const css::awt::KeyEvent& aKey)
{
    return impl_ts_findCommand(aKey);
}

OUString AcceleratorExecute::impl_ts_findCommand(const css::awt::KeyEvent& aKey)
{
    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    css::uno::Reference< css::ui::XAcceleratorConfiguration > xGlobalCfg = m_xGlobalCfg;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xModuleCfg = m_xModuleCfg;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xDocCfg    = m_xDocCfg   ;

    aLock.clear();
    // <- SAFE ----------------------------------

    OUString sCommand;

    try
    {
        if (xDocCfg.is())
            sCommand = xDocCfg->getCommandByKeyEvent(aKey);
        if (!sCommand.isEmpty())
            return sCommand;
    }
    catch(const css::container::NoSuchElementException&)
        {}

    try
    {
        if (xModuleCfg.is())
            sCommand = xModuleCfg->getCommandByKeyEvent(aKey);
        if (!sCommand.isEmpty())
            return sCommand;
    }
    catch(const css::container::NoSuchElementException&)
        {}

    try
    {
        if (xGlobalCfg.is())
            sCommand = xGlobalCfg->getCommandByKeyEvent(aKey);
        if (!sCommand.isEmpty())
            return sCommand;
    }
    catch(const css::container::NoSuchElementException&)
        {}

    // fall back to functional key codes
    if( aKey.Modifiers == 0 )
    {
        switch( aKey.KeyCode )
        {
        case css::awt::Key::DELETE_TO_BEGIN_OF_LINE:
            return OUString( ".uno:DelToStartOfLine" );
        case css::awt::Key::DELETE_TO_END_OF_LINE:
            return OUString( ".uno:DelToEndOfLine" );
        case css::awt::Key::DELETE_TO_BEGIN_OF_PARAGRAPH:
            return OUString( ".uno:DelToStartOfPara" );
        case css::awt::Key::DELETE_TO_END_OF_PARAGRAPH:
            return OUString( ".uno:DelToEndOfPara" );
        case css::awt::Key::DELETE_WORD_BACKWARD:
            return OUString( ".uno:DelToStartOfWord" );
        case css::awt::Key::DELETE_WORD_FORWARD:
            return OUString( ".uno:DelToEndOfWord" );
        case css::awt::Key::INSERT_LINEBREAK:
            return OUString( ".uno:InsertLinebreak" );
        case css::awt::Key::INSERT_PARAGRAPH:
            return OUString( ".uno:InsertPara" );
        case css::awt::Key::MOVE_WORD_BACKWARD:
            return OUString( ".uno:GoToPrevWord" );
        case css::awt::Key::MOVE_WORD_FORWARD:
            return OUString( ".uno:GoToNextWord" );
        case css::awt::Key::MOVE_TO_BEGIN_OF_LINE:
            return OUString( ".uno:GoToStartOfLine" );
        case css::awt::Key::MOVE_TO_END_OF_LINE:
            return OUString( ".uno:GoToEndOfLine" );
        case css::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
            return OUString( ".uno:GoToStartOfPara" );
        case css::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
            return OUString( ".uno:GoToEndOfPara" );
        case css::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
            return OUString( ".uno:GoToStartOfDoc" );
        case css::awt::Key::MOVE_TO_END_OF_DOCUMENT:
            return OUString( ".uno:GoToEndOfDoc" );
        case css::awt::Key::SELECT_BACKWARD:
            return OUString( ".uno:CharLeftSel" );
        case css::awt::Key::SELECT_FORWARD:
            return OUString( ".uno:CharRightSel" );
        case css::awt::Key::SELECT_WORD_BACKWARD:
            return OUString( ".uno:WordLeftSel" );
        case css::awt::Key::SELECT_WORD_FORWARD:
            return OUString( ".uno:WordRightSel" );
        case css::awt::Key::SELECT_WORD:
            return OUString( ".uno:SelectWord" );
        case css::awt::Key::SELECT_LINE:
            return OUString();
        case css::awt::Key::SELECT_PARAGRAPH:
            return OUString( ".uno:SelectText" );
        case css::awt::Key::SELECT_TO_BEGIN_OF_LINE:
            return OUString( ".uno:StartOfLineSel" );
        case css::awt::Key::SELECT_TO_END_OF_LINE:
            return OUString( ".uno:EndOfLineSel" );
        case css::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
            return OUString( ".uno:StartOfParaSel" );
        case css::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
            return OUString( ".uno:EndOfParaSel" );
        case css::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
            return OUString( ".uno:StartOfDocumentSel" );
        case css::awt::Key::SELECT_TO_END_OF_DOCUMENT:
            return OUString( ".uno:EndOfDocumentSel" );
        case css::awt::Key::SELECT_ALL:
            return OUString( ".uno:SelectAll" );
        default:
            break;
        }
    }

    return OUString();
}


css::uno::Reference< css::ui::XAcceleratorConfiguration > AcceleratorExecute::st_openModuleConfig(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                                                                                   const css::uno::Reference< css::frame::XFrame >&              xFrame)
{
    css::uno::Reference< css::frame::XModuleManager2 > xModuleDetection(
        css::frame::ModuleManager::create(rxContext));

    OUString sModule;
    try
    {
        sModule = xModuleDetection->identify(xFrame);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { return css::uno::Reference< css::ui::XAcceleratorConfiguration >(); }

    css::uno::Reference< css::ui::XModuleUIConfigurationManagerSupplier > xUISupplier(
        css::ui::theModuleUIConfigurationManagerSupplier::get(rxContext) );

    css::uno::Reference< css::ui::XAcceleratorConfiguration > xAccCfg;
    try
    {
        css::uno::Reference< css::ui::XUIConfigurationManager >   xUIManager = xUISupplier->getUIConfigurationManager(sModule);
        xAccCfg = xUIManager->getShortCutManager();
    }
    catch(const css::container::NoSuchElementException&)
        {}
    return xAccCfg;
}


css::uno::Reference< css::ui::XAcceleratorConfiguration > AcceleratorExecute::st_openDocConfig(const css::uno::Reference< css::frame::XModel >& xModel)
{
    css::uno::Reference< css::ui::XAcceleratorConfiguration >       xAccCfg;
    css::uno::Reference< css::ui::XUIConfigurationManagerSupplier > xUISupplier(xModel, css::uno::UNO_QUERY);
    if (xUISupplier.is())
    {
        css::uno::Reference< css::ui::XUIConfigurationManager >     xUIManager = xUISupplier->getUIConfigurationManager();
        xAccCfg = xUIManager->getShortCutManager();
    }
    return xAccCfg;
}


css::uno::Reference< css::util::XURLTransformer > AcceleratorExecute::impl_ts_getURLParser()
{
    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (m_xURLParser.is())
        return m_xURLParser;
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;

    aLock.clear();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::util::XURLTransformer > xParser =  css::util::URLTransformer::create( xContext );

    // SAFE -> ----------------------------------
    aLock.reset();
    m_xURLParser = xParser;
    aLock.clear();
    // <- SAFE ----------------------------------

    return xParser;
}

AsyncAccelExec::AsyncAccelExec(const css::uno::Reference<css::lang::XComponent>& xFrame,
                               const css::uno::Reference<css::frame::XDispatch>& xDispatch,
                               const css::util::URL& rURL)
    : m_xFrame(xFrame)
    , m_xDispatch(xDispatch)
    , m_aURL(rURL)
    , m_aAsyncCallback(LINK(this, AsyncAccelExec, impl_ts_asyncCallback))
{
}

AsyncAccelExec* AsyncAccelExec::createOneShotInstance(const css::uno::Reference<css::lang::XComponent> &xFrame,
                                                     const css::uno::Reference< css::frame::XDispatch >& xDispatch,
                                                     const css::util::URL& rURL)
{
    AsyncAccelExec* pExec = new AsyncAccelExec(xFrame, xDispatch, rURL);
    return pExec;
}


void AsyncAccelExec::execAsync()
{
    acquire();
    if (m_xFrame.is())
        m_xFrame->addEventListener(this);
    m_aAsyncCallback.Post();
}

IMPL_LINK_NOARG(AsyncAccelExec, impl_ts_asyncCallback, LinkParamNone*, void)
{
    if (m_xDispatch.is())
    {
        try
        {
            if (m_xFrame.is())
                m_xFrame->removeEventListener(this);
            m_xDispatch->dispatch(m_aURL, css::uno::Sequence< css::beans::PropertyValue >());
        }
        catch(const css::uno::Exception&)
        {
        }
    }
    release();
}

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
