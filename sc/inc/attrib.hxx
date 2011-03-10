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

#ifndef SC_SCATTR_HXX
#define SC_SCATTR_HXX

#include <svl/poolitem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include "scdllapi.h"
#include "global.hxx"
#include "address.hxx"

//------------------------------------------------------------------------

                                        // flags for cells hidden by merge
                                        // and control for auto filter
#define SC_MF_HOR               0x0001
#define SC_MF_VER               0x0002
#define SC_MF_AUTO              0x0004  /// autofilter arrow
#define SC_MF_BUTTON            0x0008  /// field button for datapilot
#define SC_MF_SCENARIO          0x0010
#define SC_MF_BUTTON_POPUP      0x0020  /// dp button with popup arrow
#define SC_MF_HIDDEN_MEMBER     0x0040  /// dp field button with presence of hidden member
#define SC_MF_DP_TABLE          0x0080  /// dp table output

#define SC_MF_ALL               0x00FF


class EditTextObject;
class SvxBorderLine;

BOOL SC_DLLPUBLIC ScHasPriority( const SvxBorderLine* pThis, const SvxBorderLine* pOther );

//------------------------------------------------------------------------

class SC_DLLPUBLIC ScMergeAttr: public SfxPoolItem
{
    SCsCOL      nColMerge;
    SCsROW      nRowMerge;
public:
                TYPEINFO();
                ScMergeAttr();
                ScMergeAttr( SCsCOL nCol, SCsROW nRow = 0);
                ScMergeAttr( const ScMergeAttr& );
                ~ScMergeAttr();

    virtual String              GetValueText() const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;

            SCsCOL          GetColMerge() const {return nColMerge; }
            SCsROW          GetRowMerge() const {return nRowMerge; }

            BOOL            IsMerged() const { return nColMerge>1 || nRowMerge>1; }

    inline  ScMergeAttr& operator=(const ScMergeAttr& rMerge)
            {
                nColMerge = rMerge.nColMerge;
                nRowMerge = rMerge.nRowMerge;
                return *this;
            }
};

//------------------------------------------------------------------------

class SC_DLLPUBLIC ScMergeFlagAttr: public SfxInt16Item
{
public:
            ScMergeFlagAttr();
            ScMergeFlagAttr(INT16 nFlags);
            ~ScMergeFlagAttr();

    BOOL    IsHorOverlapped() const     { return ( GetValue() & SC_MF_HOR ) != 0;  }
    BOOL    IsVerOverlapped() const     { return ( GetValue() & SC_MF_VER ) != 0;  }
    BOOL    IsOverlapped() const        { return ( GetValue() & ( SC_MF_HOR | SC_MF_VER ) ) != 0; }

    BOOL    HasAutoFilter() const       { return ( GetValue() & SC_MF_AUTO ) != 0; }
    BOOL    HasButton() const           { return ( GetValue() & SC_MF_BUTTON ) != 0; }
    bool    HasDPTable() const          { return ( GetValue() & SC_MF_DP_TABLE ) != 0; }

    BOOL    IsScenario() const          { return ( GetValue() & SC_MF_SCENARIO ) != 0; }
};

//------------------------------------------------------------------------
class SC_DLLPUBLIC ScProtectionAttr: public SfxPoolItem
{
    BOOL        bProtection;    // protect cell
    BOOL        bHideFormula;   // hide formula
    BOOL        bHideCell;      // hide cell
    BOOL        bHidePrint;     // don't print cell
public:
                            TYPEINFO();
                            ScProtectionAttr();
                            ScProtectionAttr(   BOOL bProtect,
                                                BOOL bHFormula = FALSE,
                                                BOOL bHCell = FALSE,
                                                BOOL bHPrint = FALSE);
                            ScProtectionAttr( const ScProtectionAttr& );
                            ~ScProtectionAttr();

