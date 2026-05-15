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

#include <sal/config.h>

#include <editeng/editengdllapi.h>
#include <o3tl/sorted_vector.hxx>
#include <rtl/ustring.hxx>
#include <utility>
#include <vcl/font.hxx>
#include <vcl/vclenum.hxx>

class SmartTagMgr;

namespace editeng {

class EDITENG_DLLPUBLIC IAutoCompleteString
{
private:
    OUString m_String;
public:
    explicit IAutoCompleteString(OUString aString) : m_String(std::move(aString)) {}
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
    bool owning_;

    SortedAutoCompleteStrings& operator =(SortedAutoCompleteStrings const &) = delete;

    // For createNonOwningCopy only:
    SortedAutoCompleteStrings(SortedAutoCompleteStrings const & other):
        sorted_vector(other), owning_(false) {}

public:
    SortedAutoCompleteStrings(): owning_(true) {}

    ~SortedAutoCompleteStrings() { if (owning_) DeleteAndDestroyAll(); }

    SortedAutoCompleteStrings createNonOwningCopy() const { return *this; }
};

} // namespace editeng

// Class of options for AutoFormat
struct EDITENG_DLLPUBLIC SvxSwAutoFormatFlags
{
    vcl::Font aBulletFont;
    vcl::Font aByInputBulletFont;
    /// only valid inside the Dialog!!!
    const editeng::SortedAutoCompleteStrings * m_pAutoCompleteList;
    SmartTagMgr* pSmartTagMgr;

    sal_UCS4 cBullet;
    sal_UCS4 cByInputBullet;

    sal_uInt32 nAutoCmpltListLen;
    sal_uInt16 nAutoCmpltWordLen;
    sal_uInt16 nAutoCmpltExpandKey;

    sal_uInt8 nRightMargin;

#define SVX_SW_FLAG_ENTRIES(prefix, suffix)                                                        \
    prefix bAutoCorrect suffix;                                                                    \
    prefix bCapitalStartSentence suffix;                                                           \
    prefix bCapitalStartWord suffix;                                                               \
                                                                                                   \
    prefix bChgUserColl suffix;                                                                    \
    prefix bChgEnumNum suffix;                                                                     \
                                                                                                   \
    prefix bAFormatByInput suffix;                                                                 \
    prefix bDelEmptyNode suffix;                                                                   \
    prefix bSetNumRule suffix;                                                                     \
    prefix bSetNumRuleAfterSpace suffix;                                                           \
                                                                                                   \
    prefix bChgOrdinalNumber suffix;                                                               \
    prefix bChgToEnEmDash suffix;                                                                  \
    prefix bAddNonBrkSpace suffix;                                                                 \
    prefix bTransliterateRTL suffix;                                                               \
    prefix bChgAngleQuotes suffix;                                                                 \
    prefix bEsperantoHats suffix;                                                                  \
    prefix bChgWeightUnderl suffix;                                                                \
    prefix bSetINetAttr suffix;                                                                    \
    prefix bSetDOIAttr suffix;                                                                     \
                                                                                                   \
    prefix bSetBorder suffix;                                                                      \
    prefix bCreateTable suffix;                                                                    \
    prefix bReplaceStyles suffix;                                                                  \
    prefix bReplaceStylesByInput suffix;                                                           \
                                                                                                   \
    prefix bWithRedlining suffix;                                                                  \
                                                                                                   \
    prefix bRightMargin suffix;                                                                    \
                                                                                                   \
    prefix bAutoCompleteWords suffix;                                                              \
    prefix bAutoCmpltCollectWords suffix;                                                          \
    prefix bAutoCmpltEndless suffix;                                                               \
    /* -- under NT here starts a new long */                                                       \
    prefix bAutoCmpltAppendBlank suffix;                                                           \
    prefix bAutoCmpltShowAsTip suffix;                                                             \
                                                                                                   \
    prefix bAFormatDelSpacesAtSttEnd suffix;                                                       \
    prefix bAFormatDelSpacesBetweenLines suffix;                                                   \
    prefix bAFormatByInpDelSpacesAtSttEnd suffix;                                                  \
    prefix bAFormatByInpDelSpacesBetweenLines suffix;                                              \
                                                                                                   \
    prefix bAutoCmpltKeepList suffix;

    SVX_SW_FLAG_ENTRIES(bool, : 1)

    SvxSwAutoFormatFlags();
    void resetAllFlags();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
