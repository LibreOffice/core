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
#include <propread.hxx>
#endif
#include "rtl/tencinfo.h"
#include "rtl/textenc.h"
namespace binfilter {

// ------------------------------------------------------------------------

PropItem& PropItem::operator=( PropItem& rPropItem )
{
    if ( this != &rPropItem )
    {
        Seek( STREAM_SEEK_TO_BEGIN );
        delete[] SwitchBuffer();

        mnTextEnc = rPropItem.mnTextEnc;
        sal_uInt32 nPos = rPropItem.Tell();
        rPropItem.Seek( STREAM_SEEK_TO_END );
        SvMemoryStream::Write( rPropItem.GetData(), rPropItem.Tell() );
        rPropItem.Seek( nPos );
    }
    return *this;
}

//	-----------------------------------------------------------------------

struct Dict
{
    sal_uInt32	mnId;
    String		aString;

            Dict( sal_uInt32 nId, String rString ) { mnId = nId; aString = rString; };
};

//	-----------------------------------------------------------------------

Dictionary::~Dictionary()
{
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        delete (Dict*)pPtr;
}

//	-----------------------------------------------------------------------

Dictionary& Dictionary::operator=( Dictionary& rDictionary )
{
    if ( this != &rDictionary )
    {
        void* pPtr; for ( pPtr = First(); pPtr; pPtr = Next() )
            delete (Dict*)pPtr;

        for ( pPtr = rDictionary.First(); pPtr; pPtr = rDictionary.Next() )
            Insert( new Dict( ((Dict*)pPtr)->mnId, ((Dict*)pPtr)->aString ), LIST_APPEND );
    }
    return *this;
}
}
