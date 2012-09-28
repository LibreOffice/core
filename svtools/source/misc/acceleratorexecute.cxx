/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <svtools/acceleratorexecute.hxx>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/ModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>


namespace  css = ::com::sun::star;

namespace svt
{


//-----------------------------------------------
class SVT_DLLPRIVATE AsyncAccelExec
{
    public:
        //---------------------------------------
        /** creates a new instance of this class, which can be used
            one times only!

            This instance can be forced to execute it's internal set request
            asynchronous. After that it deletes itself !
         */
        static AsyncAccelExec* createOnShotInstance(const css::uno::Reference< css::frame::XDispatch >& xDispatch,
                                                    const css::util::URL&                               aURL     );

        void execAsync();

    private:
        //---------------------------------------
        /** @short  allow creation of instances of this class
                    by using our factory only!
         */
        SVT_DLLPRIVATE AsyncAccelExec(const css::uno::Reference< css::frame::XDispatch >& xDispatch,
                                      const css::util::URL&                               aURL     );

        DECL_DLLPRIVATE_LINK(impl_ts_asyncCallback, void*);

    private:
        ::vcl::EventPoster m_aAsyncCallback;
        css::uno::Reference< css::frame::XDispatch > m_xDispatch;
        css::util::URL m_aURL;
};

//-----------------------------------------------
AcceleratorExecute::AcceleratorExecute()
    : TMutexInit      (                                                     )
    , m_aAsyncCallback(LINK(this, AcceleratorExecute, impl_ts_asyncCallback))
{
}

//-----------------------------------------------
AcceleratorExecute::AcceleratorExecute(const AcceleratorExecute&)
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
                            xSMGR->createInstance(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ))),
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
    xGlobalCfg = AcceleratorExecute::st_openGlobalConfig(xSMGR);
    if (!bDesktopIsUsed)
    {
        // module cfg
        xModuleCfg = AcceleratorExecute::st_openModuleConfig(xSMGR, xEnv);

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

//-----------------------------------------------
sal_Bool AcceleratorExecute::execute(const KeyCode& aVCLKey)
{
    css::awt::KeyEvent aAWTKey = AcceleratorExecute::st_VCLKey2AWTKey(aVCLKey);
    return execute(aAWTKey);
}

//-----------------------------------------------
sal_Bool AcceleratorExecute::execute(const css::awt::KeyEvent& aAWTKey)
{
    ::rtl::OUString sCommand = impl_ts_findCommand(aAWTKey);

    // No Command found? Do nothing! User isnt interested on any error handling .-)
    if (sCommand.isEmpty())
        return sal_False;

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
    css::uno::Reference< css::frame::XDispatch > xDispatch = xProvider->queryDispatch(aURL, ::rtl::OUString(), 0);
    sal_Bool bRet = xDispatch.is();
    if ( bRet )
    {
        // Note: Such instance can be used one times only and destroy itself afterwards .-)
        AsyncAccelExec* pExec = AsyncAccelExec::createOnShotInstance(xDispatch, aURL);
        pExec->execAsync();
    }

    return bRet;
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
        if (aVCLKey.IsMod3())
        aAWTKey.Modifiers |= css::awt::KeyModifier::MOD3;
    return aAWTKey;
}

//-----------------------------------------------
KeyCode AcceleratorExecute::st_AWTKey2VCLKey(const css::awt::KeyEvent& aAWTKey)
{
    sal_Bool bShift = ((aAWTKey.Modifiers & css::awt::KeyModifier::SHIFT) == css::awt::KeyModifier::SHIFT );
    sal_Bool bMod1  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD1 ) == css::awt::KeyModifier::MOD1  );
    sal_Bool bMod2  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD2 ) == css::awt::KeyModifier::MOD2  );
    sal_Bool bMod3  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD3 ) == css::awt::KeyModifier::MOD3  );
    sal_uInt16   nKey   = (sal_uInt16)aAWTKey.KeyCode;

    return KeyCode(nKey, bShift, bMod1, bMod2, bMod3);
}
//-----------------------------------------------
::rtl::OUString AcceleratorExecute::findCommand(const css::awt::KeyEvent& aKey)
{
    return impl_ts_findCommand(aKey);
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
        case com::sun::star::awt::Key::DELETE_TO_BEGIN_OF_LINE:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DelToStartOfLine" ) );
        case com::sun::star::awt::Key::DELETE_TO_END_OF_LINE:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DelToEndOfLine" ) );
        case com::sun::star::awt::Key::DELETE_TO_BEGIN_OF_PARAGRAPH:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DelToStartOfPara" ) );
        case com::sun::star::awt::Key::DELETE_TO_END_OF_PARAGRAPH:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DelToEndOfPara" ) );
        case com::sun::star::awt::Key::DELETE_WORD_BACKWARD:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DelToStartOfWord" ) );
        case com::sun::star::awt::Key::DELETE_WORD_FORWARD:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DelToEndOfWord" ) );
        case com::sun::star::awt::Key::INSERT_LINEBREAK:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertLinebreak" ) );
        case com::sun::star::awt::Key::INSERT_PARAGRAPH:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertPara" ) );
        case com::sun::star::awt::Key::MOVE_WORD_BACKWARD:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GoToPrevWord" ) );
        case com::sun::star::awt::Key::MOVE_WORD_FORWARD:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GoToNextWord" ) );
        case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_LINE:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GoToStartOfLine" ) );
        case com::sun::star::awt::Key::MOVE_TO_END_OF_LINE:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GoToEndOfLine" ) );
        case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GoToStartOfPara" ) );
        case com::sun::star::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GoToEndOfPara" ) );
        case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GoToStartOfDoc" ) );
        case com::sun::star::awt::Key::MOVE_TO_END_OF_DOCUMENT:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GoToEndOfDoc" ) );
        case com::sun::star::awt::Key::SELECT_BACKWARD:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CharLeftSel" ) );
        case com::sun::star::awt::Key::SELECT_FORWARD:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CharRightSel" ) );
        case com::sun::star::awt::Key::SELECT_WORD_BACKWARD:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:WordLeftSel" ) );
        case com::sun::star::awt::Key::SELECT_WORD_FORWARD:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:WordRightSel" ) );
        case com::sun::star::awt::Key::SELECT_WORD:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:SelectWord" ) );
        case com::sun::star::awt::Key::SELECT_LINE:
            return rtl::OUString();
        case com::sun::star::awt::Key::SELECT_PARAGRAPH:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:SelectText" ) );
        case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_LINE:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:StartOfLineSel" ) );
        case com::sun::star::awt::Key::SELECT_TO_END_OF_LINE:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:EndOfLineSel" ) );
        case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:StartOfParaSel" ) );
        case com::sun::star::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:EndOfParaSel" ) );
        case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:StartOfDocumentSel" ) );
        case com::sun::star::awt::Key::SELECT_TO_END_OF_DOCUMENT:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:EndOfDocumentSel" ) );
        case com::sun::star::awt::Key::SELECT_ALL:
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:SelectAll" ) );
        default:
            break;
        }
    }

    return ::rtl::OUString();
}

