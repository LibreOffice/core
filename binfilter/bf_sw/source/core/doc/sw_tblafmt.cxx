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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER



#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _ZFORMAT_HXX //autogen
#include <bf_svtools/zformat.hxx>
#endif
#ifndef _SVX_DIALOGS_HRC
#include <bf_svx/dialogs.hrc>
#endif

#define READ_OLDVERS		// erstmal noch alte Versionen lesen

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _TBLAFMT_HXX
#include <tblafmt.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _ERRHDL_HXX //autogen
#include <errhdl.hxx>
#endif
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

// bis SO5PF
const USHORT AUTOFORMAT_ID_X		= 9501;
const USHORT AUTOFORMAT_ID_358		= 9601;
const USHORT AUTOFORMAT_DATA_ID_X	= 9502;

// ab SO5
//! in nachfolgenden Versionen muss der Betrag dieser IDs groesser sein
const USHORT AUTOFORMAT_ID_504		= 9801;
const USHORT AUTOFORMAT_DATA_ID_504	= 9802;

const USHORT AUTOFORMAT_ID_552		= 9901;
const USHORT AUTOFORMAT_DATA_ID_552	= 9902;

// --- from 641 on: CJK and CTL font settings
const USHORT AUTOFORMAT_ID_641      = 10001;
const USHORT AUTOFORMAT_DATA_ID_641 = 10002;

// current version
const USHORT AUTOFORMAT_ID          = AUTOFORMAT_ID_641;
const USHORT AUTOFORMAT_DATA_ID     = AUTOFORMAT_DATA_ID_641;


#ifdef READ_OLDVERS
const USHORT AUTOFORMAT_OLD_ID		= 8201;
const USHORT AUTOFORMAT_OLD_ID1		= 8301;
const USHORT AUTOFORMAT_OLD_DATA_ID	= 8202;
#endif

}
