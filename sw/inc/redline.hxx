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

#include "pam.hxx"

#include "IDocumentRedlineAccess.hxx"

#include <cstddef>
#include <memory>
#include <vector>
#include <boost/optional.hpp>


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

class SwRedlineExtraData_FormatColl : public SwRedlineExtraData
{
    OUString m_sFormatNm;
    std::unique_ptr<SfxItemSet> m_pSet;
    sal_uInt16 m_nPoolId;
public:
    SwRedlineExtraData_FormatColl( const OUString& rColl, sal_uInt16 nPoolFormatId,
                                const SfxItemSet* pSet = nullptr );
    virtual ~SwRedlineExtraData_FormatColl() override;
    virtual SwRedlineExtraData* CreateNew() const override;
    virtual void Reject( SwPaM& rPam ) const override;
    virtual bool operator == ( const SwRedlineExtraData& ) const override;

    void SetItemSet( const SfxItemSet& rSet );
};

class SwRedlineExtraData_Format : public SwRedlineExtraData
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

/*
 * This class is used to store 'redline' data regarding formatting changes,
 * e.g. - a text portion *was* italic and now is not italic,
 * e.g. - a text portion got a highlight to it
 *
 * The way the information is stored is in an 'SfxItemSet' that holds all
 * the WhichIds with their values.
 */
class SW_DLLPUBLIC SwRedlineExtraData_FormattingChanges : public SwRedlineExtraData
{
    std::unique_ptr<SfxItemSet> m_pSet;

    SwRedlineExtraData_FormattingChanges( const SwRedlineExtraData_FormattingChanges& rCpy );

public:
    SwRedlineExtraData_FormattingChanges( const SfxItemSet* pItemSet );
    virtual ~SwRedlineExtraData_FormattingChanges() override;
    virtual SwRedlineExtraData* CreateNew() const override;
    virtual void Reject( SwPaM& rPam ) const override;
    virtual bool operator == ( const SwRedlineExtraData& ) const override;
    SfxItemSet* GetItemSet( ) const { return m_pSet.get(); }
};

class SW_DLLPUBLIC SwRedlineData
{
    friend class SwRangeRedline;
    SwRedlineData* m_pNext;       // Points to other data.
    SwRedlineExtraData* m_pExtraData;

    OUString m_sComment;
    DateTime m_aStamp;
    RedlineType_t m_eType;
    std::size_t const m_nAuthor;
    sal_uInt16 m_nSeqNo;

public:
    SwRedlineData( RedlineType_t eT, std::size_t nAut );
    SwRedlineData( const SwRedlineData& rCpy, bool bCpyNext = true );

    // For sw3io: pNext/pExtraData are taken over.
    SwRedlineData( RedlineType_t eT, std::size_t nAut, const DateTime& rDT,
                   const OUString& rCmnt, SwRedlineData* pNxt );

    ~SwRedlineData();

    bool operator==( const SwRedlineData& rCmp ) const
        {
            return m_nAuthor == rCmp.m_nAuthor &&
                    m_eType == rCmp.m_eType &&
                    m_sComment == rCmp.m_sComment &&
                    (( !m_pNext && !rCmp.m_pNext ) ||
                        ( m_pNext && rCmp.m_pNext && *m_pNext == *rCmp.m_pNext )) &&
                    (( !m_pExtraData && !rCmp.m_pExtraData ) ||
                        ( m_pExtraData && rCmp.m_pExtraData &&
                            *m_pExtraData == *rCmp.m_pExtraData ));
        }
    bool operator!=( const SwRedlineData& rCmp ) const
        {   return !operator==( rCmp ); }

    RedlineType_t GetType() const
        { return static_cast<RedlineType_t>(m_eType & nsRedlineType_t::REDLINE_NO_FLAG_MASK); }

    std::size_t GetAuthor() const                { return m_nAuthor; }
    const OUString& GetComment() const        { return m_sComment; }
    const DateTime& GetTimeStamp() const    { return m_aStamp; }
    const SwRedlineData* Next() const{ return m_pNext; }

    void SetComment( const OUString& rS )     { m_sComment = rS; }
    void SetTimeStamp( const DateTime& rDT ) { m_aStamp = rDT; }

    void SetAutoFormatFlag()
        { m_eType = static_cast<RedlineType_t>(m_eType | nsRedlineType_t::REDLINE_FORM_AUTOFMT); }
    bool CanCombine( const SwRedlineData& rCmp ) const;

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
};

