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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

//------------------------------------------------------------------

#include <bf_svx/unolingu.hxx>
#include <bf_svtools/lingucfg.hxx>

#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/lang/Locale.hpp>

using namespace ::com::sun::star;

#include "scmod.hxx"
namespace binfilter {

//------------------------------------------------------------------

//#define LINGUPROP_DEFLOCALE			"DefaultLocale"
//#define LINGUPROP_CJKLOCALE			"DefaultLocale_CJK"
//#define LINGUPROP_CTLLOCALE			"DefaultLocale_CTL"
#define LINGUPROP_AUTOSPELL			"IsSpellAuto"
#define LINGUPROP_HIDEAUTO			"IsSpellHide"

//------------------------------------------------------------------

// static
/*N*/ void ScModule::GetSpellSettings( USHORT& rDefLang, USHORT& rCjkLang, USHORT& rCtlLang,
/*N*/ 									BOOL& rAutoSpell, BOOL& rHideAuto )
/*N*/ {
/*N*/ 	//	use SvtLinguConfig instead of service LinguProperties to avoid
/*N*/ 	//	loading the linguistic component
/*N*/ 	SvtLinguConfig aConfig;
/*N*/ 
/*N*/ 	SvtLinguOptions aOptions;
/*N*/ 	aConfig.GetOptions( aOptions );
/*N*/ 
/*N*/ 	rDefLang = aOptions.nDefaultLanguage;
/*N*/ 	rCjkLang = aOptions.nDefaultLanguage_CJK;
/*N*/ 	rCtlLang = aOptions.nDefaultLanguage_CTL;
/*N*/ 	rAutoSpell = aOptions.bIsSpellAuto;
/*N*/ 	rHideAuto = aOptions.bIsSpellHideMarkings;
/*N*/ }

// static

// static


// static


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
