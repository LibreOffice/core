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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_UINUMS_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_UINUMS_HXX

#include <numrule.hxx>
#include <swdllapi.h>

#include <memory>
#include <vector>

class SfxPoolItem;
class SwWrtShell;
class SvStream;

namespace sw { class StoredChapterNumberingRules; }

#define MAX_NUM_RULES 9

class SW_DLLPUBLIC SwNumRulesWithName final
{
    OUString maName;
    // the NumRule's formats _have_ to be independent of a document
    // (They should always be there!)
    class SAL_DLLPRIVATE SwNumFormatGlobal
    {
        friend class SwNumRulesWithName;
        SwNumFormat aFormat;
        OUString sCharFormatName;
        sal_uInt16 nCharPoolId;
        std::vector<std::unique_ptr<SfxPoolItem>> m_Items;

        SwNumFormatGlobal& operator=( const SwNumFormatGlobal& ) = delete;

    public:
        SwNumFormatGlobal( const SwNumFormat& rFormat );
        SwNumFormatGlobal( const SwNumFormatGlobal& );
        ~SwNumFormatGlobal();

        SwNumFormat MakeNumFormat(SwWrtShell& rSh) const;
    };

    std::unique_ptr<SwNumFormatGlobal> aFormats[ MAXLEVEL ];

    friend class sw::StoredChapterNumberingRules;
    friend class SwChapterNumRules;
    void SetName(const OUString& rSet) {maName = rSet;}
    void SetNumFormat(size_t, SwNumFormat const&, OUString const&);
    SwNumRulesWithName() = default;

public:
    SwNumRulesWithName(const SwNumRule &, const OUString &);
    SwNumRulesWithName( const SwNumRulesWithName & );
    ~SwNumRulesWithName();

    SwNumRulesWithName &operator=(const SwNumRulesWithName &);

    const OUString& GetName() const               { return maName; }
    void ResetNumRule(SwWrtShell& rSh, SwNumRule& ) const;

    void GetNumFormat(size_t, SwNumFormat const*&, OUString const*&) const;
};

class SW_DLLPUBLIC SwChapterNumRules final
{
public:
    enum { nMaxRules = MAX_NUM_RULES };         // currently 9 defined forms
private:
    std::unique_ptr<SwNumRulesWithName> pNumRules[ MAX_NUM_RULES ];

    void Init();
    void Save();

public:
    SwChapterNumRules();
    ~SwChapterNumRules();

    inline const SwNumRulesWithName*    GetRules(sal_uInt16 nIdx) const;
    void CreateEmptyNumRule(sal_uInt16 nIdx); // for import
    void ApplyNumRules( const SwNumRulesWithName &rCopy,
                        sal_uInt16 nIdx);
};

inline const SwNumRulesWithName *SwChapterNumRules::GetRules(sal_uInt16 nIdx) const
{
    assert(nIdx < nMaxRules);
    return pNumRules[nIdx].get();
}


namespace sw
{

void ExportStoredChapterNumberingRules(
        SwChapterNumRules & rRules, SvStream & rStream, OUString const&);
void ImportStoredChapterNumberingRules(
        SwChapterNumRules & rRules, SvStream & rStream, OUString const&);

} // namespace sw

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
