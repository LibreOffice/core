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

#ifndef INCLUDED_SC_INC_ATTRIB_HXX
#define INCLUDED_SC_INC_ATTRIB_HXX

#include <svl/poolitem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <o3tl/typed_flags_set.hxx>
#include "scdllapi.h"
#include "global.hxx"
#include "address.hxx"

// flags for cells hidden by merge
// and control for auto filter
enum class ScMF {
    NONE           = 0x0000,
    Hor            = 0x0001,
    Ver            = 0x0002,
    Auto           = 0x0004,  /// autofilter arrow
    Button         = 0x0008,  /// field button for datapilot
    Scenario       = 0x0010,
    ButtonPopup    = 0x0020,  /// dp button with popup arrow
    HiddenMember   = 0x0040,  /// dp field button with presence of hidden member
    DpTable        = 0x0080,  /// dp table output
    All            = 0x00FF
};
namespace o3tl {
    template<> struct typed_flags<ScMF> : is_typed_flags<ScMF, 0xff> {};
}

class EditTextObject;
namespace editeng { class SvxBorderLine; }

bool SC_DLLPUBLIC ScHasPriority( const ::editeng::SvxBorderLine* pThis, const ::editeng::SvxBorderLine* pOther );

class SC_DLLPUBLIC ScMergeAttr: public SfxPoolItem
{
    SCsCOL      nColMerge;
    SCsROW      nRowMerge;
public:
                static SfxPoolItem* CreateDefault();
                ScMergeAttr();
                ScMergeAttr( SCsCOL nCol, SCsROW nRow = 0);
                ScMergeAttr( const ScMergeAttr& );
                virtual ~ScMergeAttr();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const override;

            SCsCOL          GetColMerge() const {return nColMerge; }
            SCsROW          GetRowMerge() const {return nRowMerge; }

            bool            IsMerged() const { return nColMerge>1 || nRowMerge>1; }

    inline  ScMergeAttr& operator=(const ScMergeAttr& rMerge)
            {
                nColMerge = rMerge.nColMerge;
                nRowMerge = rMerge.nRowMerge;
                return *this;
            }
};

class SC_DLLPUBLIC ScMergeFlagAttr: public SfxInt16Item
{
public:
            ScMergeFlagAttr();
            ScMergeFlagAttr(ScMF nFlags);
            virtual ~ScMergeFlagAttr();

    SfxPoolItem * Clone(SfxItemPool * pPool = nullptr) const override;

    ScMF    GetValue() const { return (ScMF) SfxInt16Item::GetValue(); }

    bool    IsHorOverlapped() const     { return bool( GetValue() & ScMF::Hor );  }
    bool    IsVerOverlapped() const     { return bool( GetValue() & ScMF::Ver );  }
    bool    IsOverlapped() const        { return bool( GetValue() & ( ScMF::Hor | ScMF::Ver ) ); }

    bool    HasAutoFilter() const       { return bool( GetValue() & ScMF::Auto ); }

    bool    IsScenario() const          { return bool( GetValue() & ScMF::Scenario ); }

    bool    HasPivotButton() const;
    bool    HasPivotPopupButton() const;
};

class SC_DLLPUBLIC ScProtectionAttr: public SfxPoolItem
{
    bool        bProtection;    ///< protect cell
    bool        bHideFormula;   ///< hide formula
    bool        bHideCell;      ///< hide cell
    bool        bHidePrint;     ///< don't print cell
public:
                            static SfxPoolItem* CreateDefault();
                            ScProtectionAttr();
                            ScProtectionAttr(   bool bProtect,
                                                bool bHFormula = false,
                                                bool bHCell = false,
                                                bool bHPrint = false);
                            ScProtectionAttr( const ScProtectionAttr& );
                            virtual ~ScProtectionAttr();

    OUString     GetValueText() const;
    virtual bool GetPresentation(
                                    SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString& rText,
                                    const IntlWrapper* pIntl = nullptr ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

            bool            GetProtection() const { return bProtection; }
            void            SetProtection( bool bProtect);
            bool            GetHideFormula() const { return bHideFormula; }
            void            SetHideFormula( bool bHFormula);
            bool            GetHideCell() const { return bHideCell; }
            void            SetHideCell( bool bHCell);
            bool            GetHidePrint() const { return bHidePrint; }
            void            SetHidePrint( bool bHPrint);
    inline  ScProtectionAttr& operator=(const ScProtectionAttr& rProtection)
            {
                bProtection = rProtection.bProtection;
                bHideFormula = rProtection.bHideFormula;
                bHideCell = rProtection.bHideCell;
                bHidePrint = rProtection.bHidePrint;
                return *this;
            }
};

// ScRangeItem: manages an area of a table
class ScRangeItem : public SfxPoolItem
{
public:
            static SfxPoolItem* CreateDefault();

            inline  ScRangeItem( const sal_uInt16 nWhich );
            inline  ScRangeItem( const ScRangeItem& rCpy );

    inline ScRangeItem& operator=( const ScRangeItem &rCpy );

    // "pure virtual methods" from SfxPoolItem
    virtual bool                operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 OUString &rText,
                                                 const IntlWrapper* pIntl = nullptr ) const override;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = nullptr ) const override;

private:
    ScRange aRange;
};

inline ScRangeItem::ScRangeItem( const sal_uInt16 nWhichP )
    :   SfxPoolItem( nWhichP )
{
}

inline ScRangeItem::ScRangeItem( const ScRangeItem& rCpy )
    : SfxPoolItem( rCpy.Which() ), aRange( rCpy.aRange )
{}

