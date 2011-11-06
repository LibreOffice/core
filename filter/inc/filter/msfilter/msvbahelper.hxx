/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
MSFILTER_DLLPUBLIC ::rtl::OUString getDefaultProjectName( SfxObjectShell* pShell );
MSFILTER_DLLPUBLIC ::rtl::OUString resolveVBAMacro( SfxObjectShell* pShell, const ::rtl::OUString& rLibName, const ::rtl::OUString& rModuleName, const ::rtl::OUString& rMacroName );
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
