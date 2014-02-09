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
#ifndef INCLUDED_SW_INC_REDLINE_HXX
#define INCLUDED_SW_INC_REDLINE_HXX

#include <tools/datetime.hxx>
#include <rtl/ustring.hxx>

#include <pam.hxx>
#include <swtable.hxx>

#include <IDocumentRedlineAccess.hxx>

#include <svl/smplhint.hxx>
#include <vector>

class SfxItemSet;
class SwView;

class SW_DLLPUBLIC SwRedlineExtraData
{
    SwRedlineExtraData( const SwRedlineExtraData& );
    SwRedlineExtraData& operator=( const SwRedlineExtraData& );

protected:
    SwRedlineExtraData() {}

public:
    virtual ~SwRedlineExtraData();
    virtual SwRedlineExtraData* CreateNew() const = 0;

    virtual void Accept( SwPaM& rPam ) const;
    virtual void Reject( SwPaM& rPam ) const;
    virtual bool operator == ( const SwRedlineExtraData& ) const;
};

class SwRedlineExtraData_FmtColl : public SwRedlineExtraData
{
    OUString sFmtNm;
    SfxItemSet* pSet;
    sal_uInt16 nPoolId;
public:
    SwRedlineExtraData_FmtColl( const OUString& rColl, sal_uInt16 nPoolFmtId,
                                const SfxItemSet* pSet = 0 );
    virtual ~SwRedlineExtraData_FmtColl();
    virtual SwRedlineExtraData* CreateNew() const;
    virtual void Reject( SwPaM& rPam ) const;
    virtual bool operator == ( const SwRedlineExtraData& ) const;

    void SetItemSet( const SfxItemSet& rSet );
};

class SwRedlineExtraData_Format : public SwRedlineExtraData
{
    std::vector<sal_uInt16> aWhichIds;

    SwRedlineExtraData_Format( const SwRedlineExtraData_Format& rCpy );

public:
    SwRedlineExtraData_Format( const SfxItemSet& rSet );
    virtual ~SwRedlineExtraData_Format();
    virtual SwRedlineExtraData* CreateNew() const;
    virtual void Reject( SwPaM& rPam ) const;
    virtual bool operator == ( const SwRedlineExtraData& ) const;
};

/*
 * This class is used to store 'redline' data regarding formatting changes,
 * e.g. - a text portion *was* italic and now is not italic,
 * e.g. - a text portion got a hightlight to it
 *
 * The way the information is stored is in an 'SfxItemSet' that holds all
 * the WhichIds with their values.
 */
class SW_DLLPUBLIC SwRedlineExtraData_FormattingChanges : public SwRedlineExtraData
{
    SfxItemSet* pSet;

    SwRedlineExtraData_FormattingChanges( const SwRedlineExtraData_FormattingChanges& rCpy );

public:
    SwRedlineExtraData_FormattingChanges( const SfxItemSet* pItemSet );
    virtual ~SwRedlineExtraData_FormattingChanges();
    virtual SwRedlineExtraData* CreateNew() const;
    virtual void Reject( SwPaM& rPam ) const;
    virtual bool operator == ( const SwRedlineExtraData& ) const;
    SfxItemSet* GetItemSet( ) const;
};


class SW_DLLPUBLIC SwRedlineData
{
    friend class SwRangeRedline;
    SwRedlineData* pNext;       // Points to other data.
    SwRedlineExtraData* pExtraData;

    OUString sComment;
    DateTime aStamp;
    RedlineType_t eType;
    sal_uInt16 nAuthor, nSeqNo;

public:
    SwRedlineData( RedlineType_t eT, sal_uInt16 nAut );
    SwRedlineData( const SwRedlineData& rCpy, sal_Bool bCpyNext = sal_True );

    // For sw3io: pNext/pExtraData are taken over.
    SwRedlineData( RedlineType_t eT, sal_uInt16 nAut, const DateTime& rDT,
                   const OUString& rCmnt, SwRedlineData* pNxt,
                    SwRedlineExtraData* pExtraData = 0 );

    ~SwRedlineData();

    bool operator==( const SwRedlineData& rCmp ) const
        {
            return nAuthor == rCmp.nAuthor &&
                    eType == rCmp.eType &&
                    sComment == rCmp.sComment &&
                    (( !pNext && !rCmp.pNext ) ||
                        ( pNext && rCmp.pNext && *pNext == *rCmp.pNext )) &&
                    (( !pExtraData && !rCmp.pExtraData ) ||
                        ( pExtraData && rCmp.pExtraData &&
                            *pExtraData == *rCmp.pExtraData ));
        }
    bool operator!=( const SwRedlineData& rCmp ) const
        {   return !operator==( rCmp ); }

    RedlineType_t GetType() const
  { return ((RedlineType_t)(eType & nsRedlineType_t::REDLINE_NO_FLAG_MASK)); }
    RedlineType_t GetRealType() const { return eType; }
    sal_uInt16 GetAuthor() const                { return nAuthor; }
    const OUString& GetComment() const        { return sComment; }
    const DateTime& GetTimeStamp() const    { return aStamp; }
    inline const SwRedlineData* Next() const{ return pNext; }

