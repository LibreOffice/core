/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vprint.cxx,v $
 * $Revision: 1.46 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <hintids.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/objsh.hxx>
#include <tools/resary.hxx>
#include <tools/debug.hxx>
#include <rtl/ustring.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <toolkit/awt/vclxdevice.hxx>

#include <sfx2/progress.hxx>
#include <sfx2/app.hxx>
#include <sfx2/prnmon.hxx>
#include <svx/paperinf.hxx>
#include <svx/pbinitem.hxx>
#include <svx/svdview.hxx>
#include <unotools/localedatawrapper.hxx>

#include <svtools/moduleoptions.hxx>

#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <fmtfsize.hxx>
#include <frmatr.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <doc.hxx>
#include <fesh.hxx>
#include <pam.hxx>
#include <viewimp.hxx>      // Imp->SetFirstVisPageInvalid()
#include <layact.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <docufld.hxx>      // PostItFld /-Type
#include <docfld.hxx>       // _SetGetExpFld
#include <shellres.hxx>
#include <viewopt.hxx>
#include <swprtopt.hxx>     // SwPrtOptions
#include <pagedesc.hxx>
#include <poolfmt.hxx>      // fuer RES_POOLPAGE_JAKET
#include <mdiexp.hxx>       // Ansteuern der Statusleiste
#ifndef _STATSTR_HRC
#include <statstr.hrc>      //      -- " --
#endif
#include <ptqueue.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>       // MinPrtLine
#include <viscrs.hxx>       // SwShellCrsr
#include <fmtpdsc.hxx>      // SwFmtPageDesc
#include <globals.hrc>

#define JOBSET_ERR_DEFAULT          0
#define JOBSET_ERR_ERROR            1
#define JOBSET_ERR_ISSTARTET        2


using namespace ::com::sun::star;

//--------------------------------------------------------------------
//Klasse zum Puffern von Paints
class SwQueuedPaint
{
public:
    SwQueuedPaint *pNext;
    ViewShell      *pSh;
    SwRect          aRect;

    SwQueuedPaint( ViewShell *pNew, const SwRect &rRect ) :
        pNext( 0 ),
        pSh( pNew ),
        aRect( rRect )
    {}
};

SwQueuedPaint *SwPaintQueue::pQueue = 0;

//Klasse zum Speichern einiger Druckereinstellungen
class SwPrtOptSave
{
    Printer *pPrt;
    Size aSize;
    Paper ePaper;
    Orientation eOrientation;
    USHORT nPaperBin;
public:
    SwPrtOptSave( Printer *pPrinter );
    ~SwPrtOptSave();
};

// saves some settings from the draw view
class SwDrawViewSave
{
    String sLayerNm;
    SdrView* pDV;
    sal_Bool bPrintControls;
public:
    SwDrawViewSave( SdrView* pSdrView );
    ~SwDrawViewSave();
};


void SwPaintQueue::Add( ViewShell *pNew, const SwRect &rNew )
{
    SwQueuedPaint *pPt;
    if ( 0 != (pPt = pQueue) )
    {
        while ( pPt->pSh != pNew && pPt->pNext )
            pPt = pPt->pNext;
        if ( pPt->pSh == pNew )
        {
            pPt->aRect.Union( rNew );
            return;
        }
    }
    SwQueuedPaint *pNQ = new SwQueuedPaint( pNew, rNew );
    if ( pPt )
        pPt->pNext = pNQ;
    else
        pQueue = pNQ;
}



void SwPaintQueue::Repaint()
{
    if ( !SwRootFrm::IsInPaint() && pQueue )
    {
        SwQueuedPaint *pPt = pQueue;
        do
        {   ViewShell *pSh = pPt->pSh;
            SET_CURR_SHELL( pSh );
            if ( pSh->IsPreView() )
            {
                if ( pSh->GetWin() )
                {
                    //Fuer PreView aussenherum, weil im PaintHdl (UI) die
                    //Zeilen/Spalten bekannt sind.
                    pSh->GetWin()->Invalidate();
                    pSh->GetWin()->Update();
                }
            }
            else
                pSh->Paint( pPt->aRect.SVRect() );
            pPt = pPt->pNext;
        } while ( pPt );

        do
        {   pPt = pQueue;
            pQueue = pQueue->pNext;
            delete pPt;
        } while ( pQueue );
    }
}



void SwPaintQueue::Remove( ViewShell *pSh )
{
    SwQueuedPaint *pPt;
    if ( 0 != (pPt = pQueue) )
    {
        SwQueuedPaint *pPrev = 0;
        while ( pPt && pPt->pSh != pSh )
        {
            pPrev = pPt;
            pPt = pPt->pNext;
        }
        if ( pPt )
        {
            if ( pPrev )
                pPrev->pNext = pPt->pNext;
            else if ( pPt == pQueue )
                pQueue = 0;
            delete pPt;
        }
    }
}

/*****************************************************************************/

const XubString& SwPrtOptions::MakeNextJobName()
{
    static char __READONLY_DATA sDelim[] = " - ";
    USHORT nDelim = sJobName.SearchAscii( sDelim );
    if( STRING_NOTFOUND == nDelim )
        sJobName.AppendAscii(sDelim);
    else
        sJobName.Erase( nDelim + 3/*Zeichen!*/ );

    return sJobName += XubString::CreateFromInt32( ++nJobNo );
}

/*****************************************************************************/

