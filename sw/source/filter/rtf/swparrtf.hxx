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

#ifndef _SWPARRTF_HXX
#define _SWPARRTF_HXX

#include <deque>
#include <vector>
#include <map>

#include <editeng/svxrtf.hxx>
#include <editeng/numitem.hxx>
#include <editeng/boxitem.hxx>
#include <redline.hxx>

#include <fltshell.hxx>         // fuer den Attribut Stack
#include <ndindex.hxx>
#include "../inc/msfilter.hxx"
#include <svx/svdobj.hxx>


extern void GetLineIndex(SvxBoxItem &rBox, short nLineThickness, short nSpace, sal_uInt8 nCol, short nIdx,
    sal_uInt16 nOOIndex, sal_uInt16 nWWIndex, short *pSize);

class Font;
class Graphic;

class SwNoTxtNode;
class Size;
class SwPaM;
class SwDoc;
class SwPageDesc;
class SwTxtFmtColl;
class SwTableNode;
class SwCharFmt;
class SwNumRule;
class SvxFontItem;
class SwRelNumRuleSpaces;
class SwNodeNum;
class SwTxtNode;
struct SvxRTFPictureType;

class SwNodeIdx : public SvxNodeIdx
{
    SwNodeIndex aIdx;
public:
    SwNodeIdx( const SwNodeIndex& rIdx ) : aIdx( rIdx ) {}
    virtual sal_uLong   GetIdx() const;
    virtual SvxNodeIdx* Clone() const;
};

class BookmarkPosition
{
public:
    SwNodeIndex maMkNode;
    xub_StrLen mnMkCntnt;
    BookmarkPosition(const SwPaM &rPaM);
    BookmarkPosition(const BookmarkPosition &rEntry);

    bool operator==(const BookmarkPosition);
private:
    //No assignment
    BookmarkPosition& operator=(const BookmarkPosition&);

};

class SwxPosition : public SvxPosition
{
    SwPaM* pPam;
public:
    SwxPosition( SwPaM* pCrsr ) : pPam( pCrsr ) {}

    virtual sal_uLong   GetNodeIdx() const;
    virtual xub_StrLen GetCntIdx() const;

    // erzeuge von sich selbst eine Kopie
    virtual SvxPosition* Clone() const;
    // erzeuge vom NodeIndex eine Kopie
    virtual SvxNodeIdx* MakeNodeIdx() const;
};


// zum zwischenspeichern der Flys:
struct SwFlySave
{
    SfxItemSet  aFlySet;
    SwNodeIndex nSttNd, nEndNd;
    xub_StrLen  nEndCnt;
    SwTwips     nPageWidth;
    sal_uInt16      nDropLines, nDropAnchor;

    SwFlySave( const SwPaM& rPam, SfxItemSet& rSet );
    int IsEqualFly( const SwPaM& rPos, SfxItemSet& rSet );
    void SetFlySize( const SwTableNode& rTblNd );
};

struct SwListEntry
{
    long nListId, nListTemplateId, nListNo;
    sal_uInt16 nListDocPos;
    sal_Bool bRuleUsed;

    SwListEntry()
        : nListId( 0 ), nListTemplateId( 0 ), nListNo( 0 ), nListDocPos( 0 ),
        bRuleUsed( sal_False )
    {}
    SwListEntry( long nLstId, long nLstTmplId, sal_uInt16 nLstDocPos )
        : nListId( nLstId ), nListTemplateId( nLstTmplId ), nListNo( 0 ),
        nListDocPos( nLstDocPos ), bRuleUsed( sal_False )
    {}

    void Clear() { nListId = nListTemplateId = nListNo = 0, nListDocPos = 0;
                    bRuleUsed = sal_False; }
};

class SwFlySaveArr : public std::vector<SwFlySave*>
{
public:
    SwFlySaveArr() : ::std::vector<SwFlySave*>() {}
    SwFlySaveArr(const SwFlySaveArr& rOther)
        : ::std::vector<SwFlySave*>(rOther)
    {}

    ~SwFlySaveArr()
    {
        for(const_iterator it = begin(); it != end(); ++it)
            delete *it;
    }
};

typedef std::deque< SwListEntry > SwListArr;

struct DocPageInformation
{
    SvxBoxItem maBox;
    long mnPaperw;
    long mnPaperh;
    long mnMargl;
    long mnMargr;
    long mnMargt;
    long mnMargb;
    long mnGutter;
    long mnPgnStart;
    bool mbFacingp;
    bool mbLandscape;
    bool mbRTLdoc;
    DocPageInformation();
};

struct SectPageInformation
{
    std::vector<long> maColumns;
    SvxBoxItem maBox;
    SvxNumberType maNumType;
    SwPageDesc *mpTitlePageHdFt;
    SwPageDesc *mpPageHdFt;
    long mnPgwsxn;
    long mnPghsxn;
    long mnMarglsxn;
    long mnMargrsxn;
    long mnMargtsxn;
    long mnMargbsxn;
    long mnGutterxsn;
    long mnHeadery;
    long mnFootery;
    long mnPgnStarts;
    long mnCols;
    long mnColsx;
    long mnStextflow;
    int mnBkc;
    bool mbLndscpsxn;
    bool mbTitlepg;
    bool mbFacpgsxn;
    bool mbRTLsection;
    bool mbPgnrestart;
    bool mbTitlePageHdFtUsed;
    bool mbPageHdFtUsed;
    SectPageInformation(const DocPageInformation &rDoc);
    SectPageInformation(const SectPageInformation &rSect);
};

class rtfSection
{
public:
    rtfSection(const SwPosition &rPos,
        const SectPageInformation &rPageInfo);
    SwNodeIndex maStart;
    SectPageInformation maPageInfo;
    SwSection *mpSection;
    SwPageDesc *mpTitlePage;
    SwPageDesc *mpPage;

    bool IsContinous() const { return maPageInfo.mnBkc == 0; }
    long NoCols() const { return maPageInfo.mnCols; }
    long StandardColSeperation() const { return maPageInfo.mnColsx; }
    bool HasTitlePage() const { return maPageInfo.mbTitlepg; }
    long PageStartAt() const { return maPageInfo.mnPgnStarts; }
    bool PageRestartNo() const { return maPageInfo.mbPgnrestart; }
    bool IsBiDi() const { return maPageInfo.mbRTLsection; }
    long GetPageWidth() const { return maPageInfo.mnPgwsxn; }
    long GetPageHeight() const { return maPageInfo.mnPghsxn; }
    long GetPageLeft() const { return maPageInfo.mnMarglsxn; }
    long GetPageRight() const { return maPageInfo.mnMargrsxn; }
    bool IsLandScape() const { return maPageInfo.mbLndscpsxn; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
