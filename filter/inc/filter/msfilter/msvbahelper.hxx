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
#include "filter/msfilter/msfilterdllapi.h"

namespace ooo { namespace vba 
{
    class MSFILTER_DLLPUBLIC VBAMacroResolvedInfo
    {
        SfxObjectShell* mpDocContext;
        bool mbFound;
        String msResolvedMacro;
        public:
        VBAMacroResolvedInfo() : mpDocContext(NULL), mbFound( false ){}
        void SetResolved( bool bRes ) { mbFound = bRes; }
        bool IsResolved() { return mbFound; }
        void SetMacroDocContext(SfxObjectShell* pShell )  { mpDocContext = pShell; }
        SfxObjectShell* MacroDocContext() { return mpDocContext; }
        String ResolvedMacro() { return msResolvedMacro; }
        void SetResolvedMacro(const String& sMacro ) { msResolvedMacro = sMacro; }
    };

    MSFILTER_DLLPUBLIC String makeMacroURL( const String& sMacroName );
    MSFILTER_DLLPUBLIC ::rtl::OUString extractMacroName( const ::rtl::OUString& rMacroUrl );
    MSFILTER_DLLPUBLIC VBAMacroResolvedInfo resolveVBAMacro( SfxObjectShell* pShell, const rtl::OUString& sMod, bool bSearchGlobalTemplates = false );
    MSFILTER_DLLPUBLIC sal_Bool executeMacro( SfxObjectShell* pShell, const String& sMacroName, com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArgs, com::sun::star::uno::Any& aRet, const com::sun::star::uno::Any& aCaller );
} }

#endif