SwPrintUIOptions::SwPrintUIOptions( BOOL bWeb ) :
    m_pLast( NULL )
{
    ResStringArray aLocalizedStrings( SW_RES( STR_PRINTOPTUI ) );

    DBG_ASSERT( aLocalizedStrings.Count() >= 44, "resource incomplete" );
    if( aLocalizedStrings.Count() < 44 ) // bad resource ?
        return;

    // create sequence of print UI options
    // (5 options are not available for Writer-Web)
    const int nNumProps = bWeb? 17 : 21;
    m_aUIProperties.realloc( nNumProps );
    int nIdx = 0;

    // create "writer" section (new tab page in dialog)
    SvtModuleOptions aOpt;
    String aAppGroupname( aLocalizedStrings.GetString( 0 ) );
    aAppGroupname.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ),
                                    aOpt.GetModuleName( SvtModuleOptions::E_SWRITER ) );
    m_aUIProperties[ nIdx++ ].Value = getGroupControlOpt( aAppGroupname, rtl::OUString() );

    // create sub section for Contents
    m_aUIProperties[ nIdx++ ].Value = getSubgroupControlOpt( aLocalizedStrings.GetString( 1 ), rtl::OUString() );

    // create a bool option for background
    m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 2 ),
                                                  aLocalizedStrings.GetString( 3 ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintPageBackground" ) ),
                                                  sal_True );

    // create a bool option for graphics
    m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 4 ),
                                                  aLocalizedStrings.GetString( 5 ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintTablesGraphicsAndDiagrams" ) ),
                                                  sal_True );
    if (!bWeb)
    {
        // create a bool option for hidden text
        m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 6 ),
                                                  aLocalizedStrings.GetString( 7 ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintHiddenText" ) ),
                                                  sal_False );

        // create a bool option for place holder
        m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 8 ),
                                                  aLocalizedStrings.GetString( 9 ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintTextPlaceholder" ) ),
                                                  sal_False );
    }

    // create a bool option for controls
    m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 10 ),
                                                  aLocalizedStrings.GetString( 11 ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintControls" ) ),
                                                  sal_True );

    // create sub section for Color
    m_aUIProperties[ nIdx++ ].Value = getSubgroupControlOpt( aLocalizedStrings.GetString( 12 ), rtl::OUString() );

    // create a bool option for black
    m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 13 ),
                                                  aLocalizedStrings.GetString( 14 ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintBlackFonts" ) ),
                                                  sal_False );

    // create subgroup for misc options
    m_aUIProperties[ nIdx++ ].Value = getSubgroupControlOpt( rtl::OUString( aLocalizedStrings.GetString( 15 ) ), rtl::OUString() );

    // create a bool option for blank pages
    m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 16 ),
                                                   aLocalizedStrings.GetString( 17 ),
                                                   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintEmptyPages" ) ),
                                                   sal_True );

    // create a bool option for paper tray
    vcl::PrinterOptionsHelper::UIControlOptions aPaperTrayOpt;
    aPaperTrayOpt.maGroupHint = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OptionsPageOptGroup" ) );
    m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 18 ),
                                                   aLocalizedStrings.GetString( 19 ),
                                                   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintPaperFromSetup" ) ),
                                                   sal_False,
                                                   aPaperTrayOpt
                                                   );

    // print range selection
    vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
    aPrintRangeOpt.maGroupHint = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintRange" ) );
    aPrintRangeOpt.mbInternalOnly = sal_True;
    m_aUIProperties[nIdx++].Value = getSubgroupControlOpt( rtl::OUString( aLocalizedStrings.GetString( 37 ) ),
                                                           rtl::OUString(),
                                                           aPrintRangeOpt
                                                           );

    // create a choice for the content to create
    rtl::OUString aPrintRangeName( RTL_CONSTASCII_USTRINGPARAM( "PrintContent" ) );
    uno::Sequence< rtl::OUString > aChoices( 3 );
    uno::Sequence< rtl::OUString > aHelpText( 3 );
    aChoices[0] = aLocalizedStrings.GetString( 38 );
    aHelpText[0] = aLocalizedStrings.GetString( 39 );
    aChoices[1] = aLocalizedStrings.GetString( 40 );
    aHelpText[1] = aLocalizedStrings.GetString( 41 );
    aChoices[2] = aLocalizedStrings.GetString( 42 );
    aHelpText[2] = aLocalizedStrings.GetString( 43 );
    m_aUIProperties[nIdx++].Value = getChoiceControlOpt( rtl::OUString(),
                                                         aHelpText,
                                                         aPrintRangeName,
                                                         aChoices,
                                                         0 );
    // create a an Edit dependent on "Pages" selected
    vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt( aPrintRangeName, 1, sal_True );
    m_aUIProperties[nIdx++].Value = getEditControlOpt( rtl::OUString(),
                                                       rtl::OUString(),
                                                       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) ),
                                                       rtl::OUString(),
                                                       aPageRangeOpt
                                                       );
    // print content selection
    vcl::PrinterOptionsHelper::UIControlOptions aContentsOpt;
    aContentsOpt.maGroupHint = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "JobPage" ) );
    m_aUIProperties[nIdx++].Value = getSubgroupControlOpt( rtl::OUString( aLocalizedStrings.GetString( 20 ) ),
                                                           rtl::OUString(),
                                                           aContentsOpt
                                                           );
    // create a list box for notes content
    aChoices.realloc( 4 );
    aChoices[0] = aLocalizedStrings.GetString( 21 );
    aChoices[1] = aLocalizedStrings.GetString( 22 );
    aChoices[2] = aLocalizedStrings.GetString( 23 );
    aChoices[3] = aLocalizedStrings.GetString( 24 );
    aHelpText.realloc( 2 );
    aHelpText[0] = aLocalizedStrings.GetString( 25 );
    aHelpText[1] = aLocalizedStrings.GetString( 25 );
    m_aUIProperties[ nIdx++ ].Value = getChoiceControlOpt( aLocalizedStrings.GetString( 26 ),
                                                    aHelpText,
                                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintAnnotationMode" ) ),
                                                    aChoices,
                                                    0,
                                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "List" ) )
                                                    );

    // create subsection for Page settings
    vcl::PrinterOptionsHelper::UIControlOptions aPageSetOpt;
    aPageSetOpt.maGroupHint = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutPage" ) );

    if (!bWeb)
    {
        m_aUIProperties[nIdx++].Value = getSubgroupControlOpt( rtl::OUString( aLocalizedStrings.GetString( 27 ) ),
                                                               rtl::OUString(),
                                                               aPageSetOpt
                                                               );
        uno::Sequence< rtl::OUString > aRLChoices( 3 );
        aRLChoices[0] = aLocalizedStrings.GetString( 28 );
        aRLChoices[1] = aLocalizedStrings.GetString( 29 );
        aRLChoices[2] = aLocalizedStrings.GetString( 30 );
        uno::Sequence< rtl::OUString > aRLHelp( 1 );
        aRLHelp[0] = aLocalizedStrings.GetString( 31 );
        // create a choice option for all/left/right pages
        m_aUIProperties[ nIdx++ ].Value = getChoiceControlOpt( aLocalizedStrings.GetString( 32 ),
                                                   aRLHelp,
                                                   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintLeftRightPages" ) ),
                                                   aRLChoices, 0, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "List" ) )
                                                   );
    }

    // create a bool option for brochure
    rtl::OUString aBrochurePropertyName( RTL_CONSTASCII_USTRINGPARAM( "PrintProspect" ) );
    m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 33 ),
                                                   aLocalizedStrings.GetString( 34 ),
                                                   aBrochurePropertyName,
                                                   sal_False,
                                                   aPageSetOpt
                                                   );

    // create a bool option for brochure RTL dependent on brochure
    uno::Sequence< rtl::OUString > aBRTLChoices( 3 );
    aBRTLChoices[0] = aLocalizedStrings.GetString( 35 );
    aBRTLChoices[1] = aLocalizedStrings.GetString( 36 );
    vcl::PrinterOptionsHelper::UIControlOptions aBrochureRTLOpt( aBrochurePropertyName, -1, sal_True );
    aBrochureRTLOpt.maGroupHint = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutPage" ) );
    m_aUIProperties[ nIdx++ ].Value = getChoiceControlOpt( rtl::OUString(),
                                                           uno::Sequence< rtl::OUString >(),
                                                           rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintProspectRTL" ) ),
                                                           aBRTLChoices, 0, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "List" ) ),
                                                           aBrochureRTLOpt
                                                           );


    DBG_ASSERT( nIdx == nNumProps, "number of added properties is not as expected" );
}


SwPrintUIOptions::~SwPrintUIOptions()
{
}

bool SwPrintUIOptions::IsPrintLeftPages() const
{
    // take care of different property names for the option.
    // for compatibility the old name should win

    // 0: left and right pages
    // 1: left pages only
    // 2: right pages only
    sal_Int64 nLRPages = getIntValue( "PrintLeftRightPages", 0 /* default: all */ );
    bool bRes = nLRPages == 0 || nLRPages == 1;
    bRes = getBoolValue( "PrintLeftPages", bRes /* <- default value if property is not found */ );
    return bRes;
}

bool SwPrintUIOptions::IsPrintRightPages() const
{
    // take care of different property names for the option.
    // for compatibility the old name should win

    sal_Int64 nLRPages = getIntValue( "PrintLeftRightPages", 0 /* default: all */ );
    bool bRes = nLRPages == 0 || nLRPages == 2;
    bRes = getBoolValue( "PrintRightPages", bRes /* <- default value if property is not found */ );
    return bRes;
}

bool SwPrintUIOptions::IsPrintEmptyPages( bool bIsPDFExport ) const
{
    // take care of different property names for the option.

    bool bRes = bIsPDFExport ?
            !getBoolValue( "IsSkipEmptyPages", sal_True ) :
            getBoolValue( "PrintEmptyPages", sal_True );
    return bRes;
}

bool SwPrintUIOptions::IsPrintTables() const
{
    // take care of different property names for the option.
    // for compatibility the old name should win

    bool bRes = getBoolValue( "PrintTablesGraphicsAndDiagrams", sal_True );
    bRes = getBoolValue( "PrintTables", bRes );
    return bRes;
}

bool SwPrintUIOptions::IsPrintGraphics() const
{
    // take care of different property names for the option.
    // for compatibility the old name should win

    bool bRes = getBoolValue( "PrintTablesGraphicsAndDiagrams", sal_True );
    bRes = getBoolValue( "PrintGraphics", bRes );
    return bRes;
}

bool SwPrintUIOptions::IsPrintDrawings() const
{
    // take care of different property names for the option.
    // for compatibility the old name should win

    bool bRes = getBoolValue( "PrintTablesGraphicsAndDiagrams", sal_True );
    bRes = getBoolValue( "PrintDrawings", bRes );
    return bRes;
}

