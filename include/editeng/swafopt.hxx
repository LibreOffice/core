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

#ifndef INCLUDED_EDITENG_SWAFOPT_HXX
#define INCLUDED_EDITENG_SWAFOPT_HXX

#include <editeng/editengdllapi.h>
#include <o3tl/sorted_vector.hxx>
#include <rtl/ustring.hxx>
#include <vcl/font.hxx>

class SmartTagMgr;

namespace editeng {

class EDITENG_DLLPUBLIC IAutoCompleteString
{
private:
    OUString m_String;
public:
    explicit IAutoCompleteString(OUString const& rString) : m_String(rString) {}
    virtual ~IAutoCompleteString() {}
    OUString const& GetAutoCompleteString() const { return m_String; }
};

struct CompareAutoCompleteString
{
    bool operator()(IAutoCompleteString *const& lhs,
                    IAutoCompleteString *const& rhs) const
    {
        return lhs->GetAutoCompleteString().compareToIgnoreAsciiCase(
                rhs->GetAutoCompleteString()) < 0;
    }
};

class SortedAutoCompleteStrings
  : public o3tl::sorted_vector<IAutoCompleteString*, CompareAutoCompleteString>
{
public:
    ~SortedAutoCompleteStrings() { DeleteAndDestroyAll(); }
};

}

// Class of options for AutoFormat
struct EDITENG_DLLPUBLIC SvxSwAutoFmtFlags
{
    vcl::Font aBulletFont;
    vcl::Font aByInputBulletFont;
    /// only valid inside the Dialog!!!
    const editeng::SortedAutoCompleteStrings * m_pAutoCompleteList;
    SmartTagMgr* pSmartTagMgr;

    sal_Unicode cBullet;
    sal_Unicode cByInputBullet;

    sal_uInt16 nAutoCmpltWordLen, nAutoCmpltListLen;
    sal_uInt16 nAutoCmpltExpandKey;

    sal_uInt8 nRightMargin;

    bool bAutoCorrect : 1;
    bool bCptlSttSntnc : 1;
    bool bCptlSttWrd : 1;
    bool bChkFontAttr : 1;

    bool bChgUserColl : 1;
    bool bChgEnumNum : 1;

    bool bAFmtByInput : 1;
    bool bDelEmptyNode : 1;
    bool bSetNumRule : 1;

    bool bChgOrdinalNumber : 1;
    bool bChgToEnEmDash : 1;
    bool bAddNonBrkSpace : 1;
    bool bChgWeightUnderl : 1;
    bool bSetINetAttr : 1;

    bool bSetBorder : 1;
    bool bCreateTable : 1;
    bool bReplaceStyles : 1;
    bool bDummy : 1;

    bool bWithRedlining : 1;

    bool bRightMargin : 1;

    bool bAutoCompleteWords : 1;
    bool bAutoCmpltCollectWords : 1;
    bool bAutoCmpltEndless : 1;
// -- under NT here starts a new long
    bool bAutoCmpltAppendBlanc : 1;
    bool bAutoCmpltShowAsTip : 1;

    bool bAFmtDelSpacesAtSttEnd : 1;
    bool bAFmtDelSpacesBetweenLines : 1;
    bool bAFmtByInpDelSpacesAtSttEnd : 1;
    bool bAFmtByInpDelSpacesBetweenLines : 1;

    bool bAutoCmpltKeepList : 1;

    // some dummies for any new options
    bool bDummy6 : 1,
         bDummy7 : 1,
         bDummy8 : 1
         ;

    SvxSwAutoFmtFlags();
    SvxSwAutoFmtFlags( const SvxSwAutoFmtFlags& rAFFlags ) { *this = rAFFlags; }
    SvxSwAutoFmtFlags& operator=( const SvxSwAutoFmtFlags& );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