//-----------------------------------------------
css::uno::Reference< css::ui::XAcceleratorConfiguration > AcceleratorExecute::st_openGlobalConfig(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xAccCfg(
        xSMGR->createInstance(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.GlobalAcceleratorConfiguration" ))),
        css::uno::UNO_QUERY_THROW);
    return xAccCfg;
}

//-----------------------------------------------
css::uno::Reference< css::ui::XAcceleratorConfiguration > AcceleratorExecute::st_openModuleConfig(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                                                                                                   const css::uno::Reference< css::frame::XFrame >&              xFrame)
{
    css::uno::Reference< css::frame::XModuleManager2 > xModuleDetection(
        css::frame::ModuleManager::create(comphelper::getComponentContext(xSMGR)));

    ::rtl::OUString sModule;
    try
    {
        sModule = xModuleDetection->identify(xFrame);
    }
    catch(const css::uno::RuntimeException&rEx)
        { (void) rEx; throw; }
    catch(const css::uno::Exception&)
        { return css::uno::Reference< css::ui::XAcceleratorConfiguration >(); }

    css::uno::Reference< css::ui::XModuleUIConfigurationManagerSupplier > xUISupplier(
        css::ui::ModuleUIConfigurationManagerSupplier::create(comphelper::getComponentContext(xSMGR)) );

    css::uno::Reference< css::ui::XAcceleratorConfiguration > xAccCfg;
    try
    {
        css::uno::Reference< css::ui::XUIConfigurationManager >   xUIManager = xUISupplier->getUIConfigurationManager(sModule);
        xAccCfg = css::uno::Reference< css::ui::XAcceleratorConfiguration >(xUIManager->getShortCutManager(), css::uno::UNO_QUERY_THROW);
    }
    catch(const css::container::NoSuchElementException&)
        {}
    return xAccCfg;
}

//-----------------------------------------------
css::uno::Reference< css::ui::XAcceleratorConfiguration > AcceleratorExecute::st_openDocConfig(const css::uno::Reference< css::frame::XModel >& xModel)
{
    css::uno::Reference< css::ui::XAcceleratorConfiguration >       xAccCfg;
    css::uno::Reference< css::ui::XUIConfigurationManagerSupplier > xUISupplier(xModel, css::uno::UNO_QUERY);
    if (xUISupplier.is())
    {
        css::uno::Reference< css::ui::XUIConfigurationManager >     xUIManager = xUISupplier->getUIConfigurationManager();
        xAccCfg.set(xUIManager->getShortCutManager(), css::uno::UNO_QUERY_THROW);
    }
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
                css::util::URLTransformer::create( ::comphelper::getComponentContext(xSMGR) ) );

    // SAFE -> ----------------------------------
    aLock.reset();
    m_xURLParser = xParser;
    aLock.clear();
    // <- SAFE ----------------------------------

    return xParser;
}

//-----------------------------------------------
IMPL_LINK_NOARG(AcceleratorExecute, impl_ts_asyncCallback)
{
    // replaced by AsyncAccelExec!
    return 0;
}

//-----------------------------------------------
AsyncAccelExec::AsyncAccelExec(const css::uno::Reference< css::frame::XDispatch >& xDispatch,
                               const css::util::URL&                               aURL     )
    : m_aAsyncCallback(LINK(this, AsyncAccelExec, impl_ts_asyncCallback))
    , m_xDispatch     (xDispatch                                        )
    , m_aURL          (aURL                                             )
{
}

//-----------------------------------------------
AsyncAccelExec* AsyncAccelExec::createOnShotInstance(const css::uno::Reference< css::frame::XDispatch >& xDispatch,
                                                     const css::util::URL&                               aURL     )
{
    AsyncAccelExec* pExec = new AsyncAccelExec(xDispatch, aURL);
    return pExec;
}

//-----------------------------------------------
void AsyncAccelExec::execAsync()
{
    m_aAsyncCallback.Post(0);
}

//-----------------------------------------------
IMPL_LINK(AsyncAccelExec, impl_ts_asyncCallback, void*,)
{
    if (! m_xDispatch.is())
        return 0;

    try
    {
        m_xDispatch->dispatch(m_aURL, css::uno::Sequence< css::beans::PropertyValue >());
    }
    catch(const css::lang::DisposedException&)
        {}
    catch(const css::uno::RuntimeException& )
        { throw; }
    catch(const css::uno::Exception&)
        {}

    delete this;

    return 0;
}

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
