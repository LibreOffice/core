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

#include <svx/ctredlin.hxx>
#include <tools/datetime.hxx>
#include <rtl/ustring.hxx>

#include "pam.hxx"

#include <cstddef>
#include <memory>
#include <vector>
#include <optional>


class SfxItemSet;

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

class SW_DLLPUBLIC SwRedlineExtraData_FormatColl final : public SwRedlineExtraData
{
    OUString m_sFormatNm;
    std::unique_ptr<SfxItemSet> m_pSet;
    sal_uInt16 m_nPoolId;
    bool m_bFormatAll; // don't strip the last paragraph mark
public:
    SwRedlineExtraData_FormatColl( OUString aColl, sal_uInt16 nPoolFormatId,
                                const SfxItemSet* pSet = nullptr, bool bFormatAll = true );
    virtual ~SwRedlineExtraData_FormatColl() override;
    virtual SwRedlineExtraData* CreateNew() const override;
    virtual void Reject( SwPaM& rPam ) const override;
    virtual bool operator == ( const SwRedlineExtraData& ) const override;

    const OUString& GetFormatName() const        { return m_sFormatNm; }
    void SetItemSet( const SfxItemSet& rSet );
    SfxItemSet* GetItemSet( ) const { return m_pSet.get(); }
    void SetFormatAll( bool bAll )               { m_bFormatAll = bAll; }
};

class SwRedlineExtraData_Format final : public SwRedlineExtraData
{
    std::vector<sal_uInt16> m_aWhichIds;

    SwRedlineExtraData_Format( const SwRedlineExtraData_Format& rCpy );

public:
    SwRedlineExtraData_Format( const SfxItemSet& rSet );
    virtual ~SwRedlineExtraData_Format() override;
    virtual SwRedlineExtraData* CreateNew() const override;
    virtual void Reject( SwPaM& rPam ) const override;
    virtual bool operator == ( const SwRedlineExtraData& ) const override;
};

class SW_DLLPUBLIC SwRedlineData
{
    friend class SwRangeRedline;
    SwRedlineData* m_pNext;       // Points to other data.
    SwRedlineExtraData* m_pExtraData;

    OUString m_sComment;
    DateTime m_aStamp;
    std::size_t m_nAuthor;
    RedlineType m_eType;
    sal_uInt16 m_nSeqNo;
    bool m_bAutoFormat;
    sal_uInt32 m_nMovedID;  // 0 == not moved, 1 == moved, but don't have its pair, 2+ == unique ID

public:
    SwRedlineData( RedlineType eT, std::size_t nAut, sal_uInt32 nMoveID = 0 );
    SwRedlineData( const SwRedlineData& rCpy, bool bCpyNext = true );

    // For sw3io: pNext/pExtraData are taken over.
    SwRedlineData( RedlineType eT, std::size_t nAut, const DateTime& rDT, sal_uInt32 nMovedID,
                   OUString aCmnt, SwRedlineData* pNxt );

    ~SwRedlineData();

    bool operator==( const SwRedlineData& rCmp ) const
        {
            return m_nAuthor == rCmp.m_nAuthor &&
                    m_eType == rCmp.m_eType &&
                    m_bAutoFormat == rCmp.m_bAutoFormat &&
                    m_nMovedID == rCmp.m_nMovedID &&
                    m_sComment == rCmp.m_sComment &&
                    (( !m_pNext && !rCmp.m_pNext ) ||
                        ( m_pNext && rCmp.m_pNext && *m_pNext == *rCmp.m_pNext )) &&
                    (( !m_pExtraData && !rCmp.m_pExtraData ) ||
                        ( m_pExtraData && rCmp.m_pExtraData &&
                            *m_pExtraData == *rCmp.m_pExtraData ));
        }
    bool operator!=( const SwRedlineData& rCmp ) const
        {   return !operator==( rCmp ); }

    RedlineType GetType() const { return m_eType; }

    std::size_t GetAuthor() const                { return m_nAuthor; }
    const OUString& GetComment() const        { return m_sComment; }
    const DateTime& GetTimeStamp() const    { return m_aStamp; }
    bool IsAnonymized() const
        {
            return m_aStamp.GetYear() == 1970 &&
                    m_aStamp.GetMonth() == 1 && m_aStamp.GetDay() == 1;
        }

    const SwRedlineData* Next() const{ return m_pNext; }

    void SetComment( const OUString& rS )     { m_sComment = rS; }
    void SetTimeStamp( const DateTime& rDT ) { m_aStamp = rDT; }