    virtual String              GetValueText() const;
    virtual SfxItemPresentation GetPresentation(
                                    SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String& rText,
                                    const IntlWrapper* pIntl = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

            BOOL            GetProtection() const { return bProtection; }
            BOOL            SetProtection( BOOL bProtect);
            BOOL            GetHideFormula() const { return bHideFormula; }
            BOOL            SetHideFormula( BOOL bHFormula);
            BOOL            GetHideCell() const { return bHideCell; }
            BOOL            SetHideCell( BOOL bHCell);
            BOOL            GetHidePrint() const { return bHidePrint; }
            BOOL            SetHidePrint( BOOL bHPrint);
    inline  ScProtectionAttr& operator=(const ScProtectionAttr& rProtection)
            {
                bProtection = rProtection.bProtection;
                bHideFormula = rProtection.bHideFormula;
                bHideCell = rProtection.bHideCell;
                bHidePrint = rProtection.bHidePrint;
                return *this;
            }
};


//----------------------------------------------------------------------------
// ScRangeItem: manages an area of a table

#define SCR_INVALID     0x01
#define SCR_ALLTABS     0x02
#define SCR_TONEWTAB    0x04

class ScRangeItem : public SfxPoolItem
{
public:
            TYPEINFO();

            inline  ScRangeItem( const USHORT nWhich );
            inline  ScRangeItem( const USHORT   nWhich,
                                 const ScRange& rRange,
                                 const USHORT   nNewFlags = 0 );
            inline  ScRangeItem( const ScRangeItem& rCpy );

    inline ScRangeItem& operator=( const ScRangeItem &rCpy );

    // "pure virtual methods" from SfxPoolItem
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 String &rText,
                                                 const IntlWrapper* pIntl = 0 ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;

    const ScRange&  GetRange() const                { return aRange;  }
    void            SetRange( const ScRange& rNew ) { aRange = rNew; }

    USHORT          GetFlags() const                { return nFlags;  }
    void            SetFlags( USHORT nNew )         { nFlags = nNew; }

private:
    ScRange aRange;
    USHORT  nFlags;
};

inline ScRangeItem::ScRangeItem( const USHORT nWhichP )
    :   SfxPoolItem( nWhichP ), nFlags( SCR_INVALID ) // == invalid area
{
}

inline ScRangeItem::ScRangeItem( const USHORT   nWhichP,
                                 const ScRange& rRange,
                                 const USHORT   nNew )
    : SfxPoolItem( nWhichP ), aRange( rRange ), nFlags( nNew )
{
}

inline ScRangeItem::ScRangeItem( const ScRangeItem& rCpy )
    : SfxPoolItem( rCpy.Which() ), aRange( rCpy.aRange ), nFlags( rCpy.nFlags )
{}

inline ScRangeItem& ScRangeItem::operator=( const ScRangeItem &rCpy )
{
    aRange = rCpy.aRange;
    return *this;
}

//----------------------------------------------------------------------------
// ScTableListItem: manages a list of tables
//----------------------------------------------------------------------------
class ScTableListItem : public SfxPoolItem
{
public:
    TYPEINFO();

    inline  ScTableListItem( const USHORT nWhich );
            ScTableListItem( const ScTableListItem& rCpy );
            ~ScTableListItem();

    ScTableListItem& operator=( const ScTableListItem &rCpy );

    // "pure virtual Methoden" from SfxPoolItem
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 String &rText,
                                                 const IntlWrapper* pIntl = 0 ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;

public:
    USHORT  nCount;
    SCTAB*  pTabArr;
};

inline ScTableListItem::ScTableListItem( const USHORT nWhichP )
    : SfxPoolItem(nWhichP), nCount(0), pTabArr(NULL)
{}

//----------------------------------------------------------------------------
// page format item: contents of header and footer

#define SC_HF_LEFTAREA   1
#define SC_HF_CENTERAREA 2
#define SC_HF_RIGHTAREA  3

class SC_DLLPUBLIC ScPageHFItem : public SfxPoolItem
{
    EditTextObject* pLeftArea;
    EditTextObject* pCenterArea;
    EditTextObject* pRightArea;

public:
                TYPEINFO();
                ScPageHFItem( USHORT nWhich );
                ScPageHFItem( const ScPageHFItem& rItem );
                ~ScPageHFItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    const EditTextObject* GetLeftArea() const       { return pLeftArea; }
    const EditTextObject* GetCenterArea() const     { return pCenterArea; }
    const EditTextObject* GetRightArea() const      { return pRightArea; }

