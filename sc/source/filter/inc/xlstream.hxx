/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlstream.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:04:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

const sal_Size EXC_REC_SEEK_TO_BEGIN        = 0;
const sal_Size EXC_REC_SEEK_TO_END          = static_cast< sal_Size >( -1 );

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

