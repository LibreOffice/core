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

#ifndef SC_PRINTFUN_HXX
#define SC_PRINTFUN_HXX


#ifndef SC_PAGEPAR_HXX
#include "pagepar.hxx"
#endif

#ifndef SC_EDITUTIL_HXX
#include "editutil.hxx"
#endif

#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif
class MultiSelection;
namespace binfilter {

class SfxItemSet;
class SfxPrinter;
class SfxProgress;
class ScDocShell;
class ScDocument;
class ScViewData;
class ScPageHFItem;
class EditTextObject;
class ScHeaderEditEngine;
class ScPageBreakData;
class ScPreviewLocationData;
class ScPrintOptions;
class SvxBoxItem;
class SvxBrushItem;
class SvxShadowItem;
class FmFormView;

#define RANGENO_NORANGE				USHRT_MAX

#define PRINT_HEADER_WIDTH			(1.0 * TWIPS_PER_CM)
#define PRINT_HEADER_HEIGHT 		(12.8 * TWIPS_PER_POINT)
#define PRINT_HEADER_FONTHEIGHT 	200


                                            //	Einstellungen fuer Kopf-/Fusszeilen
struct ScPrintHFParam
{
    BOOL				bEnable;
    BOOL				bDynamic;
    BOOL				bShared;
    long				nHeight;			//	insgesamt (Hoehe+Abstand+Rahmen)
    long				nManHeight;			//	eingestellte Groesse (Min. bei dynamisch)
    USHORT				nDistance;
    USHORT				nLeft;				//	Raender
    USHORT				nRight;
    const ScPageHFItem* pLeft;
    const ScPageHFItem* pRight;
    const SvxBoxItem*	pBorder;
    const SvxBrushItem* pBack;
    const SvxShadowItem* pShadow;
};


// "Ersatz" fuer SV-JobSetup:

class ScJobSetup
{
};

struct ScPrintState							//	Variablen aus ScPrintFunc retten
{
    USHORT	nPrintTab;
    USHORT	nStartCol;
    USHORT	nStartRow;
    USHORT	nEndCol;
    USHORT	nEndRow;
    USHORT	nZoom;
    USHORT	nPagesX;
    USHORT	nPagesY;
    long	nTabPages;
    long	nTotalPages;
    long	nPageStart;
    long	nDocPages;
};

class ScPageRowEntry
{
private:
    USHORT	nStartRow;
    USHORT	nEndRow;
    USHORT	nPagesX;
    BOOL*	pHidden;
    //!		Anzahl wirklich sichtbarer cachen???

public:
            ScPageRowEntry()	{ nStartRow = nEndRow = nPagesX = 0; pHidden = NULL; }
            ~ScPageRowEntry()	{ delete[] pHidden; }


    USHORT	GetStartRow() const		{ return nStartRow; }
    USHORT	GetEndRow() const		{ return nEndRow; }
    USHORT	GetPagesX() const		{ return nPagesX; }
    void	SetStartRow(USHORT n)	{ nStartRow = n; }
    void	SetEndRow(USHORT n)		{ nEndRow = n; }

    void	SetPagesX(USHORT nNew);

};

class ScPrintFunc
{
private:
    ScDocShell* 		pDocShell;
    ScDocument* 		pDoc;
    SfxPrinter* 		pPrinter;
    OutputDevice*		pDev;
    FmFormView*			pDrawView;

    MapMode				aOldPrinterMode;	//	MapMode vor dem Aufruf

    Point				aSrcOffset;			//	Papier-1/100 mm
    Point				aOffset;			//	mit Faktor aus Seitenformat skaliert
    USHORT				nManualZoom;		//	Zoom in Preview (Prozent)
    BOOL				bClearWin;			//	Ausgabe vorher loeschen
    BOOL				bUseStyleColor;
    BOOL				bIsRender;

    USHORT				nPrintTab;
    long				nPageStart;			//	Offset fuer erste Seite
    long				nDocPages;			//	Seiten im Dokument

    const ScRange*		pUserArea;			//	Selektion, wenn im Dialog eingestellt