    void SetComment( const OUString& rS )     { sComment = rS; }
    void SetTimeStamp( const DateTime& rDT ) { aStamp = rDT; }

    void SetAutoFmtFlag()
  { eType = (RedlineType_t)(eType | nsRedlineType_t::REDLINE_FORM_AUTOFMT); }
    bool CanCombine( const SwRedlineData& rCmp ) const
        {
            return nAuthor == rCmp.nAuthor &&
                    eType == rCmp.eType &&
                    sComment == rCmp.sComment &&
                    GetTimeStamp() == rCmp.GetTimeStamp() &&
                    (( !pNext && !rCmp.pNext ) ||
                        ( pNext && rCmp.pNext &&
                        pNext->CanCombine( *rCmp.pNext ))) &&
                    (( !pExtraData && !rCmp.pExtraData ) ||
                        ( pExtraData && rCmp.pExtraData &&
                            *pExtraData == *rCmp.pExtraData ));
        }

    // ExtraData gets copied, the pointer is therefore not taken over by
    // the RedlilneObject
    void SetExtraData( const SwRedlineExtraData* pData );
    const SwRedlineExtraData* GetExtraData() const { return pExtraData; }

    // For UI-side pooling of Redline-actions.
    // At the moment only used for Autoformat with Redline.
    // Value != 0 means there can be others!
    sal_uInt16 GetSeqNo() const                     { return nSeqNo; }
    void SetSeqNo( sal_uInt16 nNo )                 { nSeqNo = nNo; }

    OUString GetDescr() const;
};


class SW_DLLPUBLIC SwRangeRedline : public SwPaM
{
    SwRedlineData* pRedlineData;
    SwNodeIndex* pCntntSect;
    sal_Bool bDelLastPara : 1;
    sal_Bool bIsLastParaDelete : 1;
    sal_Bool bIsVisible : 1;

    void MoveToSection();
    void CopyToSection();
    void DelCopyOfSection();
    void MoveFromSection();

public:
    SwRangeRedline( RedlineType_t eType, const SwPaM& rPam );
    SwRangeRedline( const SwRedlineData& rData, const SwPaM& rPam );
    SwRangeRedline( const SwRedlineData& rData, const SwPosition& rPos );
    // For sw3io: pData is taken over!
    SwRangeRedline(SwRedlineData* pData, const SwPosition& rPos, sal_Bool bVsbl,
               sal_Bool bDelLP, sal_Bool bIsPD) :
        SwPaM( rPos ), pRedlineData( pData ), pCntntSect( 0 ),
        bDelLastPara( bDelLP ), bIsLastParaDelete( bIsPD ), bIsVisible( bVsbl )
    {}
    SwRangeRedline( const SwRangeRedline& );
    virtual ~SwRangeRedline();

    SwNodeIndex* GetContentIdx() const { return pCntntSect; }
    // For Undo.
    void SetContentIdx( const SwNodeIndex* );

    sal_Bool IsVisible() const { return bIsVisible; }
    sal_Bool IsDelLastPara() const { return bDelLastPara; }

    // sal_Bool indicates whether after setting of Pos no range is spanned.
    // -> sal-True else range and sal-False.
    void SetStart( const SwPosition& rPos, SwPosition* pSttPtr = 0 )
    {
        if( !pSttPtr ) pSttPtr = Start();
        *pSttPtr = rPos;
    }
    void SetEnd( const SwPosition& rPos, SwPosition* pEndPtr = 0 )
    {
        if( !pEndPtr ) pEndPtr = End();
        *pEndPtr = rPos;
    }
    /// Do we have a valid selection?
    sal_Bool HasValidRange() const;

    const SwRedlineData& GetRedlineData(sal_uInt16 nPos = 0) const;
    bool operator==( const SwRedlineData& rCmp ) const
        { return *pRedlineData == rCmp; }
    bool operator!=( const SwRedlineData& rCmp ) const
        { return *pRedlineData != rCmp; }
    void SetAutoFmtFlag()               { pRedlineData->SetAutoFmtFlag(); }

    sal_uInt16 GetStackCount() const;
    sal_uInt16 GetAuthor( sal_uInt16 nPos = 0) const;
    OUString GetAuthorString( sal_uInt16 nPos = 0 ) const;
    const DateTime& GetTimeStamp( sal_uInt16 nPos = 0) const;
    RedlineType_t GetRealType( sal_uInt16 nPos = 0 ) const;
    RedlineType_t GetType( sal_uInt16 nPos = 0) const
        { return ( (RedlineType_t)(GetRealType( nPos ) & nsRedlineType_t::REDLINE_NO_FLAG_MASK)); }
    const OUString& GetComment( sal_uInt16 nPos = 0 ) const;

    void SetComment( const OUString& rS ) { pRedlineData->SetComment( rS ); }

