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
#ifndef _UINUMS_HXX
#define _UINUMS_HXX

#include <svl/svarray.hxx>
#include <numrule.hxx>
#include "swdllapi.h"

class SfxPoolItem;
class SwWrtShell;
class SvStream;

#define MAX_NUM_RULES 9

typedef SfxPoolItem* SfxPoolItemPtr;
SV_DECL_PTRARR_DEL( _SwNumFmtsAttrs, SfxPoolItemPtr, 5,0 )

//------------------------------------------------------------------------
class SW_DLLPUBLIC SwNumRulesWithName
{
    String aName;
    // die Formate der NumRule muessen! unabhaengig von einem Document sein
    // (Sie sollen immer vorhanden sein!)
    class SW_DLLPRIVATE _SwNumFmtGlobal
    {
        SwNumFmt aFmt;
        String sCharFmtName;
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
    void SetName(const String& rSet) {aName = rSet;}

public:
    SwNumRulesWithName(const SwNumRule &, const String &);
    SwNumRulesWithName( const SwNumRulesWithName & );
    SwNumRulesWithName(SvStream &, sal_uInt16 nVersion);
    ~SwNumRulesWithName();

    const SwNumRulesWithName &operator=(const SwNumRulesWithName &);

    const String& GetName() const               { return aName; }
    void MakeNumRule( SwWrtShell& rSh, SwNumRule& rChg ) const;

    void Store( SvStream& );
};
/********************************************************************

********************************************************************/
class SwBaseNumRules
{
public:
    enum { nMaxRules = MAX_NUM_RULES };         // zur Zeit 9 definierte Forms
protected:
    SwNumRulesWithName  *pNumRules[ MAX_NUM_RULES ];
    String              sFileName;
    sal_uInt16              nVersion;
     sal_Bool               bModified;

    virtual int         Load(SvStream&);
    virtual sal_Bool        Store(SvStream&);

    void                Init();

public:
    SwBaseNumRules(const String& rFileName);
    virtual ~SwBaseNumRules();

    inline const SwNumRulesWithName*    GetRules(sal_uInt16 nIdx) const;
    virtual void                        ApplyNumRules(
                                                const SwNumRulesWithName &rCopy,
                                                sal_uInt16 nIdx);

};

/********************************************************************

********************************************************************/
class SwChapterNumRules : public SwBaseNumRules
{

public:
    SwChapterNumRules();
    virtual ~SwChapterNumRules();

    virtual void        ApplyNumRules(  const SwNumRulesWithName &rCopy,
                                            sal_uInt16 nIdx);
};

// INLINE METHODE --------------------------------------------------------
inline const SwNumRulesWithName *SwBaseNumRules::GetRules(sal_uInt16 nIdx) const
{
    ASSERT(nIdx < nMaxRules, Array der NumRules ueberindiziert.);
    return pNumRules[nIdx];
}

#endif
