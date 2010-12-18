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
#ifndef _SVTOOLS_LANGTAB_HXX_
#define _SVTOOLS_LANGTAB_HXX_


#include <tools/resary.hxx>
#include <i18npool/lang.h>
#include <svtools/svtdllapi.h>

//========================================================================
//  class SvtLanguageTable
//========================================================================

class SVT_DLLPUBLIC SvtLanguageTable : public ResStringArray
{
public:
    SvtLanguageTable();
    ~SvtLanguageTable();

    const String&   GetString( const LanguageType eType ) const;
    LanguageType    GetType( const String& rStr ) const;

    sal_uInt32      GetEntryCount() const;
    LanguageType    GetTypeAtIndex( sal_uInt32 nIndex ) const;
    static String   GetLanguageString( const LanguageType eType );
};

// Add LRE or RLE embedding characters to the string based on the
// String content (see #i78466#, #i32179#)
SVT_DLLPUBLIC const String ApplyLreOrRleEmbedding( const String &rText );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
