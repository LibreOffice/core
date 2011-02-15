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

#ifndef _RSCDB_HXX
#define _RSCDB_HXX

#include <rscall.h>
#include <rsckey.hxx>
#include <rscconst.hxx>
#include <rscflag.hxx>
#include <rscrange.hxx>
#include <rscstr.hxx>
#include <rscarray.hxx>
#include <rscdef.hxx>

#include <vector>
#include <map>

class RscError;
class REResourceList;
class RscTupel;
class RscCont;
class RscCmdLine;

struct WriteRcContext
{
    FILE *              fOutput;
    rtl::OString        aOutputRc;
    rtl::OString        aOutputSysList;
    RscCmdLine*         pCmdLine;
};

/****************** R s c T y p C o n ************************************/
// Liste die alle Basistypen enthaelt
DECLARE_LIST( RscBaseList, RscTop * )

// Tabelle fuer Systemabhaengige Resourcen
struct RscSysEntry
{
    sal_uInt32      nKey;
    sal_uInt32      nRscTyp;
    ByteString  aFileName;
    sal_uInt32      nTyp;
    sal_uInt32      nRefId;
};
DECLARE_LIST( RscSysList, RscSysEntry * )

class RscTypCont
{
    CharSet             nSourceCharSet;
    sal_uInt32              nMachineId;         // Globaler Maschinentyp
    RSCBYTEORDER_TYPE   nByteOrder;         // Intel oder
    ByteString          aLanguage;          // output language
    std::vector< sal_uInt32 > aLangFallbacks;   // language fallback list (entry 0 is language itself)
    ByteString          aSearchPath;        // Suchen der Bitmap, Icon, Pointer
    ByteString          aSysSearchPath;     // aSearchPath plus language specific paths
    sal_uInt32              nUniqueId;          // eindeutiger Id fuer Systemresourcen
    sal_uLong               nFilePos;           // Position in der Datei ( MTF )
    sal_uInt32              nPMId;              // eindeutiger Id fuer PM-Rseourcefile
                                            // muss groesser als RSC_VERSIONCONTROL_ID sein
    RscTop  *           pRoot;              // Zeiger auf die Wurzel vom Typenbaum
    RSCINST             aVersion;           // Versionskontrollinstanz

    RscBaseList         aBaseLst;           // Liste der einfachen Resourceklasse
    RscSysList          aSysLst;            // Liste der Systemresourcen

    Atom                nWinBitVarId;       // Name der Winbitvariablen
    Atom                nBorderId;
    Atom                nHideId;
    Atom                nClipChildrenId;
    Atom                nSizeableId;
    Atom                nMoveableId;
    Atom                nMinimizeId;
    Atom                nMaximizeId;
    Atom                nCloseableId;
    Atom                nAppId;
    Atom                nTabstopId;
    Atom                nGroupId;
    Atom                nSysmodalId;
    Atom                nLeftId;
    Atom                nCenterId;
    Atom                nRightId;
    Atom                nTopId;
    Atom                nVCenterId;
    Atom                nBottomId;
    Atom                nHScrollId;
    Atom                nVScrollId;
    Atom                nSortId;
    Atom                nDefaultId;
    Atom                nSVLookId;
    Atom                nRepeatId;
    Atom                nDropDownId;
    Atom                nPassWordId;
    Atom                nReadOnlyId;
    Atom                nAutoSizeId;
    Atom                nSpinId;
    Atom                nTabControlId;
    Atom                nSimpleModeId;
    Atom                nDragId;
    Atom                nScrollId;
    Atom                nZoomableId;
    Atom                nHideWhenDeactivateId;
    Atom                nAutoHScrollId;
    Atom                nAutoVScrollId;
    Atom                nDDExtraWidthId;
    Atom                nWordBreakId;
    Atom                nLeftLabelId;
    Atom                nHasLinesId;
    Atom                nHasButtonsId;
    Atom                nRectStyleId;
    Atom                nLineSpacingId;
    Atom                nSmallStyleId;
    Atom                nEnableResizingId;
    Atom                nDockableId;
    Atom                nScaleId;
    Atom                nIgnoreTabId;
    Atom                nNoSplitDrawId;
    Atom                nTopImageId;
    Atom                nNoLabelId;
    Atom                nVertId;
    Atom                nSysWinId;

