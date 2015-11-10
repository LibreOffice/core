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

#include <boost/noncopyable.hpp>
#include <svl/smplhint.hxx>
#include <vector>

class SfxItemSet;
class SwView;

class SW_DLLPUBLIC SwRedlineExtraData
{
    SwRedlineExtraData( const SwRedlineExtraData& ) = delete;
    SwRedlineExtraData& operator=( const SwRedlineExtraData& ) = delete;

protected:
    SwRedlineExtraData() {}

public:
    virtual ~SwRedlineExtraData();
    virtual SwRedlineExtraData* CreateNew() const = 0;

    virtual void Reject( SwPaM& rPam ) const;
    virtual bool operator == ( const SwRedlineExtraData& ) const;
};

class SwRedlineExtraData_FormatColl : public SwRedlineExtraData
{
    OUString sFormatNm;
    SfxItemSet* pSet;
    sal_uInt16 nPoolId;
public:
    SwRedlineExtraData_FormatColl( const OUString& rColl, sal_uInt16 nPoolFormatId,
                                const SfxItemSet* pSet = nullptr );
    virtual ~SwRedlineExtraData_FormatColl();
    virtual SwRedlineExtraData* CreateNew() const override;
    virtual void Reject( SwPaM& rPam ) const override;
    virtual bool operator == ( const SwRedlineExtraData& ) const override;

    void SetItemSet( const SfxItemSet& rSet );
};

class SwRedlineExtraData_Format : public SwRedlineExtraData
{
    std::vector<sal_uInt16> aWhichIds;

    SwRedlineExtraData_Format( const SwRedlineExtraData_Format& rCpy );

public:
    SwRedlineExtraData_Format( const SfxItemSet& rSet );
    virtual ~SwRedlineExtraData_Format();
    virtual SwRedlineExtraData* CreateNew() const override;
    virtual void Reject( SwPaM& rPam ) const override;
    virtual bool operator == ( const SwRedlineExtraData& ) const override;
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
    virtual SwRedlineExtraData* CreateNew() const override;
    virtual void Reject( SwPaM& rPam ) const override;
    virtual bool operator == ( const SwRedlineExtraData& ) const override;
    SfxItemSet* GetItemSet( ) const { return pSet; }
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
    SwRedlineData( const SwRedlineData& rCpy, bool bCpyNext = true );

    // For sw3io: pNext/pExtraData are taken over.
    SwRedlineData( RedlineType_t eT, sal_uInt16 nAut, const DateTime& rDT,
                   const OUString& rCmnt, SwRedlineData* pNxt,
                    SwRedlineExtraData* pExtraData = nullptr );

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

    sal_uInt16 GetAuthor() const                { return nAuthor; }
    const OUString& GetComment() const        { return sComment; }
    const DateTime& GetTimeStamp() const    { return aStamp; }
    inline const SwRedlineData* Next() const{ return pNext; }

    void SetComment( const OUString& rS )     { sComment = rS; }
    void SetTimeStamp( const DateTime& rDT ) { aStamp = rDT; }

    void SetAutoFormatFlag()
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
    SwNodeIndex* pContentSect;
    bool bDelLastPara : 1;
    bool bIsLastParaDelete : 1;
    bool bIsVisible : 1;

    void MoveToSection();
    void CopyToSection();
    void DelCopyOfSection(size_t nMyPos);
    void MoveFromSection(size_t nMyPos);

public:
    SwRangeRedline( RedlineType_t eType, const SwPaM& rPam );
    SwRangeRedline( const SwRedlineData& rData, const SwPaM& rPam );
    SwRangeRedline( const SwRedlineData& rData, const SwPosition& rPos );
    // For sw3io: pData is taken over!
    SwRangeRedline(SwRedlineData* pData, const SwPosition& rPos, bool bVsbl,
               bool bDelLP, bool bIsPD) :
        SwPaM( rPos ), pRedlineData( pData ), pContentSect( nullptr ),
        bDelLastPara( bDelLP ), bIsLastParaDelete( bIsPD ), bIsVisible( bVsbl )
    {}
    SwRangeRedline( const SwRangeRedline& );
    virtual ~SwRangeRedline();

    SwNodeIndex* GetContentIdx() const { return pContentSect; }
    // For Undo.
    void SetContentIdx( const SwNodeIndex* );

    bool IsVisible() const { return bIsVisible; }
    bool IsDelLastPara() const { return bDelLastPara; }

