/*************************************************************************
 *
 *  $RCSfile: rscdb.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:54 $
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

#ifndef _RSCDB_HXX
#define _RSCDB_HXX

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif

#ifndef _LANG_HXX //autogen
#include <tools/lang.hxx>
#endif

#ifndef _RSCALL_H
#include <rscall.h>
#endif
#ifndef _RSCKEY_HXX
#include <rsckey.hxx>
#endif
#ifndef _RSCCONST_HXX
#include <rscconst.hxx>
#endif
#ifndef _RSCFLAG_HXX
#include <rscflag.hxx>
#endif
#ifndef _RSCRANGE_HXX
#include <rscrange.hxx>
#endif
#ifndef _RSCSTR_HXX
#include <rscstr.hxx>
#endif
#ifndef _RSCARRAY_HXX
#include <rscarray.hxx>
#endif
#ifndef _RSCDEF_HXX
#include <rscdef.hxx>
#endif

class RscError;
class REResourceList;
class RscTupel;
class RscCont;

/****************** R s c T y p C o n ************************************/
// Liste die alle Basistypen enthaelt
DECLARE_LIST( RscBaseList, RscTop * )

// Tabelle fuer Systemabhaengige Resourcen
struct RscSysEntry
{
    USHORT      nKey;
    USHORT      nRscTyp;
    ByteString  aFileName;
    USHORT      nTyp;
    USHORT      nRefId;
};
DECLARE_LIST( RscSysList, RscSysEntry * )

class RscTypCont
{
    CharSet             nSourceCharSet;
    USHORT              nLangTypeId;        // Globale Sprachtyp
    USHORT              nDfltLangTypeId;    // fallback fuer globalen Sprachtyp
    USHORT              nMachineId;         // Globaler Maschinentyp
    RSCBYTEORDER_TYPE   nByteOrder;         // Intel oder
    ByteString          aSearchPath;        // Suchen der Bitmap, Icon, Pointer
    USHORT              nUniqueId;          // eindeutiger Id fuer Systemresourcen
    ULONG               nFilePos;           // Position in der Datei ( MTF )
    USHORT              nPMId;              // eindeutiger Id fuer PM-Rseourcefile
                                            // muss groesser als RSC_VERSIONCONTROL_ID sein
    RscTop  *           pRoot;              // Zeiger auf die Wurzel vom Typenbaum
    RSCINST             aVersion;           // Versionskontrollinstanz

    RscBaseList         aBaseLst;           // Liste der einfachen Resourceklasse
    RscSysList          aSysLst;            // Liste der Systemresourcen

    HASHID              nWinBitVarId;       // Name der Winbitvariablen
    HASHID              nBorderId;
    HASHID              nHideId;
    HASHID              nClipChildrenId;
    HASHID              nSizeableId;
    HASHID              nMoveableId;
    HASHID              nMinimizeId;
    HASHID              nMaximizeId;
    HASHID              nCloseableId;
    HASHID              nAppId;
    HASHID              nTabstopId;
    HASHID              nGroupId;
    HASHID              nSysmodalId;
    HASHID              nLeftId;
    HASHID              nCenterId;
    HASHID              nRightId;
    HASHID              nHscrollId;
    HASHID              nVscrollId;
    HASHID              nSortId;
    HASHID              nDefaultId;
    HASHID              nSVLookId;
    HASHID              nRepeatId;
    HASHID              nDropDownId;
    HASHID              nPassWordId;
    HASHID              nReadOnlyId;
    HASHID              nAutoSizeId;
    HASHID              nSpinId;
    HASHID              nTabControlId;
    HASHID              nSimpleModeId;
    HASHID              nDragId;
    HASHID              nSaveAsId;
    HASHID              nOpenId;
    HASHID              nScrollId;
    HASHID              nZoomableId;
    HASHID              nHideWhenDeactivateId;
    HASHID              nAutoHScrollId;
    HASHID              nDDExtraWidthId;
    HASHID              nWordBreakId;
    HASHID              nLeftLabelId;
    HASHID              nHasLinesId;
    HASHID              nHasButtonsId;
    HASHID              nRectStyleId;
    HASHID              nLineSpacingId;
    HASHID              nSmallStyleId;
    HASHID              nEnableResizingId;
    HASHID              nDockableId;
    HASHID              nScaleId;
    HASHID              nIgnoreTabId;
    HASHID              nNoSplitDrawId;
    HASHID              nTopImageId;
    HASHID              nNoLabelId;

