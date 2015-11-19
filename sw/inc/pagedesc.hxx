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
#ifndef INCLUDED_SW_INC_PAGEDESC_HXX
#define INCLUDED_SW_INC_PAGEDESC_HXX

#include <tools/fract.hxx>
#include <tools/color.hxx>
#include "swdllapi.h"
#include <swtypes.hxx>
#include <frmfmt.hxx>
#include <editeng/numitem.hxx>
#include <editeng/borderline.hxx>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>

using namespace ::com::sun::star;

class SfxPoolItem;
class SwTextFormatColl;
class SwNode;

/// Separator line adjustment.
enum SwFootnoteAdj
{
    FTNADJ_LEFT,
    FTNADJ_CENTER,
    FTNADJ_RIGHT
};

/// Footnote information.
class SW_DLLPUBLIC SwPageFootnoteInfo
{
private:
    SwTwips     m_nMaxHeight;   ///< maximum height of the footnote area.
    sal_uLong   m_nLineWidth;   ///< width of separator line
    editeng::SvxBorderStyle m_eLineStyle;  ///< Style of the separator line
    Color       m_LineColor;    ///< color of the separator line
    Fraction    m_Width;        ///< percentage width of the separator line.
    SwFootnoteAdj    m_eAdjust;      ///< line adjustment.
    SwTwips     m_nTopDist;     ///< distance between body and separator.
    SwTwips     m_nBottomDist;  ///< distance between separator and first footnote

public:
    SwTwips     GetHeight() const       { return m_nMaxHeight; }
    sal_uLong   GetLineWidth() const    { return m_nLineWidth; }
    const Color& GetLineColor() const   { return m_LineColor;}
    editeng::SvxBorderStyle  GetLineStyle() const { return m_eLineStyle; }
    const Fraction& GetWidth() const    { return m_Width; }
    SwFootnoteAdj    GetAdj() const          { return m_eAdjust; }
    SwTwips     GetTopDist() const      { return m_nTopDist; }
    SwTwips     GetBottomDist() const   { return m_nBottomDist; }

    void SetHeight(SwTwips const nNew)      { m_nMaxHeight = nNew; }
    void SetLineWidth(sal_uLong const nSet) { m_nLineWidth = nSet; }
    void SetLineStyle(editeng::SvxBorderStyle const eSet) {m_eLineStyle = eSet;}
    void SetLineColor(const Color& rCol)    { m_LineColor = rCol;}
    void SetWidth(const Fraction & rNew)    { m_Width = rNew; }
    void SetAdj(SwFootnoteAdj const eNew)        { m_eAdjust = eNew; }
    void SetTopDist   (SwTwips const nNew)  { m_nTopDist = nNew; }
    void SetBottomDist(SwTwips const nNew)  { m_nBottomDist = nNew; }

    SwPageFootnoteInfo();
    SwPageFootnoteInfo( const SwPageFootnoteInfo& );
    SwPageFootnoteInfo& operator=( const SwPageFootnoteInfo& );

    bool operator ==( const SwPageFootnoteInfo& ) const;
};

/*
 *  Use of UseOnPage (m_eUse) and of FrameFormats
 *
 *  RIGHT   - m_Master only for right hand (odd) pages, left hand (even) pages
 *            always empty.
 *  LEFT    - m_Left for left-hand pages, right-hand pages always empty.
 *            m_Left is a copy of master.
 *  ALL     - m_Master for right hand pages, m_Left for left hand pages.
 *          - m_Left is a copy of master.
 * MIRROR   - m_Master for right hand pages, m_Left for left hand pagers.
 *            m_Left is a copy of master, margins are mirrored.
 *
 * UI works exclusively on master! m_Left is adjusted on Chg at document
 * according to m_eUse.
 *
 * In order to simplify the work of the filters some more values are placed
 * into m_eUse:
 *
 * HEADERSHARE - Content of header is equal on left and right hand pages.
 * FOOTERSHARE - Content of footer is equal on left and right hand pages.
 *
 * The values are masked out in the respective getter and setter methods.
 * Access to complete m_eUse including the information on header and footer
 * via ReadUseOn(), WriteUseOn() (for Filter and CopyCTor)!
 *
 * The Frameformats for header/footer are adjusted by the UI according to
 * the attributes for header and footer at master (height, margin, back-
 * ground ...)
 * Header/footer for left hand pages are copied or mirrored (Chg at
 * document).
 * The respective attribute for content is cared for automatically on Chg at
 * document (contents are created or removed according to SHARE-information).
 */

