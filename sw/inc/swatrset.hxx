/*************************************************************************
 *
 *  $RCSfile: swatrset.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-30 12:49:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SWATRSET_HXX
#define _SWATRSET_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
class SwModify;
class SwDoc;

class SfxBoolItem;
class SvxPostureItem;
class SvxWeightItem;
class SvxShadowedItem;
class SvxAutoKernItem;
class SvxWordLineModeItem;
class SvxContourItem;
class SvxKerningItem;
class SvxUnderlineItem;
class SvxCrossedOutItem;
class SvxFontHeightItem;
class SvxPropSizeItem;
class SvxFontItem;
class SvxColorItem;
class SvxCharSetColorItem;
class SvxLanguageItem;
class SvxEscapementItem;
class SvxCaseMapItem;
class SvxNoHyphenItem;
class SvxBlinkItem;
class SvxEmphasisMarkItem;

// Frame-Attribute
class SwFmtFillOrder;
class SwFmtFrmSize;
class SvxPaperBinItem;
class SvxLRSpaceItem;
class SvxULSpaceItem;
class SwFmtCntnt;
class SwFmtHeader;
class SwFmtFooter;
class SvxPrintItem;
class SvxOpaqueItem;
class SvxProtectItem;
class SwFmtSurround;
class SwFmtVertOrient;
class SwFmtHoriOrient;
class SwFmtAnchor;
class SvxBoxItem;
class SvxBrushItem;
class SvxShadowItem;
class SwFmtPageDesc;
class SvxFmtBreakItem;
class SwFmtCol;
class SvxMacroItem;
class SvxFmtKeepItem;
class SwFmtURL;
class SwFmtLineNumber;
class SwFmtEditInReadonly;
class SwFmtLayoutSplit;
class SwFmtChain;
class SwFmtFtnAtTxtEnd;
class SwFmtEndAtTxtEnd;
class SwFmtNoBalancedColumns;

// Grafik-Attribute
class SwMirrorGrf;
class SwCropGrf;
class SwRotationGrf;
class SwLuminanceGrf;
class SwContrastGrf;
class SwChannelRGrf;
class SwChannelGGrf;
class SwChannelBGrf;
class SwGammaGrf;
class SwInvertGrf;
class SwTransparencyGrf;
class SwDrawModeGrf;

// Paragraph-Attribute
class SvxLineSpacingItem;
class SvxAdjustItem;
class SvxFmtSplitItem;
class SwRegisterItem;
class SwNumRuleItem;
class SvxWidowsItem;
class SvxOrphansItem;
class SvxTabStopItem;
class SvxHyphenZoneItem;
class SwFmtDrop;

// TabellenBox-Attribute
class SwTblBoxNumFormat;
class SwTblBoxFormula;
class SwTblBoxValue;


class SwAttrPool : public SfxItemPool
{
    friend void _InitCore();            // fuers anlegen/zerstoeren der
    friend void _FinitCore();           // Versionsmaps
    static USHORT* pVersionMap1;
    static USHORT* pVersionMap2;
    static USHORT* pVersionMap3;

    SwDoc* pDoc;

public:
    SwAttrPool( SwDoc* pDoc );

          SwDoc* GetDoc()           { return pDoc; }
    const SwDoc* GetDoc() const     { return pDoc; }

    static USHORT* GetVersionMap1() { return pVersionMap1; }
    static USHORT* GetVersionMap2() { return pVersionMap2; }
    static USHORT* GetVersionMap3() { return pVersionMap3; }
};


class SwAttrSet : public SfxItemSet
{
    // Pointer fuers Modify-System
    SwAttrSet *pOldSet, *pNewSet;

    // Notification-Callback
    virtual void Changed( const SfxPoolItem& rOld, const SfxPoolItem& rNew );

    void PutChgd( const SfxPoolItem& rI ) { SfxItemSet::PutDirect( rI ); }
public:
    SwAttrSet( SwAttrPool&, USHORT nWhich1, USHORT nWhich2 );
    SwAttrSet( SwAttrPool&, const USHORT* nWhichPairTable );
    SwAttrSet( const SwAttrSet& );

    int Put_BC( const SfxPoolItem& rAttr, SwAttrSet* pOld, SwAttrSet* pNew );
    int Put_BC( const SfxItemSet& rSet, SwAttrSet* pOld, SwAttrSet* pNew );

    // ein Item oder einen Bereich loeschen
    USHORT ClearItem_BC( USHORT nWhich, SwAttrSet* pOld, SwAttrSet* pNew );
    USHORT ClearItem_BC( USHORT nWhich1, USHORT nWhich2,
                        SwAttrSet* pOld = 0, SwAttrSet* pNew = 0 );

    int Intersect_BC( const SfxItemSet& rSet, SwAttrSet* pOld, SwAttrSet* pNew );
    int Differentiate_BC( const SfxItemSet& rSet, SwAttrSet* pOld, SwAttrSet* pNew );
    int MergeValues_BC( const SfxItemSet& rSet, SwAttrSet* pOld, SwAttrSet* pNew );

    void GetPresentation( SfxItemPresentation ePres,
        SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String &rText ) const;

    SwAttrPool* GetPool() const { return (SwAttrPool*)SfxItemSet::GetPool(); }

    // kopiere die Attribute ggfs. ueber Dokumentgrenzen
    void CopyToModify( SwModify& rMod ) const;

    // ----------------------------------------------------------------
    // Sonderbehandlung fuer einige Attribute
    // Setze den Modify-Pointer (alten pDefinedIn) bei folgenden Attributen:
    //  - SwFmtDropCaps
    //  - SwFmtPageDesc
    // (Wird beim Einfuegen in Formate/Nodes gerufen)
    void SetModifyAtAttr( const SwModify* pModify );

    // Das Doc wird jetzt am SwAttrPool gesetzt. Dadurch hat man es immer
    // im Zugriff.
    const SwDoc *GetDoc() const { return GetPool()->GetDoc(); }
          SwDoc *GetDoc()       { return GetPool()->GetDoc(); }

    // GetMethoden: das Bool gibt an, ob nur im Set (FALSE) oder auch in
    //              den Parents gesucht werden soll. Wird nichts gefunden,
    //              wird das deflt. Attribut returnt.
    // Charakter-Attribute  - impl. steht im charatr.hxx
    // AMA 12.10.94: Umstellung von SwFmt... auf Svx...
    inline const SvxPostureItem         &GetPosture( BOOL = TRUE ) const;
    inline const SvxWeightItem          &GetWeight( BOOL = TRUE ) const;
    inline const SvxShadowedItem        &GetShadowed( BOOL = TRUE ) const;
    inline const SvxAutoKernItem        &GetAutoKern( BOOL = TRUE ) const;
    inline const SvxWordLineModeItem    &GetWordLineMode( BOOL = TRUE ) const;
    inline const SvxContourItem         &GetContour( BOOL = TRUE ) const;
    inline const SvxKerningItem         &GetKerning( BOOL = TRUE ) const;
    inline const SvxUnderlineItem       &GetUnderline( BOOL = TRUE ) const;
    inline const SvxCrossedOutItem      &GetCrossedOut( BOOL = TRUE ) const;
    inline const SvxFontHeightItem      &GetSize( BOOL = TRUE ) const;
    inline const SvxPropSizeItem        &GetPropSize( BOOL = TRUE ) const;
    inline const SvxFontItem            &GetFont( BOOL = TRUE ) const;
    inline const SvxColorItem           &GetColor( BOOL = TRUE ) const;
    inline const SvxCharSetColorItem    &GetCharSetColor( BOOL = TRUE ) const;
    inline const SvxLanguageItem        &GetLanguage( BOOL = TRUE ) const;
    inline const SvxEscapementItem      &GetEscapement( BOOL = TRUE ) const;
    inline const SvxCaseMapItem         &GetCaseMap( BOOL = TRUE ) const;
    inline const SvxNoHyphenItem        &GetNoHyphenHere( BOOL = TRUE ) const;
    inline const SvxBlinkItem           &GetBlink( BOOL = TRUE ) const;
    inline const SvxBrushItem           &GetChrBackground( BOOL = TRUE ) const;
    inline const SvxFontItem            &GetCJKFont( BOOL = TRUE ) const;
    inline const SvxFontHeightItem      &GetCJKSize( BOOL = TRUE ) const;
    inline const SvxLanguageItem        &GetCJKLanguage( BOOL = TRUE ) const;
    inline const SvxPostureItem         &GetCJKPosture( BOOL = TRUE ) const;
    inline const SvxWeightItem          &GetCJKWeight( BOOL = TRUE ) const;
    inline const SvxFontItem            &GetCTLFont( BOOL = TRUE ) const;
    inline const SvxFontHeightItem      &GetCTLSize( BOOL = TRUE ) const;
    inline const SvxLanguageItem        &GetCTLLanguage( BOOL = TRUE ) const;
    inline const SvxPostureItem         &GetCTLPosture( BOOL = TRUE ) const;
    inline const SvxWeightItem          &GetCTLWeight( BOOL = TRUE ) const;
    inline const SfxBoolItem            &GetWritingDirection( BOOL = TRUE ) const;
    inline const SvxEmphasisMarkItem    &GetEmphasisMark( BOOL = TRUE ) const;

    // Frame-Attribute  - impl. steht im frmatr.hxx
    inline const SwFmtFillOrder         &GetFillOrder( BOOL = TRUE ) const;
    inline const SwFmtFrmSize           &GetFrmSize( BOOL = TRUE ) const;
    inline const SvxPaperBinItem        &GetPaperBin( BOOL = TRUE ) const;
    inline const SvxLRSpaceItem         &GetLRSpace( BOOL = TRUE ) const;
    inline const SvxULSpaceItem         &GetULSpace( BOOL = TRUE ) const;
    inline const SwFmtCntnt             &GetCntnt( BOOL = TRUE ) const;
    inline const SwFmtHeader            &GetHeader( BOOL = TRUE ) const;
    inline const SwFmtFooter            &GetFooter( BOOL = TRUE ) const;
    inline const SvxPrintItem           &GetPrint( BOOL = TRUE ) const;
    inline const SvxOpaqueItem          &GetOpaque( BOOL = TRUE ) const;
    inline const SvxProtectItem         &GetProtect( BOOL = TRUE ) const;
    inline const SwFmtSurround          &GetSurround( BOOL = TRUE ) const;
    inline const SwFmtVertOrient        &GetVertOrient( BOOL = TRUE ) const;
    inline const SwFmtHoriOrient        &GetHoriOrient( BOOL = TRUE ) const;
    inline const SwFmtAnchor            &GetAnchor( BOOL = TRUE ) const;
    inline const SvxBoxItem             &GetBox( BOOL = TRUE ) const;
    inline const SvxFmtKeepItem         &GetKeep( BOOL = TRUE ) const;
    inline const SvxBrushItem           &GetBackground( BOOL = TRUE ) const;
    inline const SvxShadowItem          &GetShadow( BOOL = TRUE ) const;
    inline const SwFmtPageDesc          &GetPageDesc( BOOL = TRUE ) const;
    inline const SvxFmtBreakItem        &GetBreak( BOOL = TRUE ) const;
    inline const SwFmtCol               &GetCol( BOOL = TRUE ) const;
    inline const SvxMacroItem           &GetMacro( BOOL = TRUE ) const;
    inline const SwFmtURL               &GetURL( BOOL = TRUE ) const;
    inline const SwFmtEditInReadonly    &GetEditInReadonly( BOOL = TRUE ) const;
    inline const SwFmtLayoutSplit       &GetLayoutSplit( BOOL = TRUE ) const;
    inline const SwFmtChain             &GetChain( BOOL = TRUE ) const;
    inline const SwFmtLineNumber        &GetLineNumber( BOOL = TRUE ) const;
    inline const SwFmtFtnAtTxtEnd       &GetFtnAtTxtEnd( BOOL = TRUE ) const;
    inline const SwFmtEndAtTxtEnd       &GetEndAtTxtEnd( BOOL = TRUE ) const;
    inline const SwFmtNoBalancedColumns &GetBalancedColumns( BOOL = TRUE ) const;

    // Grafik-Attribute - impl. steht im grfatr.hxx
    inline const SwMirrorGrf            &GetMirrorGrf( BOOL = TRUE ) const;
    inline const SwCropGrf              &GetCropGrf( BOOL = TRUE ) const;
    inline const SwRotationGrf          &GetRotationGrf(BOOL = TRUE ) const;
    inline const SwLuminanceGrf         &GetLuminanceGrf(BOOL = TRUE ) const;
    inline const SwContrastGrf          &GetContrastGrf(BOOL = TRUE ) const;
    inline const SwChannelRGrf          &GetChannelRGrf(BOOL = TRUE ) const;
    inline const SwChannelGGrf          &GetChannelGGrf(BOOL = TRUE ) const;
    inline const SwChannelBGrf          &GetChannelBGrf(BOOL = TRUE ) const;
    inline const SwGammaGrf             &GetGammaGrf(BOOL = TRUE ) const;
    inline const SwInvertGrf            &GetInvertGrf(BOOL = TRUE ) const;
    inline const SwTransparencyGrf      &GetTransparencyGrf(BOOL = TRUE ) const;
    inline const SwDrawModeGrf          &GetDrawModeGrf(BOOL = TRUE ) const;

    // Paragraph-Attribute  - impl. steht im paratr.hxx
    inline const SvxLineSpacingItem     &GetLineSpacing( BOOL = TRUE ) const;
    inline const SvxAdjustItem          &GetAdjust( BOOL = TRUE ) const;
    inline const SvxFmtSplitItem        &GetSplit( BOOL = TRUE ) const;
    inline const SwRegisterItem         &GetRegister( BOOL = TRUE ) const;
    inline const SwNumRuleItem          &GetNumRule( BOOL = TRUE ) const;
    inline const SvxWidowsItem          &GetWidows( BOOL = TRUE ) const;
    inline const SvxOrphansItem         &GetOrphans( BOOL = TRUE ) const;
    inline const SvxTabStopItem         &GetTabStops( BOOL = TRUE ) const;
    inline const SvxHyphenZoneItem      &GetHyphenZone( BOOL = TRUE ) const;
    inline const SwFmtDrop              &GetDrop( BOOL = TRUE ) const;

    // TabellenBox-Attribute    - impl. steht im cellatr.hxx
    inline const SwTblBoxNumFormat      &GetTblBoxNumFmt( BOOL = TRUE ) const;
    inline const SwTblBoxFormula        &GetTblBoxFormula( BOOL = TRUE ) const;
    inline const SwTblBoxValue          &GetTblBoxValue( BOOL = TRUE ) const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwAttrSet)
};


/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:28  hr
      initial import

      Revision 1.48  2000/09/18 16:03:29  willem.vandorp
      OpenOffice header added.

      Revision 1.47  2000/09/08 13:24:27  willem.vandorp
      Header and footer replaced

      Revision 1.46  2000/07/31 19:23:08  jp
      new attributes for CJK/CTL and graphic

      Revision 1.45  2000/05/26 07:08:20  os
      old SW Basic API Slots removed

      Revision 1.44  2000/01/03 18:19:55  jp
      Bug #71411#: new attribut

      Revision 1.43  1999/09/07 14:33:46  ama
      New #66878#: EndOfTxt-Items for footnotes and endnotes

      Revision 1.42  1999/03/19 12:33:58  OS
      #63701# unbenutztes Break-Attribut aus den Sections entfernt


      Rev 1.41   19 Mar 1999 13:33:58   OS
   #63701# unbenutztes Break-Attribut aus den Sections entfernt

      Rev 1.40   05 Feb 1999 17:42:26   JP
   Task #61467#: Attrset ueber Dokumentgrenzen kopieren, dabei PageDesc/Numrule mit erzeugen

      Rev 1.39   28 Jan 1999 18:11:48   JP
   Task #57749#: Undo von Bereichs-Attributen (Spalten, Hintergr...)

      Rev 1.38   04 Feb 1998 14:09:20   MA
   new: SwFmtLineNumber

      Rev 1.37   15 Dec 1997 16:23:40   AMA
   New: SwSectItem: Soll der uebergeordnete Bereich aufgebrochen werden?

*************************************************************************/
    // #ifndef _SWATRSET_HXX
#endif