    void SetAutoFormat() { m_bAutoFormat = true; }
    bool IsAutoFormat() const { return m_bAutoFormat; }
    void SetMoved( sal_uInt32 nMoveID ) { m_nMovedID = nMoveID; }
    sal_uInt32 GetMoved() const { return m_nMovedID; }
    bool IsMoved() const { return m_nMovedID != 0; }
    bool CanCombine( const SwRedlineData& rCmp ) const;
    bool CanCombineForAcceptReject( const SwRedlineData& rCmp ) const;

    // ExtraData gets copied, the pointer is therefore not taken over by
    // the RedlineObject
    void SetExtraData( const SwRedlineExtraData* pData );
    const SwRedlineExtraData* GetExtraData() const { return m_pExtraData; }

    // For UI-side pooling of Redline-actions.
    // At the moment only used for Autoformat with Redline.
    // Value != 0 means there can be others!
    sal_uInt16 GetSeqNo() const                     { return m_nSeqNo; }
    void SetSeqNo( sal_uInt16 nNo )                 { m_nSeqNo = nNo; }

    OUString GetDescr() const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

class SAL_DLLPUBLIC_RTTI SwRangeRedline final : public SwPaM
{
    SwRedlineData* m_pRedlineData;
    std::optional<SwNodeIndex> m_oContentSect;
    std::optional<tools::Long> m_oLOKLastNodeTop;
    sal_uInt32 m_nId;
    bool m_bDelLastPara : 1;
    bool m_bIsVisible : 1;

    void MoveToSection();
    void CopyToSection();
    void DelCopyOfSection(size_t nMyPos);
    void MoveFromSection(size_t nMyPos);

public:
    static sal_uInt32 s_nLastId;

    SW_DLLPUBLIC SwRangeRedline( RedlineType eType, const SwPaM& rPam, sal_uInt32 nMoveID = 0 );
    SW_DLLPUBLIC SwRangeRedline( const SwRedlineData& rData, const SwPaM& rPam );
    SwRangeRedline( const SwRedlineData& rData, const SwPosition& rPos );
    // For sw3io: pData is taken over!
    SwRangeRedline(SwRedlineData* pData, const SwPosition& rPos,
               bool bDelLP) :
        SwPaM( rPos ), m_pRedlineData( pData ),
        m_nId( s_nLastId++ ), m_bDelLastPara( bDelLP ), m_bIsVisible( true )
    {
        GetBound().SetRedline(this);
        GetBound(false).SetRedline(this);
    }
    SwRangeRedline( const SwRangeRedline& );
    virtual ~SwRangeRedline() override;

    sal_uInt32 GetId() const { return m_nId; }
    const SwNodeIndex* GetContentIdx() const { return m_oContentSect ? &*m_oContentSect : nullptr; }
    // For Undo.
    void SetContentIdx( const SwNodeIndex& );
    void ClearContentIdx();

    bool IsVisible() const { return m_bIsVisible; }
    bool IsDelLastPara() const { return m_bDelLastPara; }

    void SetStart( const SwPosition& rPos, SwPosition* pSttPtr = nullptr );
    void SetEnd( const SwPosition& rPos, SwPosition* pEndPtr = nullptr );

    /// Do we have a valid selection?
    bool HasValidRange() const;

    SW_DLLPUBLIC const SwRedlineData& GetRedlineData(sal_uInt16 nPos = 0) const;
    bool operator!=( const SwRedlineData& rCmp ) const
        { return *m_pRedlineData != rCmp; }
    void SetAutoFormat() { m_pRedlineData->SetAutoFormat(); }
    bool IsAutoFormat() const { return m_pRedlineData->IsAutoFormat(); }

    sal_uInt16 GetStackCount() const;
    SW_DLLPUBLIC std::size_t GetAuthor( sal_uInt16 nPos = 0) const;
    SW_DLLPUBLIC OUString const & GetAuthorString( sal_uInt16 nPos = 0 ) const;
    sal_uInt32 GetMovedID(sal_uInt16 nPos = 0) const;
    const DateTime& GetTimeStamp(sal_uInt16 nPos = 0) const;
    SW_DLLPUBLIC RedlineType GetType( sal_uInt16 nPos = 0 ) const;
    // text content of the redline is only an annotation placeholder
    // (i.e. a comment, but don't confuse it with comment of the redline)
    bool IsAnnotation() const;
    const OUString& GetComment( sal_uInt16 nPos = 0 ) const;

    void SetComment( const OUString& rS ) { m_pRedlineData->SetComment( rS ); }

