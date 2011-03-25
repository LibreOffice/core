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
#ifndef _FORMAT_HXX
#define _FORMAT_HXX

#include <tools/solar.h>
#include "swdllapi.h"
#include <errhdl.hxx>       // fuer ASSERT
#include <swatrset.hxx>     // fuer SfxItemPool/-Set, Attr forward decl.
#include <calbck.hxx>       // fuer SwModify
#include <hintids.hxx>

class IDocumentSettingAccess;
class IDocumentDrawModelAccess;
class IDocumentLayoutAccess;
class IDocumentTimerAccess;
class IDocumentFieldsAccess;
class IDocumentChartDataProviderAccess;
class SwDoc;

class SW_DLLPUBLIC SwFmt : public SwModify
{
    String aFmtName;
    SwAttrSet aSet;

    sal_uInt16 nWhichId;
    sal_uInt16 nFmtId;          // Format-ID fuer Lesen/Schreiben
    sal_uInt16 nPoolFmtId;      // Id-fuer "automatich" erzeugte Formate
                            // (ist keine harte Attributierung !!)
    sal_uInt16 nPoolHelpId;     // HelpId fuer diese Pool-Vorlage
    sal_uInt8 nPoolHlpFileId;   // FilePos ans Doc auf die Vorlagen-Hilfen
    sal_Bool   bWritten : 1;    // sal_True: bereits geschrieben
    sal_Bool   bAutoFmt : 1;    // sal_False: es handelt sich um eine Vorlage
                            // ist dflt immer auf sal_True !
    sal_Bool   bFmtInDTOR : 1;  // sal_True: das Format wird geloscht. Damit man in
                            // der FmtChg-Message das erkennen kann!!!
    sal_Bool   bAutoUpdateFmt : 1;  // sal_True: am Format werden die Attribute
                            // eines kompletten Absatzes gesetzt (UI-seitig!)

protected:
    SwFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
            const sal_uInt16* pWhichRanges, SwFmt *pDrvdFrm, sal_uInt16 nFmtWhich );
    SwFmt( SwAttrPool& rPool, const String &rFmtNm, const sal_uInt16* pWhichRanges,
            SwFmt *pDrvdFrm, sal_uInt16 nFmtWhich );
    SwFmt( const SwFmt& rFmt );
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNewValue );

public:
    TYPEINFO();     //Bereits in Basisklasse Client drin.

    virtual ~SwFmt();
    SwFmt &operator=(const SwFmt&);

    // fuer die Abfrage der Writer-Funktionen
    sal_uInt16 Which() const { return nWhichId; }

        // erfrage vom Format Informationen
    virtual sal_Bool GetInfo( SfxPoolItem& ) const;

    // kopiere Attribute; auch ueber Dokumentgrenzen hinweg
    void CopyAttrs( const SwFmt&, sal_Bool bReplace=sal_True );

    // loesche alle Attribute, die nicht in rFmt stehen
    void DelDiffs( const SfxItemSet& rSet );
    void DelDiffs( const SwFmt& rFmt ) { DelDiffs( rFmt.GetAttrSet() ); }

    // Umhaengen des Formats (0 = Default)
    sal_Bool SetDerivedFrom(SwFmt *pDerivedFrom = 0);

    // Ist bInParents sal_False,
    // wird nur in diesem Format nach dem Attribut gesucht.
    inline const SfxPoolItem& GetFmtAttr( sal_uInt16 nWhich,
                                          sal_Bool bInParents = sal_True ) const;
    inline SfxItemState GetItemState( sal_uInt16 nWhich, sal_Bool bSrchInParent = sal_True,
                                    const SfxPoolItem **ppItem = 0 ) const;
    // --> OD 2008-03-03 #refactorlists#
    // methods renamed and made virtual
    virtual sal_Bool SetFmtAttr( const SfxPoolItem& rAttr );
    virtual sal_Bool SetFmtAttr( const SfxItemSet& rSet );
    virtual sal_Bool ResetFmtAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 = 0 );
    // <--

    // --> OD 2007-01-24 #i73790#
    // Method renamed and made virtual
    // Nimmt alle Hints aus dem Delta-Array,
    // liefert die Anzahl der geloeschten Hints
    virtual sal_uInt16 ResetAllFmtAttr();
    // <--

    inline SwFmt* DerivedFrom() const { return (SwFmt*)GetRegisteredIn(); }
    inline sal_Bool IsDefault() const { return DerivedFrom() == 0; }

    inline const String& GetName() const        { return aFmtName; }
    void SetName( const String& rNewName, sal_Bool bBroadcast=sal_False );
    inline void SetName( const sal_Char* pNewName,
                         sal_Bool bBroadcast=sal_False);

    // zur Abfrage des Attribute Arrays
    inline const SwAttrSet& GetAttrSet() const { return aSet; }

    // Das Doc wird jetzt am SwAttrPool gesetzt. Dadurch hat man es immer
    // im Zugriff.
    const SwDoc *GetDoc() const         { return aSet.GetDoc(); }
          SwDoc *GetDoc()               { return aSet.GetDoc(); }

    /** Provides access to the document settings interface
     */
    const IDocumentSettingAccess* getIDocumentSettingAccess() const;

    /** Provides access to the document draw model interface
     */
    const IDocumentDrawModelAccess* getIDocumentDrawModelAccess() const;
          IDocumentDrawModelAccess* getIDocumentDrawModelAccess();

    /** Provides access to the document layout interface
     */
    const IDocumentLayoutAccess* getIDocumentLayoutAccess() const;
          IDocumentLayoutAccess* getIDocumentLayoutAccess();

     /** Provides access to the document idle timer interface
     */
    IDocumentTimerAccess* getIDocumentTimerAccess();

     /** Provides access to the document idle timer interface
     */
    IDocumentFieldsAccess* getIDocumentFieldsAccess();

     /** gives access to the chart data-provider
     */
    IDocumentChartDataProviderAccess* getIDocumentChartDataProviderAccess();

    // erfragen und setzen der Poolvorlagen-Id's
    sal_uInt16 GetPoolFmtId() const { return nPoolFmtId; }
    void SetPoolFmtId( sal_uInt16 nId ) { nPoolFmtId = nId; }

    // erfragen und setzen der Hilfe-Id's fuer die Document-Vorlagen
    sal_uInt16 GetPoolHelpId() const { return nPoolHelpId; }
    void SetPoolHelpId( sal_uInt16 nId ) { nPoolHelpId = nId; }
    sal_uInt8 GetPoolHlpFileId() const { return nPoolHlpFileId; }
    void SetPoolHlpFileId( sal_uInt8 nId ) { nPoolHlpFileId = nId; }
    // erfrage die Attribut-Beschreibung, returnt den reingereichten String
    void GetPresentation( SfxItemPresentation ePres,
        SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String &rText ) const
        { aSet.GetPresentation( ePres, eCoreMetric, ePresMetric, rText ); }
    // Das Format-ID fuer Lesen/Schreiben:
    void   ResetWritten()    { bWritten = sal_False; }

    // Abfragen/Setzen vom AutoFmt-Flag
    sal_Bool IsAuto() const                 { return bAutoFmt; }
    void SetAuto( sal_Bool bNew = sal_False )   { bAutoFmt = bNew; }

    // Abfragen/Setzen vom bAutoUpdateFmt-Flag
    sal_Bool IsAutoUpdateFmt() const                { return bAutoUpdateFmt; }
    void SetAutoUpdateFmt( sal_Bool bNew = sal_True )   { bAutoUpdateFmt = bNew; }

    sal_Bool IsFmtInDTOR() const { return bFmtInDTOR; }

    // GetMethoden: das Bool gibt an, ob nur im Set (sal_False) oder auch in
    //              den Parents gesucht werden soll. Wird nichts gefunden,
    //              wird das deflt. Attribut returnt.
    // Charakter-Attribute  - impl. steht im charatr.hxx
    // AMA 12.10.94: Umstellung von SwFmt... auf Svx...
    inline const SvxPostureItem      &GetPosture( sal_Bool = sal_True ) const;
    inline const SvxWeightItem       &GetWeight( sal_Bool = sal_True ) const;
    inline const SvxShadowedItem     &GetShadowed( sal_Bool = sal_True ) const;
    inline const SvxAutoKernItem     &GetAutoKern( sal_Bool = sal_True ) const;
    inline const SvxWordLineModeItem &GetWordLineMode( sal_Bool = sal_True ) const;
    inline const SvxContourItem      &GetContour( sal_Bool = sal_True ) const;
    inline const SvxKerningItem      &GetKerning( sal_Bool = sal_True ) const;
    inline const SvxUnderlineItem    &GetUnderline( sal_Bool = sal_True ) const;
    inline const SvxOverlineItem     &GetOverline( sal_Bool = sal_True ) const;
    inline const SvxCrossedOutItem   &GetCrossedOut( sal_Bool = sal_True ) const;
    inline const SvxFontHeightItem   &GetSize( sal_Bool = sal_True ) const;
    inline const SvxPropSizeItem     &GetPropSize( sal_Bool = sal_True ) const;
    inline const SvxFontItem         &GetFont( sal_Bool = sal_True ) const;
    inline const SvxColorItem        &GetColor( sal_Bool = sal_True ) const;
    inline const SvxCharSetColorItem &GetCharSetColor( sal_Bool = sal_True ) const;
    inline const SvxLanguageItem     &GetLanguage( sal_Bool = sal_True ) const;
    inline const SvxEscapementItem   &GetEscapement( sal_Bool = sal_True ) const;
    inline const SvxCaseMapItem      &GetCaseMap( sal_Bool = sal_True ) const;
    inline const SvxNoHyphenItem     &GetNoHyphenHere( sal_Bool = sal_True ) const;
    inline const SvxBlinkItem        &GetBlink( sal_Bool = sal_True ) const;
    inline const SvxBrushItem        &GetChrBackground( sal_Bool = sal_True ) const;

    inline const SvxFontItem         &GetCJKFont( sal_Bool = sal_True ) const;
    inline const SvxFontHeightItem   &GetCJKSize( sal_Bool = sal_True ) const;
    inline const SvxLanguageItem     &GetCJKLanguage( sal_Bool = sal_True ) const;
    inline const SvxPostureItem      &GetCJKPosture( sal_Bool = sal_True ) const;
    inline const SvxWeightItem       &GetCJKWeight( sal_Bool = sal_True ) const;
    inline const SvxFontItem         &GetCTLFont( sal_Bool = sal_True ) const;
    inline const SvxFontHeightItem   &GetCTLSize( sal_Bool = sal_True ) const;
    inline const SvxLanguageItem     &GetCTLLanguage( sal_Bool = sal_True ) const;
    inline const SvxPostureItem      &GetCTLPosture( sal_Bool = sal_True ) const;
    inline const SvxWeightItem       &GetCTLWeight( sal_Bool = sal_True ) const;
    inline const SfxBoolItem           &GetWritingDirection( sal_Bool = sal_True ) const;
    inline const SvxEmphasisMarkItem &GetEmphasisMark( sal_Bool = sal_True ) const;
    inline const SvxTwoLinesItem   &Get2Lines( sal_Bool = sal_True ) const;
    inline const SvxCharScaleWidthItem &GetCharScaleW( sal_Bool = sal_True ) const;
    inline const SvxCharRotateItem     &GetCharRotate( sal_Bool = sal_True ) const;
    inline const SvxCharReliefItem     &GetCharRelief( sal_Bool = sal_True ) const;
    inline const SvxCharHiddenItem   &GetCharHidden( sal_Bool = sal_True ) const;

    // Frame-Attribute  - impl. steht im frmatr.hxx,
    inline const SwFmtFillOrder           &GetFillOrder( sal_Bool = sal_True ) const;
    inline const SwFmtFrmSize             &GetFrmSize( sal_Bool = sal_True ) const;
    inline const SwFmtHeader          &GetHeader( sal_Bool = sal_True ) const;
    inline const SwFmtFooter          &GetFooter( sal_Bool = sal_True ) const;
    inline const SwFmtSurround            &GetSurround( sal_Bool = sal_True ) const;
    inline const SwFmtHoriOrient      &GetHoriOrient( sal_Bool = sal_True ) const;
    inline const SwFmtAnchor          &GetAnchor( sal_Bool = sal_True ) const;
    inline const SwFmtCol                 &GetCol( sal_Bool = sal_True ) const;
    inline const SvxPaperBinItem      &GetPaperBin( sal_Bool = sal_True ) const;
    inline const SvxLRSpaceItem           &GetLRSpace( sal_Bool = sal_True ) const;
    inline const SvxULSpaceItem           &GetULSpace( sal_Bool = sal_True ) const;
    inline const SwFmtCntnt           &GetCntnt( sal_Bool = sal_True ) const;
    inline const SvxPrintItem             &GetPrint( sal_Bool = sal_True ) const;
    inline const SvxOpaqueItem            &GetOpaque( sal_Bool = sal_True ) const;
    inline const SvxProtectItem           &GetProtect( sal_Bool = sal_True ) const;
    inline const SwFmtVertOrient      &GetVertOrient( sal_Bool = sal_True ) const;
    inline const SvxBoxItem               &GetBox( sal_Bool = sal_True ) const;
    inline const SvxFmtKeepItem         &GetKeep( sal_Bool = sal_True ) const;
    inline const SvxBrushItem           &GetBackground( sal_Bool = sal_True ) const;
    inline const SvxShadowItem            &GetShadow( sal_Bool = sal_True ) const;
    inline const SwFmtPageDesc            &GetPageDesc( sal_Bool = sal_True ) const;
    inline const SvxFmtBreakItem      &GetBreak( sal_Bool = sal_True ) const;
    inline const SvxMacroItem             &GetMacro( sal_Bool = sal_True ) const;
    inline const SwFmtURL             &GetURL( sal_Bool = sal_True ) const;
    inline const SwFmtEditInReadonly  &GetEditInReadonly( sal_Bool = sal_True ) const;
    inline const SwFmtLayoutSplit     &GetLayoutSplit( sal_Bool = sal_True ) const;
    inline const SwFmtRowSplit          &GetRowSplit( sal_Bool = sal_True ) const;
    inline const SwFmtChain               &GetChain( sal_Bool = sal_True ) const;
    inline const SwFmtLineNumber      &GetLineNumber( sal_Bool = sal_True ) const;
    inline const SwFmtFtnAtTxtEnd     &GetFtnAtTxtEnd( sal_Bool = sal_True ) const;
    inline const SwFmtEndAtTxtEnd     &GetEndAtTxtEnd( sal_Bool = sal_True ) const;
    inline const SwFmtNoBalancedColumns &GetBalancedColumns( sal_Bool = sal_True ) const;
    inline const SvxFrameDirectionItem    &GetFrmDir( sal_Bool = sal_True ) const;
    inline const SwTextGridItem         &GetTextGrid( sal_Bool = sal_True ) const;
    inline const SwHeaderAndFooterEatSpacingItem &GetHeaderAndFooterEatSpacing( sal_Bool = sal_True ) const;
    // OD 18.09.2003 #i18732#
    inline const SwFmtFollowTextFlow    &GetFollowTextFlow(sal_Bool = sal_True) const;
    // OD 2004-05-05 #i28701#
    inline const SwFmtWrapInfluenceOnObjPos& GetWrapInfluenceOnObjPos(sal_Bool = sal_True) const;

    // Grafik-Attribute - impl. steht im grfatr.hxx
    inline const SwMirrorGrf          &GetMirrorGrf( sal_Bool = sal_True ) const;
    inline const SwCropGrf            &GetCropGrf( sal_Bool = sal_True ) const;
    inline const SwRotationGrf            &GetRotationGrf(sal_Bool = sal_True ) const;
    inline const SwLuminanceGrf       &GetLuminanceGrf(sal_Bool = sal_True ) const;
    inline const SwContrastGrf            &GetContrastGrf(sal_Bool = sal_True ) const;
    inline const SwChannelRGrf            &GetChannelRGrf(sal_Bool = sal_True ) const;
    inline const SwChannelGGrf            &GetChannelGGrf(sal_Bool = sal_True ) const;
    inline const SwChannelBGrf            &GetChannelBGrf(sal_Bool = sal_True ) const;
    inline const SwGammaGrf           &GetGammaGrf(sal_Bool = sal_True ) const;
    inline const SwInvertGrf          &GetInvertGrf(sal_Bool = sal_True ) const;
    inline const SwTransparencyGrf        &GetTransparencyGrf(sal_Bool = sal_True ) const;
    inline const SwDrawModeGrf            &GetDrawModeGrf(sal_Bool = sal_True ) const;

    // Paragraph-Attribute  - impl. steht im paratr.hxx
    inline const SvxLineSpacingItem       &GetLineSpacing( sal_Bool = sal_True ) const;
    inline const SvxAdjustItem            &GetAdjust( sal_Bool = sal_True ) const;
    inline const SvxFmtSplitItem      &GetSplit( sal_Bool = sal_True ) const;
    inline const SwRegisterItem           &GetRegister( sal_Bool = sal_True ) const;
    inline const SwNumRuleItem            &GetNumRule( sal_Bool = sal_True ) const;
    inline const SvxWidowsItem            &GetWidows( sal_Bool = sal_True ) const;
    inline const SvxOrphansItem           &GetOrphans( sal_Bool = sal_True ) const;
    inline const SvxTabStopItem           &GetTabStops( sal_Bool = sal_True ) const;
    inline const SvxHyphenZoneItem        &GetHyphenZone( sal_Bool = sal_True ) const;
    inline const SwFmtDrop                &GetDrop( sal_Bool = sal_True ) const;
    inline const SvxScriptSpaceItem       &GetScriptSpace(sal_Bool = sal_True) const;
    inline const SvxHangingPunctuationItem &GetHangingPunctuation(sal_Bool = sal_True) const;
    inline const SvxForbiddenRuleItem     &GetForbiddenRule(sal_Bool = sal_True) const;
    inline const SvxParaVertAlignItem &GetParaVertAlign(sal_Bool = sal_True) const;
    inline const SvxParaGridItem        &GetParaGrid(sal_Bool = sal_True) const;
    inline const SwParaConnectBorderItem &GetParaConnectBorder(sal_Bool = sal_True ) const;

    // TabellenBox-Attribute    - impl. steht im cellatr.hxx
    inline  const SwTblBoxNumFormat     &GetTblBoxNumFmt( sal_Bool = sal_True ) const;
    inline  const SwTblBoxFormula       &GetTblBoxFormula( sal_Bool = sal_True ) const;
    inline  const SwTblBoxValue         &GetTblBoxValue( sal_Bool = sal_True ) const;

    /** SwFmt::IsBackgroundTransparent - for feature #99657#

        OD 22.08.2002
        Virtual method to determine, if background of format is transparent.
        Default implementation returns false. Thus, subclasses have to overload
        method, if the specific subclass can have a transparent background.

        @author OD

        @return false, default implementation
    */
    virtual sal_Bool IsBackgroundTransparent() const;

    /** SwFmt::IsShadowTransparent - for feature #99657#

        OD 22.08.2002
        Virtual method to determine, if shadow of format is transparent.
        Default implementation returns false. Thus, subclasses have to overload
        method, if the specific subclass can have a transparent shadow.

        @author OD

        @return false, default implementation
    */
    virtual sal_Bool IsShadowTransparent() const;
};

// --------------- inline Implementierungen ------------------------

inline const SfxPoolItem& SwFmt::GetFmtAttr( sal_uInt16 nWhich,
                                             sal_Bool bInParents ) const
{
    return aSet.Get( nWhich, bInParents );
}

inline void SwFmt::SetName( const sal_Char* pNewName,
                             sal_Bool bBroadcast )
{
    String aTmp( String::CreateFromAscii( pNewName ) );
    SetName( aTmp, bBroadcast );
}

inline SfxItemState SwFmt::GetItemState( sal_uInt16 nWhich, sal_Bool bSrchInParent,
                                        const SfxPoolItem **ppItem ) const
{
    return aSet.GetItemState( nWhich, bSrchInParent, ppItem );
}

#undef inline

#endif // _FORMAT_HXX
