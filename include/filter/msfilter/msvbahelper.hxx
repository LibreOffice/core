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
#ifndef INCLUDED_FILTER_MSFILTER_MSVBAHELPER_HXX
#define INCLUDED_FILTER_MSFILTER_MSVBAHELPER_HXX

#include <sfx2/objsh.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/vba/XVBAMacroResolver.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <filter/msfilter/msfilterdllapi.h>
namespace ooo {
namespace vba {



struct MSFILTER_DLLPUBLIC MacroResolvedInfo
{
    SfxObjectShell*     mpDocContext;
    OUString            msResolvedMacro;
    bool                mbFound;

    inline explicit MacroResolvedInfo( SfxObjectShell* pDocContext = nullptr ) : mpDocContext( pDocContext ), mbFound( false ) {}
};

MSFILTER_DLLPUBLIC OUString makeMacroURL( const OUString& sMacroName );
MSFILTER_DLLPUBLIC OUString extractMacroName( const OUString& rMacroUrl );
MSFILTER_DLLPUBLIC OUString getDefaultProjectName( SfxObjectShell* pShell );
MSFILTER_DLLPUBLIC OUString resolveVBAMacro( SfxObjectShell* pShell, const OUString& rLibName, const OUString& rModuleName, const OUString& rMacroName );
MSFILTER_DLLPUBLIC MacroResolvedInfo resolveVBAMacro( SfxObjectShell* pShell, const OUString& rMacroName, bool bSearchGlobalTemplates = false );
MSFILTER_DLLPUBLIC bool executeMacro( SfxObjectShell* pShell, const OUString& sMacroName, css::uno::Sequence< css::uno::Any >& aArgs, css::uno::Any& aRet, const css::uno::Any& aCaller );
MSFILTER_DLLPUBLIC css::awt::KeyEvent parseKeyEvent( const OUString& sKey ) throw (css::uno::RuntimeException);
MSFILTER_DLLPUBLIC void applyShortCutKeyBinding ( const css::uno::Reference< css::frame::XModel >& rxDoc, const css::awt::KeyEvent& rKeyEvent, const OUString& sMacro ) throw (css::uno::RuntimeException, std::exception);


typedef ::cppu::WeakImplHelper<
    css::lang::XServiceInfo,
    css::lang::XInitialization,
    css::script::vba::XVBAMacroResolver > VBAMacroResolverBase;

class VBAMacroResolver : public VBAMacroResolverBase
{
public:
    explicit            VBAMacroResolver();
    virtual             ~VBAMacroResolver();

    // com.sun.star.lang.XServiceInfo interface -------------------------------

    virtual OUString SAL_CALL
                        getImplementationName() throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL
                        supportsService( const OUString& rService )
                            throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
                        getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // com.sun.star.lang.XInitialization interface ----------------------------

    virtual void SAL_CALL initialize(
                            const css::uno::Sequence< css::uno::Any >& rArgs )
                            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // com.sun.star.script.vba.XVBAMacroResolver interface --------------------

    virtual OUString SAL_CALL
                        resolveVBAMacroToScriptURL( const OUString& rVBAMacroName )
                            throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL
                        resolveScriptURLtoVBAMacro( const OUString& rScriptURL )
                            throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

private:
    css::uno::Reference< css::frame::XModel > mxModel;
    SfxObjectShell*     mpObjShell;
    OUString            maProjectName;
};

OUString SAL_CALL VBAMacroResolver_getImplementationName();

css::uno::Sequence<OUString> SAL_CALL
VBAMacroResolver_getSupportedServiceNames();

css::uno::Reference<css::uno::XInterface> SAL_CALL
VBAMacroResolver_createInstance(
    css::uno::Reference<css::uno::XComponentContext > const & rxContext)
    throw (css::uno::Exception);

} // namespace vba
} // namespace ooo

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
