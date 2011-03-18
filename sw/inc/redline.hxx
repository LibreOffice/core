/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _REDLINE_HXX
#define _REDLINE_HXX

#include <tools/datetime.hxx>
#include <tools/string.hxx>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#include <pam.hxx>

#include <IDocumentRedlineAccess.hxx>

#include <svl/smplhint.hxx>

class SfxItemSet;
class SwView;

class SwRedlineExtraData
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
    virtual int operator == ( const SwRedlineExtraData& ) const;
};

class SwRedlineExtraData_FmtColl : public SwRedlineExtraData
{
    String sFmtNm;
    SfxItemSet* pSet;
    sal_uInt16 nPoolId;
public:
    SwRedlineExtraData_FmtColl( const String& rColl, sal_uInt16 nPoolFmtId,
                                const SfxItemSet* pSet = 0 );
    virtual ~SwRedlineExtraData_FmtColl();
    virtual SwRedlineExtraData* CreateNew() const;
    virtual void Reject( SwPaM& rPam ) const;
    virtual int operator == ( const SwRedlineExtraData& ) const;

    void SetItemSet( const SfxItemSet& rSet );
};

class SwRedlineExtraData_Format : public SwRedlineExtraData
{
    SvUShorts aWhichIds;

    SwRedlineExtraData_Format( const SwRedlineExtraData_Format& rCpy );

public:
    SwRedlineExtraData_Format( const SfxItemSet& rSet );
    virtual ~SwRedlineExtraData_Format();
    virtual SwRedlineExtraData* CreateNew() const;
    virtual void Reject( SwPaM& rPam ) const;
    virtual int operator == ( const SwRedlineExtraData& ) const;
};


class SW_DLLPUBLIC SwRedlineData
{
    friend class SwRedline;
    SwRedlineData* pNext;       // Verweis auf weitere Daten
    SwRedlineExtraData* pExtraData;

    String sComment;
    DateTime aStamp;
    RedlineType_t eType;
    sal_uInt16 nAuthor, nSeqNo;

public:
    SwRedlineData( RedlineType_t eT, sal_uInt16 nAut );
    SwRedlineData( const SwRedlineData& rCpy, sal_Bool bCpyNext = sal_True );

    // fuer sw3io: pNext/pExtraData gehen in eigenen Besitz ueber!
    SwRedlineData( RedlineType_t eT, sal_uInt16 nAut, const DateTime& rDT,
                   const String& rCmnt, SwRedlineData* pNxt,
                    SwRedlineExtraData* pExtraData = 0 );

    ~SwRedlineData();

    int operator==( const SwRedlineData& rCmp ) const
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
    int operator!=( const SwRedlineData& rCmp ) const
        {   return !operator==( rCmp ); }

    RedlineType_t GetType() const
  { return ((RedlineType_t)(eType & nsRedlineType_t::REDLINE_NO_FLAG_MASK)); }
    RedlineType_t GetRealType() const { return eType; }
    sal_uInt16 GetAuthor() const                { return nAuthor; }
    const String& GetComment() const        { return sComment; }
    const DateTime& GetTimeStamp() const    { return aStamp; }
    inline const SwRedlineData* Next() const{ return pNext; }

    void SetComment( const String& rS )     { sComment = rS; }
    void SetTimeStamp( const DateTime& rDT ) { aStamp = rDT; }

    void SetAutoFmtFlag()
  { eType = (RedlineType_t)(eType | nsRedlineType_t::REDLINE_FORM_AUTOFMT); }
    int CanCombine( const SwRedlineData& rCmp ) const
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

    // ExtraData wird kopiert, der Pointer geht also NICHT in den Besitz
    // des RedlineObjectes!
    void SetExtraData( const SwRedlineExtraData* pData );
    const SwRedlineExtraData* GetExtraData() const { return pExtraData; }

    // fuers UI-seitige zusammenfassen von Redline-Actionen. Wird z.Z. nur
    // fuers Autoformat mit Redline benoetigt. Der Wert != 0 bedeutet dabei,
    // das es noch weitere geben kann!
    sal_uInt16 GetSeqNo() const                     { return nSeqNo; }
    void SetSeqNo( sal_uInt16 nNo )                 { nSeqNo = nNo; }

    String GetDescr() const;
};


