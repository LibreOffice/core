/*************************************************************************
 *
 *  $RCSfile: xlstream.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:07:17 $
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

#ifndef SC_XLSTREAM_HXX
#define SC_XLSTREAM_HXX

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _SVXERR_HXX
#include <svx/svxerr.hxx>
#endif

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif

// Constants ==================================================================

const sal_uInt32 EXC_REC_SEEK_TO_BEGIN      = 0;
const sal_uInt32 EXC_REC_SEEK_TO_END        = SAL_MAX_UINT32;

const sal_uInt16 EXC_MAXRECSIZE_BIFF5       = 2080;
const sal_uInt16 EXC_MAXRECSIZE_BIFF8       = 8224;

const ErrCode EXC_ENCR_ERROR_WRONG_PASS     = ERRCODE_SVX_WRONGPASS;
const ErrCode EXC_ENCR_ERROR_UNSUPP_CRYPT   = ERRCODE_SVX_READ_FILTER_CRYPT;
const sal_uInt16 EXC_ENCR_BLOCKSIZE         = 1024;

const sal_uInt16 EXC_ID_UNKNOWN             = SAL_MAX_UINT16;
const sal_uInt16 EXC_ID_CONT                = 0x003C;

// Encryption/decryption ======================================================

/** Helper class for BIFF stream en-/decryption. */
class XclCryptoHelper : ScfNoInstance
{
public:
    /** Returns the fixed password for workbook protection. */
    static const ByteString GetBiff5WbProtPassword();

    /** Returns the fixed password for workbook protection. */
    static const String GetBiff8WbProtPassword();
};

// ============================================================================

#endif