bool SwPrintUIOptions::processPropertiesAndCheckFormat( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& i_rNewProp )
{
    bool bChanged = processProperties( i_rNewProp );

    uno::Reference< awt::XDevice >  xRenderDevice;
    uno::Any aVal( getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RenderDevice" ) ) ) );
    aVal >>= xRenderDevice;

    OutputDevice* pOut = 0;
    if (xRenderDevice.is())
    {
        VCLXDevice*     pDevice = VCLXDevice::GetImplementation( xRenderDevice );
        pOut = pDevice ? pDevice->GetOutputDevice() : 0;
    }
    bChanged = bChanged || (pOut != m_pLast);
    if( pOut )
        m_pLast = pOut;

    return bChanged;
}


/******************************************************************************
 *  Methode     :   void SetSwVisArea( ViewShell *pSh, Point aPrtOffset, ...
 *  Beschreibung:
 *  Erstellt    :   OK 04.11.94 16:27
 *  Aenderung   :
 ******************************************************************************/

void SetSwVisArea( ViewShell *pSh, const SwRect &rRect, BOOL /*bPDFExport*/ )
{
    ASSERT( !pSh->GetWin(), "Drucken mit Window?" );
    pSh->aVisArea = rRect;
    pSh->Imp()->SetFirstVisPageInvalid();
    Point aPt( rRect.Pos() );

    // calculate an offset for the rectangle of the n-th page to
    // move the start point of the output operation to a position
    // such that in the output device all pages will be painted
    // at the same position
// TLPDF    if (!bPDFExport)
// TLPDF        aPt += pSh->aPrtOffst;
    aPt.X() = -aPt.X(); aPt.Y() = -aPt.Y();

// TLPDF   OutputDevice *pOut = bPDFExport ?
// TLPDF                        pSh->GetOut() :
// TLPDF                        pSh->getIDocumentDeviceAccess()->getPrinter( false );
    OutputDevice *pOut = pSh->GetOut();

    MapMode aMapMode( pOut->GetMapMode() );
    aMapMode.SetOrigin( aPt );
    pOut->SetMapMode( aMapMode );
}

/******************************************************************************
 *  Methode     :   struct _PostItFld : public _SetGetExpFld
 *  Beschreibung:   Update an das PostItFeld
 *  Erstellt    :   OK 07.11.94 10:18
 *  Aenderung   :
 ******************************************************************************/
struct _PostItFld : public _SetGetExpFld
{
    _PostItFld( const SwNodeIndex& rNdIdx, const SwTxtFld* pFld,
                    const SwIndex* pIdx = 0 )
        : _SetGetExpFld( rNdIdx, pFld, pIdx ) {}

    USHORT GetPageNo( MultiSelection &rMulti, BOOL bRgt, BOOL bLft,
                        USHORT& rVirtPgNo, USHORT& rLineNo );
    SwPostItField* GetPostIt() const
        { return (SwPostItField*) GetFld()->GetFld().GetFld(); }
};



USHORT _PostItFld::GetPageNo( MultiSelection &rMulti, BOOL bRgt, BOOL bLft,
                                USHORT& rVirtPgNo, USHORT& rLineNo )
{
    //Problem: Wenn ein PostItFld in einem Node steht, der von mehr als
    //einer Layout-Instanz repraesentiert wird, steht die Frage im Raum,
    //ob das PostIt nur ein- oder n-mal gedruck werden soll.
    //Wahrscheinlich nur einmal, als Seitennummer soll hier keine Zufaellige
    //sondern die des ersten Auftretens des PostIts innerhalb des selektierten
    //Bereichs ermittelt werden.
    rVirtPgNo = 0;
    USHORT nPos = GetCntnt();
    SwClientIter aIter( (SwModify &)GetFld()->GetTxtNode() );
    for( SwTxtFrm* pFrm = (SwTxtFrm*)aIter.First( TYPE( SwFrm ));
            pFrm;  pFrm = (SwTxtFrm*)aIter.Next() )
    {
        if( pFrm->GetOfst() > nPos ||
            (pFrm->HasFollow() && pFrm->GetFollow()->GetOfst() <= nPos) )
            continue;
        USHORT nPgNo = pFrm->GetPhyPageNum();
        BOOL bRight = pFrm->OnRightPage();
        if( rMulti.IsSelected( nPgNo ) &&
            ( (bRight && bRgt) || (!bRight && bLft) ) )
        {
            rLineNo = (USHORT)(pFrm->GetLineCount( nPos ) +
                      pFrm->GetAllLines() - pFrm->GetThisLines());
            rVirtPgNo = pFrm->GetVirtPageNum();
            return nPgNo;
        }
    }
    return 0;
}

/******************************************************************************
 *  Methode     :   void lcl_GetPostIts( IDocumentFieldsAccess* pIDFA, _SetGetExpFlds& ...
 *  Beschreibung:
 *  Erstellt    :   OK 07.11.94 10:20
 *  Aenderung   :
 ******************************************************************************/


void lcl_GetPostIts( IDocumentFieldsAccess* pIDFA, _SetGetExpFlds& rSrtLst )
{
    SwFieldType* pFldType = pIDFA->GetSysFldType( RES_POSTITFLD );
    ASSERT( pFldType, "kein PostItType ? ");

    if( pFldType->GetDepends() )
    {
        // Modify-Object gefunden, trage alle Felder ins Array ein
        SwClientIter aIter( *pFldType );
        SwClient* pLast;
        const SwTxtFld* pTxtFld;

        for( pLast = aIter.First( TYPE(SwFmtFld)); pLast; pLast = aIter.Next() )
            if( 0 != ( pTxtFld = ((SwFmtFld*)pLast)->GetTxtFld() ) &&
                pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            {
                SwNodeIndex aIdx( pTxtFld->GetTxtNode() );
                _PostItFld* pNew = new _PostItFld( aIdx, pTxtFld );
                rSrtLst.Insert( pNew );
            }
    }
}

/******************************************************************************
 *  Methode     :   void lcl_FormatPostIt( IDocumentContentOperations* pIDCO, SwPaM& aPam, ...
 *  Beschreibung:
 *  Erstellt    :   OK 07.11.94 10:20
 *  Aenderung   :
 ******************************************************************************/


void lcl_FormatPostIt( IDocumentContentOperations* pIDCO, SwPaM& aPam, SwPostItField* pField,
                           USHORT nPageNo, USHORT nLineNo )
{
    static char __READONLY_DATA sTmp[] = " : ";

    ASSERT( ViewShell::GetShellRes(), "missing ShellRes" );

    String aStr(    ViewShell::GetShellRes()->aPostItPage   );
    aStr.AppendAscii(sTmp);

    aStr += XubString::CreateFromInt32( nPageNo );
    aStr += ' ';
    if( nLineNo )
    {
        aStr += ViewShell::GetShellRes()->aPostItLine;
        aStr.AppendAscii(sTmp);
        aStr += XubString::CreateFromInt32( nLineNo );
        aStr += ' ';
    }
    aStr += ViewShell::GetShellRes()->aPostItAuthor;
    aStr.AppendAscii(sTmp);
    aStr += pField->GetPar1();
    aStr += ' ';
    aStr += GetAppLocaleData().getDate( pField->GetDate() );
    pIDCO->Insert( aPam, aStr, true );

    pIDCO->SplitNode( *aPam.GetPoint(), false );
    aStr = pField->GetPar2();
#if defined( WIN ) || defined( WNT ) || defined( PM2 )
    // Bei Windows und Co alle CR rausschmeissen
    aStr.EraseAllChars( '\r' );
#endif
    pIDCO->Insert( aPam, aStr, true );
    pIDCO->SplitNode( *aPam.GetPoint(), false );
    pIDCO->SplitNode( *aPam.GetPoint(), false );
}

/******************************************************************************
 *  Methode     :   void lcl_PrintPostIts( ViewShell* pPrtShell )
 *  Beschreibung:
 *  Erstellt    :   OK 07.11.94 10:21
 *  Aenderung   :   MA 10. May. 95
 ******************************************************************************/


void lcl_PrintPostIts( ViewShell* pPrtShell, const XubString& rJobName,
                        BOOL& rStartJob, int& rJobStartError, BOOL bReverse)
{
#ifdef TL_NOT_NOW /* TLPDF ??? */
    // Formatieren und Ausdrucken
    pPrtShell->CalcLayout();

    SfxPrinter* pPrn = pPrtShell->getIDocumentDeviceAccess()->getPrinter( false );

    //Das Druckdokument ist ein default Dokument, mithin arbeitet es auf der
    //StandardSeite.
    SwFrm *pPage = pPrtShell->GetLayout()->Lower();

    SwPrtOptSave aPrtSave( pPrn );

    pPrn->SetOrientation( ORIENTATION_PORTRAIT );
    pPrn->SetPaperBin( pPage->GetAttrSet()->GetPaperBin().GetValue() );

    if( !rStartJob &&  JOBSET_ERR_DEFAULT == rJobStartError &&
        rJobName.Len() )
    {
        if( !pPrn->IsJobActive() )
        {
            rStartJob = pPrn->StartJob( rJobName );
            if( !rStartJob )
            {
                rJobStartError = JOBSET_ERR_ERROR;
                return;
            }
        }
        pPrtShell->InitPrt( pPrn );
        rJobStartError = JOBSET_ERR_ISSTARTET;
    }

    // Wir koennen auch rueckwaerts:
    if ( bReverse )
        pPage = pPrtShell->GetLayout()->GetLastPage();

    while( pPage )
    {
        //Mag der Anwender noch?, Abbruch erst in Prt()
        GetpApp()->Reschedule();
        ::SetSwVisArea( pPrtShell, pPage->Frm() );
        pPrn->StartPage();
        pPage->GetUpper()->Paint( pPage->Frm() );
//      SFX_APP()->SpoilDemoOutput( *pPrtShell->GetOut(), pPage->Frm().SVRect());
        SwPaintQueue::Repaint();
        pPrn->EndPage();
        pPage = bReverse ? pPage->GetPrev() : pPage->GetNext();
    }
#else
(void)pPrtShell;
(void)rJobName;
(void)rStartJob;
(void)rJobStartError;
(void)bReverse;
#endif
}

/******************************************************************************
 *  Methode     :   void lcl_PrintPostItsEndDoc( ViewShell* pPrtShell, ...
 *  Beschreibung:
 *  Erstellt    :   OK 07.11.94 10:21
 *  Aenderung   :   MA 10. May. 95
 ******************************************************************************/


void lcl_PrintPostItsEndDoc( ViewShell* pPrtShell,
            _SetGetExpFlds& rPostItFields, MultiSelection &rMulti,
            const XubString& rJobName, BOOL& rStartJob, int& rJobStartError,
            BOOL bRgt, BOOL bLft, BOOL bRev )
{
    USHORT nPostIts = rPostItFields.Count();
    if( !nPostIts )
        // Keine Arbeit
        return;

    SET_CURR_SHELL( pPrtShell );

    SwDoc* pPrtDoc = pPrtShell->GetDoc();

    // Dokument leeren und ans Dokumentende gehen
    SwPaM aPam( pPrtDoc->GetNodes().GetEndOfContent() );
    aPam.Move( fnMoveBackward, fnGoDoc );
    aPam.SetMark();
    aPam.Move( fnMoveForward, fnGoDoc );
    pPrtDoc->Delete( aPam );

    for( USHORT i = 0, nVirtPg, nLineNo; i < nPostIts; ++i )
    {
        _PostItFld& rPostIt = (_PostItFld&)*rPostItFields[ i ];
        if( rPostIt.GetPageNo( rMulti, bRgt, bLft, nVirtPg, nLineNo ) )
            lcl_FormatPostIt( pPrtShell->GetDoc(), aPam,
                           rPostIt.GetPostIt(), nVirtPg, nLineNo );
    }

    lcl_PrintPostIts( pPrtShell, rJobName, rStartJob, rJobStartError, bRev );
}

/******************************************************************************
 *  Methode     :   void lcl_PrintPostItsEndPage( ViewShell* pPrtShell, ...
 *  Beschreibung:
 *  Erstellt    :   OK 07.11.94 10:22
 *  Aenderung   :
 ******************************************************************************/


void lcl_PrintPostItsEndPage( ViewShell* pPrtShell,
            _SetGetExpFlds& rPostItFields, USHORT nPageNo, MultiSelection &rMulti,
            const XubString& rJobName, BOOL& rStartJob, int& rJobStartError,
            BOOL bRgt, BOOL bLft, BOOL bRev )
{
    USHORT nPostIts = rPostItFields.Count();
    if( !nPostIts )
        // Keine Arbeit
        return;

    SET_CURR_SHELL( pPrtShell );

    USHORT i = 0, nVirtPg, nLineNo;
    while( ( i < nPostIts ) &&
           ( nPageNo != ((_PostItFld&)*rPostItFields[ i ]).
                                GetPageNo( rMulti,bRgt, bLft, nVirtPg, nLineNo )))
        ++i;
    if(i == nPostIts)
        // Nix zu drucken
        return;

    SwDoc* pPrtDoc = pPrtShell->GetDoc();

    // Dokument leeren und ans Dokumentende gehen
    SwPaM aPam( pPrtDoc->GetNodes().GetEndOfContent() );
    aPam.Move( fnMoveBackward, fnGoDoc );
    aPam.SetMark();
    aPam.Move( fnMoveForward, fnGoDoc );
    pPrtDoc->Delete( aPam );

    while( i < nPostIts )
    {
        _PostItFld& rPostIt = (_PostItFld&)*rPostItFields[ i ];
        if( nPageNo == rPostIt.GetPageNo( rMulti, bRgt, bLft, nVirtPg, nLineNo ) )
            lcl_FormatPostIt( pPrtShell->GetDoc(), aPam,
                                rPostIt.GetPostIt(), nVirtPg, nLineNo );
        ++i;
    }
    lcl_PrintPostIts( pPrtShell, rJobName, rStartJob, rJobStartError, bRev );
}

/******************************************************************************
 *  Methode     :   void ViewShell::InitPrt( Printer *pNew, OutputDevice *pPDFOut )
 *  Beschreibung:
 *  Erstellt    :   OK 07.11.94 10:22
 *  Aenderung   :
 ******************************************************************************/

void ViewShell::InitPrt( /*Printer *pPrt,*/ OutputDevice *pOutDev ) /* TLPDF */
{
    //Fuer den Printer merken wir uns einen negativen Offset, der
    //genau dem Offset de OutputSize entspricht. Das ist notwendig,
    //weil unser Ursprung der linken ober Ecke der physikalischen
    //Seite ist, die Ausgaben (SV) aber den Outputoffset als Urstprung
    //betrachten.
    OutputDevice *pTmpDev = pOutDev; // TLPDF pPDFOut ? pPDFOut : (OutputDevice *) pPrt;
    if ( pTmpDev )
    {
// TLPDF        aPrtOffst = pPrt ? pPrt->GetPageOffset() : Point();
        aPrtOffst = Point();

        aPrtOffst += pTmpDev->GetMapMode().GetOrigin();
        MapMode aMapMode( pTmpDev->GetMapMode() );
        aMapMode.SetMapUnit( MAP_TWIP );
        pTmpDev->SetMapMode( aMapMode );
        pTmpDev->SetLineColor();
        pTmpDev->SetFillColor();
    }
    else
        aPrtOffst.X() = aPrtOffst.Y() = 0;

    if ( !pWin )
        pOut = pTmpDev;    //Oder was sonst?
}

/******************************************************************************
 *  Methode     :   void ViewShell::ChgAllPageOrientation
 *  Erstellt    :   MA 08. Aug. 95
 *  Aenderung   :
 ******************************************************************************/


void ViewShell::ChgAllPageOrientation( USHORT eOri )
{
    ASSERT( nStartAction, "missing an Action" );
    SET_CURR_SHELL( this );

    USHORT nAll = GetDoc()->GetPageDescCnt();
    BOOL bNewOri = Orientation(eOri) == ORIENTATION_PORTRAIT ? FALSE : TRUE;

    for( USHORT i = 0; i < nAll; ++ i )
    {
        const SwPageDesc& rOld =
            const_cast<const SwDoc *>(GetDoc())->GetPageDesc( i );

        if( rOld.GetLandscape() != bNewOri )
        {
            SwPageDesc aNew( rOld );
            const sal_Bool bDoesUndo( GetDoc()->DoesUndo() );
            GetDoc()->DoUndo( sal_False );
            GetDoc()->CopyPageDesc(rOld, aNew);
            GetDoc()->DoUndo( bDoesUndo );
            aNew.SetLandscape( bNewOri );
            SwFrmFmt& rFmt = aNew.GetMaster();
            SwFmtFrmSize aSz( rFmt.GetFrmSize() );
            // Groesse anpassen.
            // PORTRAIT  -> Hoeher als Breit
            // LANDSCAPE -> Breiter als Hoch
            // Hoehe ist die VarSize, Breite ist die FixSize (per Def.)
            if( bNewOri ? aSz.GetHeight() > aSz.GetWidth()
                        : aSz.GetHeight() < aSz.GetWidth() )
            {
                SwTwips aTmp = aSz.GetHeight();
                aSz.SetHeight( aSz.GetWidth() );
                aSz.SetWidth( aTmp );
                rFmt.SetFmtAttr( aSz );
            }
            GetDoc()->ChgPageDesc( i, aNew );
        }
    }
}

/******************************************************************************
 *  Methode     :   void ViewShell::ChgAllPageOrientation
 *  Erstellt    :   MA 08. Aug. 95
 *  Aenderung   :
 ******************************************************************************/


void ViewShell::ChgAllPageSize( Size &rSz )
{
    ASSERT( nStartAction, "missing an Action" );
    SET_CURR_SHELL( this );

    SwDoc* pMyDoc = GetDoc();
    USHORT nAll = pMyDoc->GetPageDescCnt();

    for( USHORT i = 0; i < nAll; ++i )
    {
        const SwPageDesc &rOld = const_cast<const SwDoc *>(pMyDoc)->GetPageDesc( i );
        SwPageDesc aNew( rOld );
        const sal_Bool bDoesUndo( GetDoc()->DoesUndo() );
        GetDoc()->DoUndo( sal_False );
        GetDoc()->CopyPageDesc( rOld, aNew );
        GetDoc()->DoUndo( bDoesUndo );
        SwFrmFmt& rPgFmt = aNew.GetMaster();
        Size aSz( rSz );
        const BOOL bOri = aNew.GetLandscape();
        if( bOri  ? aSz.Height() > aSz.Width()
                  : aSz.Height() < aSz.Width() )
        {
            SwTwips aTmp = aSz.Height();
            aSz.Height() = aSz.Width();
            aSz.Width()  = aTmp;
        }

        SwFmtFrmSize aFrmSz( rPgFmt.GetFrmSize() );
        aFrmSz.SetSize( aSz );
        rPgFmt.SetFmtAttr( aFrmSz );
        pMyDoc->ChgPageDesc( i, aNew );
    }
}


void lcl_SetState( SfxProgress& rProgress, ULONG nPage, ULONG nMax,
    const XubString *pStr, ULONG nAct, ULONG nCnt, ULONG nOffs, ULONG nPageNo )
{
    XubString aTmp = XubString::CreateFromInt64( nPageNo );
    if( pStr )
    {
        aTmp += ' ';
        aTmp += *pStr;
        if( nCnt )
        {
            nMax *= 2;
            rProgress.SetStateText( (nAct-1)*nMax+nPage+nOffs,
                                        aTmp, nCnt*nMax );
        }
        else
            rProgress.SetStateText( nPage, aTmp, nMax );
    }
    else
    {
        aTmp += ' '; aTmp += '('; aTmp += XubString::CreateFromInt64( nPage );
        aTmp += '/'; aTmp += XubString::CreateFromInt64( nMax ); aTmp += ')';
        rProgress.SetStateText( nPage, aTmp, nMax );
    }
}



void ViewShell::CalcPagesForPrint( USHORT nMax, SfxProgress* pProgress,
    const XubString* /*TLPDF pStr*/, ULONG /*TLPDF nMergeAct*/, ULONG /*TLPDF nMergeCnt*/ )
{
    SET_CURR_SHELL( this );

    //Seitenweise durchformatieren, by the way kann die Statusleiste
    //angetriggert werden, damit der Anwender sieht worauf er wartet.
    //Damit der Vorgang moeglichst transparent gestaltet werden kann
    //Versuchen wir mal eine Schaetzung.
    SfxPrinter* pPrt = getIDocumentDeviceAccess()->getPrinter( false );
    BOOL bPrtJob = pPrt ? pPrt->IsJobActive() : FALSE;
    SwRootFrm* pLayout = GetLayout();
    // ULONG nStatMax = pLayout->GetPageNum();

    const SwFrm *pPage = pLayout->Lower();
    SwLayAction aAction( pLayout, Imp() );

#ifdef TL_NOT_NOW /*TLPDF*/
//Currently we have no progress bar here. It is handled in the new Print UI now.
//Not year clear: what about Progressbar in MailMerge
    if( pProgress )
    {
        // HACK, damit die Anzeige sich nicht verschluckt.
        const XubString aTmp( SW_RES( STR_STATSTR_FORMAT ) );
        pProgress->SetText( aTmp );
        lcl_SetState( *pProgress, 1, nStatMax, pStr, nMergeAct, nMergeCnt, 0, 1 );
        pProgress->Reschedule(); //Mag der Anwender noch oder hat er genug?
        aAction.SetProgress(pProgress);
    }
#endif  // TL_NOT_NOW /*TLPDF*/

    pLayout->StartAllAction();
    for ( USHORT i = 1; pPage && i <= nMax; pPage = pPage->GetNext(), ++i )
    {
        if ( ( bPrtJob && !pPrt->IsJobActive() ) || Imp()->IsStopPrt() )
            break;

#ifdef TL_NOT_NOW /*TLPDF*/
        if( pProgress )
        {
            //HACK, damit die Anzeige sich nicht verschluckt.
            if ( i > nStatMax ) nStatMax = i;
            lcl_SetState( *pProgress, i, nStatMax, pStr, nMergeAct, nMergeCnt, 0, i );
            pProgress->Reschedule(); //Mag der Anwender noch oder hat er genug?
        }
#endif  // TL_NOT_NOW /*TLPDF*/

        if ( ( bPrtJob && !pPrt->IsJobActive() ) || Imp()->IsStopPrt() )
            break;

        pPage->Calc();
        SwRect aOldVis( VisArea() );
        aVisArea = pPage->Frm();
        Imp()->SetFirstVisPageInvalid();
        aAction.Reset();
        aAction.SetPaint( FALSE );
        aAction.SetWaitAllowed( FALSE );
        aAction.SetReschedule( TRUE );

        aAction.Action();

        aVisArea = aOldVis;             //Zuruecksetzen wg. der Paints!
        Imp()->SetFirstVisPageInvalid();
        SwPaintQueue::Repaint();

#ifdef TL_NOT_NOW /*TLPDF*/
        if ( pProgress )
            pProgress->Reschedule(); //Mag der Anwender noch oder hat er genug?
#endif  // TL_NOT_NOW /*TLPDF*/
    }

    if (pProgress)
        aAction.SetProgress( NULL );

    pLayout->EndAllAction();
}

/******************************************************************************/

SwDoc * ViewShell::CreatePrtDoc( /*Printer* pPrt,*/ SfxObjectShellRef &rDocShellRef)    // TLPDF
{
    ASSERT( this->IsA( TYPE(SwFEShell) ),"ViewShell::Prt for FEShell only");
    SwFEShell* pFESh = (SwFEShell*)this;
    // Wir bauen uns ein neues Dokument
    SwDoc *pPrtDoc = new SwDoc;
    pPrtDoc->acquire();
    pPrtDoc->SetRefForDocShell( (SfxObjectShellRef*)&(long&)rDocShellRef );
    pPrtDoc->LockExpFlds();

/* TLPDF
    // Der Drucker wird uebernommen
    if (pPrt)
        pPrtDoc->setPrinter( pPrt, true, true );
*/
    const SfxPoolItem* pCpyItem;
    const SfxItemPool& rPool = GetAttrPool();
    for( USHORT nWh = POOLATTR_BEGIN; nWh < POOLATTR_END; ++nWh )
        if( 0 != ( pCpyItem = rPool.GetPoolDefaultItem( nWh ) ) )
            pPrtDoc->GetAttrPool().SetPoolDefaultItem( *pCpyItem );

    // JP 29.07.99 - Bug 67951 - set all Styles from the SourceDoc into
    //                              the PrintDoc - will be replaced!
    pPrtDoc->ReplaceStyles( *GetDoc() );

    SwShellCrsr *pActCrsr = pFESh->_GetCrsr();
    SwShellCrsr *pFirstCrsr = dynamic_cast<SwShellCrsr*>(pActCrsr->GetNext());
    if( !pActCrsr->HasMark() ) // bei Multiselektion kann der aktuelle Cursor leer sein
    {
        pActCrsr = dynamic_cast<SwShellCrsr*>(pActCrsr->GetPrev());
    }

    // Die Y-Position der ersten Selektion
    const Point aSelPoint = pFESh->IsTableMode() ?
                            pFESh->GetTableCrsr()->GetSttPos() :
                            pFirstCrsr->GetSttPos();

    const SwPageFrm* pPage = GetLayout()->GetPageAtPos( aSelPoint );

    // und ihren Seitendescribtor
    const SwPageDesc* pPageDesc = pPrtDoc->FindPageDescByName(
                                        pPage->GetPageDesc()->GetName() );

    if( !pFESh->IsTableMode() && pActCrsr->HasMark() )
    {   // Am letzten Absatz die Absatzattribute richten:
        SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwTxtNode* pTxtNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx )->GetTxtNode();
        SwCntntNode *pLastNd =
            pActCrsr->GetCntntNode( (*pActCrsr->GetMark()) <= (*pActCrsr->GetPoint()) );
        // Hier werden die Absatzattribute des ersten Absatzes uebertragen
        if( pLastNd && pLastNd->IsTxtNode() )
            ((SwTxtNode*)pLastNd)->CopyCollFmt( *pTxtNd );
    }

    // es wurde in der CORE eine neu angelegt (OLE-Objekte kopiert!)
//      if( aDocShellRef.Is() )
//          SwDataExchange::InitOle( aDocShellRef, pPrtDoc );
    // und fuellen es mit dem selektierten Bereich
    pFESh->Copy( pPrtDoc );

    //Jetzt noch am ersten Absatz die Seitenvorlage setzen
    {
        SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwCntntNode* pCNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx ); // gehe zum 1. ContentNode
        if( pFESh->IsTableMode() )
        {
            SwTableNode* pTNd = pCNd->FindTableNode();
            if( pTNd )
                pTNd->GetTable().GetFrmFmt()->SetFmtAttr( SwFmtPageDesc( pPageDesc ) );
        }
        else
        {
            pCNd->SetAttr( SwFmtPageDesc( pPageDesc ) );
            if( pFirstCrsr->HasMark() )
            {
                SwTxtNode *pTxtNd = pCNd->GetTxtNode();
                if( pTxtNd )
                {
                    SwCntntNode *pFirstNd =
                        pFirstCrsr->GetCntntNode( (*pFirstCrsr->GetMark()) > (*pFirstCrsr->GetPoint()) );
                    // Hier werden die Absatzattribute des ersten Absatzes uebertragen
                    if( pFirstNd && pFirstNd->IsTxtNode() )
                        ((SwTxtNode*)pFirstNd)->CopyCollFmt( *pTxtNd );
                }
            }
        }
    }
    return pPrtDoc;
}