    /** ExtraData gets copied, the pointer is therefore not taken over by
     *  the RedLineObject.*/
    void SetExtraData( const SwRedlineExtraData* pData )
        { m_pRedlineData->SetExtraData( pData ); }
    const SwRedlineExtraData* GetExtraData() const
        { return m_pRedlineData->GetExtraData(); }

    // For UI-side pooling of Redline-actions.
    // At the moment only used for Autoformat with Redline.
    // Value != 0 means there can be others!
    sal_uInt16 GetSeqNo() const             { return m_pRedlineData->GetSeqNo(); }
    void SetSeqNo( sal_uInt16 nNo )         { m_pRedlineData->SetSeqNo( nNo ); }

    // At Hide/ShowOriginal the list is traversed two times in order to
    // hide the Del-Redlines via Copy and Delete.
    // Otherwise at Move the attribution would be handled incorrectly.
    // All other callers must always give 0.
    void CallDisplayFunc(size_t nMyPos);
    void Show(sal_uInt16 nLoop , size_t nMyPos, bool bForced = false);
    void Hide(sal_uInt16 nLoop , size_t nMyPos, bool bForced = false);
    void ShowOriginal(sal_uInt16 nLoop, size_t nMyPos, bool bForced = false);

    /// Calculates the intersection with text node number nNdIdx.
    void CalcStartEnd(SwNodeOffset nNdIdx, sal_Int32& rStart, sal_Int32& rEnd) const;

    enum class Invalidation { Add, Remove };
    /// Initiate the layout.
    void InvalidateRange(Invalidation);

    bool IsOwnRedline( const SwRangeRedline& rRedl ) const
        { return GetAuthor() == rRedl.GetAuthor(); }
    bool CanCombine( const SwRangeRedline& rRedl ) const;

    void PushData( const SwRangeRedline& rRedl, bool bOwnAsNext = true );
    bool PopData();
    bool PopAllDataAfter(int depth);

    /**
       Returns textual description of a redline data element of
       this redline.

       The textual description of the selected element contains the
       kind of redline and the possibly shortened text of the redline.

       @return textual description of the selected redline data element

       bSimplified = simplified shortened text to show deletions on margin
     */
    SW_DLLPUBLIC OUString GetDescr(bool bSimplified = false);

    bool operator<( const SwRangeRedline& ) const;
    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    void MaybeNotifyRedlinePositionModification(tools::Long nTop);

    void SetMoved(sal_uInt32 nMoveID = 1) { m_pRedlineData->SetMoved(nMoveID); }
    bool IsMoved() const { return m_pRedlineData->IsMoved(); }
    sal_uInt32 GetMoved(sal_uInt16 nPos = 0) const { return GetRedlineData(nPos).GetMoved(); }
};

void MaybeNotifyRedlineModification(SwRangeRedline& rRedline, SwDoc& rDoc);

/// Base object for 'Redlines' that are not of 'Ranged' type (like table row insert\delete)
class SW_DLLPUBLIC SwExtraRedline
{
private:
    SwExtraRedline(SwExtraRedline const&) = delete;
    SwExtraRedline& operator=(SwExtraRedline const&) = delete;
public:
    SwExtraRedline() = default;
    virtual ~SwExtraRedline();
};

/// Redline that holds information about a table-row that had some change
class SW_DLLPUBLIC SwTableRowRedline final : public SwExtraRedline
{
private:
    SwRedlineData m_aRedlineData;
    const SwTableLine& m_rTableLine;

public:
    SwTableRowRedline(const SwRedlineData& rData, const SwTableLine& rTableLine);
    virtual ~SwTableRowRedline() override;

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
class SW_DLLPUBLIC SwTableCellRedline final : public SwExtraRedline
{
private:
    SwRedlineData m_aRedlineData;
    const SwTableBox& m_rTableBox;

public:
    SwTableCellRedline(const SwRedlineData& rData, const SwTableBox& rTableBox);
    virtual ~SwTableCellRedline() override;

    /** ExtraData gets copied, the pointer is therefore not taken over by
     *  the RedLineObject.*/
    void SetExtraData( const SwRedlineExtraData* pData )
        { m_aRedlineData.SetExtraData( pData ); }
    const SwTableBox& GetTableBox() const
        { return m_rTableBox; }
    const SwRedlineData& GetRedlineData() const
        { return m_aRedlineData; }
};

class SW_DLLPUBLIC SwRedlineHint final : public SfxHint
{
};


namespace sw {

std::vector<std::unique_ptr<SwRangeRedline>> GetAllValidRanges(std::unique_ptr<SwRangeRedline> p);

} // namespace sw

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