    void        Init();         // Initialisiert Klassen und Tabelle
    void        SETCONST( RscConst *, char *, UINT32 );
    void        SETCONST( RscConst *, HASHID, UINT32 );
    RscEnum *   InitLangType();
    RscEnum *   InitDateFormatType();
    RscEnum *   InitTimeFormatType();
    RscEnum *   InitWeekDayFormatType();
    RscEnum *   InitMonthFormatType();
    RscEnum *   InitFieldUnitsType();
    RscEnum *   InitDayOfWeekType();
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
                                    RscTop * pClassColor );
    RscTop *    InitClassWindow( RscTop * pSuper, RscEnum * pMapUnit,
                                 RscArray * pLangGeo );
    RscTop *    InitClassSystemWindow( RscTop * pSuper );
    RscTop *    InitClassWorkWindow( RscTop * pSuper );
    RscTop *    InitClassModalDialog( RscTop * pSuper );
    RscTop *    InitClassModelessDialog( RscTop * pSuper );
    RscTop *    InitClassControl( RscTop * pSuper );
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
    RscTop *    InitClassDate( RscTop * pSuper, RscEnum * pDayOfWeek );
    RscTop *    InitClassInt1( RscTop * pSuper,
                               RscEnum * pDateFormat,
                               RscEnum * pTimeFormat, RscEnum * pWeekDayFormat,
                               RscEnum * pMonthFormat );
    RscTop *    InitClassInternational( RscTop * pSuper,
                               RscEnum * pDateFormat,
                               RscEnum * pTimeFormat, RscEnum * pWeekDayFormat,
                               RscEnum * pMonthFormat );

    RscTop *    InitClassPatternFormatter( RscTop * pSuper );
    RscTop *    InitClassNumericFormatter( RscTop * pSuper,
                                           RscTop * pClassI12 );
    RscTop *    InitClassMetricFormatter( RscTop * pSuper,
                                          RscEnum * pFieldUnits );
    RscTop *    InitClassCurrencyFormatter( RscTop * pSuper,
                                            RscEnum * pFieldUnits );
    RscTop *    InitClassDateFormatter( RscTop * pSuper, RscTop * pClassDate,
                                        RscTop * pClassI12 );
    RscTop *    InitClassTimeFormatter( RscTop * pSuper, RscTop * pClassTime,
                                        RscTop * pClassI12,
                                        RscEnum * pTimeFieldFormat );

    RscTop *    InitClassSpinField( RscTop * pSuper );
    RscTop *    InitClassPatternField( RscTop * pSuper );
    RscTop *    InitClassNumericField( RscTop * pSuper );
    RscTop *    InitClassMetricField( RscTop * pSuper );
    RscTop *    InitClassCurrencyField( const char * pClassName, USHORT nRT,
                                        RscTop * pSuper );
    RscTop *    InitClassDateField( RscTop * pSuper, RscTop * pClassDate );
    RscTop *    InitClassTimeField( RscTop * pSuper, RscTop * pClassTime );
    RscTop *    InitClassPatternBox( RscTop * pSuper );
    RscTop *    InitClassNumericBox( RscTop * pSuper );
    RscTop *    InitClassMetricBox( RscTop * pSuper );
    RscTop *    InitClassCurrencyBox( const char * pClassName, USHORT nRT,
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
                           HASHID nVal );
    void        WriteInc( FILE * fOutput, ULONG lKey );

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
    RscFlag             aWinBits;
    RscEnum             aLangType;
    RscLangArray        aLangString;
    RscLangArray        aLangShort;

    RscError*           pEH;        // Fehlerhandler
    RscNameTable        aNmTb;      // Tabelle fuer Namen
    RscFileTab          aFileTab;   // Tabelle der Dateinamen
    USHORT              nFlags;
    Table               aIdTranslator; //Ordnet Resourcetypen und Id's einen Id zu
                                       //(unter PM), oder eine Dateiposition (MTF)

                        RscTypCont( RscError *, LanguageType,
                                    RSCBYTEORDER_TYPE,
                                    CharSet nSourceCharSet,
                                    const ByteString & rSearchPath, USHORT nFlags );
                        ~RscTypCont();

    BOOL            IsPreload() const
                    { return (nFlags & PRELOAD_FLAG) ? TRUE : FALSE; }
    BOOL            IsSmart() const
                    { return (nFlags & SMART_FLAG) ? TRUE : FALSE; }
    BOOL            IsSysResTest() const
                    { return (nFlags & NOSYSRESTEST_FLAG) ? FALSE : TRUE; }
    BOOL            IsSrsDefault() const
                    { return (nFlags & SRSDEFAULT_FLAG) ? TRUE : FALSE; }
    LanguageType    ChangeLanguage( LanguageType eLang )
                    {
                        LanguageType nOldLang = (LanguageType)nLangTypeId;
                        nLangTypeId = eLang;
                        return nOldLang;
                    }
    RSCBYTEORDER_TYPE GetByteOrder() const { return nByteOrder; }
    CharSet         GetSourceCharSet() const { return nSourceCharSet; }
    void            SetSearchPath( const ByteString & rStr) { aSearchPath = rStr; }
    ByteString      GetSearchPath() const { return aSearchPath; }
    void        InsertType( RscTop * pType )
                {
                    aBaseLst.Insert( pType, LIST_APPEND );
                }
    RscTop  *   SearchType( HASHID nTypId );
    RscTop  *   Search( HASHID typ );
    CLASS_DATA  Search( HASHID typ, const RscId & rId );
    void        Delete( HASHID typ, const RscId & rId );
                // loescht alle Resourceobjekte diese Datei
    void        Delete( ULONG lFileKey );
    RscTop  *   GetRoot()         { return( pRoot ); };
    USHORT      PutSysName( USHORT nRscTyp, char * pName, USHORT nConst,
                            USHORT nId, BOOL bFirst );
    ERRTYPE     WriteRc( FILE * fOutput );
    void        WriteSrc( FILE * fOutput, ULONG nFileIndex,
                          CharSet nCharSet, BOOL bName = TRUE );
    ERRTYPE     WriteHxx( FILE * fOutput, ULONG nFileKey);
    ERRTYPE     WriteCxx( FILE * fOutput, ULONG nFileKey,
                          const ByteString & rHxxName );
    void        WriteSyntax( FILE * fOutput );
    void        WriteRcCtor( FILE * fOutput );
    void        FillNameIdList( REResourceList * pList, ULONG lFileKey );
    BOOL        MakeConsistent( RscInconsList * pList );
    USHORT      PutTranslatorKey( ULONG nKey );
    void        IncFilePos( ULONG nOffset ){ nFilePos += nOffset; }
};

#endif

