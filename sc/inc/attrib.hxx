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

#include <memory>
#include <vector>
#include <svl/poolitem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <o3tl/typed_flags_set.hxx>
#include "scdllapi.h"
#include "global.hxx"

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
    SCCOL       nColMerge;
    SCROW       nRowMerge;
public:
                ScMergeAttr();
                ScMergeAttr( SCCOL nCol, SCROW nRow );
                ScMergeAttr( const ScMergeAttr& );
                virtual ~ScMergeAttr() override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

            SCCOL          GetColMerge() const {return nColMerge; }
            SCROW          GetRowMerge() const {return nRowMerge; }

            bool            IsMerged() const { return nColMerge>1 || nRowMerge>1; }

    ScMergeAttr& operator=(const ScMergeAttr& rMerge)
            {
                nColMerge = rMerge.nColMerge;
                nRowMerge = rMerge.nRowMerge;
                return *this;
            }

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

class SC_DLLPUBLIC ScMergeFlagAttr: public SfxInt16Item
{
public:
            ScMergeFlagAttr();
            ScMergeFlagAttr(ScMF nFlags);
            virtual ~ScMergeFlagAttr() override;

    ScMergeFlagAttr(ScMergeFlagAttr const &) = default;
    ScMergeFlagAttr(ScMergeFlagAttr &&) = default;
    ScMergeFlagAttr & operator =(ScMergeFlagAttr const &) = delete; // due to SfxInt16Item
    ScMergeFlagAttr & operator =(ScMergeFlagAttr &&) = delete; // due to SfxInt16Item

    SfxPoolItem * Clone(SfxItemPool * pPool = nullptr) const override;

    ScMF    GetValue() const { return static_cast<ScMF>(SfxInt16Item::GetValue()); }

    bool    IsHorOverlapped() const     { return bool( GetValue() & ScMF::Hor );  }
    bool    IsVerOverlapped() const     { return bool( GetValue() & ScMF::Ver );  }
    bool    IsOverlapped() const        { return bool( GetValue() & ( ScMF::Hor | ScMF::Ver ) ); }

    bool    HasAutoFilter() const       { return bool( GetValue() & ScMF::Auto ); }

    bool    IsScenario() const          { return bool( GetValue() & ScMF::Scenario ); }

    bool    HasPivotButton() const;
    bool    HasPivotPopupButton() const;

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;
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
                            virtual ~ScProtectionAttr() override;

    OUString     GetValueText() const;
    virtual bool GetPresentation(
                                    SfxItemPresentation ePres,
                                    MapUnit eCoreMetric,
                                    MapUnit ePresMetric,
                                    OUString& rText,
                                    const IntlWrapper& rIntl ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

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
    ScProtectionAttr& operator=(const ScProtectionAttr& rProtection)
            {
                bProtection = rProtection.bProtection;
                bHideFormula = rProtection.bHideFormula;
                bHideCell = rProtection.bHideCell;
                bHidePrint = rProtection.bHidePrint;
                return *this;
            }
};

// page format item: contents of header and footer

#define SC_HF_LEFTAREA   1
#define SC_HF_CENTERAREA 2
#define SC_HF_RIGHTAREA  3

class SC_DLLPUBLIC ScPageHFItem : public SfxPoolItem
{
    std::unique_ptr<EditTextObject> pLeftArea;
    std::unique_ptr<EditTextObject> pCenterArea;
    std::unique_ptr<EditTextObject> pRightArea;

public:
                ScPageHFItem( sal_uInt16 nWhich );
                ScPageHFItem( const ScPageHFItem& rItem );
                virtual ~ScPageHFItem() override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const EditTextObject* GetLeftArea() const       { return pLeftArea.get(); }
    const EditTextObject* GetCenterArea() const     { return pCenterArea.get(); }
    const EditTextObject* GetRightArea() const      { return pRightArea.get(); }

