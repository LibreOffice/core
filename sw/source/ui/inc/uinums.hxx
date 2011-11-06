/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
