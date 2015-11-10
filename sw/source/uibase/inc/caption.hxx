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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CAPTION_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CAPTION_HXX

#include <rtl/ustring.hxx>
#include <tools/globname.hxx>
#include <SwCapObjType.hxx>
#include "swdllapi.h"

class SW_DLLPUBLIC InsCaptionOpt
{
private:
    bool            bUseCaption;
    SwCapObjType    eObjType;
    SvGlobalName    aOleId;
    OUString        sCategory;
    sal_uInt16      nNumType;
    OUString        sNumberSeparator;
    OUString        sCaption;
    sal_uInt16      nPos;
    sal_uInt16      nLevel;
    OUString        sSeparator;
    OUString        sCharacterStyle;

    bool         bIgnoreSeqOpts;    // is not being saved
    bool         bCopyAttributes;   //          -""-

public:
    InsCaptionOpt(const SwCapObjType eType = FRAME_CAP, const SvGlobalName* pOleId = nullptr);
    InsCaptionOpt(const InsCaptionOpt&);
    ~InsCaptionOpt();

    inline bool&            UseCaption()                    { return bUseCaption; }
    inline bool             UseCaption() const              { return bUseCaption; }

    inline SwCapObjType     GetObjType() const              { return eObjType; }

    inline const SvGlobalName&  GetOleId() const                { return aOleId; }

    inline const OUString&  GetCategory() const             { return sCategory; }
    inline void             SetCategory(const OUString& rCat) { sCategory = rCat; }

    inline sal_uInt16           GetNumType() const              { return nNumType; }
    inline void             SetNumType(const sal_uInt16 nNT)    { nNumType = nNT; }

    const OUString&  GetNumSeparator() const { return sNumberSeparator; }
    void                    SetNumSeparator(const OUString& rSet) {sNumberSeparator = rSet;}

    inline const OUString&  GetCaption() const              { return sCaption; }
    inline void             SetCaption(const OUString& rCap)  { sCaption = rCap; }

    inline sal_uInt16           GetPos() const                  { return nPos; }
    inline void             SetPos(const sal_uInt16 nP)         { nPos = nP; }

    inline sal_uInt16           GetLevel() const                { return nLevel; }
    inline void             SetLevel(const sal_uInt16 nLvl)     { nLevel = nLvl; }

    inline const OUString&  GetSeparator() const                { return sSeparator; }
    inline void             SetSeparator(const OUString& rSep)    { sSeparator = rSep; }

    const OUString&         GetCharacterStyle() const { return sCharacterStyle; }
    void                    SetCharacterStyle(const OUString& rStyle)
                                    { sCharacterStyle = rStyle; }

    inline bool&            IgnoreSeqOpts()                 { return bIgnoreSeqOpts; }
    inline bool             IgnoreSeqOpts() const           { return bIgnoreSeqOpts; }

    inline bool&            CopyAttributes()                { return bCopyAttributes; }
    inline bool             CopyAttributes() const          { return bCopyAttributes; }

    bool                    operator==( const InsCaptionOpt& rOpt ) const;
    InsCaptionOpt&          operator= ( const InsCaptionOpt& rOpt );
    inline bool             operator< ( const InsCaptionOpt & rObj ) const
                                                { return aOleId < rObj.aOleId; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