SwDoc * ViewShell::FillPrtDoc( SwDoc *pPrtDoc, const SfxPrinter* pPrt)
{
    ASSERT( this->IsA( TYPE(SwFEShell) ),"ViewShell::Prt for FEShell only");
    SwFEShell* pFESh = (SwFEShell*)this;
    // Wir bauen uns ein neues Dokument
//    SwDoc *pPrtDoc = new SwDoc;
//    pPrtDoc->acquire();
//    pPrtDoc->SetRefForDocShell( (SvEmbeddedObjectRef*)&(long&)rDocShellRef );
    pPrtDoc->LockExpFlds();

    // Der Drucker wird uebernommen
    //! Make a copy of it since it gets destroyed with the temporary document
    //! used for PDF export
    if (pPrt)
        pPrtDoc->setPrinter( new SfxPrinter(*pPrt), true, true );

    const SfxPoolItem* pCpyItem;
    const SfxItemPool& rPool = GetAttrPool();
    for( USHORT nWh = POOLATTR_BEGIN; nWh < POOLATTR_END; ++nWh )
        if( 0 != ( pCpyItem = rPool.GetPoolDefaultItem( nWh ) ) )
            pPrtDoc->GetAttrPool().SetPoolDefaultItem( *pCpyItem );

    // JP 29.07.99 - Bug 67951 - set all Styles from the SourceDoc into
    //                              the PrintDoc - will be replaced!
    pPrtDoc->ReplaceStyles( *GetDoc() );

    SwShellCrsr *pActCrsr = pFESh->_GetCrsr();
    SwShellCrsr *pFirstCrsr = dynamic_cast<SwShellCrsr*>(pActCrsr->GetNext());
    if( !pActCrsr->HasMark() ) // bei Multiselektion kann der aktuelle Cursor leer sein
    {
        pActCrsr = dynamic_cast<SwShellCrsr*>(pActCrsr->GetPrev());
    }

    // Die Y-Position der ersten Selektion
    // Die Y-Position der ersten Selektion
    const Point aSelPoint = pFESh->IsTableMode() ?
                            pFESh->GetTableCrsr()->GetSttPos() :
                            pFirstCrsr->GetSttPos();

    const SwPageFrm* pPage = GetLayout()->GetPageAtPos( aSelPoint );

    // und ihren Seitendescribtor
    const SwPageDesc* pPageDesc = pPrtDoc->FindPageDescByName(
                                pPage->GetPageDesc()->GetName() );

    if( !pFESh->IsTableMode() && pActCrsr->HasMark() )
    {   // Am letzten Absatz die Absatzattribute richten:
        SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwTxtNode* pTxtNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx )->GetTxtNode();
        SwCntntNode *pLastNd =
            pActCrsr->GetCntntNode( (*pActCrsr->GetMark()) <= (*pActCrsr->GetPoint()) );
        // Hier werden die Absatzattribute des ersten Absatzes uebertragen
        if( pLastNd && pLastNd->IsTxtNode() )
            ((SwTxtNode*)pLastNd)->CopyCollFmt( *pTxtNd );
    }

    // es wurde in der CORE eine neu angelegt (OLE-Objekte kopiert!)
