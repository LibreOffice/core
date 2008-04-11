/************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xprintext.cxx,v $
 * $Revision: 1.12 $
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
#include "precompiled_vcl.hxx"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if OSL_DEBUG_LEVEL == 0
#define NDEBUG
#endif
#include <assert.h>

#include <prex.h>
#include <X11/extensions/Print.h>
#include <postx.h>

#include <salunx.h>
#include <saldata.hxx>
#include <saldisp.hxx>
#include <vcl/salinst.hxx>
#include <vcl/salprn.hxx>
#include <vcl/salgdi.hxx>
#include <salprn.h>
#include <vcl/print.h>
#include <vcl/jobset.h>
#include "i18n_im.hxx"
#include "i18n_xkb.hxx"

// =======================================================================
//
//                          ImplSalPrinterData
//
// =======================================================================


class ImplSalPrinterData
{

private:

    SalDisplay*     mpDisplay;
    SalGraphics*        mpGraphics;
    char*           mpPrinterName;
    Display*        mpXDisplay;
    XPContext       maContext;
        Bool            XprtConnectStatus;


private:

    ImplSalPrinterData( ImplSalPrinterData& rData );

public:

                ImplSalPrinterData();
                ~ImplSalPrinterData();

        void            Init( const SalPrinterQueueInfo* pQueueInfo,
                                          ImplJobSetup*          pJobSetup );

    SalGraphics*        GetGraphics();
    void            ReleaseGraphics( SalGraphics* pGraphics = NULL );
    XLIB_Window     GetDrawable() const { return mpDisplay->GetRootWindow(); }
    SalColormap&        GetColormap() const { return mpDisplay->GetColormap(); }
    Display*        GetXDisplay() const { return mpXDisplay; }
    XPContext       GetXContext() const { return maContext; }
    const char*     GetPrinter()  const { return mpPrinterName; }
    XPContext       GetContext()  const { return maContext; }
    Bool            GetStatus()   const { return XprtConnectStatus; }
};

ImplSalPrinterData::ImplSalPrinterData() :
        mpDisplay( NULL ),
        mpGraphics( NULL ),
        mpXDisplay( NULL ),
        maContext( NULL ),
        mpPrinterName( NULL ),
        XprtConnectStatus( FALSE )
{
    Init(NULL, NULL);
}

void     ImplSalPrinterData::Init( const SalPrinterQueueInfo* pQueueInfo,
                                          ImplJobSetup*       pJobSetup )
{
    const char *printername = NULL;

    if (mpPrinterName == NULL || strcmp(mpPrinterName,printername)) {
    int nCount;
    XPContext aContext = NULL;
    char *Xprinter = getenv("XPRINTER");
    char *XpDisplayIndex;
    if (mpXDisplay == NULL && !XprtConnectStatus) {

        if (Xprinter && (XpDisplayIndex = strchr(Xprinter,'@'))) {
        if (Xprinter != XpDisplayIndex && printername == NULL) {
            char *defprinter = new char [XpDisplayIndex - Xprinter + 1];
            strncpy(defprinter, Xprinter, XpDisplayIndex - Xprinter);
            defprinter[XpDisplayIndex - Xprinter] = '\0';
            printername = defprinter;
        }
        }
        mpXDisplay = GetXpDisplay();
            // If GetXpDisplay() returns NULL (i.e. cannot connect to Xprint server) set XprtConnectStatus to FALSE.
            if (mpXDisplay == NULL) {
                fprintf(stderr, "Could not connect to Xprint server. Xprinting disabled.\n");
        XprtConnectStatus = FALSE;
            }
            else {
                //fprintf(stderr, "Connected to Xprint server.\n");
        if( getenv( "SAL_SYNCHRONIZE" ) )
            XSynchronize( mpXDisplay, True );

            if (printername == NULL || mpPrinterName == NULL
                || strcmp(mpPrinterName,printername) || maContext == NULL) {
            XpRehashPrinterList(mpXDisplay);
                XPPrinterList pList = XpGetPrinterList (mpXDisplay, NULL, &nCount);

                for ( int i = 0; i < nCount; i++ ) {
                   //fprintf (stderr, "Printer «%s»: «%s»\n",
                //pList[i].name ? pList[i].name : "(null)",
                //pList[i].desc ? pList[i].desc : "(null)" );
                if(pList[i].name)
                if (printername == NULL || strcmp (pList[i].name, printername) == 0) {
                    mpPrinterName = strdup( pList[i].name );
                    maContext = XpCreateContext ( mpXDisplay, mpPrinterName );
                    }
                }
            XpFreePrinterList (pList);
        }
        assert(maContext);
        XpSetContext (mpXDisplay, maContext);

        // New Sal
        if (mpDisplay == NULL) {
                mpDisplay = new SalDisplay( mpXDisplay, NULL );
              SalI18N_InputMethod* pInputMethod = new SalI18N_InputMethod;
                pInputMethod->Invalidate();
                mpDisplay->SetInputMethod( pInputMethod );
                SalI18N_KeyboardExtension *pKbdExtension = new SalI18N_KeyboardExtension( mpXDisplay );
                mpDisplay->SetKbdExtension( pKbdExtension );

        }
                // Connection to Xprint server successful so set XprtConnectStatus to TRUE.
        XprtConnectStatus = TRUE;
        }
    mpGraphics  = NULL;
    }
}
}

SalGraphics*
ImplSalPrinterData::GetGraphics()
{
    //If no Xprinter or mpGraphics already set then return NULL.
    if ( mpGraphics || !XprtConnectStatus) {
        return NULL;
        }
    mpGraphics = new SalGraphics;
    mpGraphics->maGraphicsData.Init( this );

    return mpGraphics;
}

void
ImplSalPrinterData::ReleaseGraphics( SalGraphics* pGraphics )
{
    if ( mpGraphics )
    {
        assert( !(pGraphics && pGraphics != mpGraphics) );
        delete mpGraphics;
        mpGraphics = NULL;
    }
}

ImplSalPrinterData::~ImplSalPrinterData()
{

    if ( mpPrinterName != NULL )
        free( mpPrinterName );
    XpDestroyContext(mpXDisplay, maContext);

      delete mpGraphics;
      delete mpDisplay;

    if ( mpXDisplay != NULL )
        XCloseDisplay( mpXDisplay );
}

// =======================================================================
//
//                          SalInfoPrinterData
//
// =======================================================================

SalInfoPrinterData::SalInfoPrinterData()
{
    mpImplData  = NULL;
}

SalInfoPrinterData::~SalInfoPrinterData()
{
    delete mpImplData;
}

void
SalInfoPrinterData::Init(
        SalPrinterQueueInfo *pQueueInfo,
        ImplJobSetup* pJobSetup )
{
    mpImplData = new ImplSalPrinterData();
}

// =======================================================================
//
//                          SalPrinterData
//
// =======================================================================

SalPrinterData::SalPrinterData()
{
    mpImplData = NULL;
}

SalPrinterData::~SalPrinterData()
{
    delete mpImplData;
}

void
SalPrinterData::Init( SalInfoPrinter *pInfoPrinter )
{
    mpImplData = new ImplSalPrinterData();
}

// =======================================================================
//
//                          SalInfoPrinter
//
// =======================================================================

SalInfoPrinter::SalInfoPrinter()
{
}

SalInfoPrinter::~SalInfoPrinter()
{
}

SalGraphics*
SalInfoPrinter::GetGraphics()
{
    return maPrinterData.mpImplData->GetGraphics();
}

void
SalInfoPrinter::ReleaseGraphics( SalGraphics* pGraphics )
{
    maPrinterData.mpImplData->ReleaseGraphics( pGraphics );
}

BOOL
SalInfoPrinter::Setup( SalFrame* pFrame, ImplJobSetup* pJobSetup )
{
    pJobSetup->mePaperFormat = PAPER_A4;
    pJobSetup->mnPaperWidth  = 21000;
    pJobSetup->mnPaperHeight = 29700;
    pJobSetup->meOrientation =  ORIENTATION_PORTRAIT;
    return TRUE;
}

BOOL
SalInfoPrinter::SetPrinterData( ImplJobSetup* pJobSetup )
{
    pJobSetup->mePaperFormat = PAPER_A4;
    pJobSetup->mnPaperWidth  = 21000;
    pJobSetup->mnPaperHeight = 29700;
    pJobSetup->meOrientation =  ORIENTATION_PORTRAIT;
    return TRUE;
}

BOOL
SalInfoPrinter::SetData( ULONG nSetDataFlags, ImplJobSetup* pJobSetup )
{
    pJobSetup->mePaperFormat = PAPER_A4;
    pJobSetup->mnPaperWidth  = 21000;
    pJobSetup->mnPaperHeight = 29700;
    pJobSetup->meOrientation =  ORIENTATION_PORTRAIT;
    return TRUE;
}

void
SalInfoPrinter::GetPageInfo( const ImplJobSetup* pImplJobSetup,
        long& rOutWidth,  long& rOutHeight,
        long& rPageOffX,  long& rPageOffY,
        long& rPageWidth, long& rPageHeight )
{
    rPageWidth  = 2550;
    rPageHeight = 3300;
    rPageOffX   = 75;
    rPageOffY   = 75;
    rOutWidth   = rPageWidth - rPageOffX - 75;
    rOutHeight  = rPageHeight- rPageOffY - 75;
}

ULONG
SalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pJobSetup )
{
    return 1;
}

XubString
SalInfoPrinter::GetPaperBinName( const ImplJobSetup* pJobSetup,
        ULONG nPaperBin )
{
    return(XubString(RTL_CONSTASCII_USTRINGPARAM("PaperBinName")));
    // return "PaperBinName";
}

ULONG
SalInfoPrinter::GetCapabilities( const ImplJobSetup* pSetupData, USHORT nType )
{
    return 0;
}

// =======================================================================
//
//                          SalPrinter
//
// =======================================================================

SalPrinter::SalPrinter()
{
}

SalPrinter::~SalPrinter()
{
}

BOOL
SalPrinter::StartJob(
        const XubString* pFileName,
        const XubString& rJobName,
        const XubString& rAppName,
        ULONG nCopies, BOOL bCollate,
        ImplJobSetup* pJobSetup )
{
    Display  *pDisplay = maPrinterData.mpImplData->GetXDisplay();
    XPContext aContext = maPrinterData.mpImplData->GetXContext();
    const char* pPrinterName = maPrinterData.mpImplData->GetPrinter();

    XpSelectInput (pDisplay, aContext, XPPrintMask);

    char pJobName[ 64 ];
    snprintf (pJobName, sizeof(pJobName), "%s.job-name: XPrint%d", pPrinterName, getpid() );

    XpStartJob( pDisplay, XPSpool );

    return TRUE;
}

BOOL
SalPrinter::EndJob()
{
    Display  *pDisplay = maPrinterData.mpImplData->GetXDisplay();
    XEvent    aEvent;;

    XpEndJob( pDisplay );
    XSync( pDisplay, False );
    // Wait until printing is done
    do
    {
        // XNextEvent (pDisplay, &aEvent);
    }
    while ( 0 );
    //     aEvent.type != XPPrintNotify
    //      && ((XPPrintEvent *) (&aEvent))->detail != XPEndJobNotify);

      return TRUE;
}

BOOL
SalPrinter::AbortJob()
{
    return FALSE;
}

SalGraphics*
SalPrinter::StartPage( ImplJobSetup* pJobSetup, BOOL bNewJobData )
{
    Display     *pDisplay  = maPrinterData.mpImplData->GetXDisplay();
    SalGraphics *pGraphics = maPrinterData.mpImplData->GetGraphics();

    Drawable aDrawable = pGraphics->maGraphicsData.GetDrawable();
    XPContext nContext = maPrinterData.mpImplData->GetContext();

    unsigned short nWidth, nHeight;
    XRectangle     aArea;
    Status nState = XpGetPageDimensions( pDisplay, nContext,
            &nWidth, &nHeight, &aArea );
    //fprintf(stderr, "PageSize = %ix%i (%i,%i %ix%i)\n", nWidth, nHeight,
    //  aArea.x,aArea.y, aArea.width, aArea.height );
    XResizeWindow( pDisplay, aDrawable, nWidth, nHeight );
    XpStartPage ( pDisplay, aDrawable );

      return pGraphics;
}

BOOL
SalPrinter::EndPage()
{
    Display *pDisplay = maPrinterData.mpImplData->GetXDisplay();
    XpEndPage ( pDisplay );

      maPrinterData.mpImplData->ReleaseGraphics();

      return TRUE;
}

ULONG
SalPrinter::GetErrorCode()
{
    return 0;
}

// =======================================================================
//
//                          SalInstance
//
// =======================================================================

SalInfoPrinter*
SalInstance::CreateInfoPrinter(
        SalPrinterQueueInfo*    pQueueInfo,
        ImplJobSetup*           pSetup )
{
      // create and initialize SalInfoPrinter
      SalInfoPrinter* pPrinter = new SalInfoPrinter;
      pPrinter->maPrinterData.Init( pQueueInfo, pSetup );

    pSetup->mePaperFormat = PAPER_A4;       // Papierformat
    pSetup->mnPaperWidth  = 21000;  // Papierbreite in 100tel mm
    pSetup->mnPaperHeight = 29700;  // Papierhoehe in 100tel mm

      return pPrinter;
}

void
SalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
  delete pPrinter;
}

SalPrinter*
SalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
  // create and initialize SalPrinter
  SalPrinter* pPrinter = new SalPrinter;
  pPrinter->maPrinterData.Init( pInfoPrinter );

  return pPrinter;
}

void
SalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
      delete pPrinter;
}


void
SalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
    // Neuen Eintrag anlegen
    int nCount;
       Display *XprtDisp = GetXpDisplay();
    if (XprtDisp == NULL) {
                fprintf(stderr, "Could not connect to Xprint server. Xprinting disabled.\n");
        return;
    }
    else {
        XpRehashPrinterList(XprtDisp);
        XPPrinterList XpList = XpGetPrinterList(XprtDisp, NULL, &nCount);

            SalPrinterQueueInfo* pInfo = new SalPrinterQueueInfo;
            String Name(XpList[0].name, RTL_TEXTENCODING_UTF8);
            pInfo->maPrinterName    = XubString(Name);
            pInfo->maDriver     = XubString(RTL_CONSTASCII_USTRINGPARAM("X Printer"));
            pInfo->maLocation   = XubString(RTL_CONSTASCII_USTRINGPARAM("X Printer"));
            pInfo->maComment    = XubString(RTL_CONSTASCII_USTRINGPARAM("X Printer"));
            pInfo->mpSysData    = NULL;
            pList->Add( pInfo );

        XpFreePrinterList(XpList);
    }
}

void
SalInstance::GetPrinterQueueState( SalPrinterQueueInfo* pInfo )
{
    return;
}

void
SalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
      delete pInfo;
}

XubString
SalInstance::GetDefaultPrinter()
{
    Display *XprtDisp = GetXpDisplay();
    int nCount;
    if (XprtDisp == NULL)
        return XubString(RTL_CONSTASCII_USTRINGPARAM("No Default"));
    else {
            XpRehashPrinterList(XprtDisp);
            XPPrinterList XpList = XpGetPrinterList(XprtDisp, NULL, &nCount);

        String Name( XpList[0].name, RTL_TEXTENCODING_UTF8 );
            XpFreePrinterList(XpList);
        return (XubString(Name));
    }

    //return(XubString(RTL_CONSTASCII_USTRINGPARAM("X Printer")));
    // return "X Printer";
}

// =======================================================================
//
//                          SalGraphicsData
//
// =======================================================================

void SalGraphicsData::Init(ImplSalPrinterData *pPrinter)
{
if (pPrinter->GetStatus()) {
  xColormap_    = &(pPrinter->GetColormap());
  hDrawable_    = pPrinter->GetDrawable();
  //pGCCache_       = pPrinter->GetGCCache();

  bPrinter_     = TRUE;

  nPenPixel_    = GetPixel( nPenColor_ );
  nTextPixel_   = GetPixel( nTextColor_ );
  nBrushPixel_  = GetPixel( nBrushColor_ );
}
else
  bPrinter_     = FALSE;

}

// =======================================================================
//
//                          Utility Functions
//
// =======================================================================

// GetXpDisplay().
// Finds and returns the Xprint display. First looks at environment variable XPRINTER
// which should be in the form <printername>@<host>:<display number>. If not defined,
// then environment variable XPDISPLAY is checked. It should be in the form
// <host>:<display number>. If it is not defined it is set by default to ":1". If an
// Xprint server is found then a pointer to Display is returned, otherwise NULL. This
// function can be used by other functions to determine the current Xprint server display.

// [ed] 6/15/02 We've got some linkage errors with this function on OS X,
// perhaps due to mismatched prototypes.  Let's take a quick route to the finish
// line and declare it C linkage! +++ FIXME
#ifdef MACOSX
extern "C"
#endif
Display*
GetXpDisplay()
{
    char *XpDisplayName=NULL;
    Display *XpDisplay;
    if (getenv("XPRINTER")) {
        XpDisplayName=strchr(getenv("XPRINTER"),'@');
        if (XpDisplayName != NULL) {
            XpDisplayName++;
        }
    }
    else {
        if (!getenv("XPDISPLAY"))
            putenv("XPDISPLAY=:1");
        XpDisplayName=getenv("XPDISPLAY");
    }
    XpDisplay=XOpenDisplay(XpDisplayName);
    if (XpDisplay==NULL || !XSalIsPrinter(XpDisplay)) {
        return NULL;
    }
    else {
        return XpDisplay;
    }
}

// [ed] 6/15/02 We've got some linkage errors with this function on OS X,
// perhaps due to mismatched prototypes.  Let's take a quick route to the finish
// line and declare it C linkage!  +++ FIXME
#ifdef MACOSX
extern "C"
#endif
Bool
XSalIsPrinter( Display * display )
{
    int nEventBase;
    int nErrorBase;

    Bool bPrinter = XpQueryExtension( display, &nEventBase, &nErrorBase );
    return bPrinter;
}

// [ed] 6/15/02 We've got some linkage errors with this function on OS X,
// perhaps due to mismatched prototypes.  Let's take a quick route to the finish
// line and declare it C linkage! +++ FIXME
#ifdef MACOSX
extern "C"
#endif
Bool
XSalIsDisplay( Display * display )
{
    return !XSalIsPrinter( display );
}

