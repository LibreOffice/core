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
#ifndef _HINTS_HXX
#define _HINTS_HXX

#include <swatrset.hxx>

class SwFmt;
class OutputDevice;
class SwTable;
class SwNode;
class SwNodes;
class SwCntntNode;
class SwPageFrm;
class SwFrm;
class SwTxtNode;
class SwHistory;

// Base class for all Message-Hints:
// "Overhead" of SfxPoolItem is handled here
class SwMsgPoolItem : public SfxPoolItem
{
public:
    SwMsgPoolItem( sal_uInt16 nWhich );

    // "Overhead" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
};


// ---------------------------------------
// SwPtrMsgPoolItem (old SwObjectDying!)
// ---------------------------------------

class SwPtrMsgPoolItem : public SwMsgPoolItem
{
public:
    void * pObject;

    SwPtrMsgPoolItem( sal_uInt16 nId, void * pObj )
        : SwMsgPoolItem( nId ), pObject( pObj )
    {}
};



/*
 * SwFmtChg is sent when a format has changed to another format. 2 Hints are always sent
 * the old and the new format
 */
class SwFmtChg: public SwMsgPoolItem
{
public:
    SwFmt *pChangedFmt;
    SwFmtChg( SwFmt *pFmt );
};


class SwInsTxt: public SwMsgPoolItem
{
public:
    xub_StrLen nPos;
    xub_StrLen nLen;

    SwInsTxt( xub_StrLen nP, xub_StrLen nL );
};

class SwDelChr: public SwMsgPoolItem
{
public:
    xub_StrLen nPos;

    SwDelChr( xub_StrLen nP );
};

class SwDelTxt: public SwMsgPoolItem
{
public:
    xub_StrLen nStart;
    xub_StrLen nLen;

    SwDelTxt( xub_StrLen nS, xub_StrLen nL );
};

class SwUpdateAttr: public SwMsgPoolItem
{
public:
    xub_StrLen nStart;
    xub_StrLen nEnd;
    sal_uInt16 nWhichAttr;
    SwUpdateAttr( xub_StrLen nS, xub_StrLen nE, sal_uInt16 nW );
};


/** SwRefMarkFldUpdate is sent when the referencemarks should be updated.
     To determine Page- / chapternumbers the current frame has to be asked.
      For this we need the current outputdevice */
class SwRefMarkFldUpdate : public SwMsgPoolItem
{
public:
    const OutputDevice* pOut; ///< pointer to the current output device
    SwRefMarkFldUpdate( const OutputDevice* );
};

/** SwDocPosUpdate is sent to signal that only the frames from or to a specified document-global position
   have to be updated. At the moment this is only needed when updating pagenumber fields. */
class SwDocPosUpdate : public SwMsgPoolItem
{
public:
    const long nDocPos;
    SwDocPosUpdate( const long nDocPos );
};

/// SwTableFmlUpdate is sent when the table has to be newly calculated or when a table itself is merged or splitted
enum TableFmlUpdtFlags { TBL_CALC = 0,
                         TBL_BOXNAME,
                         TBL_BOXPTR,
                         TBL_RELBOXNAME,
                         TBL_MERGETBL,
                         TBL_SPLITTBL
                       };
class SwTableFmlUpdate : public SwMsgPoolItem
{
public:
    const SwTable* pTbl;         ///< Pointer to the current table
    union {
        const SwTable* pDelTbl;  ///< Merge: Pointer to the table to be removed
        const OUString* pNewTblNm; ///< Split: the name of the new table
    } DATA;
    SwHistory* pHistory;
    sal_uInt16 nSplitLine;       ///< Split: from this BaseLine on will be splitted
    TableFmlUpdtFlags eFlags;
    sal_Bool bModified : 1;
    sal_Bool bBehindSplitLine : 1;

    SwTableFmlUpdate( const SwTable* );
};


class SwAutoFmtGetDocNode: public SwMsgPoolItem
{
public:
    const SwCntntNode* pCntntNode;
    const SwNodes* pNodes;

    SwAutoFmtGetDocNode( const SwNodes* pNds );
};

/*
 * SwAttrSetChg is sent when something has changed in the SwAttrSet rTheChgdSet.
 * 2 Hints are always sent, the old and the new items in the rTheChgdSet.
 */
class SwAttrSetChg: public SwMsgPoolItem
{
    sal_Bool bDelSet;
    SwAttrSet* pChgSet;           ///< what has changed
    const SwAttrSet* pTheChgdSet; ///< is only used to compare
public:
    SwAttrSetChg( const SwAttrSet& rTheSet, SwAttrSet& rSet );
    SwAttrSetChg( const SwAttrSetChg& );
    ~SwAttrSetChg();

    /// What has changed
    const SwAttrSet* GetChgSet() const     { return pChgSet; }
          SwAttrSet* GetChgSet()           { return pChgSet; }

    /// Where it has changed
    const SwAttrSet* GetTheChgdSet() const { return pTheChgdSet; }

    sal_uInt16 Count() const { return pChgSet->Count(); }
    void ClearItem( sal_uInt16 nWhichL = 0 )
#ifdef DBG_UTIL
        ;
#else
    { pChgSet->ClearItem( nWhichL ); }
#endif
};

class SwCondCollCondChg: public SwMsgPoolItem
{
public:
    SwFmt *pChangedFmt;
    SwCondCollCondChg( SwFmt *pFmt );
};

class SwVirtPageNumInfo: public SwMsgPoolItem
{
    const SwPageFrm *pPage;
    const SwPageFrm *pOrigPage;
    const SwFrm     *pFrm;
    /** Multiple attributes can be attached to a single paragraph / table
     The frame, in the end, has to decide which attribute takes effect and which physical page it involves */
public:
    SwVirtPageNumInfo( const SwPageFrm *pPg );

    const SwPageFrm *GetPage()          { return pPage;    }
    const SwPageFrm *GetOrigPage()      { return pOrigPage;}
    const SwFrm *GetFrm()               { return pFrm; }
    void  SetInfo( const SwPageFrm *pPg,
                   const SwFrm *pF )    { pFrm = pF, pPage = pPg; }
};

class SwFindNearestNode : public SwMsgPoolItem
{
    const SwNode *pNd, *pFnd;
public:
    SwFindNearestNode( const SwNode& rNd );
    void CheckNode( const SwNode& rNd );

    const SwNode* GetFoundNode() const { return pFnd; }
};

class SwStringMsgPoolItem : public SwMsgPoolItem
{
    String sStr;
public:

    const String& GetString() const { return sStr; }

    SwStringMsgPoolItem( sal_uInt16 nId, const String& rStr )
        : SwMsgPoolItem( nId ), sStr( rStr )
    {}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