//      if( aDocShellRef.Is() )
//          SwDataExchange::InitOle( aDocShellRef, pPrtDoc );
    // und fuellen es mit dem selektierten Bereich
    pFESh->Copy( pPrtDoc );

    //Jetzt noch am ersten Absatz die Seitenvorlage setzen
    {
        SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwCntntNode* pCNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx ); // gehe zum 1. ContentNode
        if( pFESh->IsTableMode() )
        {
            SwTableNode* pTNd = pCNd->FindTableNode();
            if( pTNd )
                pTNd->GetTable().GetFrmFmt()->SetFmtAttr( SwFmtPageDesc( pPageDesc ) );
        }
        else
        {
            pCNd->SetAttr( SwFmtPageDesc( pPageDesc ) );
            if( pFirstCrsr->HasMark() )
            {
                SwTxtNode *pTxtNd = pCNd->GetTxtNode();
                if( pTxtNd )
                {
                    SwCntntNode *pFirstNd =
                        pFirstCrsr->GetCntntNode( (*pFirstCrsr->GetMark()) > (*pFirstCrsr->GetPoint()) );
                    // Hier werden die Absatzattribute des ersten Absatzes uebertragen
                    if( pFirstNd && pFirstNd->IsTxtNode() )
                        ((SwTxtNode*)pFirstNd)->CopyCollFmt( *pTxtNd );
                }
            }
        }
    }
    return pPrtDoc;
}


