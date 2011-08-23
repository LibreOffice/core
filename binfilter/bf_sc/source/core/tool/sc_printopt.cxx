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


#include "printopt.hxx"
namespace binfilter {

using namespace utl;
using namespace rtl;
using namespace ::com::sun::star::uno;

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ ScPrintOptions::ScPrintOptions()
/*N*/ {
/*N*/ 	SetDefaults();
/*N*/ }

/*N*/ ScPrintOptions::ScPrintOptions( const ScPrintOptions& rCpy ) :
/*N*/ 	bSkipEmpty( rCpy.bSkipEmpty ),
/*N*/ 	bAllSheets( rCpy.bAllSheets )
/*N*/ {
/*N*/ }

/*N*/ ScPrintOptions::~ScPrintOptions()
/*N*/ {
/*N*/ }

/*N*/ void ScPrintOptions::SetDefaults()
/*N*/ {
/*N*/ 	bSkipEmpty = FALSE;
/*N*/ 	bAllSheets = TRUE;
/*N*/ }




// -----------------------------------------------------------------------








// -----------------------------------------------------------------------

#define CFGPATH_PRINT			"Office.Calc/Print"

#define SCPRINTOPT_EMPTYPAGES		0
#define SCPRINTOPT_ALLSHEETS		1
#define SCPRINTOPT_COUNT			2


/*N*/ ScPrintCfg::ScPrintCfg() :
/*N*/ 	ConfigItem( OUString::createFromAscii( CFGPATH_PRINT ) )
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 Sequence<OUString> aNames = GetPropertyNames();
/*N*/ }


void ScPrintCfg::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames )
{
}
void ScPrintCfg::Commit()
{
}




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