    void        Init();         // Initialisiert Klassen und Tabelle
    void        SETCONST( RscConst *, const char *, sal_uInt32 );
    void        SETCONST( RscConst *, Atom, sal_uInt32 );
    RscEnum *   InitLangType();
    RscEnum *   InitFieldUnitsType();
    RscEnum *   InitTimeFieldFormat();
    RscEnum *   InitColor();
    RscEnum *   InitMapUnit();
    RscEnum *   InitKey();
    RscEnum *   InitTriState();
    RscEnum *   InitMessButtons();
    RscEnum *   InitMessDefButton();
    RscTupel *  InitGeometry();
    RscArray *  InitLangGeometry( RscTupel * pGeo );
    RscArray *  InitSystemGeometry( RscTupel * pGeo );
    RscCont  *  InitStringList();
    RscArray *  InitLangStringList( RscCont * pStrLst );
    RscTupel *  InitStringTupel();
    RscTupel *  InitStringLongTupel();
    RscCont  *  InitStringTupelList( RscTupel * pStringTupel );
    RscCont  *  InitStringLongTupelList( RscTupel * pStringLongTupel );
    RscArray *  InitLangStringTupelList( RscCont * pStrTupelLst );
    RscArray *  InitLangStringLongTupelList( RscCont * pStrLongTupelLst );

    RscTop *    InitClassMgr();
    RscTop *    InitClassString( RscTop * pSuper );
    RscTop *    InitClassBitmap( RscTop * pSuper );
    RscTop *    InitClassColor( RscTop * pSuper, RscEnum * pColor );
    RscTop *    InitClassImage( RscTop * pSuper, RscTop *pClassBitmap,
                                RscTop * pClassColor );
    RscTop *    InitClassImageList( RscTop * pSuper, RscTop *pClassBitmap,
                                    RscTop * pClassColor, RscCont * pStrLst );
    RscTop *    InitClassWindow( RscTop * pSuper, RscEnum * pMapUnit,
                                 RscArray * pLangGeo );
    RscTop *    InitClassSystemWindow( RscTop * pSuper );
    RscTop *    InitClassWorkWindow( RscTop * pSuper );
    RscTop *    InitClassModalDialog( RscTop * pSuper );
    RscTop *    InitClassModelessDialog( RscTop * pSuper );
    RscTop *    InitClassControl( RscTop * pSuper );
    RscTop *    InitClassCheckBox( RscTop * pSuper );
    RscTop *    InitClassPushButton( RscTop * pSuper );
    RscTop *    InitClassTriStateBox( RscTop * pSuper, RscEnum * pTriState );
    RscTop *    InitClassMenuButton( RscTop * pSuper, RscTop * pClasMenu );
    RscTop *    InitClassImageButton( RscTop * pSuper, RscTop * pClassImage,
                                    RscEnum * pTriState );
    RscTop *    InitClassEdit( RscTop * pSuper );
    RscTop *    InitClassMultiLineEdit( RscTop * pSuper );
    RscTop *    InitClassScrollBar( RscTop * pSuper );
    RscTop *    InitClassListBox( RscTop * pSuper, RscArray * pStrLst );
    RscTop *    InitClassMultiListBox( RscTop * pSuper );
    RscTop *    InitClassComboBox( RscTop * pSuper, RscArray * pStrLst );
    RscTop *    InitClassFixedText( RscTop * pSuper );
    RscTop *    InitClassFixedBitmap( RscTop * pSuper, RscTop * pClassBitmap );
    RscTop *    InitClassFixedImage( RscTop * pSuper, RscTop * pClassImage );
    RscTop *    InitClassRadioButton( RscTop * pSuper );
    RscTop *    InitClassImageRadioButton( RscTop * pSuper, RscTop * pClassImage );
    RscTop *    InitClassKeyCode( RscTop * pSuper, RscEnum * pKey );
    RscTop *    InitClassAccelItem( RscTop * pSuper, RscTop * pKeyCode );
    RscTop *    InitClassAccel( RscTop * pSuper, RscTop * pClassAccelItem );
    RscTop *    InitClassMenuItem( RscTop * pSuper, RscTop * pClassBitmap,
                                   RscTop * pClassKeyCode );
    RscTop *    InitClassMenu( RscTop * pSuper, RscTop * pMenuItem );
    RscTop *    InitClassMessBox( RscTop * pSuper,
                                  RscEnum * pMessButtons,
                                  RscEnum * pMessDefButton );
    RscTop *    InitClassSplitter( RscTop * pSuper );
    RscTop *    InitClassSplitWindow( RscTop * pSuper );
    RscTop *    InitClassTime( RscTop * pSuper );
    RscTop *    InitClassDate( RscTop * pSuper );

