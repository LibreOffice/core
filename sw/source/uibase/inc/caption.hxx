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
#include <swdllapi.h>

class SW_DLLPUBLIC InsCaptionOpt
{
private:
    bool            m_bUseCaption;
    SwCapObjType    m_eObjType;
    SvGlobalName    m_aOleId;
    OUString        m_sCategory;
    sal_uInt16      m_nNumType;
    OUString        m_sNumberSeparator;
    OUString        m_sCaption;
    sal_uInt16      m_nPos;
    sal_uInt16      m_nLevel;
    OUString        m_sSeparator;
    OUString        m_sCharacterStyle;

    bool         m_bIgnoreSeqOpts;    // is not being saved
    bool         m_bCopyAttributes;   //          -""-

public:
    InsCaptionOpt(const SwCapObjType eType = FRAME_CAP, const SvGlobalName* pOleId = nullptr);

    bool&            UseCaption()                    { return m_bUseCaption; }
    bool             UseCaption() const              { return m_bUseCaption; }

    SwCapObjType     GetObjType() const              { return m_eObjType; }

    const SvGlobalName&  GetOleId() const                { return m_aOleId; }

    const OUString&  GetCategory() const             { return m_sCategory; }
    void             SetCategory(const OUString& rCat) { m_sCategory = rCat; }

    sal_uInt16           GetNumType() const              { return m_nNumType; }
    void             SetNumType(const sal_uInt16 nNT)    { m_nNumType = nNT; }

    const OUString&  GetNumSeparator() const { return m_sNumberSeparator; }
    void                    SetNumSeparator(const OUString& rSet) {m_sNumberSeparator = rSet;}

    const OUString&  GetCaption() const              { return m_sCaption; }
    void             SetCaption(const OUString& rCap)  { m_sCaption = rCap; }

    sal_uInt16           GetPos() const                  { return m_nPos; }
    void             SetPos(const sal_uInt16 nP)         { m_nPos = nP; }

    sal_uInt16           GetLevel() const                { return m_nLevel; }
    void             SetLevel(const sal_uInt16 nLvl)     { m_nLevel = nLvl; }

    const OUString&  GetSeparator() const                { return m_sSeparator; }
    void             SetSeparator(const OUString& rSep)    { m_sSeparator = rSep; }

    const OUString&         GetCharacterStyle() const { return m_sCharacterStyle; }
    void                    SetCharacterStyle(const OUString& rStyle)
                                    { m_sCharacterStyle = rStyle; }

    bool&            IgnoreSeqOpts()                 { return m_bIgnoreSeqOpts; }
    bool             IgnoreSeqOpts() const           { return m_bIgnoreSeqOpts; }

    bool&            CopyAttributes()                { return m_bCopyAttributes; }
    bool             CopyAttributes() const          { return m_bCopyAttributes; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
