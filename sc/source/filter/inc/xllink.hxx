/*************************************************************************
 *
 *  $RCSfile: xllink.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-14 12:13:03 $
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

#ifndef SC_XLLINK_HXX
#define SC_XLLINK_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

// Constants and Enumerations =================================================

const sal_uInt16 EXC_TAB_EXTERNAL           = 0xFFFE;     /// Special sheet index for external links.
const sal_uInt16 EXC_TAB_DELETED            = 0xFFFF;     /// Deleted sheet in a 3D reference.

// (0x0016) EXTERNCOUNT -------------------------------------------------------

const sal_uInt16 EXC_ID_EXTERNCOUNT         = 0x0016;

// (0x0017) EXTERNSHEET -------------------------------------------------------

const sal_uInt16 EXC_ID_EXTERNSHEET         = 0x0017;

const sal_Unicode EXC_EXTSH_URL             = '\x01';
const sal_Unicode EXC_EXTSH_OWNTAB          = '\x02';
const sal_Unicode EXC_EXTSH_TABNAME         = '\x03';
const sal_Unicode EXC_EXTSH_OWNDOC          = '\x04';
const sal_Unicode EXC_EXTSH_ADDIN           = '\x3A';

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

/** This enumeration specifies the type of a SUPBOOK record. */
enum XclSupbookType
{
    EXC_SBTYPE_UNKNOWN,     /// unknown SUPBOOK record type.
    EXC_SBTYPE_SELF,        /// SUPBOOK is used for internal references.
    EXC_SBTYPE_EXTERN,      /// SUPBOOK is used for external references.
    EXC_SBTYPE_ADDIN,       /// SUPBOOK contains add-in functions.
    EXC_SBTYPE_SPECIAL      /// SUPBOOK is used for DDE or OLE links.
};

// ============================================================================

#endif

