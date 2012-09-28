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
#ifndef _PAGEDESC_HXX
#define _PAGEDESC_HXX

#include <tools/fract.hxx>
#include <tools/color.hxx>
#include <tools/string.hxx>
#include "swdllapi.h"
#include <swtypes.hxx>  ///< For SwTwips.
#include <frmfmt.hxx>
#include <editeng/numitem.hxx>
#include <editeng/borderline.hxx>

class SfxPoolItem;
class SwTxtFmtColl;
class SwNode;

/// Separator line adjustment.
enum SwFtnAdj
{
    FTNADJ_LEFT,
    FTNADJ_CENTER,
    FTNADJ_RIGHT
};

/// Footnote information.
class SW_DLLPUBLIC SwPageFtnInfo
{
    SwTwips     nMaxHeight;              ///< maximum height of the footnote area.
    sal_uLong   nLineWidth;              ///< width of separator line
    editeng::SvxBorderStyle eLineStyle;  ///< Style of the separator line
    Color       aLineColor;              ///< color of the separator line
    Fraction    aWidth;                  ///< percentage width of the separator line.
    SwFtnAdj    eAdj;                    ///< line adjustment.
    SwTwips     nTopDist;                ///< distance between body and separator.
    SwTwips     nBottomDist;             ///< distance between separator and first footnote

public:
    SwTwips     GetHeight() const       { return nMaxHeight; }
    sal_uLong           GetLineWidth() const { return nLineWidth; }
    const Color&    GetLineColor() const { return aLineColor;}
    editeng::SvxBorderStyle  GetLineStyle() const { return eLineStyle; }
    const Fraction& GetWidth() const    { return aWidth; }
    SwFtnAdj    GetAdj()    const       { return eAdj; }
    SwTwips     GetTopDist()const       { return nTopDist; }
    SwTwips     GetBottomDist() const   { return nBottomDist; }

    void SetHeight( SwTwips  nNew )     { nMaxHeight = nNew; }
    void SetLineWidth(sal_uLong nSet  )     { nLineWidth = nSet; }
    void SetLineStyle( editeng::SvxBorderStyle eSet )   { eLineStyle = eSet; }
    void SetLineColor(const Color& rCol )  { aLineColor = rCol;}
    void SetWidth( const Fraction &rNew){ aWidth = rNew; }
    void SetAdj   ( SwFtnAdj eNew )     { eAdj = eNew; }
    void SetTopDist   ( SwTwips nNew )  { nTopDist = nNew; }
    void SetBottomDist( SwTwips nNew )  { nBottomDist = nNew; }

    SwPageFtnInfo();
    SwPageFtnInfo( const SwPageFtnInfo& );
    SwPageFtnInfo& operator=( const SwPageFtnInfo& );

    sal_Bool operator ==( const SwPageFtnInfo& ) const;
};

/*
 *  Use of UseOnPage (eUse) and of FrmFmts
 *
 *  RIGHT   - aMaster only for right hand (odd) pages, left hand (even) pages
 *            always empty.
 *  LEFT    - aLeft for left-hand pages, right-hand pages always empty.
 *            aLeft is a copy of master.
 *  ALL     - aMaster for right hand pages, aLeft for left hand pages.
 *          - aLeft is a copy of master.
 * MIRROR   - aMaster for right hand pages, aLeft for left hand pagers.
 *            aLeft is a copy of master, margins are mirrored.
 *
 * UI works exclusively on master! aLeft is adjusted on Chg at document
 * according to eUse.
 *
 * In order to simplify the work of the filters some more values are placed
 * into eUse:
 *
 * HEADERSHARE - Content of header is equal on left and right hand pages.
 * FOOTERSHARE - Content of footer is equal on left and right hand pages.
 *
 * The values are masked out in the respective getter and setter methods.
 * Access to complete eUse including the information on header and footer
 * via ReadUseOn(), WriteUseOn() (fuer Filter und CopyCTor)!
 *
 * The Frmformats for header/footer are adjusted by the UI according to
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
    const UseOnPage PD_FIRST          = 0x0004;
    const UseOnPage PD_ALL            = 0x0007;
    const UseOnPage PD_MIRROR         = 0x000F;
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

    String      aDescName;
    SvxNumberType   aNumType;
    SwFrmFmt    aMaster;
    SwFrmFmt    aLeft;
    SwFrmFmt    aFirst;
    SwDepend    aDepend;    ///< Because of grid alignment (Registerhaltigkeit).
    SwPageDesc *pFollow;
    sal_uInt16  nRegHeight; ///< Sentence spacing and fontascent of style.
    sal_uInt16  nRegAscent; ///< For grid alignment (Registerhaltigkeit).
    UseOnPage   eUse;
    sal_Bool        bLandscape;

    /// Footnote information.
    SwPageFtnInfo aFtnInfo;

    /** Called for mirroring of Chg (doc).
       No adjustment at any other place. */
    SW_DLLPRIVATE void Mirror();

    SW_DLLPRIVATE void ResetAllAttr( sal_Bool bLeft );

    SW_DLLPRIVATE SwPageDesc(const String&, SwFrmFmt*, SwDoc *pDc );

protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNewValue );

public:
    const String &GetName() const { return aDescName; }
          void    SetName( const String& rNewName ) { aDescName = rNewName; }

    sal_Bool GetLandscape() const { return bLandscape; }
    void SetLandscape( sal_Bool bNew ) { bLandscape = bNew; }

    const SvxNumberType &GetNumType() const { return aNumType; }
          void          SetNumType( const SvxNumberType& rNew ) { aNumType = rNew; }

    const SwPageFtnInfo &GetFtnInfo() const { return aFtnInfo; }
          SwPageFtnInfo &GetFtnInfo()       { return aFtnInfo; }
    void  SetFtnInfo( const SwPageFtnInfo &rNew ) { aFtnInfo = rNew; }

    inline sal_Bool IsHeaderShared() const;
    inline sal_Bool IsFooterShared() const;
    inline void ChgHeaderShare( sal_Bool bNew );
    inline void ChgFooterShare( sal_Bool bNew );
    sal_Bool IsFirstShared() const;
    void ChgFirstShare( sal_Bool bNew );

    inline void      SetUseOn( UseOnPage eNew );
    inline UseOnPage GetUseOn() const;

    void      WriteUseOn( UseOnPage eNew ) { eUse = eNew; }
    UseOnPage ReadUseOn () const { return eUse; }

          SwFrmFmt &GetMaster() { return aMaster; }
          SwFrmFmt &GetLeft()   { return aLeft; }
          SwFrmFmt &GetFirst()   { return aFirst; }
    const SwFrmFmt &GetMaster() const { return aMaster; }
    const SwFrmFmt &GetLeft()   const { return aLeft; }
    const SwFrmFmt &GetFirst()   const { return aFirst; }

    /** Reset all attrs of the format but keep the ones a pagedesc
       cannot live without. */
    inline void ResetAllMasterAttr();
    inline void ResetAllLeftAttr();

    /** Layout uses the following methods to obtain a format in order
       to be able to create a page. */
    inline SwFrmFmt *GetRightFmt();
    inline const SwFrmFmt *GetRightFmt() const;
    inline SwFrmFmt *GetLeftFmt();
    inline const SwFrmFmt *GetLeftFmt() const;
    inline SwFrmFmt *GetFirstFmt();
    inline const SwFrmFmt *GetFirstFmt() const;

    sal_uInt16 GetRegHeight() const { return nRegHeight; }
    sal_uInt16 GetRegAscent() const { return nRegAscent; }
    void SetRegHeight( sal_uInt16 nNew ){ nRegHeight = nNew; }
    void SetRegAscent( sal_uInt16 nNew ){ nRegAscent = nNew; }

    inline void SetFollow( const SwPageDesc* pNew );
    const SwPageDesc* GetFollow() const { return pFollow; }
          SwPageDesc* GetFollow() { return pFollow; }

    void SetRegisterFmtColl( const SwTxtFmtColl* rFmt );
    const SwTxtFmtColl* GetRegisterFmtColl() const;
    void RegisterChange();

    /// Query and set PoolFormat-Id.
    sal_uInt16 GetPoolFmtId() const         { return aMaster.GetPoolFmtId(); }
    void SetPoolFmtId( sal_uInt16 nId )     { aMaster.SetPoolFmtId( nId ); }
    sal_uInt16 GetPoolHelpId() const        { return aMaster.GetPoolHelpId(); }
    void SetPoolHelpId( sal_uInt16 nId )    { aMaster.SetPoolHelpId( nId ); }
    sal_uInt8 GetPoolHlpFileId() const      { return aMaster.GetPoolHlpFileId(); }
    void SetPoolHlpFileId( sal_uInt8 nId )  { aMaster.SetPoolHlpFileId( nId ); }

    /// Query information from Client.
    virtual sal_Bool GetInfo( SfxPoolItem& ) const;

    const SwFrmFmt* GetPageFmtOfNode( const SwNode& rNd,
                                    sal_Bool bCheckForThisPgDc = sal_True ) const;
    sal_Bool IsFollowNextPageOfNode( const SwNode& rNd ) const;

    /// Given a SwNode return the pagedesc in use at that location.
    static const SwPageDesc* GetPageDescOfNode(const SwNode& rNd);

    SwPageDesc& operator=( const SwPageDesc& );

    SwPageDesc( const SwPageDesc& );
    ~SwPageDesc();
};

