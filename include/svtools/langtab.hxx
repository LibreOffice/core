/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _SVTOOLS_LANGTAB_HXX_
#define _SVTOOLS_LANGTAB_HXX_


#include <tools/resary.hxx>
#include <i18nlangtag/lang.h>
#include <svtools/svtdllapi.h>

//========================================================================
//  class SvtLanguageTable
//========================================================================

class SVT_DLLPUBLIC SvtLanguageTable : public ResStringArray
{
public:
    SvtLanguageTable();
    ~SvtLanguageTable();

    /**
        @param bUserInterfaceSelection
            If TRUE, don't replace an UI-only locale. Only use for
                     Tools->Options->LanguageSettings->UserInterface listbox.
            If FALSE, do replace.
     */
    const OUString GetString( const LanguageType eType, bool bUserInterfaceSelection = false ) const;
    LanguageType    GetType( const OUString& rStr ) const;

    sal_uInt32      GetEntryCount() const;
    LanguageType    GetTypeAtIndex( sal_uInt32 nIndex ) const;
    static OUString   GetLanguageString( const LanguageType eType );
};

// Add LRE or RLE embedding characters to the string based on the
// String content (see #i78466#, #i32179#)
SVT_DLLPUBLIC const OUString ApplyLreOrRleEmbedding( const OUString &rText );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