typedef sal_uInt16 UseOnPage;
namespace nsUseOnPage
{
    const UseOnPage PD_NONE           = 0x0000; ///< For internal use only.
    const UseOnPage PD_LEFT           = 0x0001;
    const UseOnPage PD_RIGHT          = 0x0002;
    const UseOnPage PD_ALL            = 0x0003;
    const UseOnPage PD_MIRROR         = 0x0007;
    const UseOnPage PD_HEADERSHARE    = 0x0040;
    const UseOnPage PD_FOOTERSHARE    = 0x0080;
    const UseOnPage PD_NOHEADERSHARE  = 0xFFBF; ///< For internal use only.
    const UseOnPage PD_NOFOOTERSHARE  = 0xFF7F; ///< For internal use only.
    const UseOnPage PD_FIRSTSHARE = 0x0100;
    const UseOnPage PD_NOFIRSTSHARE = 0xFEFF;
}

class SW_DLLPUBLIC SwPageDesc : public SwModify
{
    friend class SwDoc;

    OUString    m_StyleName;
    SvxNumberType m_NumType;
    SwFrameFormat    m_Master;
    SwFrameFormat    m_Left;
    // FIXME epicycles growing here - page margins need to be stored differently
    SwFrameFormat    m_FirstMaster;
    SwFrameFormat    m_FirstLeft;
    SwDepend    m_Depend; ///< Because of grid alignment (Registerhaltigkeit).
    SwPageDesc *m_pFollow;
    sal_uInt16  m_nRegHeight; ///< Sentence spacing and fontascent of style.
    sal_uInt16  m_nRegAscent; ///< For grid alignment (Registerhaltigkeit).
    drawing::TextVerticalAdjust   m_nVerticalAdjustment; // doc/docx: vertically center / justify / bottom
    UseOnPage   m_eUse;
    bool        m_IsLandscape;
    bool        m_IsHidden;

    /// Footnote information.
    SwPageFootnoteInfo m_IsFootnoteInfo;

    /** Called for mirroring of Chg (doc).
       No adjustment at any other place. */
    SAL_DLLPRIVATE void Mirror();

    SAL_DLLPRIVATE void ResetAllAttr( bool bLeft );

    SAL_DLLPRIVATE SwPageDesc(const OUString&, SwFrameFormat*, SwDoc *pDc );

protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNewValue ) override;

