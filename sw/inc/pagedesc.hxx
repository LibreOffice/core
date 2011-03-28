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
#include <swtypes.hxx>  //fuer SwTwips
#include <frmfmt.hxx>
//#ifndef _NUMRULE_HXX
//#include <numrule.hxx>
//#endif
#include <editeng/numitem.hxx>

class SfxPoolItem;
class SwTxtFmtColl;
class SwNode;

//Separator line adjustment
enum SwFtnAdj
{
    FTNADJ_LEFT,
    FTNADJ_CENTER,
    FTNADJ_RIGHT
};

//footnote information
class SW_DLLPUBLIC SwPageFtnInfo
{
    SwTwips     nMaxHeight;     //maximum height of the footnote area.
    sal_uLong       nLineWidth;     //width of separator line
    Color       aLineColor;     //color of the separator line
    Fraction    aWidth;         //percentage width of the separator line.
    SwFtnAdj    eAdj;           //line adjustment.
    SwTwips     nTopDist;       //distance between body and separator.
    SwTwips     nBottomDist;    //distance between separator and first footnote

public:
    SwTwips     GetHeight() const       { return nMaxHeight; }
    sal_uLong           GetLineWidth() const { return nLineWidth; }
    const Color&    GetLineColor() const { return aLineColor;}
    const Fraction& GetWidth() const    { return aWidth; }
    SwFtnAdj    GetAdj()    const       { return eAdj; }
    SwTwips     GetTopDist()const       { return nTopDist; }
    SwTwips     GetBottomDist() const   { return nBottomDist; }

    void SetHeight( SwTwips  nNew )     { nMaxHeight = nNew; }
    void SetLineWidth(sal_uLong nSet  )     { nLineWidth = nSet; }
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
 * Verwendung des UseOnPage (eUse) und der FrmFmt'e
 *
 *  RIGHT   - aMaster nur fuer rechte Seiten, linke  Seiten immer leer.
 *  LEFT    - aLeft fuer linke  Seiten, rechte Seiten immer leer.
 *            aLeft ist eine Kopie des Master.
 *  ALL     - aMaster fuer rechte Seiten, aLeft fuer Linke Seiten.
 *            aLeft ist eine Kopie des Master.
 *  MIRROR  - aMaster fuer rechte Seiten, aLeft fuer linke Seiten.
 *            aLeft ist eine Kopie des Master, Raender sind gespiegelt.
 *
 * UI dreht auschliesslich am Master! aLeft wird beim Chg am Dokument
 * enstprechend dem eUse eingestellt.
 *
 * Damit es die Filter etwas einfacher haben werden weitere Werte im
 * eUse untergebracht:
 *
 * HEADERSHARE - Headerinhalt auf beiden Seiten gleich
 * FOOTERSHARE - Footerinhalt auf beiden Seiten gleich
 *
 * Die Werte werden bei den entsprechenden Get-/Set-Methden ausmaskiert.
 * Zugriff auf das volle eUse inclusive der Header-Footer information
 * per ReadUseOn(), WriteUseOn() (fuer Filter und CopyCTor)!
 *
 * Die FrmFormate fuer Header/Footer werden anhand der Attribute fuer
 * Header/Footer vom UI am Master eingestellt (Hoehe, Raender, Hintergrund...);
 * Header/Footer fuer die Linke Seite werden entsprechen kopiert bzw.
 * gespielt (Chg am Dokument).
 * Das jew. Attribut fuer den Inhalt wird automatisch beim Chg am
 * Dokument versorgt (entsprechen den SHARE-informationen werden Inhalte
 * erzeugt bzw. entfernt).
 *
 */

typedef sal_uInt16 UseOnPage;
namespace nsUseOnPage
{
    const UseOnPage PD_NONE           = 0x0000; //for internal use only.
    const UseOnPage PD_LEFT           = 0x0001;
    const UseOnPage PD_RIGHT          = 0x0002;
    const UseOnPage PD_ALL            = 0x0003;
    const UseOnPage PD_MIRROR         = 0x0007;
    const UseOnPage PD_HEADERSHARE    = 0x0040;
    const UseOnPage PD_FOOTERSHARE    = 0x0080;
    const UseOnPage PD_NOHEADERSHARE  = 0x00BF; //for internal use only
    const UseOnPage PD_NOFOOTERSHARE  = 0x007F; //for internal use only
}

class SW_DLLPUBLIC SwPageDesc : public SwModify
{
    friend class SwDoc;
    friend class SwUndoPageDescExt;

    String      aDescName;
    SvxNumberType   aNumType;
    SwFrmFmt    aMaster;
    SwFrmFmt    aLeft;
    SwDepend    aDepend;    // wg. Registerhaltigkeit
    SwPageDesc *pFollow;
    sal_uInt16      nRegHeight; // Zeilenabstand und Fontascent der Vorlage
    sal_uInt16      nRegAscent; // fuer die Registerhaltigkeit
    UseOnPage   eUse;
    sal_Bool        bLandscape;