    void SetLeftArea( const EditTextObject& rNew );
    void SetCenterArea( const EditTextObject& rNew );
    void SetRightArea( const EditTextObject& rNew );
};

// page format item: contents of header and footer

class SC_DLLPUBLIC ScViewObjectModeItem: public SfxEnumItem<ScVObjMode>
{
public:
                ScViewObjectModeItem( sal_uInt16 nWhich );
                ScViewObjectModeItem( sal_uInt16 nWhich, ScVObjMode eMode );
                virtual ~ScViewObjectModeItem() override;

    ScViewObjectModeItem(ScViewObjectModeItem const &) = default;
    ScViewObjectModeItem(ScViewObjectModeItem &&) = default;
    ScViewObjectModeItem & operator =(ScViewObjectModeItem const &) = delete; // due to SfxEnumItem<ScVObjMode>
    ScViewObjectModeItem & operator =(ScViewObjectModeItem &&) = delete; // due to SfxEnumItem<ScVObjMode>

    virtual sal_uInt16          GetValueCount() const override;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileVersion ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString& rText,
                                  const IntlWrapper& rIntl ) const override;
};

/** Member ID for "page scale to width" value in QueryValue() and PutValue(). */
const sal_uInt8 SC_MID_PAGE_SCALETO_WIDTH    = 1;
/** Member ID for "page scale to height" value in QueryValue() and PutValue(). */
const sal_uInt8 SC_MID_PAGE_SCALETO_HEIGHT   = 2;

/** Contains the "scale to width/height" attribute in page styles. */
class SC_DLLPUBLIC ScPageScaleToItem : public SfxPoolItem
{
public:
    /** Default c'tor sets the width and height to 0. */
    explicit                    ScPageScaleToItem();
    explicit                    ScPageScaleToItem( sal_uInt16 nWidth, sal_uInt16 nHeight );

    virtual                     ~ScPageScaleToItem() override;

    ScPageScaleToItem(ScPageScaleToItem const &) = default;
    ScPageScaleToItem(ScPageScaleToItem &&) = default;
    ScPageScaleToItem & operator =(ScPageScaleToItem const &) = delete; // due to SfxPoolItem
    ScPageScaleToItem & operator =(ScPageScaleToItem &&) = delete; // due to SfxPoolItem

    virtual ScPageScaleToItem*  Clone( SfxItemPool* = nullptr ) const override;

    virtual bool                operator==( const SfxPoolItem& rCmp ) const override;

    sal_uInt16           GetWidth() const { return mnWidth; }
    sal_uInt16           GetHeight() const { return mnHeight; }
    bool                 IsValid() const { return mnWidth || mnHeight; }

    void                 SetWidth( sal_uInt16 nWidth ) { mnWidth = nWidth; }
    void                 SetHeight( sal_uInt16 nHeight ) { mnHeight = nHeight; }
    void                 Set( sal_uInt16 nWidth, sal_uInt16 nHeight )
                                    { mnWidth = nWidth; mnHeight = nHeight; }

    virtual bool GetPresentation( SfxItemPresentation ePresentation,
                                  MapUnit, MapUnit,
                                  OUString& rText,
                                  const IntlWrapper& ) const override;

    virtual bool                QueryValue( css::uno::Any& rAny, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool                PutValue( const css::uno::Any& rAny, sal_uInt8 nMemberId ) override;

private:
    sal_uInt16                  mnWidth;
    sal_uInt16                  mnHeight;
};

class ScCondFormatItem : public SfxPoolItem
{
public:
    explicit ScCondFormatItem();
    explicit ScCondFormatItem(const std::vector<sal_uInt32>& nIndex);

    virtual ~ScCondFormatItem() override;

    virtual bool operator==(const SfxPoolItem& rCmp ) const override;
    virtual ScCondFormatItem*  Clone( SfxItemPool* = nullptr ) const override;

    const std::vector<sal_uInt32>& GetCondFormatData() const { return maIndex;}
    void AddCondFormatData( sal_uInt32 nIndex );
    void SetCondFormatData( const std::vector<sal_uInt32>& aIndex );

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;

private:
    std::vector<sal_uInt32> maIndex;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
