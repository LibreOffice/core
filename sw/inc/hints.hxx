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
#ifndef INCLUDED_SW_INC_HINTS_HXX
#define INCLUDED_SW_INC_HINTS_HXX

#include "swatrset.hxx"
#include "swtypes.hxx"
#include <vcl/vclptr.hxx>

class SwFormat;
class OutputDevice;
class SwTable;
class SwNode;
class SwNodes;
class SwPageFrame;
class SwFrame;
class SwHistory;
class SwTextNode;

// Base class for all Message-Hints:
// "Overhead" of SfxPoolItem is handled here
class SwMsgPoolItem : public SfxPoolItem
{
public:
    SwMsgPoolItem( sal_uInt16 nWhich );

    // "Overhead" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
};

// SwPtrMsgPoolItem (old SwObjectDying!)

class SwPtrMsgPoolItem : public SwMsgPoolItem
{
public:
    void * pObject;

    SwPtrMsgPoolItem( sal_uInt16 nId, void * pObj )
        : SwMsgPoolItem( nId ), pObject( pObj )
    {}
};

/*
 * SwFormatChg is sent when a format has changed to another format. 2 Hints are always sent
 * the old and the new format
 */
class SwFormatChg: public SwMsgPoolItem
{
public:
    SwFormat * const pChangedFormat;
    SwFormatChg( SwFormat *pFormat );
};

class SwInsText: public SwMsgPoolItem
{
public:
    sal_Int32 const nPos;
    sal_Int32 const nLen;

    SwInsText( sal_Int32 nP, sal_Int32 nL );
};

class SwDelChr: public SwMsgPoolItem
{
public:
    sal_Int32 const nPos;

    SwDelChr( sal_Int32 nP );
};

class SwDelText: public SwMsgPoolItem
{
public:
    sal_Int32 const nStart;
    sal_Int32 const nLen;

    SwDelText( sal_Int32 nS, sal_Int32 nL );
};

namespace sw {

/// text is moved into pDestNode
class MoveText : public SfxHint
{
public:
    SwTextNode * pDestNode;
    sal_Int32 nDestStart;
    sal_Int32 nSourceStart;
    sal_Int32 nLen;

    MoveText(SwTextNode *pD, sal_Int32 nD, sal_Int32 nS, sal_Int32 nL);
};

/// new delete redline is created
class RedlineDelText : public SfxHint
{
public:
    sal_Int32 const nStart;
    sal_Int32 const nLen;

    RedlineDelText(sal_Int32 nS, sal_Int32 nL);
};

/// delete redline is removed
class RedlineUnDelText : public SfxHint
{
public:
    sal_Int32 nStart;
    sal_Int32 nLen;

    RedlineUnDelText(sal_Int32 nS, sal_Int32 nL);
};

}

class SwUpdateAttr : public SwMsgPoolItem
{
private:
    sal_Int32 const m_nStart;
    sal_Int32 const m_nEnd;
    sal_uInt16 const m_nWhichAttr;
    std::vector<sal_uInt16> m_aWhichFmtAttrs; // attributes changed inside RES_TXTATR_AUTOFMT

public:
    SwUpdateAttr( sal_Int32 nS, sal_Int32 nE, sal_uInt16 nW );
    SwUpdateAttr( sal_Int32 nS, sal_Int32 nE, sal_uInt16 nW, std::vector<sal_uInt16> aW );

    sal_Int32 getStart() const
    {
        return m_nStart;
    }

    sal_Int32 getEnd() const
    {
        return m_nEnd;
    }

    sal_uInt16 getWhichAttr() const
    {
        return m_nWhichAttr;
    }

    const std::vector<sal_uInt16>& getFmtAttrs() const
    {
        return m_aWhichFmtAttrs;
    }
};

/** SwRefMarkFieldUpdate is sent when the referencemarks should be updated.
     To determine Page- / chapternumbers the current frame has to be asked.
      For this we need the current outputdevice */
class SwRefMarkFieldUpdate : public SwMsgPoolItem
{
    VclPtr<OutputDevice> pOut; ///< pointer to the current output device
public:
    /** Is sent if reference marks should be updated.

        To get the page/chapter number, the frame has to be asked. For that we need
        the current OutputDevice.
    */
    SwRefMarkFieldUpdate( OutputDevice* );
};

