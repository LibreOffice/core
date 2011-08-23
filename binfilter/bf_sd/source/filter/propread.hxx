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

#ifndef _PROPREAD_HXX_
#define _PROPREAD_HXX_

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _SVSTOR_HXX
#include <bf_so3/svstor.hxx>
#endif

namespace binfilter {

// SummaryInformation
#define PID_TITLE				0x02
#define PID_SUBJECT 			0x03
#define PID_AUTHOR				0x04
#define PID_KEYWORDS			0x05
#define PID_COMMENTS			0x06
#define PID_TEMPLATE			0x07
#define PID_LASTAUTHOR			0x08
#define PID_REVNUMBER			0x09
#define PID_EDITTIME			0x0a
#define PID_LASTPRINTED_DTM 	0x0b
#define PID_CREATE_DTM			0x0c
#define PID_LASTSAVED_DTM		0x0d

// DocumentSummaryInformation
#define PID_CATEGORY			0x02
#define PID_PRESFORMAT			0x03
#define PID_BYTECOUNT			0x04
#define PID_LINECOUNT			0x05
#define PID_PARACOUNT			0x06
#define PID_SLIDECOUNT			0x07
#define PID_NOTECOUNT			0x08
#define PID_HIDDENCOUNT 		0x09
#define PID_MMCLIPCOUNT 		0x0a
#define PID_SCALE				0x0b
#define PID_HEADINGPAIR 		0x0c
#define PID_DOCPARTS			0x0d
#define PID_MANAGER 			0x0e
#define PID_COMPANY 			0x0f
#define PID_LINKSDIRTY			0x10

#define VT_EMPTY			0
#define VT_NULL 			1
#define VT_I2				2
#define VT_I4				3
#define VT_R4				4
#define VT_R8				5
#define VT_CY				6
#define VT_DATE 			7
#define VT_BSTR 			8
#define VT_UI4				9
#define VT_ERROR			10
#define VT_BOOL 			11
#define VT_VARIANT			12
#define VT_DECIMAL			14
#define VT_I1				16
#define VT_UI1				17
#define VT_UI2				18
#define VT_I8				20
#define VT_UI8				21
#define VT_INT				22
#define VT_UINT 			23
#define VT_LPSTR			30
#define VT_LPWSTR			31
#define VT_FILETIME 		64
#define VT_BLOB 			65
#define VT_STREAM			66
#define VT_STORAGE			67
#define VT_STREAMED_OBJECT	68
#define VT_STORED_OBJECT	69
#define VT_BLOB_OBJECT		70
#define VT_CF				71
#define VT_CLSID			72
#define VT_VECTOR			0x1000
#define VT_ARRAY			0x2000
#define VT_BYREF			0x4000
#define VT_TYPEMASK 		0xFFF

// ------------------------------------------------------------------------

class PropItem : public SvMemoryStream
{
        sal_uInt16		mnTextEnc;

    public :
                        PropItem(){};
        void			SetTextEncoding( sal_uInt16 nTextEnc ){ mnTextEnc = nTextEnc; };
        PropItem&		operator=( PropItem& rPropItem );
};

// ------------------------------------------------------------------------

class Dictionary : protected List
{
    friend class Section;

    public :
                    Dictionary(){};
                    ~Dictionary();
        Dictionary& operator=( Dictionary& rDictionary );
};

} //namespace binfilter
#endif