sal_Bool ViewShell::PrintOrPDFExportMM(
    const boost::shared_ptr< vcl::PrinterController > & rpPrinterController,
    const SwPrtOptions &rPrintData, /* TLPDF can't we make use of just SwPrintData only as it is the case in PrintProspect???  */
    bool bIsPDFExport )
{
    (void) rpPrinterController; (void) rPrintData; (void) bIsPDFExport;


    Printer::PrintJob( rpPrinterController, JobSetup() );
    return sal_True;
}


sal_Bool ViewShell::PrintOrPDFExport(
    OutputDevice* pOutDev,
    const SwPrtOptions &rPrintData, /* TLPDF can't we make use of just SwPrintData only as it is the case in PrintProspect???  */
    sal_Int32 nRenderer,    // the index in the vector of pages to be printed
    bool bIsPDFExport )
{
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Immer die Druckroutinen in viewpg.cxx (PrintPreViewPage und PrintProspect) mitpflegen!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    BOOL bStartJob = FALSE;
    //! Note: Since for PDF export of (multi-)selection a temporary
    //! document is created that contains only the selects parts,
    //! and thus that document is to printed in whole the,
    //! rPrintData.bPrintSelection parameter will be false.
    BOOL bSelection = rPrintData.bPrintSelection;

// TLPDF: this one should hold just one page now. Thus clean-up should be possible
    MultiSelection aMulti( rPrintData.aMulti );

    if ( !pOutDev || !aMulti.GetSelectCount() )
        return bStartJob;

    // save settings of OutputDevice (should be done always now since the
    // output device is now provided by a call from outside the Writer)
    pOutDev->Push();

    Range aPages( aMulti.FirstSelected(), aMulti.LastSelected() );
    if ( aPages.Max() > USHRT_MAX )
        aPages.Max() = USHRT_MAX;

    ASSERT( aPages.Min() > 0,
            "Seite 0 Drucken?" );
    ASSERT( aPages.Min() <= aPages.Max(),
            "MinSeite groesser MaxSeite." );

// TLPDF
    ASSERT( aPages.Min() == aPages.Max(), "Min page should be equal to Max page now" ); /* TLPDF */
// TLPDF TODO: clean-up aPage.Min/Max which should be identical now


    // Einstellungen am Drucker merken
//    SwPrtOptSave aPrtSave( pOutDev );      /* TLPDF wo dann heutzutage ??? */

    // eine neue Shell fuer den Printer erzeugen
    ViewShell *pShell;
    SwDoc *pOutDevDoc;

    //!! muss warum auch immer hier in diesem scope existieren !!
    //!! (h?ngt mit OLE Objekten im Dokument zusammen.)
    SfxObjectShellRef aDocShellRef;

    // PDF export for (multi-)selection has already generated a temporary document
    // with the selected text. (see XRenderable implementation in unotxdoc.cxx)
    // Thus we like to go in the 'else' part here in that case.
    // Is is implemented this way because PDF export calls this Prt function
    // once per page and we do not like to always have the temporary document
    // to be created that often here in the 'then' part.
    if ( bSelection )
    {
        pOutDevDoc = CreatePrtDoc( /*TLPDF pOutDev,*/ aDocShellRef );

        // eine ViewShell darauf
// TLPDF       OutputDevice *pTmpDev = bIsPDFExport ? pOutDev : 0;
// TLPDF       pShell = new ViewShell( *pOutDevDoc, 0, pOpt, pTmpDev );
        pShell = new ViewShell( *pOutDevDoc, 0, pOpt, pOutDev );
        pOutDevDoc->SetRefForDocShell( 0 );
    }
    else
    {
        pOutDevDoc = GetDoc();
// TLPDF        OutputDevice *pTmpDev = bIsPDFExport ? pOutDev : 0;
// TLPDF        pShell = new ViewShell( *this, 0, pTmpDev );
        pShell = new ViewShell( *this, 0, pOutDev );
    }
    SdrView *pDrawView = pShell->GetDrawView();
    if (pDrawView)
    {
        pDrawView->SetBufferedOutputAllowed( false );  // TLPDF
        pDrawView->SetBufferedOverlayAllowed( false ); // TLPDF
    }

    {   //Zusaetzlicher Scope, damit die CurrShell vor dem zerstoeren der
        //Shell zurueckgesetzt wird.

    SET_CURR_SHELL( pShell );

    //JP 01.02.99: das ReadOnly Flag wird NIE mitkopiert; Bug 61335
    if( pOpt->IsReadonly() )
        pShell->pOpt->SetReadonly( TRUE );

    // save options at draw view:
    SwDrawViewSave aDrawViewSave( pShell->GetDrawView() );

    pShell->PrepareForPrint( rPrintData );

    XubString* pStr = 0;
//TLPDF    ULONG nMergeAct = rPrintData.nMergeAct, nMergeCnt = rPrintData.nMergeCnt;


/* TLPDF neu: start */
#if OSL_DEBUG_LEVEL > 1
    DBG_ASSERT( 0 <= nRenderer && nRenderer < (sal_Int32)rPrintData.GetPrintUIOptions().GetPagesToPrint().size(), "nRenderer out of bounds");
#endif
    const sal_Int32 nPage = rPrintData.GetPrintUIOptions().GetPagesToPrint()[ nRenderer ]; /* TLPDF */
#if OSL_DEBUG_LEVEL > 1
    DBG_ASSERT( rPrintData.GetPrintUIOptions().GetValidPagesSet().count( nPage ) == 1, "nPage not valid" );
#endif
    const SwPrintUIOptions::ValidStartFramesMap_t &rFrms = rPrintData.GetPrintUIOptions().GetValidStartFrms();
    SwPrintUIOptions::ValidStartFramesMap_t::const_iterator aIt( rFrms.find( nPage ) );
    DBG_ASSERT( aIt != rFrms.end(), "failed to find start frame" );
    const SwPageFrm *pStPage = aIt->second;
/* TLPDF neu: end */


    // ben�tigte Seiten fuers Drucken formatieren
    pShell->CalcPagesForPrint( (USHORT)nPage, 0 /*TLPDF*/, pStr,
                                0, 0 /* TLPDF, there is no progressbar right now nMergeAct, nMergeCnt */ );

    // Some field types, can require a valid layout
    // (expression fields in tables). For these we do an UpdateFlds
    // here after calculation of the pages.
    // --> FME 2004-06-21 #i9684# For performance reasons, we do not update
    //                            the fields during pdf export.
    // #i56195# prevent update of fields (for mail merge)
    if ( !bIsPDFExport && rPrintData.bUpdateFieldsInPrinting )
    // <--
        pShell->UpdateFlds(TRUE);

// TLPDF   if( !pShell->Imp()->IsStopOutDev() &&
// TLPDF       ( bIsPDFExport || rPrintData.GetJobName().Len() || pOutDev->IsJobActive()) )
    if( /*!pShell->Imp()->IsStopOutDev() && */
        ( bIsPDFExport || rPrintData.GetJobName().Len() /*TLPDF|| pOutDev->IsJobActive()*/) )
    {
        BOOL bStop = FALSE;
        int nJobStartError = JOBSET_ERR_DEFAULT;

        XubString sJobName( rPrintData.GetJobName() );

// HACK: Hier muss von der MultiSelection noch eine akzeptable Moeglichkeit
// geschaffen werden, alle Seiten von Seite x an zu deselektieren.
// Z.B. durch SetTotalRange ....

//          aMulti.Select( Range( nLastPageNo+1, SELECTION_MAX ), FALSE );
// TLPDF           MultiSelection aTmpMulti( Range( 1, nLastPageNo ) );
            MultiSelection aTmpMulti( Range( 1, nPage ) );
            long nTmpIdx = aMulti.FirstSelected();
            static long nEndOfSelection = SFX_ENDOFSELECTION;
// TLPDF            while ( nEndOfSelection != nTmpIdx && nTmpIdx <= long(nLastPageNo) )
            while ( nEndOfSelection != nTmpIdx && nTmpIdx <= long(nPage) )
            {
                aTmpMulti.Select( nTmpIdx );
                nTmpIdx = aMulti.NextSelected();
            }
            aMulti = aTmpMulti;
// Ende des HACKs

#if 0
            const USHORT nSelCount = USHORT(aMulti.GetSelectCount()
                            /* * nCopyCnt*/);
#endif

            // PostitListe holen
            _SetGetExpFlds aPostItFields;
            SwDoc*     pPostItDoc   = 0;
            ViewShell* pPostItShell = 0;
            if( rPrintData.nPrintPostIts != POSTITS_NONE )
            {
                lcl_GetPostIts( pDoc, aPostItFields );
                pPostItDoc   = new SwDoc;
/* TLPDF
                if (pOutDev)
                    pPostItDoc->setPrinter( pOutDev, true, true );
*/
                pPostItShell = new ViewShell( *pPostItDoc, 0,
                                               pShell->GetViewOptions() );
                // Wenn PostIts am Dokumentenende gedruckt werden sollen,
                // die Druckreihenfolge allerdings umgekehrt ist, dann hier
                if ( ( rPrintData.nPrintPostIts == POSTITS_ENDDOC ) &&
                        rPrintData.bPrintReverse )
                        lcl_PrintPostItsEndDoc( pPostItShell, aPostItFields,
                        aMulti, sJobName, bStartJob, nJobStartError,
                        rPrintData.bPrintRightPage, rPrintData.bPrintLeftPage, TRUE );

            }

            // aOldMapMode wird fuer das Drucken von Umschlaegen gebraucht.
            MapMode aOldMapMode;

            const SwPageDesc *pLastPageDesc = NULL;
            // BOOL bSetOrient   = FALSE;
            // BOOL bSetPaperSz  = FALSE;
            BOOL bSetPrt      = FALSE;

            if ( rPrintData.nPrintPostIts != POSTITS_ONLY )
            {
//TLPDF                while( pStPage && !bStop )
                {
                    ::SetSwVisArea( pShell, pStPage->Frm(), bIsPDFExport );

                    //  wenn wir einen Umschlag drucken wird ein Offset beachtet
                    if( pStPage->GetFmt()->GetPoolFmtId() == RES_POOLPAGE_JAKET )
                    {
                        aOldMapMode = pOutDev->GetMapMode();
                        Point aNewOrigin = pOutDev->GetMapMode().GetOrigin();
                        aNewOrigin += rPrintData.aOffset;
                        MapMode aTmp( pOutDev->GetMapMode() );
                        aTmp.SetOrigin( aNewOrigin );
                        pOutDev->SetMapMode( aTmp );
                    }

                    {
                        if ( bSetPrt )
                        {
                            // check for empty page
                            const SwPageFrm& rFormatPage = pStPage->GetFormatPage();

                            if ( pLastPageDesc != rFormatPage.GetPageDesc() )
                            {
                                pLastPageDesc = rFormatPage.GetPageDesc();
#if 0 // TL_PDF
                                const BOOL bLandScp = rFormatPage.GetPageDesc()->GetLandscape();

                                if( bSetPaperBin )      // Schacht einstellen.
                                    pPrt->SetPaperBin( rFormatPage.GetFmt()->
                                                       GetPaperBin().GetValue() );

                                if (bSetOrient )
                                 {
                                        // Orientation einstellen: Breiter als Hoch
                                        //  -> Landscape, sonst -> Portrait.
                                        if( bLandScp )
                                            pPrt->SetOrientation(ORIENTATION_LANDSCAPE);
                                        else
                                            pPrt->SetOrientation(ORIENTATION_PORTRAIT);
                                 }

                                 if (bSetPaperSz )
                                 {
                                    Size aSize = pStPage->Frm().SSize();
                                    if ( bLandScp && bSetOrient )
                                    {
                                            // landscape is always interpreted as a rotation by 90 degrees !
                                            // this leads to non WYSIWIG but at least it prints!
                                            // #i21775#
                                            long nWidth = aSize.Width();
                                            aSize.Width() = aSize.Height();
                                            aSize.Height() = nWidth;
                                    }
                                    Paper ePaper = SvxPaperInfo::GetSvxPaper(aSize,MAP_TWIP,TRUE);
                                    if ( PAPER_USER == ePaper )
                                            pPrt->SetPaperSizeUser( aSize );
                                    else
                                            pPrt->SetPaper( ePaper );
                                 }
#endif
                            }
                        }

                        // Wenn PostIts nach Seite gedruckt werden sollen,
                        // jedoch Reverse eingestellt ist ...
                        if( rPrintData.bPrintReverse &&
                            rPrintData.nPrintPostIts == POSTITS_ENDPAGE )
                                lcl_PrintPostItsEndPage( pPostItShell, aPostItFields,
                                    nPage /* TLPDF nPageNo*/, aMulti, sJobName, bStartJob, nJobStartError,
                                    rPrintData.bPrintRightPage, rPrintData.bPrintLeftPage,
                                    rPrintData.bPrintReverse );


                        if( !bStartJob && JOBSET_ERR_DEFAULT == nJobStartError
                            && sJobName.Len() )
                        {

// TLPDF                            pShell->InitPrt( pOutDev, bIsPDFExport ? pOutDev : 0 );
                           pShell->InitPrt( pOutDev );

                            ::SetSwVisArea( pShell, pStPage->Frm(), bIsPDFExport );     // TLPDF
                            nJobStartError = JOBSET_ERR_ISSTARTET;
                        }
                        // --> FME 2005-12-12 #b6354161# Feature - Print empty pages
// TLPDF                        if ( rPrintData.bPrintEmptyPages || pStPage->Frm().Height() )
                        // <--
                        {
                            pStPage->GetUpper()->Paint( pStPage->Frm() );
                        }
                        SwPaintQueue::Repaint();

                        // Wenn PostIts nach Seite gedruckt werden sollen ...
                        if( (!rPrintData.bPrintReverse) &&
                            rPrintData.nPrintPostIts == POSTITS_ENDPAGE )
                                lcl_PrintPostItsEndPage( pPostItShell, aPostItFields,
                                    nPage /* TLPDF nPageNo */, aMulti, sJobName, bStartJob, nJobStartError,
                                    rPrintData.bPrintRightPage, rPrintData.bPrintLeftPage,
                                    rPrintData.bPrintReverse );
                    }

                    // den eventl. fuer Umschlaege modifizierte OutDevOffset wieder
                    // zuruecksetzen.
                    if( pStPage->GetFmt()->GetPoolFmtId() == RES_POOLPAGE_JAKET )
                        pOutDev->SetMapMode( aOldMapMode );

                }  // TLPDF loop end: while( pStPage && !bStop )
            }

            if (!bStop) // TLPDF: see break above
            {
                // Wenn PostIts am Dokumentenende gedruckt werden sollen, dann hier machen
                if( ((rPrintData.nPrintPostIts == POSTITS_ENDDOC) && !rPrintData.bPrintReverse)
                    || (rPrintData.nPrintPostIts == POSTITS_ONLY) )
                        lcl_PrintPostItsEndDoc( pPostItShell, aPostItFields, aMulti,
                            sJobName, bStartJob, nJobStartError,
                            rPrintData.bPrintRightPage, rPrintData.bPrintLeftPage,
                            rPrintData.bPrintReverse );

                if( pPostItShell )
                {
                    pPostItDoc->setPrinter( 0, false, false );  //damit am echten DOC der Drucker bleibt
                    delete pPostItShell;        //Nimmt das PostItDoc mit ins Grab.
                }

//TLPDF                if( bStartJob )
//TLPDF                    rPrintData.bJobStartet = TRUE;
            }   // TLPDF: if (!bStop) see break above

    }
    delete pStr;

    }  //Zus. Scope wg. CurShell!

    delete pShell;

    if (bSelection )
    {
         // damit das Dokument nicht den Drucker mit ins Grab nimmt
//        pOutDevDoc->setPrinter( 0, false, false );  /*TLPDF should not be needed anymore*/

        if ( !pOutDevDoc->release() )
            delete pOutDevDoc;
    }

    // restore settings of OutputDevice (should be done always now since the
    // output device is now provided by a call from outside the Writer)
    pOutDev->Pop();

    return bStartJob;
}