inline ScRangeItem& ScRangeItem::operator=( const ScRangeItem &rCpy )
{
    aRange = rCpy.aRange;
    return *this;
}

// ScTableListItem: manages a list of tables

class ScTableListItem : public SfxPoolItem
{
public:
    static SfxPoolItem* CreateDefault();

    inline  ScTableListItem( const sal_uInt16 nWhich );
            ScTableListItem( const ScTableListItem& rCpy );
            virtual ~ScTableListItem();

    ScTableListItem& operator=( const ScTableListItem &rCpy );

    // "pure virtual Methoden" from SfxPoolItem
    virtual bool                operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 OUString &rText,
                                                 const IntlWrapper* pIntl = nullptr ) const override;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = nullptr ) const override;

public:
    sal_uInt16  nCount;
    SCTAB*  pTabArr;
};

inline ScTableListItem::ScTableListItem( const sal_uInt16 nWhichP )
    : SfxPoolItem(nWhichP), nCount(0), pTabArr(nullptr)
{}

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
                ScPageHFItem( sal_uInt16 nWhich );
                ScPageHFItem( const ScPageHFItem& rItem );
                virtual ~ScPageHFItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const EditTextObject* GetLeftArea() const       { return pLeftArea; }
    const EditTextObject* GetCenterArea() const     { return pCenterArea; }
    const EditTextObject* GetRightArea() const      { return pRightArea; }

    void SetLeftArea( const EditTextObject& rNew );
    void SetCenterArea( const EditTextObject& rNew );
    void SetRightArea( const EditTextObject& rNew );

    // Set method with pointer assignment, nArea see defines above
    void SetArea( EditTextObject *pNew, int nArea );
};

// page format item: contents of header and footer

class SC_DLLPUBLIC ScViewObjectModeItem: public SfxEnumItem
{
public:
                static SfxPoolItem* CreateDefault();

                ScViewObjectModeItem( sal_uInt16 nWhich );
                ScViewObjectModeItem( sal_uInt16 nWhich, ScVObjMode eMode );
                virtual ~ScViewObjectModeItem();

    virtual sal_uInt16          GetValueCount() const override;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*        Create(SvStream &, sal_uInt16) const override;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileVersion ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 OUString& rText,
                                                 const IntlWrapper* pIntl = nullptr ) const override;
};

class ScDoubleItem : public SfxPoolItem
{
public:
                static SfxPoolItem* CreateDefault();
                ScDoubleItem( sal_uInt16 nWhich, double nVal=0 );
                ScDoubleItem( const ScDoubleItem& rItem );
                virtual ~ScDoubleItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const override;

private:
    double  nValue;
};

/** Member ID for "page scale to width" value in QueryValue() and PutValue(). */
const sal_uInt8 SC_MID_PAGE_SCALETO_WIDTH    = 1;
/** Member ID for "page scale to height" value in QueryValue() and PutValue(). */
const sal_uInt8 SC_MID_PAGE_SCALETO_HEIGHT   = 2;

/** Contains the "scale to width/height" attribute in page styles. */
class SC_DLLPUBLIC ScPageScaleToItem : public SfxPoolItem
{
public:
                                static SfxPoolItem* CreateDefault();

    /** Default c'tor sets the width and height to 0. */
    explicit                    ScPageScaleToItem();
    explicit                    ScPageScaleToItem( sal_uInt16 nWidth, sal_uInt16 nHeight );

    virtual                     ~ScPageScaleToItem();

    virtual ScPageScaleToItem*  Clone( SfxItemPool* = nullptr ) const override;

    virtual bool                operator==( const SfxPoolItem& rCmp ) const override;

    inline sal_uInt16           GetWidth() const { return mnWidth; }
    inline sal_uInt16           GetHeight() const { return mnHeight; }
    inline bool                 IsValid() const { return mnWidth || mnHeight; }

    inline void                 SetWidth( sal_uInt16 nWidth ) { mnWidth = nWidth; }
    inline void                 SetHeight( sal_uInt16 nHeight ) { mnHeight = nHeight; }
    inline void                 Set( sal_uInt16 nWidth, sal_uInt16 nHeight )
                                    { mnWidth = nWidth; mnHeight = nHeight; }

    virtual bool GetPresentation(
                                    SfxItemPresentation ePresentation,
                                    SfxMapUnit, SfxMapUnit,
                                    OUString& rText,
                                    const IntlWrapper* = nullptr ) const override;

    virtual bool                QueryValue( css::uno::Any& rAny, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool                PutValue( const css::uno::Any& rAny, sal_uInt8 nMemberId ) override;

private:
    sal_uInt16                  mnWidth;
    sal_uInt16                  mnHeight;
};

class ScCondFormatItem : public SfxPoolItem
{
public:
    static SfxPoolItem* CreateDefault();

    explicit ScCondFormatItem();
    explicit ScCondFormatItem(const std::vector<sal_uInt32>& nIndex);

    virtual ~ScCondFormatItem();

    virtual bool operator==(const SfxPoolItem& rCmp ) const override;
    virtual ScCondFormatItem*  Clone( SfxItemPool* = nullptr ) const override;

    const std::vector<sal_uInt32>& GetCondFormatData() const { return maIndex;}
    void AddCondFormatData( sal_uInt32 nIndex );
    void SetCondFormatData( const std::vector<sal_uInt32>& aIndex );

private:
    std::vector<sal_uInt32> maIndex;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
