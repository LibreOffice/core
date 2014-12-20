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

#ifndef INCLUDED_RSC_INC_RSCDB_HXX
#define INCLUDED_RSC_INC_RSCDB_HXX

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
class RscTupel;
class RscCont;
class RscCmdLine;
enum class SymbolType : sal_uInt16;
enum class ToolBoxItemBits;
enum class WindowBorderStyle : sal_Int16;
enum class KeyFuncType : sal_Int32;
enum class MenuItemBits : sal_Int16;
enum class ToolBoxItemType;
enum class ButtonType;

struct WriteRcContext
{
    FILE *              fOutput;
    OString        aOutputRc;
    OString        aOutputSysList;
    RscCmdLine*         pCmdLine;
};

// Liste die alle Basistypen enthaelt
typedef ::std::vector< RscTop* > RscBaseList;

// Tabelle fuer Systemabhaengige Resourcen
struct RscSysEntry
{
    sal_uInt32      nKey;
    sal_uInt32      nRscTyp;
    OString    aFileName;
    sal_uInt32      nTyp;
    sal_uInt32      nRefId;
};

typedef ::std::vector< RscSysEntry* > RscSysList;

class RscTypCont
{
    rtl_TextEncoding    nSourceCharSet;
    sal_uInt32          nMachineId;         // Globaler Maschinentyp
    RSCBYTEORDER_TYPE   nByteOrder;         // Intel oder
    OString             aLanguage;          // output language
    std::vector< sal_uInt32 > aLangFallbacks;   // language fallback list (entry 0 is language itself)
    OString             aSearchPath;        // Suchen der Bitmap, Icon, Pointer
    OString             aSysSearchPath;     // aSearchPath plus language specific paths
    sal_uInt32          nUniqueId;          // eindeutiger Id fuer Systemresourcen
    sal_uLong           nFilePos;           // Position in der Datei ( MTF )
    sal_uInt32          nPMId;              // eindeutiger Id fuer PM-Rseourcefile
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
    Atom                nStdPopupId;
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
    inline void SETCONST( RscConst *p1, const char * p2, SymbolType p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, Atom p2, ToolBoxItemBits p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, Atom p2, WindowBorderStyle p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, const char * p2, KeyFuncType p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, const char * p2, MenuItemBits p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, Atom p2, MenuItemBits p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, const char * p2, ToolBoxItemType p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, Atom p2, ButtonType p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, const char * p2, ButtonType p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    RscEnum *   InitLangType();
    RscEnum *   InitFieldUnitsType();
    RscEnum *   InitColor();
    RscEnum *   InitMapUnit();
    RscEnum *   InitKey();
    RscEnum *   InitTriState();
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
    RscTop *    InitClassImageList( RscTop * pSuper,
                                    RscTop * pClassColor, RscCont * pStrLst );
    RscTop *    InitClassWindow( RscTop * pSuper, RscEnum * pMapUnit,
                                 RscArray * pLangGeo );
    RscTop *    InitClassSystemWindow( RscTop * pSuper );
    RscTop *    InitClassWorkWindow( RscTop * pSuper );
    RscTop *    InitClassControl( RscTop * pSuper );
    RscTop *    InitClassCheckBox( RscTop * pSuper );
    RscTop *    InitClassPushButton( RscTop * pSuper );
    RscTop *    InitClassImageButton( RscTop * pSuper, RscTop * pClassImage,
                                    RscEnum * pTriState );
    RscTop *    InitClassEdit( RscTop * pSuper );
    RscTop *    InitClassScrollBar( RscTop * pSuper );
    RscTop *    InitClassListBox( RscTop * pSuper, RscArray * pStrLst );
    RscTop *    InitClassComboBox( RscTop * pSuper, RscArray * pStrLst );
    RscTop *    InitClassFixedText( RscTop * pSuper );
    RscTop *    InitClassFixedImage( RscTop * pSuper, RscTop * pClassImage );
    RscTop *    InitClassRadioButton( RscTop * pSuper );
    RscTop *    InitClassKeyCode( RscTop * pSuper, RscEnum * pKey );
    RscTop *    InitClassAccelItem( RscTop * pSuper, RscTop * pKeyCode );
    RscTop *    InitClassAccel( RscTop * pSuper, RscTop * pClassAccelItem );
    RscTop *    InitClassMenuItem( RscTop * pSuper, RscTop * pClassBitmap,
                                   RscTop * pClassKeyCode );
    RscTop *    InitClassMenu( RscTop * pSuper, RscTop * pMenuItem );
    RscTop *    InitClassSplitWindow( RscTop * pSuper );

    RscTop *    InitClassNumericFormatter( RscTop * pSuper );
    RscTop *    InitClassMetricFormatter( RscTop * pSuper,
                                          RscEnum * pFieldUnits );

    RscTop *    InitClassSpinField( RscTop * pSuper );
    RscTop *    InitClassNumericField( RscTop * pSuper );
    RscTop *    InitClassMetricField( RscTop * pSuper );

    RscTop *    InitClassDockingWindow( RscTop * pSuper,
                                        RscEnum * pMapUnit );
    RscTop *    InitClassToolBoxItem( RscTop * pSuper, RscTop * pClassBitmap,
                                      RscTop * pClassImage,
                                      RscEnum * pTriState );
    RscTop *    InitClassToolBox( RscTop * pSuper, RscTop * pClassToolBoxItem,
                                  RscTop * pClassImageList );
    RscTop *    InitClassSfxStyleFamilyItem( RscTop * pSuper,
                                             RscTop * pClassBitmap,
                                             RscTop * pClassImage,
                                             RscArray * pStrLst );
    RscTop *    InitClassSfxTemplateDialog(  RscTop * pSuper,
                                             RscTop * pStyleFamily );
    RscTop *    InitClassSfxSlotInfo( RscTop * pSuper );

    void        InsWinBit( RscTop * pClass, const OString& rName,
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
    sal_uInt32          nFlags;
    std::map<sal_uInt64, sal_uLong> aIdTranslator; //Ordnet Resourcetypen und Id's einen Id zu
                                       //(unter PM), oder eine Dateiposition (MTF)

    RscTypCont( RscError *, RSCBYTEORDER_TYPE, const OString& rSearchPath, sal_uInt32 nFlags );
    ~RscTypCont();

    Atom AddLanguage( const char* );

    bool              IsPreload() const
                          { return (nFlags & PRELOAD_FLAG) ? true : false; }
    bool              IsSysResTest() const
                          { return (nFlags & NOSYSRESTEST_FLAG) ? false : true; }
    bool              IsSrsDefault() const
                          { return (nFlags & SRSDEFAULT_FLAG) ? true : false; }
    OString           ChangeLanguage(const OString & rNewLang);
    const std::vector< sal_uInt32 >& GetFallbacks() const
                          { return aLangFallbacks; }

    RSCBYTEORDER_TYPE GetByteOrder() const { return nByteOrder; }
    rtl_TextEncoding  GetSourceCharSet() const { return nSourceCharSet; }
    rtl_TextEncoding  SetSourceCharSet( rtl_TextEncoding aCharSet )
                          {
                              rtl_TextEncoding aOld = nSourceCharSet;
                              nSourceCharSet = aCharSet;
                              return aOld;
                          }
    void              SetSearchPath( const OString& rStr) { aSearchPath = rStr; }
    OString           GetSearchPath() const { return aSearchPath; }
    void              SetSysSearchPath( const OString& rStr ) { aSysSearchPath = rStr; }
    void              InsertType( RscTop * pType )
                          {
                              aBaseLst.push_back( pType );
                          }
    RscTop  *         SearchType( Atom nTypId );
                      // loescht alle Resourceobjekte diese Datei
    void              Delete( sal_uLong lFileKey );
    RscTop  *         GetRoot()         { return pRoot; }
    sal_uInt32        PutSysName( sal_uInt32 nRscTyp, char * pName, sal_uInt32 nConst,
                                  sal_uInt32 nId, bool bFirst );
    void              ClearSysNames();
    ERRTYPE           WriteRc( WriteRcContext& rContext );
    void              WriteSrc( FILE * fOutput, sal_uLong nFileIndex,
                                bool bName = true );
    sal_uInt32        PutTranslatorKey( sal_uInt64 nKey );
    void              IncFilePos( sal_uLong nOffset ){ nFilePos += nOffset; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
