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
#ifndef _MSOLEEXP_HXX
#define _MSOLEEXP_HXX

#include <com/sun/star/uno/Reference.h>
#include "filter/msfilter/msfilterdllapi.h"

// for the CreateSdrOLEFromStorage we need the information, how we handle
// convert able OLE-Objects - this ist stored in
#define OLE_STARMATH_2_MATHTYPE 			0x0001
#define OLE_STARWRITER_2_WINWORD			0x0002
#define OLE_STARCALC_2_EXCEL				0x0004
#define OLE_STARIMPRESS_2_POWERPOINT		0x0008

class SotStorage;

#include <svtools/embedhlp.hxx>

class MSFILTER_DLLPUBLIC SvxMSExportOLEObjects
{
    UINT32 nConvertFlags;
public:
    SvxMSExportOLEObjects( UINT32 nCnvrtFlgs ) : nConvertFlags(nCnvrtFlgs) {}

    void SetFlags( UINT32 n ) 		{ nConvertFlags = n; }
    UINT32 GetFlags() const 		{ return nConvertFlags; }

    void ExportOLEObject( svt::EmbeddedObjectRef& rObj, SotStorage& rDestStg );
    void ExportOLEObject( const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject>& rObj, SotStorage& rDestStg );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