/** SwDocPosUpdate is sent to signal that only the frames from or to a specified document-global position
   have to be updated. At the moment this is only needed when updating pagenumber fields. */
class SwDocPosUpdate : public SwMsgPoolItem
{
public:
    const SwTwips nDocPos;
    SwDocPosUpdate( const SwTwips nDocPos );
};

/// SwTableFormulaUpdate is sent when the table has to be newly calculated or when a table itself is merged or splitted
enum TableFormulaUpdateFlags { TBL_CALC = 0,
                         TBL_BOXNAME,
                         TBL_BOXPTR,
                         TBL_RELBOXNAME,
                         TBL_MERGETBL,
                         TBL_SPLITTBL
                       };
class SwTableFormulaUpdate : public SwMsgPoolItem
{
public:
    const SwTable* m_pTable;         ///< Pointer to the current table
    union {
        const SwTable* pDelTable;  ///< Merge: Pointer to the table to be removed
        const OUString* pNewTableNm; ///< Split: the name of the new table
    } m_aData;
    SwHistory* m_pHistory;
    sal_uInt16 m_nSplitLine;       ///< Split: from this BaseLine on will be splitted
    TableFormulaUpdateFlags m_eFlags;
    bool m_bModified : 1;
    bool m_bBehindSplitLine : 1;

    /** Is sent if a table should be recalculated */
    SwTableFormulaUpdate( const SwTable* );
};

class SwAutoFormatGetDocNode: public SwMsgPoolItem
{
public:
    const SwNodes* pNodes;

    SwAutoFormatGetDocNode( const SwNodes* pNds );
};

/*
 * SwAttrSetChg is sent when something has changed in the SwAttrSet rTheChgdSet.
 * 2 Hints are always sent, the old and the new items in the rTheChgdSet.
 */
class SwAttrSetChg: public SwMsgPoolItem
{
    bool const m_bDelSet;
    SwAttrSet* m_pChgSet;           ///< what has changed
    const SwAttrSet* m_pTheChgdSet; ///< is only used to compare
public:
    SwAttrSetChg( const SwAttrSet& rTheSet, SwAttrSet& rSet );
    SwAttrSetChg( const SwAttrSetChg& );
    virtual ~SwAttrSetChg() override;

    /// What has changed
    const SwAttrSet* GetChgSet() const     { return m_pChgSet; }
          SwAttrSet* GetChgSet()           { return m_pChgSet; }

    /// Where it has changed
    const SwAttrSet* GetTheChgdSet() const { return m_pTheChgdSet; }

    sal_uInt16 Count() const { return m_pChgSet->Count(); }
    void ClearItem( sal_uInt16 nWhichL )
#ifdef DBG_UTIL
        ;
#else
    { m_pChgSet->ClearItem( nWhichL ); }
#endif
};

class SwCondCollCondChg: public SwMsgPoolItem
{
public:
    SwFormat * const pChangedFormat;
    SwCondCollCondChg( SwFormat *pFormat );
};

class SwVirtPageNumInfo: public SwMsgPoolItem
{
    const SwPageFrame *m_pPage;
    const SwPageFrame *m_pOrigPage;
    const SwFrame     *m_pFrame;
    /** Multiple attributes can be attached to a single paragraph / table
     The frame, in the end, has to decide which attribute takes effect and which physical page it involves */
public:
    SwVirtPageNumInfo( const SwPageFrame *pPg );

    const SwPageFrame *GetPage()          { return m_pPage;    }
    const SwPageFrame *GetOrigPage()      { return m_pOrigPage;}
    const SwFrame *GetFrame()               { return m_pFrame; }
    void  SetInfo( const SwPageFrame *pPg,
                   const SwFrame *pF )    { m_pFrame = pF; m_pPage = pPg; }
};

class SwFindNearestNode : public SwMsgPoolItem
{
    const SwNode *m_pNode, *m_pFound;
public:
    SwFindNearestNode( const SwNode& rNd );
    void CheckNode( const SwNode& rNd );

    const SwNode* GetFoundNode() const { return m_pFound; }
};

class SwStringMsgPoolItem : public SwMsgPoolItem
{
    OUString const m_sStr;
public:

    const OUString& GetString() const { return m_sStr; }

    SwStringMsgPoolItem( sal_uInt16 nId, const OUString& rStr )
        : SwMsgPoolItem( nId ), m_sStr( rStr )
    {}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
