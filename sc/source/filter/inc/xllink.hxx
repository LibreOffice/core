/*************************************************************************
 *
 *  $RCSfile: xllink.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:42:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// ============================================================================

#ifndef SC_XLLINK_HXX
#define SC_XLLINK_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif


// Constants and Enumerations =================================================

// (0x0017) EXTERNSHEET -------------------------------------------------------

const sal_uInt16 EXC_ID_EXTERNSHEET         = 0x0017;
const sal_uInt16 EXC_EXTSH_EXTERNAL           = 0xFFFE;     /// Addins, DDE, OLE.
const sal_uInt16 EXC_EXTSH_DELETED            = 0xFFFF;     /// Deleted 3D reference.


// (0x0018, 0x0218) NAME ------------------------------------------------------

const sal_uInt16 EXC_ID_NAME                = 0x0018;
const sal_uInt16 EXC_ID34_NAME              = 0x0218;

// flags
const sal_uInt16 EXC_NAME_HIDDEN            = 0x0001;
const sal_uInt16 EXC_NAME_FUNC              = 0x0002;
const sal_uInt16 EXC_NAME_VB                = 0x0004;
const sal_uInt16 EXC_NAME_PROC              = 0x0008;
const sal_uInt16 EXC_NAME_CALCEXP           = 0x0010;
const sal_uInt16 EXC_NAME_BUILTIN           = 0x0020;
const sal_uInt16 EXC_NAME_FGROUPMASK        = 0x0FC0;
const sal_uInt16 EXC_NAME_BIG               = 0x1000;

const sal_uInt8 EXC_NAME2_FUNC              = 0x02;     /// BIFF2 function/command flag.

const sal_uInt16 EXC_NAME_GLOBAL            = 0;        /// 0 = Globally defined name.

// codes for built-in names
const sal_Unicode EXC_BUILTIN_CONSOLIDATEAREA   = '\x00';
const sal_Unicode EXC_BUILTIN_AUTOOPEN          = '\x01';
const sal_Unicode EXC_BUILTIN_AUTOCLOSE         = '\x02';
const sal_Unicode EXC_BUILTIN_EXTRACT           = '\x03';
const sal_Unicode EXC_BUILTIN_DATABASE          = '\x04';
const sal_Unicode EXC_BUILTIN_CRITERIA          = '\x05';
const sal_Unicode EXC_BUILTIN_PRINTAREA         = '\x06';
const sal_Unicode EXC_BUILTIN_PRINTTITLES       = '\x07';
const sal_Unicode EXC_BUILTIN_RECORDER          = '\x08';
const sal_Unicode EXC_BUILTIN_DATAFORM          = '\x09';
const sal_Unicode EXC_BUILTIN_AUTOACTIVATE      = '\x0A';
const sal_Unicode EXC_BUILTIN_AUTODEACTIVATE    = '\x0B';
const sal_Unicode EXC_BUILTIN_SHEETTITLE        = '\x0C';
const sal_Unicode EXC_BUILTIN_AUTOFILTER        = '\x0D';
const sal_Unicode EXC_BUILTIN_UNKNOWN           = '\x0E';


// (0x0023) EXTERNNAME --------------------------------------------------------

const sal_uInt16 EXC_ID_EXTERNNAME          = 0x0023;

const sal_uInt16 EXC_EXTN_BUILTIN           = 0x0001;
const sal_uInt16 EXC_EXTN_OLE               = 0x0010;
const sal_uInt16 EXC_EXTN_OLE_OR_DDE        = 0xFFFE;

const sal_uInt16 EXC_EXTN_EXPDDE_STDDOC     = 0x7FEA;  /// for export
const sal_uInt16 EXC_EXTN_EXPDDE            = 0x7FE2;  /// for export


// (0x0059, 0x005A) XCT, CRN --------------------------------------------------

const sal_uInt16 EXC_ID_XCT                 = 0x0059;
const sal_uInt16 EXC_ID_CRN                 = 0x005A;


// (0x013D) TABID -------------------------------------------------------------

const sal_uInt16 EXC_ID_TABID               = 0x013D;


// (0x01AE) SUPBOOK -----------------------------------------------------------

const sal_uInt16 EXC_ID_SUPBOOK             = 0x01AE;

const sal_uInt16 EXC_SUPB_SELF              = 0x0401;
const sal_uInt16 EXC_SUPB_ADDIN             = 0x3A01;


// ============================================================================

#endif