class SW_DLLPUBLIC SwRangeRedline : public SwPaM
{
    SwRedlineData* m_pRedlineData;
    SwNodeIndex* m_pContentSect;
    bool m_bDelLastPara : 1;
    bool m_bIsVisible : 1;
    sal_uInt32 const m_nId;

    boost::optional<long> m_oLOKLastNodeTop;

    void MoveToSection();
    void CopyToSection();
    void DelCopyOfSection(size_t nMyPos);
    void MoveFromSection(size_t nMyPos);

public:
    static sal_uInt32 m_nLastId;

    SwRangeRedline( RedlineType_t eType, const SwPaM& rPam );
    SwRangeRedline( const SwRedlineData& rData, const SwPaM& rPam );
    SwRangeRedline( const SwRedlineData& rData, const SwPosition& rPos );
    // For sw3io: pData is taken over!
    SwRangeRedline(SwRedlineData* pData, const SwPosition& rPos,
               bool bDelLP) :
        SwPaM( rPos ), m_pRedlineData( pData ), m_pContentSect( nullptr ),
        m_bDelLastPara( bDelLP ), m_bIsVisible( true ), m_nId( m_nLastId++ )
    {}
    SwRangeRedline( const SwRangeRedline& );
    virtual ~SwRangeRedline() override;

    sal_uInt32 GetId() const { return m_nId; }
    SwNodeIndex* GetContentIdx() const { return m_pContentSect; }
    // For Undo.
    void SetContentIdx( const SwNodeIndex* );

    bool IsVisible() const { return m_bIsVisible; }
    bool IsDelLastPara() const { return m_bDelLastPara; }

    void SetStart( const SwPosition& rPos, SwPosition* pSttPtr = nullptr );
    void SetEnd( const SwPosition& rPos, SwPosition* pEndPtr = nullptr );

    /// Do we have a valid selection?
    bool HasValidRange() const;

    const SwRedlineData& GetRedlineData(sal_uInt16 nPos = 0) const;
    bool operator!=( const SwRedlineData& rCmp ) const
        { return *m_pRedlineData != rCmp; }
    void SetAutoFormatFlag()               { m_pRedlineData->SetAutoFormatFlag(); }

    sal_uInt16 GetStackCount() const;
    std::size_t GetAuthor( sal_uInt16 nPos = 0) const;
    OUString const & GetAuthorString( sal_uInt16 nPos = 0 ) const;
    const DateTime& GetTimeStamp( sal_uInt16 nPos = 0) const;
    RedlineType_t GetRealType( sal_uInt16 nPos = 0 ) const;
    RedlineType_t GetType( sal_uInt16 nPos = 0) const
        { return static_cast<RedlineType_t>(GetRealType( nPos ) & nsRedlineType_t::REDLINE_NO_FLAG_MASK); }
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
    void Show(sal_uInt16 nLoop , size_t nMyPos);
    void Hide(sal_uInt16 nLoop , size_t nMyPos);
    void ShowOriginal(sal_uInt16 nLoop, size_t nMyPos);

    /// Calculates the intersection with text node number nNdIdx.
    void CalcStartEnd(sal_uLong nNdIdx, sal_Int32& rStart, sal_Int32& rEnd) const;

    enum class Invalidation { Add, Remove };
    /// Initiate the layout.
    void InvalidateRange(Invalidation);

    bool IsOwnRedline( const SwRangeRedline& rRedl ) const
        { return GetAuthor() == rRedl.GetAuthor(); }
    bool CanCombine( const SwRangeRedline& rRedl ) const;

    void PushData( const SwRangeRedline& rRedl, bool bOwnAsNext = true );
    bool PopData();

    /**
       Returns textual description of a redline data element of
       this redline.

       The textual description of the selected element contains the
       kind of redline and the possibly shortened text of the redline.

       @return textual description of the selected redline data element
     */
    OUString GetDescr();

    bool operator<( const SwRangeRedline& ) const;
    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    void MaybeNotifyRedlinePositionModification(long nTop);
};

SW_DLLPUBLIC void MaybeNotifyRedlineModification(SwRangeRedline* pRedline, SwDoc* pDoc);

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
class SW_DLLPUBLIC SwTableRowRedline : public SwExtraRedline
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
class SW_DLLPUBLIC SwTableCellRedline : public SwExtraRedline
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

class SW_DLLPUBLIC SwRedlineHint : public SfxHint
{
};


namespace sw {

std::vector<SwRangeRedline*> GetAllValidRanges(std::unique_ptr<SwRangeRedline> p);

} // namespace sw

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