    RscTop *    InitClassPatternFormatter( RscTop * pSuper );
    RscTop *    InitClassNumericFormatter( RscTop * pSuper );
    RscTop *    InitClassMetricFormatter( RscTop * pSuper,
                                          RscEnum * pFieldUnits );
    RscTop *    InitClassCurrencyFormatter( RscTop * pSuper,
                                            RscEnum * pFieldUnits );
    RscTop *    InitClassDateFormatter( RscTop * pSuper, RscTop * pClassDate );
    RscTop *    InitClassTimeFormatter( RscTop * pSuper, RscTop * pClassTime,
                                        RscEnum * pTimeFieldFormat );

    RscTop *    InitClassSpinField( RscTop * pSuper );
    RscTop *    InitClassPatternField( RscTop * pSuper );
    RscTop *    InitClassNumericField( RscTop * pSuper );
    RscTop *    InitClassMetricField( RscTop * pSuper );
    RscTop *    InitClassCurrencyField( const char * pClassName, sal_uInt32 nRT,
                                        RscTop * pSuper );
    RscTop *    InitClassDateField( RscTop * pSuper, RscTop * pClassDate );
    RscTop *    InitClassTimeField( RscTop * pSuper, RscTop * pClassTime );
    RscTop *    InitClassPatternBox( RscTop * pSuper );
    RscTop *    InitClassNumericBox( RscTop * pSuper );
    RscTop *    InitClassMetricBox( RscTop * pSuper );
    RscTop *    InitClassCurrencyBox( const char * pClassName, sal_uInt32 nRT,
                                        RscTop * pSuper );
    RscTop *    InitClassDateBox( RscTop * pSuper, RscTop * pClassDate );
    RscTop *    InitClassTimeBox( RscTop * pSuper, RscTop * pClassTime );

    RscTop *    InitClassDockingWindow( RscTop * pSuper,
                                        RscEnum * pMapUnit );
    RscTop *    InitClassToolBoxItem( RscTop * pSuper, RscTop * pClassBitmap,
                                      RscTop * pClassImage,
                                      RscEnum * pTriState );
    RscTop *    InitClassToolBox( RscTop * pSuper, RscTop * pClassToolBoxItem,
                                  RscTop * pClassImageList );
    RscTop *    InitClassStatusBar( RscTop * pSuper );
    RscTop *    InitClassMoreButton( RscTop * pSuper, RscEnum * pMapUnit );
    RscTop *    InitClassFloatingWindow( RscTop * pSuper,
                                         RscEnum * pMapUnit );
    RscTop *    InitClassTabControlItem( RscTop * pSuper,
                                         RscTop * pClassTabPage );
    RscTop *    InitClassTabControl(  RscTop * pSuper,
                                     RscTop * pClassTabControlItem );
    RscTop *    InitClassSfxStyleFamilyItem( RscTop * pSuper,
                                             RscTop * pClassBitmap,
                                             RscTop * pClassImage,
                                             RscArray * pStrLst );
    RscTop *    InitClassSfxTemplateDialog(  RscTop * pSuper,
                                             RscTop * pStyleFamily );
    RscTop *    InitClassSfxSlotInfo( RscTop * pSuper );

