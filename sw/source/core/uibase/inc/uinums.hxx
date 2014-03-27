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
#ifndef INCLUDED_SW_SOURCE_UI_INC_UINUMS_HXX
#define INCLUDED_SW_SOURCE_UI_INC_UINUMS_HXX

#include <numrule.hxx>
#include "swdllapi.h"
#include <boost/ptr_container/ptr_vector.hpp>

class SfxPoolItem;
class SwWrtShell;
class SvStream;

#define MAX_NUM_RULES 9

typedef boost::ptr_vector<SfxPoolItem> _SwNumFmtsAttrs;

class SW_DLLPUBLIC SwNumRulesWithName
{
    OUString maName;
    // the NumRule's formats _have_ to be independent of a document
    // (They should always be there!)
    class SAL_DLLPRIVATE _SwNumFmtGlobal
    {
        SwNumFmt aFmt;
        OUString sCharFmtName;
        sal_uInt16 nCharPoolId;
        _SwNumFmtsAttrs aItems;

        _SwNumFmtGlobal& operator=( const _SwNumFmtGlobal& );

    public:
        _SwNumFmtGlobal( const SwNumFmt& rFmt );
        _SwNumFmtGlobal( const _SwNumFmtGlobal& );
        _SwNumFmtGlobal( SvStream&, sal_uInt16 nVersion );
        ~_SwNumFmtGlobal();

        void Store( SvStream& );
        void ChgNumFmt( SwWrtShell& rSh, SwNumFmt& rChg ) const;
    };

    _SwNumFmtGlobal* aFmts[ MAXLEVEL ];

protected:
    void SetName(const OUString& rSet) {maName = rSet;}

public:
    SwNumRulesWithName(const SwNumRule &, const OUString &);
    SwNumRulesWithName( const SwNumRulesWithName & );
    SwNumRulesWithName(SvStream &, sal_uInt16 nVersion);
    ~SwNumRulesWithName();

    const SwNumRulesWithName &operator=(const SwNumRulesWithName &);

    const OUString& GetName() const               { return maName; }
    void MakeNumRule( SwWrtShell& rSh, SwNumRule& rChg ) const;

    void Store( SvStream& );
};

class SwBaseNumRules
{
public:
    enum { nMaxRules = MAX_NUM_RULES };         // currently 9 defined forms
protected:
    SwNumRulesWithName   *pNumRules[ MAX_NUM_RULES ];
    OUString              sFileName;
    sal_uInt16            nVersion;
    sal_Bool              bModified;

    virtual int         Load(SvStream&);
    virtual sal_Bool        Store(SvStream&);

    void                Init();

public:
    SwBaseNumRules(const OUString& rFileName);
    virtual ~SwBaseNumRules();

    inline const SwNumRulesWithName*    GetRules(sal_uInt16 nIdx) const;
    virtual void                        ApplyNumRules(
                                                const SwNumRulesWithName &rCopy,
                                                sal_uInt16 nIdx);

};

class SwChapterNumRules : public SwBaseNumRules
{

public:
    SwChapterNumRules();
    virtual ~SwChapterNumRules();

    virtual void        ApplyNumRules(  const SwNumRulesWithName &rCopy,
                                            sal_uInt16 nIdx) SAL_OVERRIDE;
};

// INLINE METHODE --------------------------------------------------------
inline const SwNumRulesWithName *SwBaseNumRules::GetRules(sal_uInt16 nIdx) const
{
    OSL_ENSURE(nIdx < nMaxRules, "Array der NumRules ueberindiziert.");
    return pNumRules[nIdx];
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
