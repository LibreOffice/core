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

#include <swatrset.hxx>
#include <vcl/vclptr.hxx>
#include <vector>

class SwFormat;
class OutputDevice;
class SwTable;
class SwNode;
class SwNodes;
class SwContentNode;
class SwPageFrame;
class SwFrame;
class SwTextNode;
class SwHistory;

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

    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return pObject == static_cast<const SwPtrMsgPoolItem&>( rAttr ).pObject;
    }
};

/*
 * SwFormatChg is sent when a format has changed to another format. 2 Hints are always sent
 * the old and the new format
 */
class SwFormatChg: public SwMsgPoolItem
{
public:
    SwFormat *pChangedFormat;
    SwFormatChg( SwFormat *pFormat );

    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return pChangedFormat == static_cast<const SwFormatChg&>( rAttr ).pChangedFormat;
    }
};

class SwInsText: public SwMsgPoolItem
{
public:
    sal_Int32 nPos;
    sal_Int32 nLen;

    SwInsText( sal_Int32 nP, sal_Int32 nL );

    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return nPos == static_cast<const SwInsText&>( rAttr ).nPos
               && nLen == static_cast<const SwInsText&>( rAttr ).nLen;
    }
};

class SwDelChr: public SwMsgPoolItem
{
public:
    sal_Int32 nPos;

    SwDelChr( sal_Int32 nP );

    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return nPos == static_cast<const SwDelChr&>( rAttr ).nPos;
    }
};

class SwDelText: public SwMsgPoolItem
{
public:
    sal_Int32 nStart;
    sal_Int32 nLen;

    SwDelText( sal_Int32 nS, sal_Int32 nL );

    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return nStart == static_cast<const SwDelText&>( rAttr ).nStart
               && nLen == static_cast<const SwDelText&>( rAttr ).nLen;
    }
};

class SwUpdateAttr : public SwMsgPoolItem
{
private:
    sal_Int32 m_nStart;
    sal_Int32 m_nEnd;
    sal_uInt16 m_nWhichAttr;
    std::vector<sal_uInt16> m_aWhichFormatAttr; // attributes changed inside RES_TXTATR_AUTOFMT

public:
    SwUpdateAttr( sal_Int32 nS, sal_Int32 nE, sal_uInt16 nW );

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

    const std::vector<sal_uInt16>& getFormatAttr() const
    {
        return m_aWhichFormatAttr;
    }

    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return m_nStart == static_cast<const SwUpdateAttr&>( rAttr ).m_nStart
              && m_nEnd == static_cast<const SwUpdateAttr&>( rAttr ).m_nEnd
              && m_nWhichAttr == static_cast<const SwUpdateAttr&>( rAttr ).m_nWhichAttr;
    }
};

/** SwRefMarkFieldUpdate is sent when the referencemarks should be updated.
     To determine Page- / chapternumbers the current frame has to be asked.
      For this we need the current outputdevice */
class SwRefMarkFieldUpdate : public SwMsgPoolItem
{
public:
    VclPtr<OutputDevice> pOut; ///< pointer to the current output device
    /** Is sent if reference marks should be updated.

        To get the page/chapter number, the frame has to be asked. For that we need
        the current OutputDevice.
    */
    SwRefMarkFieldUpdate( OutputDevice* );

    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return pOut == static_cast<const SwRefMarkFieldUpdate&>( rAttr ).pOut;
    }
};

/** SwDocPosUpdate is sent to signal that only the frames from or to a specified document-global position
   have to be updated. At the moment this is only needed when updating pagenumber fields. */
class SwDocPosUpdate : public SwMsgPoolItem
{
public:
    const long nDocPos;
    SwDocPosUpdate( const long nDocPos );

    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return nDocPos == static_cast<const SwDocPosUpdate&>( rAttr ).nDocPos;
    }
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

    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return m_pTable == static_cast<const SwTableFormulaUpdate&>( rAttr ).m_pTable;
    }
};

class SwAutoFormatGetDocNode: public SwMsgPoolItem
{
public:
    const SwContentNode* pContentNode;
    const SwNodes* pNodes;

    SwAutoFormatGetDocNode( const SwNodes* pNds );

    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return pNodes == static_cast<const SwAutoFormatGetDocNode&>( rAttr ).pNodes;
    }
};

/*
 * SwAttrSetChg is sent when something has changed in the SwAttrSet rTheChgdSet.
 * 2 Hints are always sent, the old and the new items in the rTheChgdSet.
 */
class SwAttrSetChg: public SwMsgPoolItem
{
    bool m_bDelSet;
    SwAttrSet* m_pChgSet;           ///< what has changed
    const SwAttrSet* m_pTheChgdSet; ///< is only used to compare
public:
    SwAttrSetChg( const SwAttrSet& rTheSet, SwAttrSet& rSet );
    SwAttrSetChg( const SwAttrSetChg& );
    virtual ~SwAttrSetChg();

    /// What has changed
    const SwAttrSet* GetChgSet() const     { return m_pChgSet; }
          SwAttrSet* GetChgSet()           { return m_pChgSet; }

    /// Where it has changed
    const SwAttrSet* GetTheChgdSet() const { return m_pTheChgdSet; }

    sal_uInt16 Count() const { return m_pChgSet->Count(); }
    void ClearItem( sal_uInt16 nWhichL = 0 )
#ifdef DBG_UTIL
        ;
#else
    { m_pChgSet->ClearItem( nWhichL ); }
#endif
    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return pChgSet == static_cast<const SwAttrSetChg&>( rAttr ).pChgSet;
    }
};

class SwCondCollCondChg: public SwMsgPoolItem
{
public:
    SwFormat *pChangedFormat;
    SwCondCollCondChg( SwFormat *pFormat );
    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return pChangedFormat == static_cast<const SwCondCollCondChg&>( rAttr ).pChangedFormat;
    }
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
                   const SwFrame *pF )    { m_pFrame = pF, m_pPage = pPg; }

    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return m_pPage == static_cast<const SwVirtPageNumInfo&>( rAttr ).m_pPage;
    }
};

class SwFindNearestNode : public SwMsgPoolItem
{
    const SwNode *m_pNode, *m_pFound;
public:
    SwFindNearestNode( const SwNode& rNd );
    void CheckNode( const SwNode& rNd );

    const SwNode* GetFoundNode() const { return m_pFound; }

    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return m_pNode == static_cast<const SwFindNearestNode&>( rAttr ).m_pNode;
    }
};

class SwStringMsgPoolItem : public SwMsgPoolItem
{
    OUString m_sStr;
public:

    OUString GetString() const { return m_sStr; }

    SwStringMsgPoolItem( sal_uInt16 nId, const OUString& rStr )
        : SwMsgPoolItem( nId ), m_sStr( rStr )
    {}
    virtual bool operator==( const SfxPoolItem& rAttr ) const override
    {
        DBG_ASSERT( SwMsgPoolItem::operator==(rAttr), "unequal types" );
        return m_sStr == static_cast<const SwStringMsgPoolItem&>( rAttr ).m_sStr;
    }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