inline void SwPageDesc::SetFollow( const SwPageDesc* pNew )
{
    pFollow = pNew ? (SwPageDesc*)pNew : this;
}

inline sal_Bool SwPageDesc::IsHeaderShared() const
{
    return eUse & nsUseOnPage::PD_HEADERSHARE ? sal_True : sal_False;
}
inline sal_Bool SwPageDesc::IsFooterShared() const
{
    return eUse & nsUseOnPage::PD_FOOTERSHARE ? sal_True : sal_False;
}
inline void SwPageDesc::ChgHeaderShare( sal_Bool bNew )
{
    if ( bNew )
        eUse = (UseOnPage) (eUse | nsUseOnPage::PD_HEADERSHARE);
    else
        eUse = (UseOnPage) (eUse & nsUseOnPage::PD_NOHEADERSHARE);
}
inline void SwPageDesc::ChgFooterShare( sal_Bool bNew )
{
    if ( bNew )
        eUse = (UseOnPage) (eUse | nsUseOnPage::PD_FOOTERSHARE);
    else
        eUse = (UseOnPage) (eUse & nsUseOnPage::PD_NOFOOTERSHARE);
}
inline void SwPageDesc::SetUseOn( UseOnPage eNew )
{
    UseOnPage eTmp = nsUseOnPage::PD_NONE;
    if ( eUse & nsUseOnPage::PD_HEADERSHARE )
        eTmp = nsUseOnPage::PD_HEADERSHARE;
    if ( eUse & nsUseOnPage::PD_FOOTERSHARE )
        eTmp = (UseOnPage) (eTmp | nsUseOnPage::PD_FOOTERSHARE);
    if ( eUse & nsUseOnPage::PD_FIRSTSHARE )
        eTmp = (UseOnPage) (eTmp | nsUseOnPage::PD_FIRSTSHARE);
    eUse = (UseOnPage) (eTmp | eNew);

}
inline UseOnPage SwPageDesc::GetUseOn() const
{
    UseOnPage eRet = eUse;
    eRet = (UseOnPage) (eRet & nsUseOnPage::PD_NOHEADERSHARE);
    eRet = (UseOnPage) (eRet & nsUseOnPage::PD_NOFOOTERSHARE);
    return eRet;
}

inline void SwPageDesc::ResetAllMasterAttr()
{
    ResetAllAttr( sal_False );
}

inline void SwPageDesc::ResetAllLeftAttr()
{
    ResetAllAttr( sal_True );
}

inline SwFrmFmt *SwPageDesc::GetRightFmt()
{
    return nsUseOnPage::PD_RIGHT & eUse ? &aMaster : 0;
}
inline const SwFrmFmt *SwPageDesc::GetRightFmt() const
{
    return nsUseOnPage::PD_RIGHT & eUse ? &aMaster : 0;
}
inline SwFrmFmt *SwPageDesc::GetLeftFmt()
{
    return nsUseOnPage::PD_LEFT & eUse ? &aLeft : 0;
}
inline const SwFrmFmt *SwPageDesc::GetLeftFmt() const
{
    return nsUseOnPage::PD_LEFT & eUse ? &aLeft : 0;
}
inline SwFrmFmt *SwPageDesc::GetFirstFmt()
{
    return nsUseOnPage::PD_FIRST & eUse ? &aFirst : 0;
}
inline const SwFrmFmt *SwPageDesc::GetFirstFmt() const
{
    return nsUseOnPage::PD_FIRST & eUse ? &aFirst : 0;
}

class SwPageDescExt
{
    SwPageDesc aPageDesc;
    SwDoc * pDoc;
    String sFollow;

    void SetPageDesc(const SwPageDesc & aPageDesc);

public:
    SwPageDescExt(const SwPageDesc & rPageDesc, SwDoc * pDoc);
    SwPageDescExt(const SwPageDescExt & rSrc);
    ~SwPageDescExt();

    SwPageDescExt & operator = (const SwPageDescExt & rSrc);
    SwPageDescExt & operator = (const SwPageDesc & rSrc);

    const String & GetName() const;

    operator SwPageDesc() const; // #i7983#
};


SwPageDesc* GetPageDescByName_Impl(SwDoc& rDoc, const String& rName);

#endif  //_PAGEDESC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