/******************************************************************************
 *  Methode     :   PrtOle2()
 *  Beschreibung:
 *  Erstellt    :   PK 07.12.94
 *  Aenderung   :   MA 16. Feb. 95
 ******************************************************************************/



void ViewShell::PrtOle2( SwDoc *pDoc, const SwViewOption *pOpt, const SwPrintData& rOptions,
                         OutputDevice* pOleOut, const Rectangle& rRect )
{
  //Wir brauchen eine Shell fuer das Drucken. Entweder hat das Doc schon
    //eine, dann legen wir uns eine neue Sicht an, oder das Doc hat noch
    //keine, dann erzeugen wir die erste Sicht.
    ViewShell *pSh;
    if( pDoc->GetRootFrm() && pDoc->GetRootFrm()->GetCurrShell() )
        pSh = new ViewShell( *pDoc->GetRootFrm()->GetCurrShell(), 0, pOleOut );
    else
        pSh = new ViewShell( *pDoc, 0, pOpt, pOleOut );

    {
        SET_CURR_SHELL( pSh );
        pSh->PrepareForPrint( rOptions );
        pSh->SetPrtFormatOption( TRUE );

        SwRect aSwRect( rRect );
        pSh->aVisArea = aSwRect;

        if ( pSh->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) &&
             pSh->GetNext() == pSh )
        {
            pSh->CheckBrowseView( FALSE );
            pDoc->GetRootFrm()->Lower()->InvalidateSize();
        }

        // --> FME 2005-02-10 #119474#
        // CalcPagesForPrint() should not be necessary here. The pages in the
        // visible area will be formatted in SwRootFrm::Paint().
        // Removing this gives us a performance gain during saving the
        // document because the thumbnail creation will not trigger a complete
        // formatting of the document.
        // Seiten fuers Drucken formatieren
        // pSh->CalcPagesForPrint( SHRT_MAX );
        // <--

        //#39275# jetzt will der Meyer doch ein Clipping
        pOleOut->Push( PUSH_CLIPREGION );
        pOleOut->IntersectClipRegion( aSwRect.SVRect() );
        pSh->GetLayout()->Paint( aSwRect );
//      SFX_APP()->SpoilDemoOutput( *pOleOut, rRect );
        pOleOut->Pop();

        // erst muss das CurrShell Object zerstoert werden!!
    }
    delete pSh;
}