    //Fussnoteninformationen
    SwPageFtnInfo aFtnInfo;

    //Wird zum Spiegeln vom Chg (Doc) gerufen.
    //Kein Abgleich an anderer Stelle.
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

    inline void      SetUseOn( UseOnPage eNew );
    inline UseOnPage GetUseOn() const;

    void      WriteUseOn( UseOnPage eNew ) { eUse = eNew; }
    UseOnPage ReadUseOn () const { return eUse; }

          SwFrmFmt &GetMaster() { return aMaster; }
          SwFrmFmt &GetLeft()   { return aLeft; }
    const SwFrmFmt &GetMaster() const { return aMaster; }
    const SwFrmFmt &GetLeft()   const { return aLeft; }

    // Reset all attrs of the format but keep the ones a pagedesc
    // cannot live without.
    inline void ResetAllMasterAttr();
    inline void ResetAllLeftAttr();

    //Mit den folgenden Methoden besorgt sich das Layout ein Format
    //um eine Seite erzeugen zu koennen
    inline SwFrmFmt *GetRightFmt();
    inline const SwFrmFmt *GetRightFmt() const;
    inline SwFrmFmt *GetLeftFmt();
    inline const SwFrmFmt *GetLeftFmt() const;

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

    // erfragen und setzen der PoolFormat-Id
    sal_uInt16 GetPoolFmtId() const         { return aMaster.GetPoolFmtId(); }
    void SetPoolFmtId( sal_uInt16 nId )     { aMaster.SetPoolFmtId( nId ); }
    sal_uInt16 GetPoolHelpId() const        { return aMaster.GetPoolHelpId(); }
    void SetPoolHelpId( sal_uInt16 nId )    { aMaster.SetPoolHelpId( nId ); }
    sal_uInt8 GetPoolHlpFileId() const      { return aMaster.GetPoolHlpFileId(); }
    void SetPoolHlpFileId( sal_uInt8 nId )  { aMaster.SetPoolHlpFileId( nId ); }

        // erfrage vom Client Informationen
    virtual sal_Bool GetInfo( SfxPoolItem& ) const;

    const SwFrmFmt* GetPageFmtOfNode( const SwNode& rNd,
                                    sal_Bool bCheckForThisPgDc = sal_True ) const;
    sal_Bool IsFollowNextPageOfNode( const SwNode& rNd ) const;

    //Given a SwNode return the pagedesc in use at that location.
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
        // (sal_uInt16&)eUse |= (sal_uInt16)nsUseOnPage::PD_HEADERSHARE;
    else
        eUse = (UseOnPage) (eUse & nsUseOnPage::PD_NOHEADERSHARE);
        // (sal_uInt16&)eUse &= (sal_uInt16)nsUseOnPage::PD_NOHEADERSHARE;
}
inline void SwPageDesc::ChgFooterShare( sal_Bool bNew )
{
    if ( bNew )
        eUse = (UseOnPage) (eUse | nsUseOnPage::PD_FOOTERSHARE);
        // (sal_uInt16&)eUse |= (sal_uInt16)nsUseOnPage::PD_FOOTERSHARE;
    else
        eUse = (UseOnPage) (eUse & nsUseOnPage::PD_NOFOOTERSHARE);
        // (sal_uInt16&)eUse &= (sal_uInt16)nsUseOnPage::PD_NOFOOTERSHARE;
}
inline void SwPageDesc::SetUseOn( UseOnPage eNew )
{
    UseOnPage eTmp = nsUseOnPage::PD_NONE;
    if ( eUse & nsUseOnPage::PD_HEADERSHARE )
        eTmp = nsUseOnPage::PD_HEADERSHARE;
        // (sal_uInt16&)eTmp |= (sal_uInt16)nsUseOnPage::PD_HEADERSHARE;
    if ( eUse & nsUseOnPage::PD_FOOTERSHARE )
        eTmp = (UseOnPage) (eTmp | nsUseOnPage::PD_FOOTERSHARE);
        // (sal_uInt16&)eTmp |= (sal_uInt16)nsUseOnPage::PD_FOOTERSHARE;
    eUse = (UseOnPage) (eTmp | eNew);
    // (sal_uInt16&)eUse = eTmp | eNew;
}
inline UseOnPage SwPageDesc::GetUseOn() const
{
    UseOnPage eRet = eUse;
    eRet = (UseOnPage) (eRet & nsUseOnPage::PD_NOHEADERSHARE);
    // (sal_uInt16&)eRet &= (sal_uInt16)nsUseOnPage::PD_NOHEADERSHARE;
    eRet = (UseOnPage) (eRet & nsUseOnPage::PD_NOFOOTERSHARE);
    // (sal_uInt16&)eRet &= (sal_uInt16)nsUseOnPage::PD_NOFOOTERSHARE;
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