class SW_DLLPUBLIC SwRedline : public SwPaM
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
    SwRedline( RedlineType_t eType, const SwPaM& rPam );
    SwRedline( const SwRedlineData& rData, const SwPaM& rPam );
    SwRedline( const SwRedlineData& rData, const SwPosition& rPos );
    // fuer sw3io: pData geht in eigenen Besitz ueber!
    SwRedline(SwRedlineData* pData, const SwPosition& rPos, sal_Bool bVsbl,
               sal_Bool bDelLP, sal_Bool bIsPD) :
        SwPaM( rPos ), pRedlineData( pData ), pCntntSect( 0 ),
        bDelLastPara( bDelLP ), bIsLastParaDelete( bIsPD ), bIsVisible( bVsbl )
    {}
    SwRedline( const SwRedline& );
    virtual ~SwRedline();

    SwNodeIndex* GetContentIdx() const { return pCntntSect; }
    // fuers Undo
    void SetContentIdx( const SwNodeIndex* );

    sal_Bool IsVisible() const { return bIsVisible; }
    sal_Bool IsDelLastPara() const { return bDelLastPara; }

    // das sal_Bool besagt, ob nach dem setzen der Pos kein Bereich mehr
    // aufgespannt ist. -> sal_True, ansonten Bereich und sal_False
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
    // liegt eine gueltige Selektion vor?
    sal_Bool HasValidRange() const;

    const SwRedlineData& GetRedlineData(sal_uInt16 nPos = 0) const;
    int operator==( const SwRedlineData& rCmp ) const
        { return *pRedlineData == rCmp; }
    int operator!=( const SwRedlineData& rCmp ) const
        { return *pRedlineData != rCmp; }
    void SetAutoFmtFlag()               { pRedlineData->SetAutoFmtFlag(); }

    sal_uInt16 GetStackCount() const;
    sal_uInt16 GetAuthor( sal_uInt16 nPos = 0) const;
    const String& GetAuthorString( sal_uInt16 nPos = 0 ) const;
    const DateTime& GetTimeStamp( sal_uInt16 nPos = 0) const;
    RedlineType_t GetRealType( sal_uInt16 nPos = 0 ) const;
    RedlineType_t GetType( sal_uInt16 nPos = 0) const
  { return ( (RedlineType_t)(GetRealType( nPos ) & nsRedlineType_t::REDLINE_NO_FLAG_MASK)); }
    const String& GetComment( sal_uInt16 nPos = 0 ) const;

    void SetComment( const String& rS ) { pRedlineData->SetComment( rS ); }

    // ExtraData wird kopiert, der Pointer geht also NICHT in den Besitz
    // des RedlineObjectes!
    void SetExtraData( const SwRedlineExtraData* pData )
        { pRedlineData->SetExtraData( pData ); }
    const SwRedlineExtraData* GetExtraData() const
        { return pRedlineData->GetExtraData(); }

    // fuers UI-seitige zusammenfassen von Redline-Actionen. Wird z.Z. nur
    // fuers Autoformat mit Redline benoetigt. Der Wert != 0 bedeutet dabei,
    // das es noch weitere geben kann!
    sal_uInt16 GetSeqNo() const             { return pRedlineData->GetSeqNo(); }
    void SetSeqNo( sal_uInt16 nNo )         { pRedlineData->SetSeqNo( nNo ); }

    // Beim Hide/ShowOriginal wird 2 mal ueber die Liste gelaufen, damit
    //  die Del-Redlines per Copy und Delete versteckt werden. Beim Move
    //  wird sonst die Attributierung falsch behandelt.
    // Alle anderen Aufrufer muessen immer 0 angeben.
    void CallDisplayFunc( sal_uInt16 nLoop = 0 );
    void Show( sal_uInt16 nLoop = 0 );
    void Hide( sal_uInt16 nLoop = 0 );
    void ShowOriginal( sal_uInt16 nLoop = 0 );

    // calculates the intersection with text node number nNdIdx
    void CalcStartEnd( sal_uLong nNdIdx, sal_uInt16& nStart, sal_uInt16& nEnd ) const;

    void InvalidateRange();     // das Layout anstossen

    sal_Bool IsOwnRedline( const SwRedline& rRedl ) const
        { return GetAuthor() == rRedl.GetAuthor(); }
    sal_Bool CanCombine( const SwRedline& rRedl ) const;

    void PushData( const SwRedline& rRedl, sal_Bool bOwnAsNext = sal_True );
    sal_Bool PopData();

    /**
       Returns textual description of this a redline data element of
       this redline.

       @param nPos index of the redline data element to describe

       The textual description of the selected element contains the
       kind of redline and the possibly shortened text of the redline.

       @return textual description of the selected redline data element
     */
    String GetDescr(sal_uInt16 nPos = 0);

    int operator==( const SwRedline& ) const;
    int operator<( const SwRedline& ) const;
};

class SW_DLLPUBLIC SwRedlineHint : public SfxHint
{
#define SWREDLINE_INSERTED  1
#define SWREDLINE_REMOVED   2
#define SWREDLINE_FOCUS     3
#define SWREDLINE_CHANGED   4
#define SWREDLINE_LANGUAGE  5

    const SwRedline* pRedline;
    sal_Int16 nWhich;
    const SwView* pView;

public:
    SwRedlineHint( const SwRedline* p, sal_Int16 n, const SwView* pV = 0)
        : pRedline(p)
        , nWhich(n)
        , pView(pV)
        {}

    TYPEINFO();
    const SwRedline* GetRedline() const { return pRedline; }
    sal_Int16 Which() const { return nWhich; }
        const SwView* GetView() const { return pView; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
