/*************************************************************************
 *
 *  $RCSfile: printfun.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:00 $
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

class SfxPrinter;
class SfxProgress;
class ScDocShell;
class ScDocument;
class ScViewData;
class SfxItemSet;
class ScPageHFItem;
class EditTextObject;
class MultiSelection;
class ScHeaderEditEngine;
class ScPageBreakData;
class SvxBoxItem;
class SvxBrushItem;
class SvxShadowItem;
class FmFormView;

#define RANGENO_NORANGE             USHRT_MAX

#define PRINT_HEADER_WIDTH          (1.0 * TWIPS_PER_CM)
#define PRINT_HEADER_HEIGHT         (12.8 * TWIPS_PER_POINT)
#define PRINT_HEADER_FONTHEIGHT     200


                                            //  Einstellungen fuer Kopf-/Fusszeilen
struct ScPrintHFParam
{
    BOOL                bEnable;
    BOOL                bDynamic;
    BOOL                bShared;
    long                nHeight;            //  insgesamt (Hoehe+Abstand+Rahmen)
    long                nManHeight;         //  eingestellte Groesse (Min. bei dynamisch)
    USHORT              nDistance;
    USHORT              nLeft;              //  Raender
    USHORT              nRight;
    const ScPageHFItem* pLeft;
    const ScPageHFItem* pRight;
    const SvxBoxItem*   pBorder;
    const SvxBrushItem* pBack;
    const SvxShadowItem* pShadow;
};


// "Ersatz" fuer SV-JobSetup:

class ScJobSetup
{
public:
    ScJobSetup( SfxPrinter* pPrinter );

    Size        aUserSize;
    MapMode     aUserMapMode;
    Paper       ePaper;
    Orientation eOrientation;
    USHORT      nPaperBin;
};

struct ScPrintState                         //  Variablen aus ScPrintFunc retten
{
    USHORT  nPrintTab;
    USHORT  nStartCol;
    USHORT  nStartRow;
    USHORT  nEndCol;
    USHORT  nEndRow;
    USHORT  nZoom;
    USHORT  nPagesX;
    USHORT  nPagesY;
    long    nTabPages;
    long    nTotalPages;
    long    nPageStart;
    long    nDocPages;
};

class ScPageRowEntry
{
private:
    USHORT  nStartRow;
    USHORT  nEndRow;
    USHORT  nPagesX;
    BOOL*   pHidden;
    //!     Anzahl wirklich sichtbarer cachen???

public:
            ScPageRowEntry()    { nStartRow = nEndRow = nPagesX = 0; pHidden = NULL; }
            ~ScPageRowEntry()   { delete[] pHidden; }

            ScPageRowEntry(const ScPageRowEntry& r);
    const ScPageRowEntry& operator=(const ScPageRowEntry& r);

    USHORT  GetStartRow() const     { return nStartRow; }
    USHORT  GetEndRow() const       { return nEndRow; }
    USHORT  GetPagesX() const       { return nPagesX; }
    void    SetStartRow(USHORT n)   { nStartRow = n; }
    void    SetEndRow(USHORT n)     { nEndRow = n; }

    void    SetPagesX(USHORT nNew);
    void    SetHidden(USHORT nX);
    BOOL    IsHidden(USHORT nX) const;

    USHORT  CountVisible() const;
};

class ScPrintFunc
{
private:
    ScDocShell*         pDocShell;
    ScDocument*         pDoc;
    SfxPrinter*         pPrinter;
    OutputDevice*       pDev;

    FmFormView*         pDrawView;

    MapMode             aOldPrinterMode;    //  MapMode vor dem Aufruf

    Point               aSrcOffset;         //  Papier-1/100 mm
    Point               aOffset;            //  mit Faktor aus Seitenformat skaliert
    USHORT              nManualZoom;        //  Zoom in Preview (Prozent)
    BOOL                bClearWin;          //  Ausgabe vorher loeschen

    USHORT              nPrintTab;
    long                nPageStart;         //  Offset fuer erste Seite
    long                nDocPages;          //  Seiten im Dokument

    const ScRange*      pUserArea;          //  Selektion, wenn im Dialog eingestellt

    const SfxItemSet*   pParamSet;          //  eingestellte Vorlage
    BOOL                bState;             //  aus State-struct erzeugt

                                            //  Parameter aus Vorlage:
    USHORT              nLeftMargin;
    USHORT              nTopMargin;
    USHORT              nRightMargin;
    USHORT              nBottomMargin;
    BOOL                bCenterHor;
    BOOL                bCenterVer;
    BOOL                bLandscape;
    USHORT              nPageUsage;
    Size                aPageSize;          //  Drucker-Twips
    const SvxBoxItem*   pBorderItem;
    const SvxBrushItem* pBackgroundItem;
    const SvxShadowItem* pShadowItem;

    ScPrintHFParam      aHdr;
    ScPrintHFParam      aFtr;
    ScPageTableParam    aTableParam;
    ScPageAreaParam     aAreaParam;

                                            //  berechnete Werte:
    USHORT              nZoom;
    BOOL                bPrintCurrentTable;
    BOOL                bMultiArea;
    long                nTabPages;
    long                nTotalPages;

    USHORT              nObjectFlags;

    Rectangle           aPageRect;          //  Dokument-Twips

    MapMode             aLogicMode;         //  in DoPrint gesetzt
    MapMode             aOffsetMode;
    MapMode             aTwipMode;
    double              nScaleX;
    double              nScaleY;

    USHORT              nRepeatStartCol;
    USHORT              nRepeatEndCol;
    USHORT              nRepeatStartRow;
    USHORT              nRepeatEndRow;

    USHORT              nStartCol;
    USHORT              nStartRow;
    USHORT              nEndCol;
    USHORT              nEndRow;

    USHORT*             pPageEndX;          // Seitenaufteilung
    USHORT*             pPageEndY;
    ScPageRowEntry*     pPageRows;
    USHORT              nPagesX;
    USHORT              nPagesY;
    USHORT              nTotalY;

    ScHeaderEditEngine* pEditEngine;
    SfxItemSet*         pEditDefaults;

    ScHeaderFieldData   aFieldData;

    List                aNotePosList;       //  Reihenfolge der Notizen

    ScPageBreakData*    pPageData;          // zum Eintragen der Umbrueche etc.

public:
                    ScPrintFunc( ScDocShell* pShell, SfxPrinter* pNewPrinter, USHORT nTab,
                                 long nPage = 0, long nDocP = 0,
                                 const ScRange* pArea = NULL,
                                 ScPageBreakData* pData = NULL );

                    ScPrintFunc( ScDocShell* pShell, Window* pWindow, USHORT nTab,
                                 long nPage = 0, long nDocP = 0,
                                 const ScRange* pArea = NULL );

                    ScPrintFunc( ScDocShell* pShell, Window* pWindow,
                                 const ScPrintState& rState );

                    ~ScPrintFunc();

    static void     DrawToDev( ScDocument* pDoc, OutputDevice* pDev, double nPrintFactor,
                                const Rectangle& rBound, ScViewData* pViewData, BOOL bMetaFile );

    void            SetDrawView( FmFormView* pNew );

    void            SetOffset( const Point& rOfs );
    void            SetManualZoom( USHORT nNewZoom );
    void            SetDateTime( const Date& rDate, const Time& rTime );

    void            SetClearFlag( BOOL bFlag );

    BOOL            UpdatePages();

    void            ApplyPrintSettings();       // aus DoPrint() schon gerufen
    long            DoPrint(const MultiSelection& rPageRanges,
                                long nStartPage, long nDisplayStart,
                                SfxProgress* pProgress = NULL );

                    //  Werte abfragen - sofort

    Size            GetPageSize() const { return aPageSize; }
    Size            GetDataSize() const;
    void            GetScaleData( Size& rPhysSize, long& rDocHdr, long& rDocFtr );
    long            GetFirstPageNo() const  { return aTableParam.nFirstPageNo; }

                    //  letzte Werte abfragen - nach DoPrint !!!

    double          GetScaleX() const { return nScaleX; }
    double          GetScaleY() const { return nScaleY; }
    long            GetTotalPages() const { return nTotalPages; }
    USHORT          GetZoom() const { return nZoom; }

    void            ResetBreaks( USHORT nTab );

    void            GetPrintState( ScPrintState& rState );

#ifdef _PRINTFUN_CXX
private:
    void            Construct();
    void            InitParam();
    void            CalcZoom( USHORT nRangeNo );
    void            CalcPages();
    long            CountPages();
    long            CountNotePages();

    BOOL            AdjustPrintArea( BOOL bNew );

    Size            GetDocPageSize();

    long            TextHeight( const EditTextObject* pObject );
    void            MakeEditEngine();
    void            UpdateHFHeight( ScPrintHFParam& rParam );

    void            InitModes();

    BOOL            IsLeft( long nPageNo );
    BOOL            IsMirror( long nPageNo );
    void            ReplaceFields( long nPageNo );      // aendert Text in pEditEngine
    void            MakeTableString();                  // setzt aTableStr

    void            PrintPage( long nPageNo,
                                    USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2 );
    void            PrintArea( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                                    long nScrX, long nScrY,
                                    BOOL bShLeft, BOOL bShTop, BOOL bShRight, BOOL bShBottom );
    void            PrintColHdr( USHORT nX1, USHORT nX2, long nScrX, long nScrY );
    void            PrintRowHdr( USHORT nY1, USHORT nY2, long nScrX, long nScrY );
    void            PrintHF( long nPageNo, const ScPrintHFParam& rParam, long nStartY );

    long            PrintNotes( long nPageNo, long nNoteStart, BOOL bDoPrint );
    long            DoNotes( long nNoteStart, BOOL bDoPrint );

    void            DrawBorder( long nScrX, long nScrY, long nScrW, long nScrH,
                                    const SvxBoxItem* pBorderData,
                                    const SvxBrushItem* pBackground,
                                    const SvxShadowItem* pShadow );

    void            FillPageData();
#endif
};



#endif