    void        InsWinBit( RscTop * pClass, const ByteString & rName,
                           Atom nVal );
    void        WriteInc( FILE * fOutput, sal_uLong lKey );

public:
    RscBool             aBool;
    RscRange            aShort;
    RscRange            aUShort;
    RscLongRange        aLong;
    RscLongEnumRange    aEnumLong;
    RscIdRange          aIdUShort;
    RscIdRange          aIdNoZeroUShort;
    RscBreakRange       aNoZeroShort;
    RscRange            a1to12Short;
    RscRange            a0to23Short;
    RscRange            a1to31Short;
    RscRange            a0to59Short;
    RscRange            a0to99Short;
    RscRange            a0to9999Short;
    RscIdRange          aIdLong;
    RscString           aString;
    RscString           aStringLiteral;
    RscFlag             aWinBits;
    RscLangEnum         aLangType;
    RscLangArray        aLangString;
    RscLangArray        aLangShort;

    Atom                nAcceleratorType;

    RscError*           pEH;        // Fehlerhandler
    RscNameTable        aNmTb;      // Tabelle fuer Namen
    RscFileTab          aFileTab;   // Tabelle der Dateinamen
    sal_uInt32              nFlags;
    std::map<sal_uInt64, sal_uLong> aIdTranslator; //Ordnet Resourcetypen und Id's einen Id zu
                                       //(unter PM), oder eine Dateiposition (MTF)

    RscTypCont( RscError *, RSCBYTEORDER_TYPE, const ByteString & rSearchPath, sal_uInt32 nFlags );
    ~RscTypCont();

    Atom AddLanguage( const char* );

    sal_Bool            IsPreload() const
                    { return (nFlags & PRELOAD_FLAG) ? sal_True : sal_False; }
    sal_Bool            IsSmart() const
                    { return (nFlags & SMART_FLAG) ? sal_True : sal_False; }
    sal_Bool            IsSysResTest() const
                    { return (nFlags & NOSYSRESTEST_FLAG) ? sal_False : sal_True; }
    sal_Bool            IsSrsDefault() const
                    { return (nFlags & SRSDEFAULT_FLAG) ? sal_True : sal_False; }
    ByteString      ChangeLanguage( const ByteString& rNewLang );
    const std::vector< sal_uInt32 >& GetFallbacks() const
    { return aLangFallbacks; }

    RSCBYTEORDER_TYPE GetByteOrder() const { return nByteOrder; }
    CharSet         GetSourceCharSet() const { return nSourceCharSet; }
    CharSet         SetSourceCharSet( CharSet aCharSet )
    {
        CharSet aOld = nSourceCharSet;
        nSourceCharSet = aCharSet;
        return aOld;
    }
    void            SetSearchPath( const ByteString & rStr) { aSearchPath = rStr; }
    ByteString      GetSearchPath() const { return aSearchPath; }
    void            SetSysSearchPath( const ByteString& rStr ) { aSysSearchPath = rStr; }
    ByteString      GetSysSearchPath() const { return aSysSearchPath; }
    void        InsertType( RscTop * pType )
                {
                    aBaseLst.Insert( pType, LIST_APPEND );
                }
    RscTop  *   SearchType( Atom nTypId );
    RscTop  *   Search( Atom typ );
    CLASS_DATA  Search( Atom typ, const RscId & rId );
    void        Delete( Atom typ, const RscId & rId );
                // loescht alle Resourceobjekte diese Datei
    void        Delete( sal_uLong lFileKey );
    RscTop  *   GetRoot()         { return( pRoot ); };
    sal_uInt32      PutSysName( sal_uInt32 nRscTyp, char * pName, sal_uInt32 nConst,
                            sal_uInt32 nId, sal_Bool bFirst );
    void        ClearSysNames();
    ERRTYPE     WriteRc( WriteRcContext& rContext );
    void        WriteSrc( FILE * fOutput, sal_uLong nFileIndex,
                          CharSet nCharSet, sal_Bool bName = sal_True );
    ERRTYPE     WriteHxx( FILE * fOutput, sal_uLong nFileKey);
    ERRTYPE     WriteCxx( FILE * fOutput, sal_uLong nFileKey,
                          const ByteString & rHxxName );
    void        WriteSyntax( FILE * fOutput );
    void        WriteRcCtor( FILE * fOutput );
    void        FillNameIdList( REResourceList * pList, sal_uLong lFileKey );
    sal_Bool        MakeConsistent( RscInconsList * pList );
    sal_uInt32      PutTranslatorKey( sal_uInt64 nKey );
    void        IncFilePos( sal_uLong nOffset ){ nFilePos += nOffset; }
};

#endif