    void SetLeftArea( const EditTextObject& rNew );
    void SetCenterArea( const EditTextObject& rNew );
    void SetRightArea( const EditTextObject& rNew );

    // Set method with pointer assignment, nArea see defines above
    void SetArea( EditTextObject *pNew, int nArea );
};


//----------------------------------------------------------------------------
// page format item: contents of header and footer

class SC_DLLPUBLIC ScViewObjectModeItem: public SfxEnumItem
{
public:
                TYPEINFO();

                ScViewObjectModeItem( USHORT nWhich );
                ScViewObjectModeItem( USHORT nWhich, ScVObjMode eMode );
                ~ScViewObjectModeItem();

    virtual USHORT              GetValueCount() const;
    virtual String              GetValueText( USHORT nVal ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*        Create(SvStream &, USHORT) const;
    virtual USHORT              GetVersion( USHORT nFileVersion ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 String& rText,
                                                 const IntlWrapper* pIntl = 0 ) const;
};

//----------------------------------------------------------------------------
//

class ScDoubleItem : public SfxPoolItem
{
public:
                TYPEINFO();
                ScDoubleItem( USHORT nWhich, double nVal=0 );
                ScDoubleItem( const ScDoubleItem& rItem );
                ~ScDoubleItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;

    double GetValue() const     { return nValue; }

    void SetValue( const double nVal ) { nValue = nVal;}

private:
    double  nValue;
};


// ============================================================================

/** Member ID for "page scale to width" value in QueryValue() and PutValue(). */
const BYTE SC_MID_PAGE_SCALETO_WIDTH    = 1;
/** Member ID for "page scale to height" value in QueryValue() and PutValue(). */
const BYTE SC_MID_PAGE_SCALETO_HEIGHT   = 2;


/** Contains the "scale to width/height" attribute in page styles. */
class SC_DLLPUBLIC ScPageScaleToItem : public SfxPoolItem
{
public:
                                TYPEINFO();

    /** Default c'tor sets the width and height to 0. */
    explicit                    ScPageScaleToItem();
    explicit                    ScPageScaleToItem( sal_uInt16 nWidth, sal_uInt16 nHeight );

    virtual                     ~ScPageScaleToItem();

    virtual ScPageScaleToItem*  Clone( SfxItemPool* = 0 ) const;

    virtual int                 operator==( const SfxPoolItem& rCmp ) const;

    inline sal_uInt16           GetWidth() const { return mnWidth; }
    inline sal_uInt16           GetHeight() const { return mnHeight; }
    inline bool                 IsValid() const { return mnWidth || mnHeight; }

    inline void                 SetWidth( sal_uInt16 nWidth ) { mnWidth = nWidth; }
    inline void                 SetHeight( sal_uInt16 nHeight ) { mnHeight = nHeight; }
    inline void                 Set( sal_uInt16 nWidth, sal_uInt16 nHeight )
                                    { mnWidth = nWidth; mnHeight = nHeight; }
    inline void                 SetInvalid() { mnWidth = mnHeight = 0; }

    virtual SfxItemPresentation GetPresentation(
                                    SfxItemPresentation ePresentation,
                                    SfxMapUnit, SfxMapUnit,
                                    XubString& rText,
                                    const IntlWrapper* = 0 ) const;

    virtual bool                QueryValue( ::com::sun::star::uno::Any& rAny, BYTE nMemberId = 0 ) const;
    virtual bool                PutValue( const ::com::sun::star::uno::Any& rAny, BYTE nMemberId = 0 );

private:
    sal_uInt16                  mnWidth;
    sal_uInt16                  mnHeight;
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
