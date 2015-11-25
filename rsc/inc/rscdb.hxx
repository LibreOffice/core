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
enum class WindowAlign;

struct WriteRcContext
{
    FILE *              fOutput;
    OString        aOutputRc;
    OString        aOutputSysList;
    RscCmdLine*         pCmdLine;
};

// table for system dependent resources
struct RscSysEntry
{
    sal_uInt32      nKey;
    sal_uInt32      nRscTyp;
    OString    aFileName;
    sal_uInt32      nTyp;
    sal_uInt32      nRefId;
};

class RscTypCont
{
    rtl_TextEncoding    nSourceCharSet;
    RSCBYTEORDER_TYPE   nByteOrder;         // Intel oder
    OString             aLanguage;          // output language
    std::vector< sal_uInt32 > aLangFallbacks;   // language fallback list (entry 0 is language itself)
    OString             aSearchPath;        // search path for bitmap, icon and pointer
    OString             aSysSearchPath;     // aSearchPath plus language specific paths
    sal_uInt32          nUniqueId;          // unique id for system resources
    sal_uLong           nFilePos;           // position in file (MTF)
    sal_uInt32          nPMId;              // unique id for PR-resource file
                                            // must be greater that RSC_VERSIONCONTROL_ID
    RscTop  *           pRoot;              // pointer to the root of type tree
    RSCINST             aVersion;           // version control instance

    ::std::vector< RscTop* >
                        aBaseLst;           // list of simple resource class
    ::std::vector< RscSysEntry* >
                        aSysLst;            // list of system resources

    Atom                nWinBitVarId;       // name of the winbit variable
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

    void        Init();         // initializes classes and tables
    void        SETCONST( RscConst *, const char *, sal_uInt32 );
    void        SETCONST( RscConst *, Atom, sal_uInt32 );
    inline void SETCONST( RscConst *p1, const char * p2, SymbolType p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, Atom p2, ToolBoxItemBits p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, Atom p2, WindowBorderStyle p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, const char * p2, KeyFuncType p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, Atom p2, MenuItemBits p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, const char * p2, ToolBoxItemType p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, const char * p2, ButtonType p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    inline void SETCONST( RscConst *p1, const char * p2, WindowAlign p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    RscEnum *   InitLangType();
    RscEnum *   InitFieldUnitsType();
    RscEnum *   InitColor();
    RscEnum *   InitMapUnit();
    RscEnum *   InitKey();
    RscEnum *   InitTriState();
    RscTupel *  InitGeometry();
    RscArray *  InitLangGeometry( RscTupel * pGeo );
    RscCont  *  InitStringList();
    RscArray *  InitLangStringList( RscCont * pStrLst );
    RscTupel *  InitStringTupel();
    RscTupel *  InitStringLongTupel();
    static RscCont  *  InitStringTupelList( RscTupel * pStringTupel );
    static RscCont  *  InitStringLongTupelList( RscTupel * pStringLongTupel );
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

    RscError*           pEH;        // error handler
    RscNameTable        aNmTb;      // name table
    RscFileTab          aFileTab;   // fila name table
    sal_uInt32          nFlags;
    std::map<sal_uInt64, sal_uLong> aIdTranslator; // map resources types and ids to an id (under PM9 or to a file position (MTF)

    RscTypCont( RscError *, RSCBYTEORDER_TYPE, const OString& rSearchPath, sal_uInt32 nFlags );
    ~RscTypCont();

    Atom AddLanguage( const char* );
    bool              IsSrsDefault() const
                          { return (nFlags & SRSDEFAULT_FLAG) != 0; }
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
    OString           GetSearchPath() const { return aSearchPath; }
    void              SetSysSearchPath( const OString& rStr ) { aSysSearchPath = rStr; }
    void              InsertType( RscTop * pType )
                          {
                              aBaseLst.push_back( pType );
                          }
    RscTop  *         SearchType( Atom nTypId );
                      // deletes all resource objects of this file
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
