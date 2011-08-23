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

#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGS
#include <bf_svtools/svstdarr.hxx>
#ifndef _SW_XMLSECTIONLIST_HXX
#include <SwXMLSectionList.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <bf_xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <bf_xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <bf_xmloff/xmlnmspe.hxx>
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::binfilter::xmloff::token;
#include <tools/debug.hxx>  //STRIP001 
namespace binfilter {
sal_Char __READONLY_DATA sXML_np__block_list[] = "_block-list";

// #110680#
/*N*/ SwXMLSectionList::SwXMLSectionList(
/*N*/ 	const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
/*N*/ 	SvStrings & rNewSectionList)
/*N*/ :	SvXMLImport( xServiceFactory ),
/*N*/ 	rSectionList ( rNewSectionList )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 GetNamespaceMap().Add( OUString ( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__block_list ) ),
/*N*/ }






}