public:
    OUString GetName() const { return m_StyleName; }
    void SetName(const OUString& rNewName) { m_StyleName = rNewName; }

    bool GetLandscape() const { return m_IsLandscape; }
    void SetLandscape( bool bNew ) { m_IsLandscape = bNew; }

    const SvxNumberType &GetNumType() const { return m_NumType; }
    void  SetNumType(const SvxNumberType& rNew) { m_NumType = rNew; }

    const SwPageFootnoteInfo &GetFootnoteInfo() const { return m_IsFootnoteInfo; }
          SwPageFootnoteInfo &GetFootnoteInfo()       { return m_IsFootnoteInfo; }
    void  SetFootnoteInfo(const SwPageFootnoteInfo &rNew) { m_IsFootnoteInfo = rNew; }

    inline bool IsHeaderShared() const;
    inline bool IsFooterShared() const;
    inline void ChgHeaderShare( bool bNew );
    inline void ChgFooterShare( bool bNew );
    bool IsFirstShared() const;
    void ChgFirstShare( bool bNew );

    bool IsHidden() const { return m_IsHidden; }
    void SetHidden(bool const bValue) { m_IsHidden = bValue; }

    inline void      SetUseOn( UseOnPage eNew );
    inline UseOnPage GetUseOn() const;

    void      WriteUseOn(UseOnPage const eNew) { m_eUse = eNew; }
    UseOnPage ReadUseOn() const { return m_eUse; }

          SwFrameFormat &GetMaster()      { return m_Master; }
          SwFrameFormat &GetLeft()        { return m_Left; }
          SwFrameFormat &GetFirstMaster() { return m_FirstMaster; }
          SwFrameFormat &GetFirstLeft()   { return m_FirstLeft; }
    const SwFrameFormat &GetMaster() const      { return m_Master; }
    const SwFrameFormat &GetLeft()   const      { return m_Left; }
    const SwFrameFormat &GetFirstMaster() const { return m_FirstMaster; }
    const SwFrameFormat &GetFirstLeft()   const { return m_FirstLeft; }

    /** Reset all attrs of the format but keep the ones a pagedesc
       cannot live without. */
    inline void ResetAllMasterAttr();

    /** Layout uses the following methods to obtain a format in order
       to be able to create a page. */
           SwFrameFormat *GetRightFormat(bool const bFirst = false);
    inline const SwFrameFormat *GetRightFormat(bool const bFirst = false) const;
           SwFrameFormat *GetLeftFormat(bool const bFirst = false);
    inline const SwFrameFormat *GetLeftFormat(bool const bFirst = false) const;

    sal_uInt16 GetRegHeight() const { return m_nRegHeight; }
    sal_uInt16 GetRegAscent() const { return m_nRegAscent; }
    void SetRegHeight(sal_uInt16 const nNew) { m_nRegHeight = nNew; }
    void SetRegAscent(sal_uInt16 const nNew) { m_nRegAscent = nNew; }

    drawing::TextVerticalAdjust GetVerticalAdjustment () const {return m_nVerticalAdjustment; }
    void SetVerticalAdjustment (const drawing::TextVerticalAdjust nVA) {m_nVerticalAdjustment = nVA; }

    inline void SetFollow( const SwPageDesc* pNew );
    const SwPageDesc* GetFollow() const { return m_pFollow; }
          SwPageDesc* GetFollow() { return m_pFollow; }

    void SetRegisterFormatColl( const SwTextFormatColl* rFormat );
    const SwTextFormatColl* GetRegisterFormatColl() const;
    void RegisterChange();

    /// Query and set PoolFormat-Id.
    sal_uInt16 GetPoolFormatId() const         { return m_Master.GetPoolFormatId(); }
    void SetPoolFormatId(sal_uInt16 const nId) { m_Master.SetPoolFormatId(nId); }
    sal_uInt16 GetPoolHelpId() const        { return m_Master.GetPoolHelpId(); }
    void SetPoolHelpId(sal_uInt16 const nId){ m_Master.SetPoolHelpId(nId); }
    sal_uInt8 GetPoolHlpFileId() const      { return m_Master.GetPoolHlpFileId(); }
    void SetPoolHlpFileId(sal_uInt8 const nId) { m_Master.SetPoolHlpFileId(nId); }

    /// Query information from Client.
    virtual bool GetInfo( SfxPoolItem& ) const override;

    const SwFrameFormat* GetPageFormatOfNode( const SwNode& rNd,
                                    bool bCheckForThisPgDc = true ) const;
    bool IsFollowNextPageOfNode( const SwNode& rNd ) const;

    /// Given a SwNode return the pagedesc in use at that location.
    static const SwPageDesc* GetPageDescOfNode(const SwNode& rNd);

    static SwPageDesc* GetByName(SwDoc& rDoc, const OUString& rName);

    SwPageDesc& operator=( const SwPageDesc& );

    SwPageDesc( const SwPageDesc& );
    virtual ~SwPageDesc();
};

