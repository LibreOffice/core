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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <bf_svtools/zforlist.hxx>

#include <tools/debug.hxx>
#include "misccfg.hxx"
namespace binfilter {

#define DEFAULT_TAB 2000

#define DEF_INCH	2540L
#define DEF_RELTWIP	1440L

using namespace rtl;
using namespace ::com::sun::star::uno;

#define C2U(cChar) OUString::createFromAscii(cChar)
/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/
/*N*/ SfxMiscCfg::SfxMiscCfg() :
/*N*/ 	ConfigItem(C2U("Office.Common") ),
/*N*/ 	nYear2000( SvNumberFormatter::GetYear2000Default() ),
/*N*/ 	bNotFound (FALSE),
/*N*/ 	bPaperSize(FALSE),
/*N*/ 	bPaperOrientation (FALSE)
/*N*/ {
/*N*/ 	Load();
/*N*/ }
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SfxMiscCfg::~SfxMiscCfg()
/*N*/ {
/*N*/ }
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ const Sequence<OUString>& SfxMiscCfg::GetPropertyNames()
/*N*/ {
/*N*/ 	static Sequence<OUString> aNames;
/*N*/ 	if(!aNames.getLength())
/*N*/ 	{
/*N*/ 		static const char* aPropNames[] =
/*N*/ 		{
/*N*/    			"Print/Warning/PaperSize",				//  0
/*N*/    			"Print/Warning/PaperOrientation",		//  1
/*N*/    			"Print/Warning/NotFound",				//  2
/*N*/ 			"DateFormat/TwoDigitYear",            	//  3
/*N*/ 		};
/*N*/         const int nCount = 4;
/*N*/ 		aNames.realloc(nCount);
/*N*/ 		OUString* pNames = aNames.getArray();
/*N*/ 		for(int i = 0; i < nCount; i++)
/*N*/ 			pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/ 	}
/*N*/ 	return aNames;
/*N*/ }
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void SfxMiscCfg::Load()
/*N*/ {
/*N*/ 	const Sequence<OUString>& aNames = GetPropertyNames();
/*N*/ 	Sequence<Any> aValues = GetProperties(aNames);
/*N*/ 	EnableNotification(aNames);
/*N*/ 	const Any* pValues = aValues.getConstArray();
/*N*/ 	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*N*/ 	if(aValues.getLength() == aNames.getLength())
/*N*/ 	{
/*N*/ 		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*N*/ 		{
/*N*/ 			if(pValues[nProp].hasValue())
/*N*/ 			{
/*N*/ 				switch(nProp)
/*N*/ 				{
/*N*/ 					case  0: bPaperSize 	   = *(sal_Bool*)pValues[nProp].getValue(); break;		//"Print/Warning/PaperSize",
/*N*/                     case  1: bPaperOrientation = *(sal_Bool*)pValues[nProp].getValue();  break;     //"Print/Warning/PaperOrientation",
/*N*/                     case  2: bNotFound         = *(sal_Bool*)pValues[nProp].getValue()  ;  break;   //"Print/Warning/NotFound",
/*N*/ 					case  3: pValues[nProp] >>= nYear2000;break;                                    //"DateFormat/TwoDigitYear",
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
/*?*/ void SfxMiscCfg::Notify( const ::com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames)
/*?*/ {
/*?*/ 	Load();
/*?*/ }
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
/*?*/ void SfxMiscCfg::Commit()
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
