/*************************************************************************
 *
 *  $RCSfile: format.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 09:36:59 $
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
#ifndef _FORMAT_HXX
#define _FORMAT_HXX


#ifndef  _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>       // fuer ASSERT
#endif
#ifndef _SWATRSET_HXX
#include <swatrset.hxx>     // fuer SfxItemPool/-Set, Attr forward decl.
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>       // fuer SwModify
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

class SwDoc;

class SwFmt : public SwModify
{
    friend class SwSwgReader;
    friend class SwSwgWriter;

    String aFmtName;
    SwAttrSet aSet;

    USHORT nWhichId;
    USHORT nFmtId;          // Format-ID fuer Lesen/Schreiben
    USHORT nPoolFmtId;      // Id-fuer "automatich" erzeugte Formate
                            // (ist keine harte Attributierung !!)
    USHORT nPoolHelpId;     // HelpId fuer diese Pool-Vorlage
    BYTE nPoolHlpFileId;    // FilePos ans Doc auf die Vorlagen-Hilfen
    BOOL   bWritten : 1;    // TRUE: bereits geschrieben
    BOOL   bAutoFmt : 1;    // FALSE: es handelt sich um eine Vorlage
                            // ist dflt immer auf TRUE !
    BOOL   bFmtInDTOR : 1;  // TRUE: das Format wird geloscht. Damit man in
                            // der FmtChg-Message das erkennen kann!!!
    BOOL   bAutoUpdateFmt : 1;  // TRUE: am Format werden die Attribute
                            // eines kompletten Absatzes gesetzt (UI-seitig!)
    BOOL   bLayerFmt : 1;   // nur fuer SwFrmFmt ( LAYER_IMPL )


protected:
    SwFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
            const USHORT* pWhichRanges, SwFmt *pDrvdFrm, USHORT nFmtWhich );
    SwFmt( SwAttrPool& rPool, const String &rFmtNm, const USHORT* pWhichRanges,
            SwFmt *pDrvdFrm, USHORT nFmtWhich );
    SwFmt( SwAttrPool& rPool, const String &rFmtNm, USHORT nWhich1,
            USHORT nWhich2, SwFmt *pDrvdFrm, USHORT nFmtWhich );
    SwFmt( const SwFmt& rFmt );

public:
    TYPEINFO();     //Bereits in Basisklasse Client drin.

    virtual ~SwFmt();
    SwFmt &operator=(const SwFmt&);

    // fuer die Abfrage der Writer-Funktionen
    USHORT Which() const { return nWhichId; }

    virtual void Modify( SfxPoolItem* pOldValue, SfxPoolItem* pNewValue );
        // erfrage vom Format Informationen
    virtual BOOL GetInfo( SfxPoolItem& ) const;

    // kopiere Attribute; auch ueber Dokumentgrenzen hinweg
    void CopyAttrs( const SwFmt&, BOOL bReplace=TRUE );

    // loesche alle Attribute, die nicht in rFmt stehen
    void DelDiffs( const SfxItemSet& rSet );
    void DelDiffs( const SwFmt& rFmt ) { DelDiffs( rFmt.GetAttrSet() ); }

    // Umhaengen des Formats (0 = Default)
    BOOL SetDerivedFrom(SwFmt *pDerivedFrom = 0);

    // Ist bInParents FALSE,
    // wird nur in diesem Format nach dem Attribut gesucht.
    inline const SfxPoolItem& GetAttr( USHORT nWhich,
                                        BOOL bInParents = TRUE ) const;
    inline SfxItemState GetItemState( USHORT nWhich, BOOL bSrchInParent = TRUE,
                                    const SfxPoolItem **ppItem = 0 ) const;
    BOOL SetAttr( const SfxPoolItem& );
    BOOL SetAttr( const SfxItemSet& rSet );

    // Nimmt den Hint mit nWhich aus dem Delta-Array
    BOOL ResetAttr( USHORT nWhich1, USHORT nWhich2 = 0 );

    // Nimmt alle Hints aus dem Delta-Array,
    // liefert die Anzahl der geloeschten Hints
    USHORT ResetAllAttr();

    inline SwFmt* DerivedFrom() const { return (SwFmt*)pRegisteredIn; }
    inline BOOL IsDefault() const { return DerivedFrom() == 0; }

    inline const String& GetName() const        { return aFmtName; }
    inline void SetName( const String& rNewName );
    inline void SetName( const sal_Char* pNewName );

    // zur Abfrage des Attribute Arrays
    inline const SwAttrSet& GetAttrSet() const { return aSet; }

    // Das Doc wird jetzt am SwAttrPool gesetzt. Dadurch hat man es immer
    // im Zugriff.
    const SwDoc *GetDoc() const         { return aSet.GetDoc(); }
          SwDoc *GetDoc()               { return aSet.GetDoc(); }

    // erfragen und setzen der Poolvorlagen-Id's
    USHORT GetPoolFmtId() const { return nPoolFmtId; }
    void SetPoolFmtId( USHORT nId ) { nPoolFmtId = nId; }

    // erfragen und setzen der Hilfe-Id's fuer die Document-Vorlagen
    USHORT GetPoolHelpId() const { return nPoolHelpId; }
    void SetPoolHelpId( USHORT nId ) { nPoolHelpId = nId; }
    BYTE GetPoolHlpFileId() const { return nPoolHlpFileId; }
    void SetPoolHlpFileId( BYTE nId ) { nPoolHlpFileId = nId; }
    // erfrage die Attribut-Beschreibung, returnt den reingereichten String
    void GetPresentation( SfxItemPresentation ePres,
        SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String &rText ) const
        { aSet.GetPresentation( ePres, eCoreMetric, ePresMetric, rText ); }
    // Das Format-ID fuer Lesen/Schreiben:
    USHORT GetFmtId() const  { return nFmtId;    }
    void   SetWritten()      { bWritten = TRUE;  }
    void   ResetWritten()    { bWritten = FALSE; }
    BOOL   IsWritten() const { return bWritten;  }

    // Abfragen/Setzen vom AutoFmt-Flag
    BOOL IsAuto() const                 { return bAutoFmt; }
    void SetAuto( BOOL bNew = FALSE )   { bAutoFmt = bNew; }

    // Abfragen/Setzen vom bAutoUpdateFmt-Flag
    BOOL IsAutoUpdateFmt() const                { return bAutoUpdateFmt; }
    void SetAutoUpdateFmt( BOOL bNew = TRUE )   { bAutoUpdateFmt = bNew; }

    BOOL IsFmtInDTOR() const { return bFmtInDTOR; }

    BOOL IsLayerFmt() const { return bLayerFmt; }
    void SetLayerFmt( BOOL bNew ) { bLayerFmt = bNew; }

    BOOL IsFrmFmt() const { return nWhichId == RES_FLYFRMFMT ||
                                   nWhichId == RES_DRAWFRMFMT||
                                   nWhichId == RES_FRMFMT; }

    // GetMethoden: das Bool gibt an, ob nur im Set (FALSE) oder auch in
    //              den Parents gesucht werden soll. Wird nichts gefunden,
    //              wird das deflt. Attribut returnt.
    // Charakter-Attribute  - impl. steht im charatr.hxx
    // AMA 12.10.94: Umstellung von SwFmt... auf Svx...
    inline const SvxPostureItem      &GetPosture( BOOL = TRUE ) const;
    inline const SvxWeightItem       &GetWeight( BOOL = TRUE ) const;
    inline const SvxShadowedItem     &GetShadowed( BOOL = TRUE ) const;
    inline const SvxAutoKernItem     &GetAutoKern( BOOL = TRUE ) const;
    inline const SvxWordLineModeItem &GetWordLineMode( BOOL = TRUE ) const;
    inline const SvxContourItem      &GetContour( BOOL = TRUE ) const;
    inline const SvxKerningItem      &GetKerning( BOOL = TRUE ) const;
    inline const SvxUnderlineItem    &GetUnderline( BOOL = TRUE ) const;
    inline const SvxCrossedOutItem   &GetCrossedOut( BOOL = TRUE ) const;
    inline const SvxFontHeightItem   &GetSize( BOOL = TRUE ) const;
    inline const SvxPropSizeItem     &GetPropSize( BOOL = TRUE ) const;
    inline const SvxFontItem         &GetFont( BOOL = TRUE ) const;
    inline const SvxColorItem        &GetColor( BOOL = TRUE ) const;
    inline const SvxCharSetColorItem &GetCharSetColor( BOOL = TRUE ) const;
    inline const SvxLanguageItem     &GetLanguage( BOOL = TRUE ) const;
    inline const SvxEscapementItem   &GetEscapement( BOOL = TRUE ) const;
    inline const SvxCaseMapItem      &GetCaseMap( BOOL = TRUE ) const;
    inline const SvxNoHyphenItem     &GetNoHyphenHere( BOOL = TRUE ) const;
    inline const SvxBlinkItem        &GetBlink( BOOL = TRUE ) const;
    inline const SvxBrushItem        &GetChrBackground( BOOL = TRUE ) const;

    inline const SvxFontItem         &GetCJKFont( BOOL = TRUE ) const;
    inline const SvxFontHeightItem   &GetCJKSize( BOOL = TRUE ) const;
    inline const SvxLanguageItem     &GetCJKLanguage( BOOL = TRUE ) const;
    inline const SvxPostureItem      &GetCJKPosture( BOOL = TRUE ) const;
    inline const SvxWeightItem       &GetCJKWeight( BOOL = TRUE ) const;
    inline const SvxFontItem         &GetCTLFont( BOOL = TRUE ) const;
    inline const SvxFontHeightItem   &GetCTLSize( BOOL = TRUE ) const;
    inline const SvxLanguageItem     &GetCTLLanguage( BOOL = TRUE ) const;
    inline const SvxPostureItem      &GetCTLPosture( BOOL = TRUE ) const;
    inline const SvxWeightItem       &GetCTLWeight( BOOL = TRUE ) const;
    inline const SfxBoolItem         &GetWritingDirection( BOOL = TRUE ) const;

    inline const SvxEmphasisMarkItem &GetEmphasisMark( BOOL = TRUE ) const;
    inline const SvxTwoLinesItem    &Get2Lines( BOOL = TRUE ) const;

    // Frame-Attribute  - impl. steht im frmatr.hxx,
    // fuer LINUX, SINIX, HPUX auch in gcc_outl.cxx
#if ( defined GCC && defined C272 && \
      ( defined LINUX || defined SINIX || defined HPUX ))
    const SwFmtFillOrder        &GetFillOrder( BOOL = TRUE ) const;
    const SwFmtFrmSize          &GetFrmSize( BOOL = TRUE ) const;
    const SwFmtHeader           &GetHeader( BOOL = TRUE ) const;
    const SwFmtFooter           &GetFooter( BOOL = TRUE ) const;
    const SwFmtSurround         &GetSurround( BOOL = TRUE ) const;
    const SwFmtHoriOrient       &GetHoriOrient( BOOL = TRUE ) const;
    const SwFmtAnchor           &GetAnchor( BOOL = TRUE ) const;
    const SwFmtCol              &GetCol( BOOL = TRUE ) const;
#endif
#if ( defined GCC && defined C272 && defined HPUX )
    const SvxPaperBinItem       &GetPaperBin( BOOL = TRUE ) const;
    const SvxLRSpaceItem        &GetLRSpace( BOOL = TRUE ) const;
    const SvxULSpaceItem        &GetULSpace( BOOL = TRUE ) const;
    const SwFmtCntnt            &GetCntnt( BOOL = TRUE ) const;
    const SvxPrintItem          &GetPrint( BOOL = TRUE ) const;
    const SvxOpaqueItem         &GetOpaque( BOOL = TRUE ) const;
    const SvxProtectItem        &GetProtect( BOOL = TRUE ) const;
    const SwFmtVertOrient       &GetVertOrient( BOOL = TRUE ) const;
    const SvxBoxItem            &GetBox( BOOL = TRUE ) const;
    const SvxFmtKeepItem        &GetKeep( BOOL = TRUE ) const;
    const SvxBrushItem          &GetBackground( BOOL = TRUE ) const;
    const SvxShadowItem         &GetShadow( BOOL = TRUE ) const;
    const SwFmtPageDesc         &GetPageDesc( BOOL = TRUE ) const;
    const SvxFmtBreakItem       &GetBreak( BOOL = TRUE ) const;
    const SvxMacroItem          &GetMacro( BOOL = TRUE ) const;
    const SwFmtURL              &GetURL( BOOL = TRUE ) const;
#endif
#if !( defined GCC && defined C272 && \
       ( defined LINUX || defined SINIX || defined HPUX ))
    inline const SwFmtFillOrder         &GetFillOrder( BOOL = TRUE ) const;
    inline const SwFmtFrmSize           &GetFrmSize( BOOL = TRUE ) const;
    inline const SwFmtHeader            &GetHeader( BOOL = TRUE ) const;
    inline const SwFmtFooter            &GetFooter( BOOL = TRUE ) const;
    inline const SwFmtSurround          &GetSurround( BOOL = TRUE ) const;
    inline const SwFmtHoriOrient        &GetHoriOrient( BOOL = TRUE ) const;
    inline const SwFmtAnchor            &GetAnchor( BOOL = TRUE ) const;
    inline const SwFmtCol               &GetCol( BOOL = TRUE ) const;
#endif
#if !( defined GCC && defined C272 && defined HPUX )
    inline const SvxPaperBinItem        &GetPaperBin( BOOL = TRUE ) const;
    inline const SvxLRSpaceItem         &GetLRSpace( BOOL = TRUE ) const;
    inline const SvxULSpaceItem         &GetULSpace( BOOL = TRUE ) const;
    inline const SwFmtCntnt             &GetCntnt( BOOL = TRUE ) const;
    inline const SvxPrintItem           &GetPrint( BOOL = TRUE ) const;
    inline const SvxOpaqueItem          &GetOpaque( BOOL = TRUE ) const;
    inline const SvxProtectItem         &GetProtect( BOOL = TRUE ) const;
    inline const SwFmtVertOrient        &GetVertOrient( BOOL = TRUE ) const;
    inline const SvxBoxItem             &GetBox( BOOL = TRUE ) const;
    inline const SvxFmtKeepItem         &GetKeep( BOOL = TRUE ) const;
    inline const SvxBrushItem           &GetBackground( BOOL = TRUE ) const;
    inline const SvxShadowItem          &GetShadow( BOOL = TRUE ) const;
    inline const SwFmtPageDesc          &GetPageDesc( BOOL = TRUE ) const;
    inline const SvxFmtBreakItem        &GetBreak( BOOL = TRUE ) const;
    inline const SvxMacroItem           &GetMacro( BOOL = TRUE ) const;
    inline const SwFmtURL               &GetURL( BOOL = TRUE ) const;
    inline const SwFmtEditInReadonly    &GetEditInReadonly( BOOL = TRUE ) const;
    inline const SwFmtLayoutSplit       &GetLayoutSplit( BOOL = TRUE ) const;
    inline const SwFmtChain             &GetChain( BOOL = TRUE ) const;
    inline const SwFmtLineNumber        &GetLineNumber( BOOL = TRUE ) const;
    inline const SwFmtFtnAtTxtEnd       &GetFtnAtTxtEnd( BOOL = TRUE ) const;
    inline const SwFmtEndAtTxtEnd       &GetEndAtTxtEnd( BOOL = TRUE ) const;
    inline const SwFmtNoBalancedColumns &GetBalancedColumns( BOOL = TRUE ) const;
#endif

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
    inline const SvxScriptSpaceItem     &GetScriptSpace(BOOL = TRUE) const;
    inline const SvxHangingPunctuationItem &GetHangingPunctuation(BOOL = TRUE) const;

    // TabellenBox-Attribute    - impl. steht im cellatr.hxx
    inline const SwTblBoxNumFormat      &GetTblBoxNumFmt( BOOL = TRUE ) const;
    inline const SwTblBoxFormula        &GetTblBoxFormula( BOOL = TRUE ) const;
    inline const SwTblBoxValue          &GetTblBoxValue( BOOL = TRUE ) const;
};

// --------------- Inline Implementierungen ------------------------

inline const SfxPoolItem& SwFmt::GetAttr( USHORT nWhich,
                                        BOOL bInParents ) const
{
    return aSet.Get( nWhich, bInParents );
}

inline void SwFmt::SetName( const String& rNewName )
{
    ASSERT(!IsDefault(), "SetName: Defaultformat" );
    aFmtName = rNewName;
}
inline void SwFmt::SetName( const sal_Char* pNewName )
{
    ASSERT(!IsDefault(), "SetName: Defaultformat" );
    aFmtName.AssignAscii( pNewName );
}

inline SfxItemState SwFmt::GetItemState( USHORT nWhich, BOOL bSrchInParent,
                                        const SfxPoolItem **ppItem ) const
{
    return aSet.GetItemState( nWhich, bSrchInParent, ppItem );
}


#endif // _FORMAT_HXX