/******************************************************************************
 *  Methode     :   IsAnyFieldInDoc()
 *  Beschreibung:   Stellt fest, ob im DocNodesArray Felder verankert sind
 *  Erstellt    :   JP 27.07.95
 *  Aenderung   :   JP 10.12.97
 ******************************************************************************/



BOOL ViewShell::IsAnyFieldInDoc() const
{
    const SfxPoolItem* pItem;
    USHORT nMaxItems = pDoc->GetAttrPool().GetItemCount( RES_TXTATR_FIELD );
    for( USHORT n = 0; n < nMaxItems; ++n )
        if( 0 != (pItem = pDoc->GetAttrPool().GetItem( RES_TXTATR_FIELD, n )))
        {
            const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
            const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
            if( pTxtFld && pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
                return TRUE;
        }
    return FALSE;
}



/******************************************************************************
 *  Klasse      :   SwPrtOptSave
 *  Erstellt    :   AMA 12.07.95
 *  Aenderung   :   AMA 12.07.95
 *  Holt sich im Ctor folgende Einstellungen des Druckers, die im Dtor dann
 *  wieder im Drucker gesetzt werden (falls sie sich ueberhaupt geaendert haben)
 *  - PaperBin - Orientation - PaperSize -
 ******************************************************************************/



SwPrtOptSave::SwPrtOptSave( Printer *pPrinter )
    : pPrt( pPrinter )
{
    if ( pPrt )
    {
        ePaper = pPrt->GetPaper();
        if ( PAPER_USER == ePaper )
            aSize = pPrt->GetPaperSize();
        eOrientation = pPrt->GetOrientation();
        nPaperBin = pPrt->GetPaperBin();

    }
}



SwPrtOptSave::~SwPrtOptSave()
{
    if ( pPrt )
    {
        if ( PAPER_USER == ePaper )
        {
            if( pPrt->GetPaperSize() != aSize )
                pPrt->SetPaperSizeUser( aSize );
        }
        else if ( pPrt->GetPaper() != ePaper )
            pPrt->SetPaper( ePaper );
        if ( pPrt->GetOrientation() != eOrientation)
            pPrt->SetOrientation( eOrientation );
        if ( pPrt->GetPaperBin() != nPaperBin )
            pPrt->SetPaperBin( nPaperBin );
    }
}


/******************************************************************************
 *  SwDrawViewSave
 *
 *  Saves some settings at the draw view
 ******************************************************************************/

SwDrawViewSave::SwDrawViewSave( SdrView* pSdrView )
    : pDV( pSdrView )
{
    if ( pDV )
    {
        sLayerNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM("Controls" ) );
        bPrintControls = pDV->IsLayerPrintable( sLayerNm );
    }
}

SwDrawViewSave::~SwDrawViewSave()
{
    if ( pDV )
    {
        pDV->SetLayerPrintable( sLayerNm, bPrintControls );
    }
}


// OD 09.01.2003 #i6467# - method also called for page preview
void ViewShell::PrepareForPrint( const SwPrintData &rOptions )
{
    // Viewoptions fuer den Drucker setzen
    pOpt->SetGraphic ( TRUE == rOptions.bPrintGraphic );
    pOpt->SetTable   ( TRUE == rOptions.bPrintTable );
    pOpt->SetDraw    ( TRUE == rOptions.bPrintDraw  );
    pOpt->SetControl ( TRUE == rOptions.bPrintControl );
    pOpt->SetPageBack( TRUE == rOptions.bPrintPageBackground );
    pOpt->SetBlackFont( TRUE == rOptions.bPrintBlackFont );

    if ( HasDrawView() )
    {
        SdrView *pDrawView = GetDrawView();
        String sLayerNm;
        sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Controls" ));
        // OD 09.01.2003 #i6467# - consider, if view shell belongs to page preview
        if ( !IsPreView() )
        {
            pDrawView->SetLayerPrintable( sLayerNm, rOptions.bPrintControl );
        }
        else
        {
            pDrawView->SetLayerVisible( sLayerNm, rOptions.bPrintControl );
        }
    }
}
