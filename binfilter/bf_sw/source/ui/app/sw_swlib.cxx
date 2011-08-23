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


#define _SWLIB_CXX

#include <bf_sfx2/fcontnr.hxx>
#include <bf_sfx2/docfile.hxx>

#include <swdll0.hxx> // wird vom makefile generiert
#include <swmodule.hxx>
#include <wdocsh.hxx>
#include <globdoc.hxx>
#include <shellio.hxx>
#include <comphelper/classids.hxx>

#include <bf_svtools/moduleoptions.hxx>

#include <app.hrc>
#include <web.hrc>
#include <globdoc.hrc>

#include <iodetect.hxx>

namespace binfilter {

#define C2S(cChar) String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(cChar))
//os: the Icon Id is unused since start of VCL
#define ICON_ID_INVALID 0

inline BOOL IsDocShellRegistered()
{
    return SvtModuleOptions().IsWriter();
}

USHORT AutoDetec( const String& FileName, USHORT & rVersion );

IO_DETECT_IMPL1
IO_DETECT_IMPL2
IO_DETECT_IMPL3
IO_DETECT_IMPL4

//-------------------------------------------------------------------------
extern "C" { static void SAL_CALL thisModule() {} }

SFX_IMPL_MODULE_LIB(Sw, C2S(DLL_NAME))

TYPEINIT1( SwModuleDummy, SfxModule );


//-------------------------------------------------------------------------
SFX_IMPL_OBJECTFACTORY_MOD(SwDocShell, 						\
        SFXOBJECTSHELL_STD_NORMAL|SFXOBJECTSHELL_HASMENU,   \
        swriter, 											\
        SvGlobalName(BF_SO3_SW_CLASSID),						\
        Sw,													\
        DLL_NAME)
{
    SfxObjectFactory& rFactory = (SfxObjectFactory&)Factory();
    rFactory.SetDocumentServiceName(C2S("com.sun.star.text.TextDocument"));
    rFactory.GetFilterContainer()->SetDetectFilter( &SwDLL::DetectFilter );
}

//-------------------------------------------------------------------------
SFX_IMPL_OBJECTFACTORY_MOD(SwWebDocShell, 								\
        SFXOBJECTSHELL_STD_NORMAL|SFXOBJECTSHELL_HASMENU, swriter/web, 	\
        SvGlobalName(BF_SO3_SWWEB_CLASSID),								\
        Sw,																\
        DLL_NAME)
{
    SfxObjectFactory& rFactory = (SfxObjectFactory&)Factory();
    rFactory.SetDocumentServiceName(C2S("com.sun.star.text.WebDocument"));
    rFactory.GetFilterContainer()->SetDetectFilter( &SwDLL::DetectFilter );
}

//-------------------------------------------------------------------------
SFX_IMPL_OBJECTFACTORY_MOD(SwGlobalDocShell, 										  \
         SFXOBJECTSHELL_STD_NORMAL|SFXOBJECTSHELL_HASMENU, swriter/GlobalDocument, \
        SvGlobalName(BF_SO3_SWGLOB_CLASSID),								\
        Sw,																			  \
        DLL_NAME)
{
    SfxObjectFactory& rFactory = (SfxObjectFactory&)Factory();
    rFactory.SetDocumentServiceName(C2S("com.sun.star.text.GlobalDocument"));
    rFactory.GetFilterContainer()->SetDetectFilter( &SwDLL::GlobDetectFilter );
}

//-------------------------------------------------------------------------

void 	SwDLL::LibInit()
{
    // in most cases you must initialize your document-factory here
    SvtModuleOptions aOpt;
    SfxObjectFactory* pWriterFactory = NULL;
    SfxObjectFactory* pGlobalFactory = NULL;
    if ( aOpt.IsWriter() )
    {
        SwDocShell::RegisterFactory(    SDT_SW_DOCFACTPRIO      );
        SwGlobalDocShell::RegisterFactory(SDT_SW_DOCFACTPRIO + 2);
        pWriterFactory = &SwDocShell::Factory();
        pGlobalFactory = &SwGlobalDocShell::Factory();
    }

    // WebWriter alway needed because it is used for the help viewer
    SwWebDocShell::RegisterFactory(	SDT_SW_DOCFACTPRIO + 1	);

    // create a dummy-module for Object-Factory-Pointer
    SW_MOD() = (SwModule*)new SwModuleDummy( 0, TRUE, &SwWebDocShell::Factory(), pWriterFactory, pGlobalFactory );
    SwModule* pMod = SW_MOD();
}

//-------------------------------------------------------------------------
void 	SwDLL::LibExit()
{
        // Innerhalb von FreeLibSw wird Exit() gerufen
    FreeLibSw();

    // destroy the dummy-module with Object-Factory-Pointer
    DELETEZ( *(SwModuleDummy**) GetAppData(BF_SHL_WRITER) );
}

//-------------------------------------------------------------------------

SfxModule* SwModuleDummy::Load()
{
    return (LoadLibSw() ? SW_MOD() : NULL);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