    const SfxItemSet*	pParamSet;			//	eingestellte Vorlage
    BOOL				bState;				//	aus State-struct erzeugt

                                            //	Parameter aus Vorlage:
    USHORT				nLeftMargin;
    USHORT				nTopMargin;
    USHORT				nRightMargin;
    USHORT				nBottomMargin;
    BOOL				bCenterHor;
    BOOL				bCenterVer;
    BOOL				bLandscape;
    BOOL				bSourceRangeValid;

    USHORT				nPageUsage;
    Size				aPageSize;			//	Drucker-Twips
    const SvxBoxItem*	pBorderItem;
    const SvxBrushItem* pBackgroundItem;
    const SvxShadowItem* pShadowItem;

    ScRange				aLastSourceRange;
    ScPrintHFParam		aHdr;
    ScPrintHFParam		aFtr;
    ScPageTableParam	aTableParam;
    ScPageAreaParam 	aAreaParam;

                                            //	berechnete Werte:
    USHORT				nZoom;
    BOOL				bPrintCurrentTable;
    BOOL				bMultiArea;
    long				nTabPages;
    long				nTotalPages;

    USHORT				nObjectFlags;

    Rectangle			aPageRect;			//	Dokument-Twips

    MapMode 			aLogicMode; 		//	in DoPrint gesetzt
    MapMode 			aOffsetMode;
    MapMode 			aTwipMode;
    double				nScaleX;
    double				nScaleY;

    USHORT				nRepeatStartCol;
    USHORT				nRepeatEndCol;
    USHORT				nRepeatStartRow;
    USHORT				nRepeatEndRow;

    USHORT				nStartCol;
    USHORT				nStartRow;
    USHORT				nEndCol;
    USHORT				nEndRow;

    USHORT* 			pPageEndX;			// Seitenaufteilung
    USHORT* 			pPageEndY;
    ScPageRowEntry*		pPageRows;
    USHORT				nPagesX;
    USHORT				nPagesY;
    USHORT				nTotalY;

    ScHeaderEditEngine*	pEditEngine;
    SfxItemSet* 		pEditDefaults;

    ScHeaderFieldData	aFieldData;

    List				aNotePosList;		//	Reihenfolge der Notizen

    ScPageBreakData*	pPageData;			// zum Eintragen der Umbrueche etc.

public:
                    ScPrintFunc( ScDocShell* pShell, SfxPrinter* pNewPrinter, USHORT nTab,
                                 long nPage = 0, long nDocP = 0,
                                 const ScRange* pArea = NULL,
                                 const ScPrintOptions* pOptions = NULL,
                                 ScPageBreakData* pData = NULL );

                    ~ScPrintFunc();


    void			SetDateTime( const Date& rDate, const Time& rTime );

    BOOL			UpdatePages();

    Size			GetPageSize() const { return aPageSize; }
    long			GetFirstPageNo() const	{ return aTableParam.nFirstPageNo; }

                    //	letzte Werte abfragen - nach DoPrint !!!

    double			GetScaleX() const { return nScaleX; }
    double			GetScaleY() const { return nScaleY; }
    long			GetTotalPages() const { return nTotalPages; }
    USHORT			GetZoom() const { return nZoom; }

    void			ResetBreaks( USHORT nTab );

    BOOL			GetLastSourceRange( ScRange& rRange ) const{DBG_BF_ASSERT(0, "STRIP"); return FALSE;} //STRIP001 BOOL			GetLastSourceRange( ScRange& rRange ) const;

#ifdef _PRINTFUN_CXX
private:
    void			Construct( const ScPrintOptions* pOptions );
    void			InitParam( const ScPrintOptions* pOptions );
    void			CalcZoom( USHORT nRangeNo );
    void			CalcPages();
    long			CountPages();
    long			CountNotePages();

    BOOL			AdjustPrintArea( BOOL bNew );

    Size			GetDocPageSize();

    long			TextHeight( const EditTextObject* pObject );
    void			MakeEditEngine();
    void			UpdateHFHeight( ScPrintHFParam& rParam );

    void			InitModes();

    void			ReplaceFields( long nPageNo );		// aendert Text in pEditEngine




#endif
};



} //namespace binfilter
#endif