inline void SwPageDesc::SetFollow( const SwPageDesc* pNew )
{
    m_pFollow = pNew ? const_cast<SwPageDesc*>(pNew) : this;
}

inline bool SwPageDesc::IsHeaderShared() const
{
    return (m_eUse & nsUseOnPage::PD_HEADERSHARE) != 0;
}
inline bool SwPageDesc::IsFooterShared() const
{
    return (m_eUse & nsUseOnPage::PD_FOOTERSHARE) != 0;
}
inline void SwPageDesc::ChgHeaderShare( bool bNew )
{
    if ( bNew )
        m_eUse = (UseOnPage) (m_eUse | nsUseOnPage::PD_HEADERSHARE);
    else
        m_eUse = (UseOnPage) (m_eUse & nsUseOnPage::PD_NOHEADERSHARE);
}
inline void SwPageDesc::ChgFooterShare( bool bNew )
{
    if ( bNew )
        m_eUse = (UseOnPage) (m_eUse | nsUseOnPage::PD_FOOTERSHARE);
    else
        m_eUse = (UseOnPage) (m_eUse & nsUseOnPage::PD_NOFOOTERSHARE);
}
inline void SwPageDesc::SetUseOn( UseOnPage eNew )
{
    UseOnPage eTmp = nsUseOnPage::PD_NONE;
    if (m_eUse & nsUseOnPage::PD_HEADERSHARE)
        eTmp = nsUseOnPage::PD_HEADERSHARE;
    if (m_eUse & nsUseOnPage::PD_FOOTERSHARE)
        eTmp = (UseOnPage) (eTmp | nsUseOnPage::PD_FOOTERSHARE);
    if (m_eUse & nsUseOnPage::PD_FIRSTSHARE)
        eTmp = (UseOnPage) (eTmp | nsUseOnPage::PD_FIRSTSHARE);
    m_eUse = (UseOnPage) (eTmp | eNew);

}
inline UseOnPage SwPageDesc::GetUseOn() const
{
    UseOnPage eRet = m_eUse;
    eRet = (UseOnPage) (eRet & nsUseOnPage::PD_NOHEADERSHARE);
    eRet = (UseOnPage) (eRet & nsUseOnPage::PD_NOFOOTERSHARE);
    eRet = (UseOnPage) (eRet & nsUseOnPage::PD_NOFIRSTSHARE);
    return eRet;
}

inline void SwPageDesc::ResetAllMasterAttr()
{
    ResetAllAttr( false );
}

inline const SwFrameFormat *SwPageDesc::GetRightFormat(bool const bFirst) const
{
    return const_cast<SwPageDesc*>(this)->GetRightFormat(bFirst);
}
inline const SwFrameFormat *SwPageDesc::GetLeftFormat(bool const bFirst) const
{
    return const_cast<SwPageDesc*>(this)->GetLeftFormat(bFirst);
}

class SwPageDescExt
{
public:
    SwPageDesc m_PageDesc;
private:
    SwDoc * m_pDoc;
    OUString m_sFollow;

    void SetPageDesc(const SwPageDesc & rPageDesc);

public:
    SwPageDescExt(const SwPageDesc & rPageDesc, SwDoc * pDoc);
    SwPageDescExt(const SwPageDescExt & rSrc);
    ~SwPageDescExt();

    SwPageDescExt & operator = (const SwPageDescExt & rSrc);
    SwPageDescExt & operator = (const SwPageDesc & rSrc);

    OUString GetName() const;

    operator SwPageDesc() const; // #i7983#
};

namespace sw {
    class PageFootnoteHint final : public SfxHint {};
}

#endif // INCLUDED_SW_INC_PAGEDESC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