    /** ExtraData gets copied, the pointer is therefor not taken over by
     *  the RedLineObject.*/
    void SetExtraData( const SwRedlineExtraData* pData )
        { pRedlineData->SetExtraData( pData ); }
    const SwRedlineExtraData* GetExtraData() const
        { return pRedlineData->GetExtraData(); }

    // For UI-side pooling of Redline-actions.
    // At the moment only used for Autoformat with Redline.
    // Value != 0 means there can be others!
    sal_uInt16 GetSeqNo() const             { return pRedlineData->GetSeqNo(); }
    void SetSeqNo( sal_uInt16 nNo )         { pRedlineData->SetSeqNo( nNo ); }

    // At Hide/ShowOriginal the list is traversed two times in order to
    // hide the Del-Redlines via Copy and Delete.
    // Otherwise at Move the attribution would be handled incorrectly.
    // All other callers must always give 0.
    void CallDisplayFunc( sal_uInt16 nLoop = 0 );
    void Show( sal_uInt16 nLoop = 0 );
    void Hide( sal_uInt16 nLoop = 0 );
    void ShowOriginal( sal_uInt16 nLoop = 0 );

    /// Calculates the intersection with text node number nNdIdx.
    void CalcStartEnd(sal_uLong nNdIdx, sal_Int32& rStart, sal_Int32& rEnd) const;

    /// Initiate the layout.
    void InvalidateRange();

    sal_Bool IsOwnRedline( const SwRangeRedline& rRedl ) const
        { return GetAuthor() == rRedl.GetAuthor(); }
    sal_Bool CanCombine( const SwRangeRedline& rRedl ) const;

    void PushData( const SwRangeRedline& rRedl, sal_Bool bOwnAsNext = sal_True );
    sal_Bool PopData();

    /**
       Returns textual description of this a redline data element of
       this redline.

       @param nPos index of the redline data element to describe

       The textual description of the selected element contains the
       kind of redline and the possibly shortened text of the redline.

       @return textual description of the selected redline data element
     */
    OUString GetDescr(sal_uInt16 nPos = 0);

    bool operator==( const SwRangeRedline& ) const;
    bool operator<( const SwRangeRedline& ) const;
};

/// Base object for 'Redlines' that are not of 'Ranged' type (like table row insert\delete)
class SW_DLLPUBLIC SwExtraRedline
{
public:
    SwExtraRedline( );
    SwExtraRedline( const SwExtraRedline& );
    virtual ~SwExtraRedline();
};

/// Redline that holds information about a table-row that had some change
class SW_DLLPUBLIC SwTableRowRedline : public SwExtraRedline
{
private:
    SwRedlineData* pRedlineData;
    const SwTableLine* pTableLine;

public:
    SwTableRowRedline( const SwRedlineData& rData, SwTableLine& aTableLine );
    SwTableRowRedline( const SwTableRowRedline& );
    virtual ~SwTableRowRedline();

    /** ExtraData gets copied, the pointer is therefor not taken over by
     *  the RedLineObject.*/
    void SetExtraData( const SwRedlineExtraData* pData )
        { pRedlineData->SetExtraData( pData ); }
    const SwRedlineExtraData* GetExtraData() const
        { return pRedlineData->GetExtraData(); }
    const SwTableLine* GetTableLine() const
        { return pTableLine; }
    const SwRedlineData& GetRedlineData() const
        { return *pRedlineData; }
};

/// Redline that holds information about a table-cell that had some change
class SW_DLLPUBLIC SwTableCellRedline : public SwExtraRedline
{
private:
    SwRedlineData* pRedlineData;
    const SwTableBox* pTableBox;

public:
    SwTableCellRedline( const SwRedlineData& rData, SwTableBox& aTableBox );
    SwTableCellRedline( const SwTableCellRedline& );
    virtual ~SwTableCellRedline();

    /** ExtraData gets copied, the pointer is therefor not taken over by
     *  the RedLineObject.*/
    void SetExtraData( const SwRedlineExtraData* pData )
        { pRedlineData->SetExtraData( pData ); }
    const SwRedlineExtraData* GetExtraData() const
        { return pRedlineData->GetExtraData(); }
    const SwTableBox* GetTableBox() const
        { return pTableBox; }
    const SwRedlineData& GetRedlineData() const
        { return *pRedlineData; }
};

class SW_DLLPUBLIC SwRedlineHint : public SfxHint
{
#define SWREDLINE_INSERTED  1
#define SWREDLINE_REMOVED   2
#define SWREDLINE_FOCUS     3
#define SWREDLINE_CHANGED   4
#define SWREDLINE_LANGUAGE  5

    const SwRangeRedline* pRedline;
    sal_Int16 nWhich;
    const SwView* pView;

public:
    SwRedlineHint( const SwRangeRedline* p, sal_Int16 n, const SwView* pV = 0)
        : pRedline(p)
        , nWhich(n)
        , pView(pV)
        {}

    TYPEINFO();
    const SwRangeRedline* GetRedline() const { return pRedline; }
    sal_Int16 Which() const { return nWhich; }
        const SwView* GetView() const { return pView; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
