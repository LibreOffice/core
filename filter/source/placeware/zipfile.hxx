/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zipfile.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:54:05 $
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

/* Local file head offsets.  The header is followed by the filename and
   possibly the extra field. */
#define zf_lfhSig       0
#define zf_lfhExVers        4
#define zf_lfhFlags     6
#define zf_lfhComp      8
#define zf_lfhModTime       10
#define zf_lfhModDate       12
#define zf_lfhCRC       14
#define zf_lfhCompSize      18
#define zf_lfhUncompSize    22
#define zf_lfhNameLen       26
#define zf_lfhExtraLen      28
#define zf_lfhSIZE      30 /* total size of header */

/* We don't write data descriptors. */

/* Central dir file header offsets.  Each entry is followed by the filename,
   optional extra field, and file comment. */
#define zf_cdhSig       0
#define zf_cdhVers      4
#define zf_cdhExVers        6
#define zf_cdhFlags     8
#define zf_cdhComp      10
#define zf_cdhTime      12
#define zf_cdhDate      14
#define zf_cdhCRC       16
#define zf_cdhCompSize      20
#define zf_cdhUncompSize    24
#define zf_cdhNameLen       28
#define zf_cdhExtraLen      30
#define zf_cdhCommentLen    32
#define zf_cdhDiskNum       34
#define zf_cdhIFAttrs       36
#define zf_cdhEFAttrs       38
#define zf_cdhLHOffset      42
#define zf_cdhSIZE      46 /* total size of header */

/* End of central dir record offsets.  It is followed by the zipfile
   comment. */
#define zf_ecdSig       0
#define zf_ecdDiskNum       4
#define zf_ecdDirDiskNum    6
#define zf_ecdNumEntries    8
#define zf_ecdTotalEntries  10
#define zf_ecdDirSize       12
#define zf_ecdDirOffset     16
#define zf_ecdCommentLen    20
#define zf_ecdSIZE      22 /* total size */

/* Magic constants to put in these structures. */
#define zf_LFHSIGValue      0x04034b50
#define zf_CDHSIGValue      0x02014b50
#define zf_ECDSIGValue      0x06054b50

/* OS values for upper byte of version field. */
#define zf_osUnix       3

/* Encode a major,minor version in a byte. */
#define zf_Vers(major,minor)    ((major) * 10 + (minor))

/* Compression values. */
#define zf_compNone     0