    void SetStart( const SwPosition& rPos, SwPosition* pSttPtr = nullptr )
    {
        if( !pSttPtr ) pSttPtr = Start();
        *pSttPtr = rPos;
    }
    void SetEnd( const SwPosition& rPos, SwPosition* pEndPtr = nullptr )
    {
        if( !pEndPtr ) pEndPtr = End();
        *pEndPtr = rPos;
    }
    /// Do we have a valid selection?
    bool HasValidRange() const;

    const SwRedlineData& GetRedlineData(sal_uInt16 nPos = 0) const;
    bool operator==( const SwRedlineData& rCmp ) const
        { return *pRedlineData == rCmp; }
    bool operator!=( const SwRedlineData& rCmp ) const
        { return *pRedlineData != rCmp; }
    void SetAutoFormatFlag()               { pRedlineData->SetAutoFormatFlag(); }

    sal_uInt16 GetStackCount() const;
    sal_uInt16 GetAuthor( sal_uInt16 nPos = 0) const;
    OUString GetAuthorString( sal_uInt16 nPos = 0 ) const;
    const DateTime& GetTimeStamp( sal_uInt16 nPos = 0) const;
    RedlineType_t GetRealType( sal_uInt16 nPos = 0 ) const;
    RedlineType_t GetType( sal_uInt16 nPos = 0) const
        { return ( (RedlineType_t)(GetRealType( nPos ) & nsRedlineType_t::REDLINE_NO_FLAG_MASK)); }
    const OUString& GetComment( sal_uInt16 nPos = 0 ) const;

    void SetComment( const OUString& rS ) { pRedlineData->SetComment( rS ); }

    /** ExtraData gets copied, the pointer is therefore not taken over by
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
    void CallDisplayFunc(sal_uInt16 nLoop, size_t nMyPos);
    void Show(sal_uInt16 nLoop , size_t nMyPos);
    void Hide(sal_uInt16 nLoop , size_t nMyPos);
    void ShowOriginal(sal_uInt16 nLoop, size_t nMyPos);

    /// Calculates the intersection with text node number nNdIdx.
    void CalcStartEnd(sal_uLong nNdIdx, sal_Int32& rStart, sal_Int32& rEnd) const;

    /// Initiate the layout.
    void InvalidateRange();

    bool IsOwnRedline( const SwRangeRedline& rRedl ) const
        { return GetAuthor() == rRedl.GetAuthor(); }
    bool CanCombine( const SwRangeRedline& rRedl ) const;

    void PushData( const SwRangeRedline& rRedl, bool bOwnAsNext = true );
    bool PopData();

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
    void dumpAsXml(struct _xmlTextWriter* pWriter) const;
};

/// Base object for 'Redlines' that are not of 'Ranged' type (like table row insert\delete)
class SW_DLLPUBLIC SwExtraRedline : private boost::noncopyable
{
public:
    virtual ~SwExtraRedline();
};

/// Redline that holds information about a table-row that had some change
class SW_DLLPUBLIC SwTableRowRedline : public SwExtraRedline
{
private:
    SwRedlineData m_aRedlineData;
    const SwTableLine& m_rTableLine;

public:
    SwTableRowRedline(const SwRedlineData& rData, const SwTableLine& rTableLine);
    virtual ~SwTableRowRedline();

    /** ExtraData gets copied, the pointer is therefore not taken over by
     *  the RedLineObject.*/
    void SetExtraData( const SwRedlineExtraData* pData )
        { m_aRedlineData.SetExtraData( pData ); }
    const SwTableLine& GetTableLine() const
        { return m_rTableLine; }
    const SwRedlineData& GetRedlineData() const
        { return m_aRedlineData; }
};

/// Redline that holds information about a table-cell that had some change
class SW_DLLPUBLIC SwTableCellRedline : public SwExtraRedline
{
private:
    SwRedlineData m_aRedlineData;
    const SwTableBox& m_rTableBox;

public:
    SwTableCellRedline(const SwRedlineData& rData, const SwTableBox& rTableBox);
    virtual ~SwTableCellRedline();

    /** ExtraData gets copied, the pointer is therefore not taken over by
     *  the RedLineObject.*/
    void SetExtraData( const SwRedlineExtraData* pData )
        { m_aRedlineData.SetExtraData( pData ); }
    const SwTableBox& GetTableBox() const
        { return m_rTableBox; }
    const SwRedlineData& GetRedlineData() const
        { return m_aRedlineData; }
};

class SW_DLLPUBLIC SwRedlineHint : public SfxHint
{
#define SWREDLINE_INSERTED  1
#define SWREDLINE_FOCUS     3

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
