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
#ifndef _MSVBAHELPER_HXX
#define _MSVBAHELPER_HXX

#include <sfx2/objsh.hxx>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/vba/XVBAMacroResolver.hpp>
#include "filter/msfilter/msfilterdllapi.h"

namespace ooo {
namespace vba {

// ============================================================================

struct MSFILTER_DLLPUBLIC MacroResolvedInfo
{
    SfxObjectShell*     mpDocContext;
    String              msResolvedMacro;
    bool                mbFound;

    inline explicit MacroResolvedInfo( SfxObjectShell* pDocContext = 0 ) : mpDocContext( pDocContext ), mbFound( false ) {}
};

MSFILTER_DLLPUBLIC String makeMacroURL( const String& sMacroName );
MSFILTER_DLLPUBLIC ::rtl::OUString extractMacroName( const ::rtl::OUString& rMacroUrl );
MSFILTER_DLLPUBLIC MacroResolvedInfo resolveVBAMacro( SfxObjectShell* pShell, const ::rtl::OUString& rMacroName, bool bSearchGlobalTemplates = false );
MSFILTER_DLLPUBLIC sal_Bool executeMacro( SfxObjectShell* pShell, const String& sMacroName, com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArgs, com::sun::star::uno::Any& aRet, const com::sun::star::uno::Any& aCaller );

// ============================================================================

typedef ::cppu::WeakImplHelper3<
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XInitialization,
    ::com::sun::star::script::vba::XVBAMacroResolver > VBAMacroResolverBase;

class VBAMacroResolver : public VBAMacroResolverBase
{
public:
    explicit            VBAMacroResolver();
    virtual             ~VBAMacroResolver();

    // com.sun.star.lang.XServiceInfo interface -------------------------------

    virtual ::rtl::OUString SAL_CALL
                        getImplementationName() throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
                        supportsService( const ::rtl::OUString& rService )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                        getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // com.sun.star.lang.XInitialization interface ----------------------------

    virtual void SAL_CALL initialize(
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rArgs )
                            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // com.sun.star.script.vba.XVBAMacroResolver interface --------------------

    virtual ::rtl::OUString SAL_CALL
                        resolveVBAMacroToScriptURL( const ::rtl::OUString& rVBAMacroName )
                            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL
                        resolveScriptURLtoVBAMacro( const ::rtl::OUString& rScriptURL )
                            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxModel;
    SfxObjectShell*     mpObjShell;
    ::rtl::OUString     maProjectName;
};

// ============================================================================

} // namespace vba
} // namespace ooo

#endif
