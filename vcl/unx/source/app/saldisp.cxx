/*************************************************************************
 *
 *  $RCSfile: saldisp.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: cp $ $Date: 2001-03-19 08:30:58 $
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

#define _SV_SALDISP_CXX

#define SAL_XT

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#ifdef __SunOS_5_5_1
extern "C" { int gethostname(char*,int); }
#endif

#include <prex.h>
#include <X11/cursorfont.h>
#include "nodrop_curs.h"
#include "nodrop_mask.h"
#include "wait_curs.h"
#include "wait_mask.h"
#include "hsplit_curs.h"
#include "hsplit_mask.h"
#include "vsplit_curs.h"
#include "vsplit_mask.h"
#include "neswsize_curs.h"
#include "neswsize_mask.h"
#include "nwsesize_curs.h"
#include "nwsesize_mask.h"
#include "magnify_curs.h"
#include "magnify_mask.h"
#include "rotate_curs.h"
#include "rotate_mask.h"
#include "hshear_curs.h"
#include "hshear_mask.h"
#include "vshear_curs.h"
#include "vshear_mask.h"
#include "drawline_curs.h"
#include "drawline_mask.h"
#include "drawrect_curs.h"
#include "drawrect_mask.h"
#include "drawpolygon_curs.h"
#include "drawpolygon_mask.h"
#include "drawbezier_curs.h"
#include "drawbezier_mask.h"
#include "drawarc_curs.h"
#include "drawarc_mask.h"
#include "drawpie_curs.h"
#include "drawpie_mask.h"
#include "drawcirclecut_curs.h"
#include "drawcirclecut_mask.h"
#include "drawellipse_curs.h"
#include "drawellipse_mask.h"
#include "drawconnect_curs.h"
#include "drawconnect_mask.h"
#include "drawtext_curs.h"
#include "drawtext_mask.h"
#include "mirror_curs.h"
#include "mirror_mask.h"
#include "crook_curs.h"
#include "crook_mask.h"
#include "crop_curs.h"
#include "crop_mask.h"
#include "move_curs.h"
#include "move_mask.h"
#include "movepoint_curs.h"
#include "movepoint_mask.h"
#include "movebezierweight_curs.h"
#include "movebezierweight_mask.h"
#include "drawfreehand_curs.h"
#include "drawfreehand_mask.h"
#include "drawcaption_curs.h"
#include "drawcaption_mask.h"
#include "movedata_curs.h"
#include "movedata_mask.h"
#include "copydata_curs.h"
#include "copydata_mask.h"
#include "linkdata_curs.h"
#include "linkdata_mask.h"
#include "movedlnk_curs.h"
#include "movedlnk_mask.h"
#include "copydlnk_curs.h"
#include "copydlnk_mask.h"
#include "movefile_curs.h"
#include "movefile_mask.h"
#include "copyfile_curs.h"
#include "copyfile_mask.h"
#include "linkfile_curs.h"
#include "linkfile_mask.h"
#include "moveflnk_curs.h"
#include "moveflnk_mask.h"
#include "copyflnk_curs.h"
#include "copyflnk_mask.h"
#include "movefiles_curs.h"
#include "movefiles_mask.h"
#include "copyfiles_curs.h"
#include "copyfiles_mask.h"

#include "chart_curs.h"
#include "chart_mask.h"
#include "detective_curs.h"
#include "detective_mask.h"
#include "pivotcol_curs.h"
#include "pivotcol_mask.h"
#include "pivotfld_curs.h"
#include "pivotfld_mask.h"
#include "pivotrow_curs.h"
#include "pivotrow_mask.h"

#include "chain_curs.h"
#include "chain_mask.h"
#include "chainnot_curs.h"
#include "chainnot_mask.h"

#include "timemove_curs.h"
#include "timemove_mask.h"
#include "timesize_curs.h"
#include "timesize_mask.h"

#include "ase_curs.h"
#include "ase_mask.h"
#include "asn_curs.h"
#include "asn_mask.h"
#include "asne_curs.h"
#include "asne_mask.h"
#include "asns_curs.h"
#include "asns_mask.h"
#include "asnswe_curs.h"
#include "asnswe_mask.h"
#include "asnw_curs.h"
#include "asnw_mask.h"
#include "ass_curs.h"
#include "ass_mask.h"
#include "asse_curs.h"
#include "asse_mask.h"
#include "assw_curs.h"
#include "assw_mask.h"
#include "asw_curs.h"
#include "asw_mask.h"
#include "aswe_curs.h"
#include "aswe_mask.h"
#include "null_curs.h"
#include "null_mask.h"

#include "airbrush_curs.h"
#include "airbrush_mask.h"
#include "fill_curs.h"
#include "fill_mask.h"
#include "vertcurs_curs.h"
#include "vertcurs_mask.h"

#include "invert50.h"
#if !(defined S390 || defined AIX)
#include <X11/extensions/XShm.h>
#endif
#include <X11/keysym.h>

#ifdef USE_XMU
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/SysUtil.h>
#endif
#include <X11/Xatom.h>
#include <postx.h>

#include <salunx.h>

#ifndef _SAL_I18N_INPUTMETHOD_HXX
#include "i18n_im.hxx"
#endif
#ifndef _SAL_I18N_XKBDEXTENSION_HXX
#include "i18n_xkb.hxx"
#endif

#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_KEYCODES_HXX
#include <keycodes.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#ifndef _SV_SALSYS_HXX
#include <salsys.hxx>
#endif
#ifndef _SV_SALOGL_HXX
#include <salogl.hxx>
#endif
#ifndef _OSL_THREADMUTEX_H_
#include <osl/mutex.h>
#endif
#ifndef _SV_SALOBJ_HXX
#include <salobj.hxx>
#endif
#ifndef _VCL_SM_HXX
#include <sm.hxx>
#endif

#include <osl/socket.h>
#include <rtl/ustring>

// -=-= #defines -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define PSEUDOCOLOR12
#define PSEUDOCOLOR8
#define TRUECOLOR24
#define TRUECOLOR16
#define TRUECOLOR15
#define TRUECOLOR12
#define TRUECOLOR8

#define SALCOLOR_WHITE      MAKE_SALCOLOR( 0xFF, 0xFF, 0xFF )
#define SALCOLOR_BLACK      MAKE_SALCOLOR( 0x00, 0x00, 0x00 )

// -=-= Prototyps =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
EXTERN_C int XShmGetEventBase( Display* );

// -=-= static variables -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const char* const VisualClassName[] = {
    "StaticGray",
    "GrayScale",
    "StaticColor",
    "PseudoColor",
    "TrueColor",
    "DirectColor"
};

static const char* const AtomStrings[] =
{
    "WM_PROTOCOLS",         // window manager
    "WM_STATE",
    "WM_DELETE_WINDOW",
    "WM_SAVE_YOURSELF",
    "WM_COMMAND",

    "SAL_QUITEVENT",        // client message events
    "SAL_USEREVENT",
    #if !defined(__synchronous_extinput__)
    "SAL_EXTTEXTEVENT",
    #endif
};

static const char* const EventNames[] =
{
    NULL,
    NULL,
    "KeyPress",
    "KeyRelease",
    "ButtonPress",
    "ButtonRelease",
    "MotionNotify",
    "EnterNotify",
    "LeaveNotify",
    "FocusIn",
    "FocusOut",
    "KeymapNotify",
    "Expose",
    "GraphicsExpose",
    "NoExpose",
    "VisibilityNotify",
    "CreateNotify",
    "DestroyNotify",
    "UnmapNotify",
    "MapNotify",
    "MapRequest",
    "ReparentNotify",
    "ConfigureNotify",
    "ConfigureRequest",
    "GravityNotify",
    "ResizeRequest",
    "CirculateNotify",
    "CirculateRequest",
    "PropertyNotify",
    "SelectionClear",
    "SelectionRequest",
    "SelectionNotify",
    "ColormapNotify",
    "ClientMessage",
    "MappingNotify"
};

static UINT32 nIn___, nOut___;

// -=-= global inline =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline const char *Null( const char *p ) { return p ? p : ""; }
inline const char *GetEnv( const char *p ) { return Null( getenv( p ) ); }
inline const char *KeyStr( KeySym n ) { return Null( XKeysymToString( n ) ); }

#ifdef USE_XMU
inline const char *GetAtomName( Display *d, Atom a )
{ return Null( XmuGetAtomName( d, a ) ); }
#else
inline const char *GetAtomName( Display *d, Atom a )
{ return Null( XGetAtomName( d, a ) ); }
#endif

inline double Hypothenuse( long w, long h )
{ return sqrt( (w*w)+(h*h) ); }

inline int ColorDiff( int r, int g, int b )
{ return (r*r)+(g*g)+(b*b); }

inline int ColorDiff( SalColor c1, int r, int g, int b )
{ return ColorDiff( (int)SALCOLOR_RED  (c1)-r,
                    (int)SALCOLOR_GREEN(c1)-g,
                    (int)SALCOLOR_BLUE (c1)-b ); }

// -=-= global functions -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static int sal_Shift( Pixel nMask )
{
    int i = 24;
    if( nMask < 0x00010000 ) { nMask <<= 16; i -= 16; }
    if( nMask < 0x01000000 ) { nMask <<=  8; i -=  8; }
    if( nMask < 0x10000000 ) { nMask <<=  4; i -=  4; }
    if( nMask < 0x40000000 ) { nMask <<=  2; i -=  2; }
    if( nMask < 0x80000000 ) { nMask <<=  1; i -=  1; }
    return i;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static BOOL sal_GetVisualInfo( Display *pDisplay, XID nVID, XVisualInfo &rVI )
{
    int         nInfos;
    XVisualInfo aTemplate;
    XVisualInfo*pInfos;

    aTemplate.visualid = nVID;

    pInfos = XGetVisualInfo( pDisplay, VisualIDMask, &aTemplate, &nInfos );
    if( !pInfos )
        return FALSE;

    rVI = *pInfos;
    XFree( pInfos );

    DBG_ASSERT( rVI.visualid == nVID,
                "sal_GetVisualInfo: could not get correct visual by visualId" )
    return TRUE;
}

// ---------------------------------------------------------------------------

// check wether displaystring is in format N.M or N. or just N
// with N and M beeing natural numbers
static BOOL
sal_IsDisplayNumber( const char *pDisplayString )
{
    if ( ! isdigit(*pDisplayString) )
        return FALSE;
    while ( isdigit(*(++pDisplayString)) )
        ; /* do nothing */

    if ( *pDisplayString == '.' )
    {
        while ( isdigit(*(++pDisplayString)) )
            ; /* do nothing */
    }

    return (*pDisplayString == '\0');
}

// check whether host1 and host2 point to the same ip address
static BOOL
sal_EqualHosts( const ::rtl::OUString& Host1, const ::rtl::OUString& Host2)
{
    oslSocketAddr pHostAddr1;
    oslSocketAddr pHostAddr2;
    BOOL bEqualAddress;

    if ( Host1.toChar() >= '0' && Host1.toChar() <= '9' )
        pHostAddr1 = osl_createInetSocketAddr( Host1.pData, 0 );
    else
        pHostAddr1 = osl_resolveHostname( Host1.pData );

    if ( Host2.toChar() >= '0' && Host2.toChar() <= '9' )
        pHostAddr2 = osl_createInetSocketAddr( Host2.pData, 0 );
    else
        pHostAddr2 = osl_resolveHostname( Host2.pData );

    bEqualAddress = osl_isEqualSocketAddr( pHostAddr1, pHostAddr2 ) ? TRUE : FALSE;

    osl_destroySocketAddr( pHostAddr1 );
    osl_destroySocketAddr( pHostAddr2 );

    return bEqualAddress;
}

static BOOL
sal_IsLocalDisplay( Display *pDisplay )
{
    const char *pDisplayString = DisplayString( pDisplay );

    // no string, no idea
    if (   pDisplayString == NULL || pDisplayString[ 0 ] == '\0')
        return FALSE;

    // check for ":x.y"
    if ( pDisplayString[ 0 ] == ':' )
        return sal_IsDisplayNumber( pDisplayString + 1 );

    // check for fixed token which all mean localhost:x.y
    const char  pLocal[]    = "localhost:";
    const int   nLocalLen   = sizeof(pLocal) - 1;
    if ( strncmp(pDisplayString, pLocal, nLocalLen) == 0 )
        return sal_IsDisplayNumber( pDisplayString + nLocalLen );

    const char  pUnix[]     = "unix:";
    const int   nUnixLen    = sizeof(pUnix) - 1;
    if ( strncmp(pDisplayString, pUnix,      nUnixLen)      == 0 )
        return sal_IsDisplayNumber( pDisplayString + nUnixLen );

    const char  pLoopback[] = "127.0.0.1:";
    const int   nLoopbackLen= sizeof(pLoopback) - 1;
    if ( strncmp(pDisplayString, pLoopback,  nLoopbackLen)  == 0 )
        return sal_IsDisplayNumber( pDisplayString + nLoopbackLen );

    // compare local hostname to displaystring, both may be ip address or
    // hostname
    BOOL  bEqual = FALSE;
    char *pDisplayHost  = strdup(  pDisplayString );
    char *pPtr          = strrchr( pDisplayHost, ':' );

    if( pPtr != NULL )
    {
        ::rtl::OUString aLocalHostname;
        if( osl_getLocalHostname( &aLocalHostname.pData ) == osl_Socket_Ok)
        {
            *pPtr = '\0';
            ::rtl::OUString aDisplayHostname( pDisplayHost, strlen( pDisplayHost ), gsl_getSystemTextEncoding() );
            bEqual = sal_EqualHosts( aLocalHostname, aDisplayHostname );
            bEqual = bEqual && sal_IsDisplayNumber( pPtr + 1 );
        }
    }
    free( pDisplayHost );

    return bEqual;
}

// ---------------------------------------------------------------------------
// IsLocal means soffice is running on the same host as the xserver
// since it is not called very often and sal_IsLocalDisplay() is relative
// expensive bLocal_ is initialized on first call

BOOL SalDisplay::IsLocal()
{
    if ( ! mbLocalIsValid )
    {
        bLocal_ = sal_IsLocalDisplay( pDisp_ );
        mbLocalIsValid = TRUE;
    }
    return (BOOL)bLocal_;
}

// ---------------------------------------------------------------------------
extern "C" srv_vendor_t
sal_GetServerVendor( Display *p_display )
{
    typedef struct {
        srv_vendor_t  e_vendor; // vendor as enum
        char         *p_name;   // vendor name as returned by VendorString()
        unsigned int  n_len;    // number of chars to compare
    } vendor_t;

    const vendor_t p_vendorlist[] = {
        { vendor_xfree,       "The XFree86 Project, Inc",        13 },
        { vendor_sun,         "Sun Microsystems, Inc.",          10 },
        { vendor_attachmate,  "Attachmate Corporation",          10 },
        { vendor_excursion,
            "DECWINDOWS DigitalEquipmentCorporation, eXcursion", 42 },
        { vendor_hp,          "Hewlett-Packard Company",         17 },
        { vendor_hummingbird, "Hummingbird Communications Ltd.", 11 },
        { vendor_ibm,         "International Business Machines", 24 },
        { vendor_sgi,         "Silicon Graphics",                 9 },
        { vendor_sco,         "The Santa Cruz Operation",        16 },
        { vendor_xinside,     "X Inside Inc.",                   10 },
        // allways the last entry: vendor_none to indicate eol
        { vendor_none,        NULL,                               0 },
    };

#ifndef USE_PSPRINT
    // handle xprinter separately, since it doesn´t implement ServerVendor()
    if ( ! XSalIsDisplay( p_display ) )
        return vendor_xprinter;
#endif

    // handle regular server vendors
    char     *p_name   = ServerVendor( p_display );
    vendor_t *p_vendor;
    for (p_vendor = const_cast<vendor_t*>(p_vendorlist); p_vendor->e_vendor != vendor_none; p_vendor++)
    {
        if ( strncmp (p_name, p_vendor->p_name, p_vendor->n_len) == 0 )
            return p_vendor->e_vendor;
    }

    // vendor not found in list
    return vendor_unknown;
}

// -=-= SalDisplay -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
BOOL SalDisplay::BestVisual( Display     *pDisplay,
                                   int          nScreen,
                                   XVisualInfo &rVI )
{
    VisualID nDefVID = XVisualIDFromVisual( DefaultVisual( pDisplay, nScreen ) );
    VisualID    nVID = 0;
    char       *pVID = getenv( "SAL_VISUAL" );
    if( pVID )
        sscanf( pVID, "%li", &nVID );

    if( nVID && sal_GetVisualInfo( pDisplay, nVID, rVI ) )
        return rVI.visualid == nDefVID;

    XVisualInfo aVI;
    // get all visuals
    int nVisuals;
    XVisualInfo* pVInfos = XGetVisualInfo( pDisplay, VisualNoMask,
                                           &aVI, &nVisuals );
    // pVInfos should contain at least one visual, otherwise
    // we're in trouble
    int* pWeight = new int[ nVisuals ];
    int i;
    for( i = 0; i < nVisuals; i++ )
    {
        BOOL bUsable = FALSE;
        int nTrueColor = 1;

        if ( pVInfos[i].screen != nScreen )
        {
            bUsable = FALSE;
        }
        else
        if( pVInfos[i].c_class == TrueColor )
        {
            nTrueColor = 2048;
            if( pVInfos[i].depth == 24 )
                bUsable = TRUE;
#ifdef TRUECOLOR8
            else if( pVInfos[i].depth == 8 )
            {
                nTrueColor = -1; // strongly discourage 8 bit true color
                bUsable = TRUE;
            }
#endif
#ifdef TRUECOLOR15
            else if( pVInfos[i].depth == 15 )
                bUsable = TRUE;
#endif
#ifdef TRUECOLOR16
            else if( pVInfos[i].depth == 16 )
                bUsable = TRUE;
#endif
#ifdef TRUECOLOR32
            else if( pVInfos[i].depth == 32 )
            {
                nTrueColor = 256;
                // we do not have use for an alpha channel
                // better use a 24 or 16 bit truecolor visual if possible
                bUsable = TRUE;
            }
#endif
        }
        else if( pVInfos[i].c_class == PseudoColor )
        {
            if( pVInfos[i].depth <= 8 )
                bUsable = TRUE;
#ifdef PSEUDOCOLOR12
            else if( pVInfos[i].depth == 12 )
                bUsable = TRUE;
#endif
        }
        pWeight[ i ] = bUsable ? nTrueColor*pVInfos[i].depth : -1024;
        pWeight[ i ] -= pVInfos[ i ].visualid;
    }

    SalOpenGL::MakeVisualWeights( pDisplay, pVInfos, pWeight, nVisuals );

    int nBestVisual = 0;
    int nBestWeight = -1024;
    for( i = 0; i < nVisuals; i++ )
    {
        if( pWeight[ i ] > nBestWeight )
        {
            nBestWeight = pWeight[ i ];
            nBestVisual = i;
        }
    }

    rVI = pVInfos[ nBestVisual ];

    XFree( pVInfos );
    return rVI.visualid == nDefVID;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

SalDisplay::SalDisplay( Widget w ) :
        mpFallbackFactory ( NULL )
{
    SalData *pSalData = GetSalData();

    if( !pSalData->GetDefDisp() )
        pSalData->SetDefDisp( this );
    if( !pSalData->GetCurDisp() )
        pSalData->SetCurDisp( this );

    pXLib_          = pSalData->GetLib();
    pDisp_          = XtDisplay( w );
    nScreen_        = DefaultScreen( pDisp_ );
    hShell_         = w;
    hComposite_     = XtVaCreateManagedWidget(
        "ShellComposite",
        SAL_COMPOSITE_WIDGET,
        hShell_,
        NULL );

    Visual     *pVisual = NULL;
    Colormap    hColMap = None;
    Arg         aArgs[10];

    XtSetArg( aArgs[0], XtNvisual,      &pVisual    );
    XtSetArg( aArgs[1], XtNcolormap,    &hColMap    );

    XtGetValues( hShell_, aArgs, 2 );
    if( !pVisual )
        pVisual = DefaultVisual( pDisp_, nScreen_ );
    if (!hColMap)
        hColMap = DefaultColormap( pDisp_, nScreen_ );
    if( !IsDisplay() && !hColMap)
        hColMap = 1;   // trick for XPrinter

    XVisualInfo aXVI;
    sal_GetVisualInfo( pDisp_, XVisualIDFromVisual( pVisual ), aXVI );

    Init( hColMap, &aXVI );
}

SalDisplay::SalDisplay( Display *display, Visual *pVisual, Colormap aColMap ) :
        pDisp_( display ),
        mpFallbackFactory ( NULL )
{
    SalData *pSalData  = GetSalData();
    XVisualInfo aXVI;

    if( !pSalData->GetDefDisp() )
        pSalData->SetDefDisp( this );
    if( !pSalData->GetCurDisp() )
        pSalData->SetCurDisp( this );

#ifndef USE_PSPRINT
    pXLib_    = XSalIsDisplay( pDisp_ ) ? pSalData->GetLib() : NULL;
#else
    pXLib_    = pSalData->GetLib();
#endif
    nScreen_  = DefaultScreen( pDisp_ );

    if (!aColMap)
        aColMap = DefaultColormap( display, nScreen_ );
    if( !IsDisplay() && !aColMap)
        aColMap = 1;   // trick for XPrinter
    if (!pVisual)
        pVisual = DefaultVisual( pDisp_, nScreen_ );

    sal_GetVisualInfo( pDisp_, XVisualIDFromVisual( pVisual ), aXVI );
    Init( aColMap, &aXVI );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalDisplay::~SalDisplay( )
{
    SalData *pSalData = GetSalData();

    SalBitmap::ImplDestroyCache();
    DestroyFontCache();

    if( IsDisplay() )
    {
        osl_destroyMutex( hEventGuard_ );

        XDestroyWindow( pDisp_, hRefWindow_ );
        if( pMonoGC_ != pCopyGC_ )
            XFreeGC( pDisp_, pMonoGC_ );
        XFreeGC( pDisp_, pCopyGC_ );
        XFreeGC( pDisp_, pAndInvertedGC_ );
        XFreeGC( pDisp_, pAndGC_ );
        XFreeGC( pDisp_, pOrGC_ );
        XFreeGC( pDisp_, pStippleGC_ );
        XFreePixmap( pDisp_, hInvert50_ );

        hEventGuard_            = (oslMutex)ILLEGAL_POINTER;
        hRefWindow_             = (XLIB_Window)ILLEGAL_POINTER;
        pMonoGC_                = (GC)ILLEGAL_POINTER;
        pCopyGC_                = (GC)ILLEGAL_POINTER;
        pAndInvertedGC_         = (GC)ILLEGAL_POINTER;
        pAndGC_                 = (GC)ILLEGAL_POINTER;
        pOrGC_                  = (GC)ILLEGAL_POINTER;
        pStippleGC_             = (GC)ILLEGAL_POINTER;
        hInvert50_              = None;

        for( size_t i = 0; i < POINTER_COUNT; i++ )
        {
            XFreeCursor( pDisp_, aPointerCache_[i] );
#ifdef DBG_UTIL
            aPointerCache_[i] = None;
#endif
        }

        if( hComposite_ )
            XtDestroyWidget( hComposite_ );
        if( hShell_ )
            XtDestroyWidget( hShell_ );

        pXLib_->Remove( ConnectionNumber( pDisp_ ) );

        delete mpInputMethod;
        delete mpKbdExtension;
        XtCloseDisplay( pDisp_ );
    }

    pDisp_  = (Display*)ILLEGAL_POINTER;

    pSalData->Remove( this );

    // free colormap before modifying pVisual_
    xColor_.Clear();

    delete pICCCM_;
    delete pVisual_;

    if( pRootVisual_ != pVisual_ )
        delete pRootVisual_;

    pICCCM_         = (SalICCCM*)ILLEGAL_POINTER;
    pVisual_        = (SalVisual*)ILLEGAL_POINTER;
    pRootVisual_    = (SalVisual*)ILLEGAL_POINTER;

    if( pSalData->GetDefDisp() == this )
        pSalData->SetDefDisp( NULL );
    if( pSalData->GetCurDisp() == this )
        pSalData->SetCurDisp( NULL );
}

static int DisplayHasEvent( int fd, SalDisplay *pDisplay  )
{
  DBG_ASSERT( ConnectionNumber( pDisplay->GetDisplay() ) == fd,
              "wrong fd in DisplayHasEvent" )
  return pDisplay->IsDisplay() && pDisplay->IsEvent();
}
static int DisplayQueue( int fd, SalDisplay *pDisplay )
{
  DBG_ASSERT( ConnectionNumber( pDisplay->GetDisplay() ) == fd,
              "wrong fd in DisplayHasEvent" )
  return XEventsQueued( pDisplay->GetDisplay(),
                        QueuedAfterReading );
}
static int DisplayYield( int fd, SalDisplay *pDisplay )
{
  DBG_ASSERT( ConnectionNumber( pDisplay->GetDisplay() ) == fd,
              "wrong fd in DisplayHasEvent" )
  pDisplay->Yield( TRUE );
  return TRUE;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalDisplay::Init( Colormap hXColmap, const XVisualInfo* pXVI )
{
    GetSalData()->Insert( this );

    for( size_t i = 0; i < POINTER_COUNT; i++ )
        aPointerCache_[i] = None;

    eWindowManager_     = otherwm;
    nProperties_        = PROPERTY_DEFAULT;
    nStateOfYield_      = 0;
    nStateOfSendEvent_  = 0;
    hEventGuard_        = NULL;
    pEventQueue_        = NULL;
    pDispatchStack_     = NULL;
    pFontCache_         = NULL;
    mpFontList          = (XlfdStorage*)NULL;
    mpFactory           = (AttributeProvider*)NULL;
    pCapture_           = NULL;
    pVisual_            = new SalVisual( pXVI );
    aSize_              = Size( DisplayWidth ( pDisp_, nScreen_ ),
                                DisplayHeight( pDisp_, nScreen_ ) );
    aResolution_        =
        Pair( DPI( aSize_.Width(),  DisplayWidthMM ( pDisp_, nScreen_ ) ),
              DPI( aSize_.Height(), DisplayHeightMM( pDisp_, nScreen_ ) ) );

    nMaxRequestSize_    = XExtendedMaxRequestSize( pDisp_ ) * 4;
    if( !nMaxRequestSize_ )
        nMaxRequestSize_ = XMaxRequestSize( pDisp_ ) * 4;

    SetServerVendor();
    SalBitmap::ImplCreateCache();

    if( IsDisplay() )
    {
        hEventGuard_    = osl_createMutex();

        pXLib_->Insert( ConnectionNumber( pDisp_ ),
                        this,
                        (YieldFunc) DisplayHasEvent,
                        (YieldFunc) DisplayQueue,
                        (YieldFunc) DisplayYield );

        pICCCM_         = new SalICCCM( this );
        pScreen_        = ScreenOfDisplay( pDisp_, nScreen_ );
        hRootWindow_    = RootWindowOfScreen( pScreen_ );

        // we are interested in create and destroy notify events
        // for salsystem
        if( hRootWindow_ != None )
            XSelectInput( pDisp_, hRootWindow_, SubstructureNotifyMask );

        bLocal_         = FALSE; /* dont care, initialize later by
                                    calling SalDisplay::IsLocal() */
        mbLocalIsValid  = FALSE; /* bLocal_ is not yet initialized */

        // - - - - - - - - - - Visuals - - - - - - - - - - - - - - -
        Visual *pRootVisual = DefaultVisual( pDisp_, nScreen_ );
        // if( pRootVisual != pXVI->visual )
        if( pRootVisual->visualid != pVisual_->GetVisualId() )
        {
            XVisualInfo aXVI;
            sal_GetVisualInfo( pDisp_, pRootVisual->visualid, aXVI );
            pRootVisual_ = new SalVisual( &aXVI );
        }
        else
            pRootVisual_ = pVisual_;

        // start session management
        SessionManagerClient::open();

        // - - - - - - - - - - Reference Window/Default Drawable - -
        XSetWindowAttributes aXWAttributes;
        aXWAttributes.border_pixel      = 0;
        aXWAttributes.background_pixel  = 0;
        aXWAttributes.colormap          = hXColmap;
        hRefWindow_     = XCreateWindow( pDisp_,
                                         hRootWindow_,
                                         0,0, 16,16, 0,
                                         pVisual_->GetDepth(),
                                         InputOutput,
                                         pVisual_->GetVisual(),
                                         CWBorderPixel|CWBackPixel|CWColormap,
                                         &aXWAttributes );

        // set client leader and session id
        char* pSessionID = SessionManagerClient::getSessionID();
        if( hRefWindow_ )
        {
            if( pSessionID )
            {
                XChangeProperty( pDisp_,
                                 hRefWindow_,
                                 XInternAtom( pDisp_, "SM_CLIENT_ID", False ),
                                 XA_STRING,
                                 8,
                                 PropModeReplace,
                                 (unsigned char*)pSessionID,
                                 strlen( pSessionID )
                                 );
            }
            // client leader must have WM_CLIENT_LEADER pointing to itself

            XChangeProperty( pDisp_,
                             hRefWindow_,
                             XInternAtom( pDisp_, "WM_CLIENT_LEADER", False ),
                             XA_WINDOW,
                             32,
                             PropModeReplace,
                             (unsigned char*)&hRefWindow_,
                             1

                             );

            ByteString aExec( SessionManagerClient::getExecName(), gsl_getSystemTextEncoding() );
            char* argv[2];
            argv[0] = "/bin/sh";
            argv[1] = const_cast<char*>(aExec.GetBuffer());
            XSetCommand( pDisp_, hRefWindow_, argv, 2 );
        }

        // - - - - - - - - - - Synchronize - - - - - - - - - - - - -
        if( getenv( "SAL_SYNCHRONIZE" ) )
            XSynchronize( pDisp_, True );

        // - - - - - - - - - - Window Manager  - - - - - - - - - - -
        const char *pWM = getenv( "SAL_WM" );
        if( pWM )
            sscanf( pWM, "%li", &eWindowManager_ );
        else if( XInternAtom( pDisp_, "_SGI_TELL_WM", True ) )
            eWindowManager_ = _4Dwm;
        else if( XInternAtom( pDisp_, "KWM_RUNNING", True ) )
            eWindowManager_ = mwm; // naja, eigentlich kwm ...
        else if( XInternAtom( pDisp_, "_OL_WIN_ATTR", True ) )
            eWindowManager_ = olwm;

        // - - - - - - - - - - Properties  - - - - - - - - - - - - -
        const char *pProperties = getenv( "SAL_PROPERTIES" );
        if( pProperties )
            sscanf( pProperties, "%li", &nProperties_ );
        else
        {
#if defined DBG_UTIL || defined SUN || defined LINUX || defined FREEBSD
            nProperties_ |= PROPERTY_FEATURE_Maximize;
#endif
            // Server Bugs & Properties
            if( GetServerVendor() == vendor_excursion )
            {
                nProperties_ |= PROPERTY_BUG_Stipple;
                nProperties_ |= PROPERTY_BUG_DrawLine;
                nProperties_ &= ~PROPERTY_SUPPORT_XSetClipMask;
            }
            else
            if( GetServerVendor() == vendor_attachmate )
            {
                nProperties_ |= PROPERTY_BUG_CopyPlane_RevertBWPixel;
            }
            else
            if( GetServerVendor() == vendor_ibm )
            {
                nProperties_ |= PROPERTY_BUG_XA_FAMILY_NAME_nil;

                if( otherwm == eWindowManager_ ) eWindowManager_ = mwm;
            }
            else
            if( GetServerVendor() == vendor_xfree )
            {
                nProperties_ |= PROPERTY_BUG_XCopyArea_GXxor;
#ifdef ARM32 // ??? Server! nicht Client ???
                nProperties_ &= ~PROPERTY_SUPPORT_XSetClipMask;
#endif
#if defined LINUX || defined FREEBSD
                // otherwm and olwm are a kind of default, which are not detected
                // carefully. if we are running linux (i.e. not netbsd) on an xfree
                // display, fvwm is most probable the wm to choose, confusing with mwm
                // doesn't harm. #57791# start maximized if possible
                if(    (otherwm == eWindowManager_)
                    || (olwm    == eWindowManager_ ))
                {
                    eWindowManager_ = fvwm; // ???
                    nProperties_ |= PROPERTY_FEATURE_Maximize;
                }
#else
                if( otherwm == eWindowManager_ ) eWindowManager_ = winmgr;
#endif
#if defined SOLARIS && defined SPARC
                nProperties_ |= PROPERTY_BUG_Bitmap_Bit_Order;
                // solaris xlib seems to have problems with putting images
                // in correct bit order to xfree 8 bit displays
#endif
            }
            else
            if( GetServerVendor() == vendor_sun )
            {
                // nicht alle! (bekannt: nur Sparc II CG3, CG6?)
                nProperties_ &= ~PROPERTY_SUPPORT_XSetClipMask;

                // Fehler im Sun-Solaris X86 Server !
                if (ImageByteOrder(GetDisplay()) == LSBFirst)
                {
                    nProperties_ |= PROPERTY_BUG_Tile;
                    nProperties_ |= PROPERTY_SUPPORT_3ButtonMouse;
                }
                else // MSBFirst Sun-Solaris Sparc Server
                {
                    // XCopyPlane reverts black and white for 1bit bitmaps
                    // only sun, only 8bit pseudocolor target
                    if (   (pVisual_->GetDepth() == 8)
                        && (pVisual_->GetClass() == PseudoColor))
                        nProperties_ |= PROPERTY_BUG_CopyPlane_RevertBWPixel;
                    // Fehler in Solaris 2.5.1
                    if (VendorRelease ( GetDisplay() ) < 3600)
                        nProperties_ |= PROPERTY_BUG_FillPolygon_Tile;
                }

                if( otherwm == eWindowManager_ )
                    if( XInternAtom( pDisp_, "_MOTIF_WM_INFO", True ) )
                        eWindowManager_ = dtwm;
                    else
                        eWindowManager_ = olwm;
            }
            else
            if( GetServerVendor() == vendor_sco )
            {
                if( otherwm == eWindowManager_ ) eWindowManager_ = pmwm;
            }
            else
            if( GetServerVendor() == vendor_sgi )
            {
                if( pVisual_->GetDepth() > 8 && pVisual_->GetDepth() <= 16 )
                    nProperties_ |= PROPERTY_BUG_XCopyArea_GXxor;
                nProperties_ |= PROPERTY_SUPPORT_XSetClipMask;

                if( otherwm == eWindowManager_ ) eWindowManager_ = _4Dwm;
            }
            else
            if( GetServerVendor() == vendor_hp )
            {
                if( otherwm == eWindowManager_ ) eWindowManager_ = dtwm;
            }
            else
            if( GetServerVendor() == vendor_hummingbird )
            {
                if (pVisual_->GetDepth() == 24)
                    nProperties_ |= PROPERTY_BUG_CopyArea_OnlySmallSlices;
            }

            if( otherwm == eWindowManager_ )
            {
                if( !XInternAtom( pDisp_, "_MOTIF_WM_INFO", True ) )
                    eWindowManager_ = olwm;
                // ???
            }

            if( winmgr == eWindowManager_ )
            {
                nProperties_ &= ~PROPERTY_SUPPORT_WM_SetPos;
                nProperties_ &= ~PROPERTY_SUPPORT_WM_Screen;
                nProperties_ |= PROPERTY_FEATURE_Maximize;
            }
            else if( dtwm == eWindowManager_ )
            {
                nProperties_ &= ~PROPERTY_SUPPORT_WM_ClientPos;
            }
            else if( pmwm == eWindowManager_ )
            {
                nProperties_ &= ~PROPERTY_SUPPORT_WM_ClientPos;
            }
        }

        // - - - - - - - - - - Shared Images - - - - - - - - - - - -
#if defined _XSHM_H_ // && defined DBG_UTIL
        // SharedMem wird nur noch ueber SalProperties enabled
        // wegen Bugs #47289 und #46512

        if(    ( nProperties_ & PROPERTY_FEATURE_SharedMemory )
            && (    *DisplayString( pDisp_ ) == ':'
                || !strncmp( DisplayString( pDisp_ ), "localhost:", 10 ))
          )
        {
            int  nMinor, nMajor;
            Bool bPixmaps;

            nSharedImages_      = XShmQueryVersion( pDisp_,
                                                    &nMajor,
                                                    &nMinor,
                                                    &bPixmaps )
                ? 0x80000000 : 0;
        }
        else
            nSharedImages_      = 0;
#else
        nSharedImages_      = 0;
#endif
        // - - - - - - - - - - Images  - - - - - - - - - - - - - - -
        nImageDepths_       = 0x00000000;
        int nCount, *pDepths = XListDepths( pDisp_, nScreen_, &nCount );
        if( pDepths )
        {
            for( int i = 0; i < nCount; i++ )
                nImageDepths_ |= 1 << (pDepths[i]-1);
            XFree( pDepths );
        }
    }
    else
    {
        pICCCM_             = NULL;
        pScreen_            = NULL;
        hRootWindow_        = None;
        pRootVisual_        = pVisual_;
#ifdef DBG_UTIL
        hRefWindow_         = (XLIB_Window)ILLEGAL_POINTER;
#endif
#if defined(_USE_PRINT_EXTENSION_)

      pScreen_            = ScreenOfDisplay( pDisp_, nScreen_ );
      hRootWindow_        = RootWindowOfScreen( pScreen_ );
      pRootVisual_        = pVisual_;

      XSetWindowAttributes aXWAttributes;
      aXWAttributes.border_pixel              = 0;
      aXWAttributes.background_pixel          = 0;
      aXWAttributes.colormap                  = hXColmap;
      hRefWindow_                             = XCreateWindow( pDisp_,
                                                               hRootWindow_,
                                                               0, 0, 16, 16, 0,
                                                               pVisual_->GetDepth(),
                                                               InputOutput,
                                                               pVisual_->GetVisual(),
                                                               CWBorderPixel|CWBackPixel|CWColormap,
                                                               &aXWAttributes );

#endif

        hInvert50_          = None;
        bLocal_             = TRUE; /* always true for xprinter */
        mbLocalIsValid      = TRUE; /* yes bLocal_ is initialized */
        nProperties_       &= ~PROPERTY_SUPPORT_XSetClipMask; //XPrinter doesnt
        nSharedImages_      = 0;
        nImageDepths_       = 0xFFFFFFFF;
    }

    // - - - - - - - - - - Images  - - - - - - - - - - - - - - -
    // 0x8080C889
    nImageDepths_ &= (1<<(32-1))
                     |(1<<(24-1))
                     |(1<<(16-1))
                     |(1<<(15-1))
                     |(1<<(12-1))
                     |(1<<(8-1))
                     |(1<<(4-1))
                     |(1<<(1-1));

    xColor_             = new SalColormap( this, hXColmap );

    // - - - - - - - - - - GCs - - - - - - - - - - - - - - - - -
    XGCValues values;
    values.graphics_exposures   = True;
    values.fill_style           = FillOpaqueStippled;
    values.background           = (1<<pVisual_->GetDepth())-1;
    values.foreground           = 0;

    pCopyGC_            = XCreateGC( pDisp_,
                                     hRefWindow_,
                                     GCGraphicsExposures
                                     | GCForeground
                                     | GCBackground,
                                     &values );
    pAndInvertedGC_     = XCreateGC( pDisp_,
                                     hRefWindow_,
                                     GCGraphicsExposures
                                     | GCForeground
                                     | GCBackground,
                                     &values );
    pAndGC_             = XCreateGC( pDisp_,
                                     hRefWindow_,
                                     GCGraphicsExposures
                                     | GCForeground
                                     | GCBackground,
                                     &values );
    pOrGC_              = XCreateGC( pDisp_,
                                     hRefWindow_,
                                     GCGraphicsExposures
                                     | GCForeground
                                     | GCBackground,
                                     &values    );
    pStippleGC_         = XCreateGC( pDisp_,
                                     hRefWindow_,
                                     GCGraphicsExposures
                                     | GCFillStyle
                                     | GCForeground
                                     | GCBackground,
                                     &values );

    XSetFunction( pDisp_, pAndInvertedGC_,  GXandInverted );
    XSetFunction( pDisp_, pAndGC_,          GXand );
    // #44556# PowerPC Solaris 2.5 (XSun 3500) Bug: GXor = GXnop
    //XSetFunction( pDisp_, pOrGC_,         GXor );
    XSetFunction( pDisp_, pOrGC_,           GXxor );

    if( 1 == pVisual_->GetDepth() ) // irgendwer dreht immer
    {
        XSetFunction( pDisp_, pCopyGC_, GXcopyInverted );
        pMonoGC_    = pCopyGC_;
    }
    else
    {
        Pixmap hPixmap = XCreatePixmap( pDisp_, hRefWindow_, 1, 1, 1 );
        pMonoGC_    = XCreateGC( pDisp_,
                                 hPixmap,
                                 GCGraphicsExposures,
                                 &values );
        XFreePixmap( pDisp_, hPixmap );
    }

    if( IsDisplay() )
    {
        hInvert50_ = XCreateBitmapFromData( pDisp_,
                                            hRefWindow_,
                                            invert50_bits,
                                            invert50_width,
                                            invert50_height );

        // - - - - - - - - - - Sound - - - - - - - - - - - - - - - -

        nBeepVolume_    = 0;

        // - - - - - - - - - - Fonts - - - - - - - - - - - - - - - -

#ifndef USE_BUILTIN_RASTERIZER
        const char *pFontPath = getenv( "SAL_FONTPATH" );
        if( pFontPath )
            AddFontPath( ByteString( pFontPath ) );

        pFontPath = getenv( "SAL_FONTPATH_PRIVATE" );
        if( pFontPath )
            AddFontPath( ByteString( pFontPath ) );
#endif // USE_BUILTIN_RASTERIZER

        // - - - - - - - - - - Keyboardmapping - - - - - - - - - - -

        ModifierMapping();

        // - - - - - - - - - - ShellWidget - - - - - - - - - - - - -


        if( !XtWindow( hShell_ ) )
        {
            Position w, h;
            Arg aArgs[10];

            XtSetArg( aArgs[0], XtNwidth,   &w );
            XtSetArg( aArgs[1], XtNheight,  &h  );
            XtGetValues( hShell_, aArgs, 2 );

            if( !w || !h )
            {
                if( GetProperties() & PROPERTY_FEATURE_Maximize )
                {
                    XtSetArg( aArgs[0], XtNwidth,   aSize_.Width() );
                    XtSetArg( aArgs[1], XtNheight,  aSize_.Height() );
                }
                else
                {
                    XtSetArg( aArgs[0], XtNwidth,   500 );
                    XtSetArg( aArgs[1], XtNheight,  400 );
                }
                XtSetValues( hShell_, aArgs, 2 );
            }

            // X-Window erzeugen
            XtSetMappedWhenManaged( hShell_, FALSE );
            XtRealizeWidget( hShell_ );
        }
        if( !XtWindow( hComposite_ ) )
            XtRealizeWidget( hComposite_ );
    }
#ifdef DBG_UTIL
    PrintInfo();
#endif

#ifdef DEBUG
    fprintf( stderr, "Keyboard name: %s\n", GetKeyboardName() );
    String aConvert = GetKeyNameFromKeySym( XK_Control_L );
    fprintf( stderr, "Control: %s\n", aConvert.Len() ? ByteString( aConvert, gsl_getSystemTextEncoding() ).GetBuffer() : "<nil>" );
    aConvert = GetKeyNameFromKeySym( XK_Shift_L );
    fprintf( stderr, "Shift: %s\n", aConvert.Len() ? ByteString( aConvert, gsl_getSystemTextEncoding() ).GetBuffer() : "<nil>" );
    aConvert = GetKeyNameFromKeySym( XK_Alt_L );
    fprintf( stderr, "Alt: %s\n", aConvert.Len() ? ByteString( aConvert, gsl_getSystemTextEncoding() ).GetBuffer() : "<nil>" );
    aConvert = GetKeyNameFromKeySym( XK_Alt_R );
    fprintf( stderr, "AltGr: %s\n", aConvert.Len() ? ByteString( aConvert, gsl_getSystemTextEncoding() ).GetBuffer() : "<nil>" );
#endif
}

/*----------------------------
 keep track of windows that are sized in ::Init but repositioned
 by the window-mgr. If they are not resized by SalFrame::Set[Pos]Size
 then SalFrameData::HandleReparentEvent takes a look at it, to prevent it
 from beeing moved outside the screen
 ------------------------------*/

static unsigned int nRefWindow = 0;

void
MarkWindowAsBadPositioned( unsigned int nWindow )
{
    nRefWindow = nWindow;
}

void
MarkWindowAsGoodPositioned( unsigned int nWindow )
{
    if ( nRefWindow == nWindow )
        nRefWindow = 0;
}

Boolean
WindowNeedGoodPosition( unsigned int nWindow )
{
    return (Boolean) nWindow == nRefWindow;
}

// Sound
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalDisplay::Beep() const
{
    XBell( pDisp_, nBeepVolume_ );
}

// Keyboard
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
String SalDisplay::GetKeyNameFromKeySym( KeySym nKeySym ) const
{
    String aRet;
    if( !nKeySym )
        aRet = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "???" ) );
    else
    {
        aRet = ::vcl_sal::getKeysymReplacementName( const_cast<SalDisplay*>(this)->GetKeyboardName(), nKeySym );
        if( ! aRet.Len() )
        {
            const char *pString = XKeysymToString( nKeySym );
            int n = strlen( pString );
            if( n > 2 && pString[n-2] == '_' )
                aRet = String( pString, n-2, RTL_TEXTENCODING_ISO_8859_1 );
            else
                aRet = String( pString, n, RTL_TEXTENCODING_ISO_8859_1 );
        }
    }
    return aRet;
}

inline KeySym sal_XModifier2Keysym( Display         *pDisplay,
                                    XModifierKeymap *pXModMap,
                                    int              n )
{
    return XKeycodeToKeysym( pDisplay,
                             pXModMap->modifiermap[n*pXModMap->max_keypermod],
                             0 );
}

void SalDisplay::ModifierMapping()
{
    XModifierKeymap *pXModMap = XGetModifierMapping( pDisp_ );

    bNumLockFromXS_ = True;
    nShiftKeySym_   = sal_XModifier2Keysym( pDisp_, pXModMap, ShiftMapIndex );
    nCtrlKeySym_    = sal_XModifier2Keysym( pDisp_, pXModMap, ControlMapIndex );
    nMod1KeySym_    = sal_XModifier2Keysym( pDisp_, pXModMap, Mod1MapIndex );

    // Auf Sun-Servern und SCO-Severn beruecksichtigt XLookupString
    // nicht den NumLock Modifier.
    if(     (GetServerVendor() == vendor_sun)
        ||  (GetServerVendor() == vendor_sco) )
    {
        XLIB_KeyCode aNumLock = XKeysymToKeycode( pDisp_, XK_Num_Lock );

        if( aNumLock ) for( int i = ShiftMapIndex; i <= Mod5MapIndex; i++ )
        {
            if( pXModMap->modifiermap[i*pXModMap->max_keypermod] == aNumLock )
            {
                bNumLockFromXS_ = False;
                nNumLockIndex_  = i;
                nNumLockMask_   = 1<<i;
                break;
            }
        }
    }

    XFreeModifiermap( pXModMap );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
XubString SalDisplay::GetKeyName( USHORT nKeyCode ) const
{
    String aStrMap;

    if( nKeyCode & KEY_MOD2 )
    {
        aStrMap += GetKeyNameFromKeySym( nMod1KeySym_ );
    }
    if( nKeyCode & KEY_MOD1 )
    {
        if( aStrMap.Len() )
            aStrMap += '+';
        aStrMap += GetKeyNameFromKeySym( nCtrlKeySym_ );
    }
    if( nKeyCode & KEY_SHIFT )
    {
        if( aStrMap.Len() )
            aStrMap += '+';
        aStrMap += GetKeyNameFromKeySym( nShiftKeySym_ );
    }
    nKeyCode &= 0x0FFF;

    KeySym nKeySym = 0;

    if( KEY_0 <= nKeyCode && nKeyCode <= KEY_9 )
        nKeySym = XK_0 + (nKeyCode - KEY_0);
    else if( KEY_A <= nKeyCode && nKeyCode <= KEY_Z )
        nKeySym = XK_A + (nKeyCode - KEY_A);
    else if( KEY_F1 <= nKeyCode && nKeyCode <= KEY_F26 ) // Existiert die Taste
        nKeySym = XK_F1 + (nKeyCode - KEY_F1);
    else switch( nKeyCode )
    {
        case KEY_DOWN:
            nKeySym = XK_Down;
            break;
        case KEY_UP:
            nKeySym = XK_Up;
            break;
        case KEY_LEFT:
            nKeySym = XK_Left;
            break;
        case KEY_RIGHT:
            nKeySym = XK_Right;
            break;
        case KEY_HOME:
            nKeySym = XK_Home;
            break;
        case KEY_END:
            nKeySym = XK_End;
            break;
        case KEY_PAGEUP:
            nKeySym = XK_Prior;
            break;
        case KEY_PAGEDOWN:
            nKeySym = XK_Next;
            break;
        case KEY_RETURN:
            nKeySym = XK_Return;
            break;
        case KEY_ESCAPE:
            nKeySym = XK_Escape;
            break;
        case KEY_TAB:
            nKeySym = XK_Tab;
            break;
        case KEY_BACKSPACE:
            nKeySym = XK_BackSpace;
            break;
        case KEY_SPACE:
            nKeySym = XK_space;
            break;
        case KEY_INSERT:
            nKeySym = XK_Insert;
            break;
        case KEY_DELETE:
            nKeySym = XK_Delete;
            break;
        case KEY_OPEN:
            nKeySym = XK_L7;
            break;
        case KEY_CUT:
            nKeySym = XK_L10;
            break;
        case KEY_COPY:
            nKeySym = XK_L6;
            break;
        case KEY_PASTE:
            nKeySym = XK_L8;
            break;
        case KEY_UNDO:
            nKeySym = XK_L4; // XK_Undo; ???
            break;
        case KEY_REPEAT:
//            nKeySym = XK_L2; // XK_Redo; ???
            nKeySym = XK_Redo;
            break;
        case KEY_FIND:
            nKeySym = XK_L9; // XK_Find; ???
            break;
        case KEY_PROPERTIES:
            nKeySym = XK_L3;
            break;
        case KEY_FRONT:
            nKeySym = XK_L5;
            break;

        case KEY_ADD:
            nKeySym = XK_plus;
            break;
        case KEY_SUBTRACT:
            nKeySym = XK_minus;
            break;
        case KEY_MULTIPLY:
            nKeySym = XK_asterisk;
            break;
        case KEY_DIVIDE:
            nKeySym = XK_slash;
            break;
        case KEY_POINT:
            nKeySym = XK_period;
            break;
        case KEY_COMMA:
            nKeySym = XK_comma;
            break;
        case KEY_LESS:
            nKeySym = XK_less;
            break;
        case KEY_GREATER:
            nKeySym = XK_greater;
            break;
        case KEY_EQUAL:
            nKeySym = XK_equal;
            break;
        case KEY_FRONT+1: // KEY_HELP
            nKeySym = XK_F1; // XK_Help; ???
            break;

        default:
            nKeySym = 0;
            break;
    }

    if( nKeySym )
    {
        if( aStrMap.Len() )
            aStrMap += '+';
        aStrMap += GetKeyNameFromKeySym( nKeySym );
    }

    return aStrMap;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef IsISOKey
#define IsISOKey( n ) (0x0000FE00==((n)&0xFFFFFF00))
#endif

USHORT SalDisplay::GetKeyCode( KeySym keysym, char*pcPrintable ) const
{
    USHORT nKey = 0;

    if( XK_a <= keysym && XK_z >= keysym )
        nKey = (USHORT)(KEY_A + (keysym - XK_a));
    else if( XK_A <= keysym && XK_Z >= keysym )
        nKey = (USHORT)(KEY_A + (keysym - XK_A));
    else if( XK_0 <= keysym && XK_9 >= keysym )
        nKey = (USHORT)(KEY_0 + (keysym - XK_0));
    else if( IsModifierKey( keysym ) )
        ;
    else if( IsKeypadKey( keysym ) )
    {
        if( (keysym >= XK_KP_0) && (keysym <= XK_KP_9) )
        {
            nKey = (USHORT)(KEY_0 + (keysym - XK_KP_0));
            *pcPrintable = '0' + nKey - KEY_0;
        }
        else if( IsPFKey( keysym ) )
            nKey = (USHORT)(KEY_F1 + (keysym - XK_KP_F1));
        else switch( keysym )
        {
            case XK_KP_Space:
                nKey = KEY_SPACE;
                *pcPrintable = ' ';
                break;
            case XK_KP_Tab:
                nKey = KEY_TAB;
                break;
            case XK_KP_Enter:
                nKey = KEY_RETURN;
                break;
            case XK_KP_Begin:
            case XK_KP_Home:
                nKey = KEY_HOME;
                break;
            case XK_KP_Left:
                nKey = KEY_LEFT;
                break;
            case XK_KP_Up:
                nKey = KEY_UP;
                break;
            case XK_KP_Right:
                nKey = KEY_RIGHT;
                break;
            case XK_KP_Down:
                nKey = KEY_DOWN;
                break;
            case XK_KP_Prior: // XK_KP_Page_Up
                nKey = KEY_PAGEUP;
                break;
            case XK_KP_Next: // XK_KP_Page_Down
                nKey = KEY_PAGEDOWN;
                break;
            case XK_KP_End:
                nKey = KEY_END;
                break;
            case XK_KP_Insert:
                nKey = KEY_INSERT;
                break;
            case XK_KP_Delete:
                nKey = KEY_DELETE;
                break;
            case XK_KP_Equal:
                nKey = KEY_EQUAL;
                *pcPrintable = '=';
                break;
            case XK_KP_Multiply:
                nKey = KEY_MULTIPLY;
                *pcPrintable = '*';
                break;
            case XK_KP_Add:
                nKey = KEY_ADD;
                *pcPrintable = '+';
                break;
            case XK_KP_Separator:
                nKey = KEY_COMMA; // ???
                *pcPrintable = ',';
                break;
            case XK_KP_Subtract:
                nKey = KEY_SUBTRACT;
                *pcPrintable = '-';
                break;
            case XK_KP_Decimal:
                nKey = KEY_POINT;
                *pcPrintable = '.';
                break;
            case XK_KP_Divide:
                nKey = KEY_DIVIDE;
                *pcPrintable = '/';
                break;
        }
    }
    else if( IsFunctionKey( keysym ) )
    {
        if( bNumLockFromXS_ )
        {
            if( keysym >= XK_F1 && keysym <= XK_F26 )
                nKey = (USHORT)(KEY_F1 + keysym - XK_F1);
        }
        else switch( keysym )
        {
#if 1 // Sun schaltet mit "Alt Graph"/XK_Mode_switch um
            // - - - - - Sun X-Server Tastatur ohne Cursorblock ??? - - -
#if 0 // Sal supports F1 - F26
            case XK_R1: // XK_F21:
                nKey = KEY_F21; // KEY_PRINT/KEY_SYSREQ
                break;
            case XK_R2: // XK_F22:
                nKey = KEY_F22; // KEY_SCROLLLOCK
                break;
            case XK_R3: // XK_F23:
                nKey = KEY_F23; // KEY_PAUSE/KEY_BREAK
                break;
            case XK_R4: // XK_F24:
                nKey = KEY_F24; // KEY_EQUAL
                break;
            case XK_R5: // XK_F25:
                nKey = KEY_F25; // KEY_DIVIDE
                break;
            case XK_R6: // XK_F26:
                nKey = KEY_F26; // KEY_MULTIPLY
                break;
#endif
            case XK_R7: // XK_F27:
                nKey = KEY_HOME;
                break;
            case XK_R8: // XK_F28:
                nKey = KEY_UP;
                break;
            case XK_R9: // XK_F29:
                nKey = KEY_PAGEUP;
                break;
            case XK_R10: // XK_F30:
                nKey = KEY_LEFT;
                break;
            case XK_R11: // XK_F31:
                nKey = 0; // KEY_F31
                break;
            case XK_R12: // XK_F32:
                nKey = KEY_RIGHT;
                break;
            case XK_R13: // XK_F33:
                nKey = KEY_END;
                break;
            case XK_R14: // XK_F34:
                nKey = KEY_DOWN;
                break;
            case XK_R15: // XK_F35:
                nKey = KEY_PAGEDOWN;
                break;
            // - - - - - Sun X-Server Tastatur ??? - - - - - - - - - - - -
            case XK_L1: // XK_F11:
                nKey = KEY_F11; // KEY_CANCEL
                break;
            case XK_L2: // XK_F12:
                if ( GetServerVendor() == vendor_sun )
                    nKey = KEY_REPEAT;
                else
                    nKey = KEY_F12;
                break;
            case XK_L3: // XK_F13:
                nKey = KEY_PROPERTIES; // KEY_F13
                break;
            case XK_L4: // XK_F14:
                nKey = KEY_UNDO; // KEY_F14
                break;
            case XK_L5: // XK_F15:
                nKey = KEY_F15; // KEY_FRONT
                break;
            case XK_L6: // XK_F16:
                nKey = KEY_COPY; // KEY_F16
                break;
            case XK_L7: // XK_F17:
                nKey = KEY_F17; // KEY_OPEN
                break;
            case XK_L8: // XK_F18:
                nKey = KEY_PASTE; // KEY_F18
                break;
            case XK_L9: // XK_F19:
                nKey = KEY_F19; // KEY_FIND
                break;
            case XK_L10: // XK_F20:
                nKey = KEY_CUT; // KEY_F20
                break;
#endif
            default:
                if( keysym >= XK_F1 && keysym <= XK_F26 )
                    nKey = (USHORT)(KEY_F1 + keysym - XK_F1);
                break;
        }
    }
    else if( IsCursorKey( keysym ) )
    {
        switch( keysym )
        {
            case XK_Begin:
            case XK_Home:
                nKey = KEY_HOME;
                break;
            case XK_Left:
                nKey = KEY_LEFT;
                break;
            case XK_Up:
                nKey = KEY_UP;
                break;
            case XK_Right:
                nKey = KEY_RIGHT;
                break;
            case XK_Down:
                nKey = KEY_DOWN;
                break;
            case XK_Prior: // XK_Page_Up
                nKey = KEY_PAGEUP;
                break;
            case XK_Next: // XK_Page_Down
                nKey = KEY_PAGEDOWN;
                break;
            case XK_End:
                nKey = KEY_END;
                break;
        }
    }
    else if( IsMiscFunctionKey( keysym ) )
    {
        switch( keysym )
        {
            case XK_Insert:
                nKey = KEY_INSERT;
                break;
            case XK_Redo:
                nKey = KEY_REPEAT;
                break;
            case XK_Undo:
                nKey = KEY_UNDO;
                break;
            case XK_Find:
                nKey = KEY_FIND;
                break;
            case XK_Help:
                nKey = KEY_FRONT+1; // KEY_HELP
                nKey = KEY_F1;
                break;
            case XK_Menu:
                nKey = KEY_F10;
                break;
#if 0
            case XK_Break:
            case XK_Select:
            case XK_Execute:
            case XK_Print:
            case XK_Cancel:
#endif
        }
    }
    else if( IsISOKey( keysym ) )  // XK_ISO_
    {
        switch( keysym )
        {
            case 0xFE20: // XK_ISO_Left_Tab:
                nKey = KEY_TAB;
                break;
        }
    }
    else switch( keysym )
    {
        case XK_Return:
            nKey = KEY_RETURN;
            break;
        case XK_BackSpace:
            nKey = KEY_BACKSPACE;
            break;
        case XK_Delete:
            nKey = KEY_DELETE;
            break;
        case XK_space:
            nKey = KEY_SPACE;
            break;
        case XK_Tab:
            nKey = KEY_TAB;
            break;
        case XK_Escape:
            nKey = KEY_ESCAPE;
            break;
        case XK_plus:
            nKey = KEY_ADD;
            break;
        case XK_minus:
            nKey = KEY_SUBTRACT;
            break;
        case XK_asterisk:
            nKey = KEY_MULTIPLY;
            break;
        case XK_slash:
            nKey = KEY_DIVIDE;
            break;
        case XK_period:
            nKey = KEY_POINT;
            break;
        case XK_comma:
            nKey = KEY_COMMA;
            break;
        case XK_less:
            nKey = KEY_LESS;
            break;
        case XK_greater:
            nKey = KEY_GREATER;
            break;
        case XK_equal:
            nKey = KEY_EQUAL;
            break;
//      case XK_Linefeed:
//          *pcPrintable = '\n';
//          break;
        // - - - - - - - - - - - - -  Apollo - - - - - - - - - - - - - 0x1000
        case 0x1000FF02: // apXK_Copy
            nKey = KEY_COPY;
            break;
        case 0x1000FF03: // apXK_Cut
            nKey = KEY_CUT;
            break;
        case 0x1000FF04: // apXK_Paste
            nKey = KEY_PASTE;
            break;
        case 0x1000FF14: // apXK_Repeat
            nKey = KEY_REPEAT;
            break;
        // Exit, Save
        // - - - - - - - - - - - - - - D E C - - - - - - - - - - - - - 0x1000
        case 0x1000FF00:
            nKey = KEY_DELETE;
            break;
        // - - - - - - - - - - - - - -  H P  - - - - - - - - - - - - - 0x1000
        case 0x1000FF73: // hpXK_DeleteChar
            nKey = KEY_DELETE;
            break;
        case 0x1000FF74: // hpXK_BackTab
        case 0x1000FF75: // hpXK_KP_BackTab
            nKey = KEY_TAB;
            break;
        // - - - - - - - - - - - - - - I B M - - - - - - - - - - - - -
        // - - - - - - - - - - - - - - O S F - - - - - - - - - - - - - 0x1004
        case 0x1004FF02: // osfXK_Copy
            nKey = KEY_COPY;
            break;
        case 0x1004FF03: // osfXK_Cut
            nKey = KEY_CUT;
            break;
        case 0x1004FF04: // osfXK_Paste
            nKey = KEY_PASTE;
            break;
        case 0x1004FF07: // osfXK_BackTab
            nKey = KEY_TAB;
            break;
        case 0x1004FF08: // osfXK_BackSpace
            nKey = KEY_BACKSPACE;
            break;
        case 0x1004FF1B: // osfXK_Escape
            nKey = KEY_ESCAPE;
            break;
        // Up, Down, Left, Right, PageUp, PageDown
        // - - - - - - - - - - - - - - S C O - - - - - - - - - - - - -
        // - - - - - - - - - - - - - - S G I - - - - - - - - - - - - - 0x1007
        // - - - - - - - - - - - - - - S N I - - - - - - - - - - - - -
        // - - - - - - - - - - - - - - S U N - - - - - - - - - - - - - 0x1005
        case 0x1005FF10: // SunXK_F36
            nKey = KEY_F11;
            break;
        case 0x1005FF11: // SunXK_F37
            nKey = KEY_F12;
            break;
        case 0x1005FF70: // SunXK_Props
            nKey = KEY_PROPERTIES;
            break;
        case 0x1005FF71: // SunXK_Front
            nKey = KEY_FRONT;
            break;
        case 0x1005FF72: // SunXK_Copy
            nKey = KEY_COPY;
            break;
        case 0x1005FF73: // SunXK_Open
            nKey = KEY_OPEN;
            break;
        case 0x1005FF74: // SunXK_Paste
            nKey = KEY_PASTE;
            break;
        case 0x1005FF75: // SunXK_Cut
            nKey = KEY_CUT;
            break;
    }
    return nKey;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
KeySym SalDisplay::GetKeySym( XKeyEvent        *pEvent,
                                    unsigned char    *pPrintable,
                                    int              *pLen,
                                    Status           *pStatusReturn,
                                    XIC              aInputContext ) const
{
    KeySym nKeySym;
    memset( pPrintable, 0, *pLen );

    if ( (aInputContext == 0) || (pEvent->type == KeyRelease) )
    {
        XComposeStatus nStatus;
        *pLen = XLookupString( pEvent, (char*)pPrintable, 1,
                &nKeySym, &nStatus );
    }
    else
    {
        *pStatusReturn = 0;
        // not really sufficient for multibyte lookup: cannot handle more
        // than one byte in sprintable, cannot handle conversion error
        *pLen = XmbLookupString( aInputContext,
                      pEvent, (char*)pPrintable, *pLen - 1, &nKeySym, pStatusReturn );

        // Lookup the string again, now with appropriate size
        if ( *pStatusReturn == XBufferOverflow )
        {
            pPrintable[ 0 ] = (char)0;
            return 0;
        }

        switch ( *pStatusReturn )
        {
            case XBufferOverflow:
                /* unhandled error */
                break;
            case XLookupNone:
                /* unhandled error */
                break;
            case XLookupKeySym:
                /* #72223# this is a strange one: on exceed sometimes
                 * no printable is returned for the first char entered,
                 * just to retry lookup solves the problem. The problem
                 * is not yet fully understood, so restrict 2nd lookup
                 * to 7bit ascii chars */
                if ( (XK_space <= nKeySym) && (XK_asciitilde >= nKeySym) )
                {
                    *pLen = 1;
                    pPrintable[ 0 ] = (char)nKeySym;
                }
                break;
            case XLookupBoth:
            case XLookupChars:

                /* nothing to, char allready in pPrintable */
                break;
        }
    }

    if( !bNumLockFromXS_
        && (IsCursorKey(nKeySym)
            || IsFunctionKey(nKeySym)
            || IsKeypadKey(nKeySym)
            || XK_Delete == nKeySym ) )
    {
    // Bei einigen X-Servern muss man bei den Keypadtasten
    // schon sehr genau hinschauen. ZB. Solaris XServer:
    // 2, 4, 6, 8 werden als Cursorkeys klassifiziert (Up, Down, Left, Right
    // 1, 3, 5, 9 werden als Functionkeys klassifiziert (F27,F29,F33,F35)
    // 0 als Keypadkey und der Dezimalpunkt gar nicht (KP_Insert)
        KeySym nNewKeySym = XLookupKeysym( pEvent, nNumLockIndex_ );
//      fprintf( stderr, "Key%s: sym=%x state=%x code=%d new=%x\n",
//               KeyRelease == pEvent->type ? "Release" : "Press",
//               nKeySym, pEvent->state, pEvent->keycode, nNewKeySym );
        if( nNewKeySym != NoSymbol )
            nKeySym = nNewKeySym;
    }
//  else
//      fprintf( stderr, "Key%s: sym=%x state=%x code=%d\n",
//               KeyRelease == pEvent->type ? "Release" : "Press",
//               nKeySym, pEvent->state, pEvent->keycode );

    return nKeySym;
}

// Pointer
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define MAKE_BITMAP( name ) \
    XCreateBitmapFromData( pDisp_, \
                           DefaultRootWindow( pDisp_ ), \
                           name##_bits, \
                           name##_width, \
                           name##_height )

#define MAKE_CURSOR( name ) \
    aCursBitmap = MAKE_BITMAP( name##curs ); \
    aMaskBitmap = MAKE_BITMAP( name##mask ); \
    nXHot = name##curs_x_hot; \
    nYHot = name##curs_y_hot

XLIB_Cursor SalDisplay::GetPointer( int ePointerStyle )
{
    if( ePointerStyle > POINTER_COUNT )
        return NULL;

    XLIB_Cursor &aCur = aPointerCache_[ePointerStyle];

    if( aCur != None )
        return aCur;

    Pixmap          aCursBitmap, aMaskBitmap;
    unsigned int    nXHot, nYHot;

    switch( ePointerStyle )
    {
        case POINTER_NULL:
            MAKE_CURSOR( null );
            break;
        case POINTER_ARROW:
            aCur = XCreateFontCursor( pDisp_, XC_top_left_arrow );
            break;
        case POINTER_WAIT:
            MAKE_CURSOR( wait_ );
            break;
        case POINTER_TEXT:          // Mouse Pointer ist ein "I" Beam
            aCur = XCreateFontCursor( pDisp_, XC_xterm );
            break;
        case POINTER_HELP:
            aCur = XCreateFontCursor( pDisp_, XC_question_arrow );
            break;
        case POINTER_CROSS:         // Mouse Pointer ist ein Kreuz
            aCur = XCreateFontCursor( pDisp_, XC_crosshair );
            break;
        case POINTER_NSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_sb_v_double_arrow );
            break;
        case POINTER_SSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_sb_v_double_arrow );
            break;
        case POINTER_WSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_sb_h_double_arrow );
            break;
        case POINTER_ESIZE:
            aCur = XCreateFontCursor( pDisp_, XC_sb_h_double_arrow );
            break;
        case POINTER_WINDOW_NSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_top_tee );
            break;
        case POINTER_WINDOW_SSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_bottom_tee );
            break;
        case POINTER_WINDOW_WSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_left_tee );
            break;
        case POINTER_WINDOW_ESIZE:
            aCur = XCreateFontCursor( pDisp_, XC_right_tee );
            break;
        case POINTER_NWSIZE:
            MAKE_CURSOR( nwsesize_ );
            break;
        case POINTER_NESIZE:
            MAKE_CURSOR( neswsize_ );
            break;
        case POINTER_SWSIZE:
            MAKE_CURSOR( neswsize_ );
            break;
        case POINTER_SESIZE:
            MAKE_CURSOR( nwsesize_ );
            break;
        case POINTER_WINDOW_NWSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_top_left_corner );
            break;
        case POINTER_WINDOW_NESIZE:
            aCur = XCreateFontCursor( pDisp_, XC_top_right_corner );
            break;
        case POINTER_WINDOW_SWSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_bottom_left_corner );
            break;
        case POINTER_WINDOW_SESIZE:
            aCur = XCreateFontCursor( pDisp_, XC_bottom_right_corner );
            break;
        case POINTER_HSPLIT:
            MAKE_CURSOR( hsplit_ );
            break;
        case POINTER_VSPLIT:
            MAKE_CURSOR( vsplit_ );
            break;
        case POINTER_HSIZEBAR:
            aCur = XCreateFontCursor( pDisp_, XC_sb_h_double_arrow ); // ???
            break;
        case POINTER_VSIZEBAR:
            aCur = XCreateFontCursor( pDisp_, XC_sb_v_double_arrow ); // ???
            break;
        case POINTER_REFHAND:
            aCur = XCreateFontCursor( pDisp_, XC_hand1 );
            break;
        case POINTER_HAND:
            aCur = XCreateFontCursor( pDisp_, XC_hand2 );
            break;
        case POINTER_MAGNIFY:
            MAKE_CURSOR( magnify_ );
            break;
        case POINTER_FILL:
            MAKE_CURSOR( fill_ );
            break;
        case POINTER_MOVE:
            MAKE_CURSOR( move_ );
            break;
        case POINTER_MOVEDATA:
            MAKE_CURSOR( movedata_ );
            break;
        case POINTER_COPYDATA:
            MAKE_CURSOR( copydata_ );
            break;
        case POINTER_MOVEFILE:
            MAKE_CURSOR( movefile_ );
            break;
        case POINTER_COPYFILE:
            MAKE_CURSOR( copyfile_ );
            break;
        case POINTER_MOVEFILES:
            MAKE_CURSOR( movefiles_ );
            break;
        case POINTER_COPYFILES:
            MAKE_CURSOR( copyfiles_ );
            break;
        case POINTER_NOTALLOWED:
            MAKE_CURSOR( nodrop_ );
            break;
        case POINTER_ROTATE:
            MAKE_CURSOR( rotate_ );
            break;
        case POINTER_HSHEAR:
            MAKE_CURSOR( hshear_ );
            break;
        case POINTER_VSHEAR:
            MAKE_CURSOR( vshear_ );
            break;
        case POINTER_DRAW_LINE:
            MAKE_CURSOR( drawline_ );
            break;
        case POINTER_DRAW_RECT:
            MAKE_CURSOR( drawrect_ );
            break;
        case POINTER_DRAW_POLYGON:
            MAKE_CURSOR( drawpolygon_ );
            break;
        case POINTER_DRAW_BEZIER:
            MAKE_CURSOR( drawbezier_ );
            break;
        case POINTER_DRAW_ARC:
            MAKE_CURSOR( drawarc_ );
            break;
        case POINTER_DRAW_PIE:
            MAKE_CURSOR( drawpie_ );
            break;
        case POINTER_DRAW_CIRCLECUT:
            MAKE_CURSOR( drawcirclecut_ );
            break;
        case POINTER_DRAW_ELLIPSE:
            MAKE_CURSOR( drawellipse_ );
            break;
        case POINTER_DRAW_CONNECT:
            MAKE_CURSOR( drawconnect_ );
            break;
        case POINTER_DRAW_TEXT:
            MAKE_CURSOR( drawtext_ );
            break;
        case POINTER_MIRROR:
            MAKE_CURSOR( mirror_ );
            break;
        case POINTER_CROOK:
            MAKE_CURSOR( crook_ );
            break;
        case POINTER_CROP:
            MAKE_CURSOR( crop_ );
            break;
        case POINTER_MOVEPOINT:
            MAKE_CURSOR( movepoint_ );
            break;
        case POINTER_MOVEBEZIERWEIGHT:
            MAKE_CURSOR( movebezierweight_ );
            break;
        case POINTER_DRAW_FREEHAND:
            MAKE_CURSOR( drawfreehand_ );
            break;
        case POINTER_DRAW_CAPTION:
            MAKE_CURSOR( drawcaption_ );
            break;
        case POINTER_PEN:       // Mouse Pointer ist ein Stift
            aCur = XCreateFontCursor( pDisp_, XC_pencil );
            break;
        case POINTER_LINKDATA:
            MAKE_CURSOR( linkdata_ );
            break;
        case POINTER_MOVEDATALINK:
            MAKE_CURSOR( movedlnk_ );
            break;
        case POINTER_COPYDATALINK:
            MAKE_CURSOR( copydlnk_ );
            break;
        case POINTER_LINKFILE:
            MAKE_CURSOR( linkfile_ );
            break;
        case POINTER_MOVEFILELINK:
            MAKE_CURSOR( moveflnk_ );
            break;
        case POINTER_COPYFILELINK:
            MAKE_CURSOR( copyflnk_ );
            break;
        case POINTER_CHART:
            MAKE_CURSOR( chart_ );
            break;
        case POINTER_DETECTIVE:
            MAKE_CURSOR( detective_ );
            break;
        case POINTER_PIVOT_COL:
            MAKE_CURSOR( pivotcol_ );
            break;
        case POINTER_PIVOT_ROW:
            MAKE_CURSOR( pivotrow_ );
            break;
        case POINTER_PIVOT_FIELD:
            MAKE_CURSOR( pivotfld_ );
            break;
        case POINTER_CHAIN:
            MAKE_CURSOR( chain_ );
            break;
        case POINTER_CHAIN_NOTALLOWED:
            MAKE_CURSOR( chainnot_ );
            break;
        case POINTER_TIMEEVENT_MOVE:
            MAKE_CURSOR( timemove_ );
            break;
        case POINTER_TIMEEVENT_SIZE:
            MAKE_CURSOR( timesize_ );
            break;
        case POINTER_AUTOSCROLL_N:
            MAKE_CURSOR(asn_ );
            break;
        case POINTER_AUTOSCROLL_S:
            MAKE_CURSOR( ass_ );
            break;
        case POINTER_AUTOSCROLL_W:
            MAKE_CURSOR( asw_ );
            break;
        case POINTER_AUTOSCROLL_E:
            MAKE_CURSOR( ase_ );
            break;
        case POINTER_AUTOSCROLL_NW:
            MAKE_CURSOR( asnw_ );
            break;
        case POINTER_AUTOSCROLL_NE:
            MAKE_CURSOR( asne_ );
            break;
        case POINTER_AUTOSCROLL_SW:
            MAKE_CURSOR( assw_ );
            break;
        case POINTER_AUTOSCROLL_SE:
            MAKE_CURSOR( asse_ );
            break;
        case POINTER_AUTOSCROLL_NS:
            MAKE_CURSOR( asns_ );
            break;
        case POINTER_AUTOSCROLL_WE:
            MAKE_CURSOR( aswe_ );
            break;
        case POINTER_AUTOSCROLL_NSWE:
            MAKE_CURSOR( asnswe_ );
            break;
        case POINTER_AIRBRUSH:
            MAKE_CURSOR( airbrush_ );
            break;
        case POINTER_TEXT_VERTICAL:
            MAKE_CURSOR( vertcurs_ );
            break;
        default:
            DBG_ERROR("pointer not implemented");
            aCur = XCreateFontCursor( pDisp_, XC_arrow );
            break;
    }

    if( None == aCur )
    {
        XColor      aBlack, aWhite, aDummy;
        Colormap    hColormap = xColor_->GetXColormap();

        XAllocNamedColor( pDisp_, hColormap, "black", &aBlack, &aDummy );
        XAllocNamedColor( pDisp_, hColormap, "white", &aWhite, &aDummy );

        aCur = XCreatePixmapCursor( pDisp_,
                                    aCursBitmap, aMaskBitmap,
                                    &aBlack, &aWhite,
                                    nXHot, nYHot );

        XFreePixmap( pDisp_, aCursBitmap );
        XFreePixmap( pDisp_, aMaskBitmap );
    }

    return aCur;
}

int SalDisplay::CaptureMouse( SalFrameData *pCapture )
{
    if( !pCapture )
    {
        pCapture_ = NULL;
        XUngrabPointer( GetDisplay(), CurrentTime );
        XFlush( GetDisplay() );
        return 0;
    }

    if( pCapture_ )
        //pCapture_->CaptureMouse( FALSE );
        pCapture_ = NULL;

    int ret = XGrabPointer( GetDisplay(),
                            pCapture->GetWindow(),
                            False,
                            PointerMotionMask| ButtonPressMask|ButtonReleaseMask,
                            GrabModeAsync,
                            GrabModeAsync,
                            None,
                            pCapture->GetCursor(),
                            CurrentTime );

    if( ret != GrabSuccess )
    {
        DBG_ASSERT( 1, "SalDisplay::CaptureMouse could not grab pointer\n");
        return -1;
    }

    pCapture_ = pCapture;
    return 1;
}

// Fonts
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static BOOL
sal_IsValidFontpath( const ByteString &rFontPath,
                     const srv_vendor_t eServerVendor )
{
    // filter fontpath with ':unscaled' attribute for server other
    // than xfree (e.g.: /opt/Office51/fonts/75dpi:unscaled)

    const char   aAttr[]  = ":unscaled";
    const USHORT nAttrLen = sizeof( aAttr ) - 1;
    USHORT       nAttrPos;

    nAttrPos = rFontPath.Search( aAttr, 0 );
    if ( nAttrPos == (rFontPath.Len() - nAttrLen) )
        return (eServerVendor == vendor_xfree);

    return TRUE;
}

void SalDisplay::AddFontPath( const ByteString &rPath ) const
{
#ifndef USE_BUILTIN_RASTERIZER
    const char cSeparator = ';' ;

    if( rPath.Len()
        && (GetServerVendor() != vendor_excursion)
        && (GetServerVendor() != vendor_hummingbird) )
    {
        USHORT nCount = rPath.GetTokenCount( cSeparator );
        int     i;
        int     nPaths          = 0;
        char  **pOldFontPath    = XGetFontPath( pDisp_, &nPaths );
        int     nOriginalPaths  = nPaths;
        char  **pNewFontPath    = new char*[nPaths+nCount];
        BOOL    bOld            = pXLib_->GetIgnoreXErrors();

        for( i = 0; i < nPaths; i++ )
            pNewFontPath[i] = pOldFontPath[i];

        for( USHORT nNew = 0; nNew < nCount; nNew++ )
        {
            ByteString aPathName = rPath.GetToken( nNew, cSeparator );

            if( aPathName.Len() )
            {
                for( i = 0; i < nPaths; i++ )
                    if( !strcmp( pNewFontPath[i], aPathName.GetBuffer() ) )
                        break;

                if (   (i == nPaths)
                    && sal_IsValidFontpath(aPathName, GetServerVendor()) )
                {
                    pNewFontPath[nPaths] = new char[aPathName.Len()+1];
                    strcpy( pNewFontPath[nPaths++], aPathName.GetBuffer() );
                    pXLib_->SetIgnoreXErrors( TRUE ); // reset WasXError

                    XSetFontPath( pDisp_, pNewFontPath, nPaths );
                    XSync( pDisp_, False );
                    if( pXLib_->WasXError() )
                        delete pNewFontPath[--nPaths];
                }
            }
        }

        while( nPaths-- > nOriginalPaths )
            delete pNewFontPath[ nPaths ];

        delete pNewFontPath;
        XFreeFontPath( pOldFontPath );


        pXLib_->SetIgnoreXErrors( bOld );
    }
#endif
}

// Events
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalDisplay::Remove( XEvent *pEvent )
{
}

void SalDisplay::SendEvent( Atom          aEvent,
                                  void         *pData,
                                  XLIB_Window   hReceiver ) const
{
    UINT32 aData[5];

#if __SIZEOFLONG > 4
    aData[0] = (UINT32)((long)pData & 0xffffffff);
    aData[1] = (UINT32)((long)pData >> 32);
#else
    aData[0] = (UINT32)(long)pData;
    aData[1] = NULL;
#endif
    aData[2] = NULL;
    aData[3] = NULL;
    aData[4] = NULL;

    SendEvent( aEvent, aData, hReceiver );
}

void SalDisplay::SendEvent( Atom          aEvent,
                                  UINT32       *Data,
                                  XLIB_Window   hReceiver ) const
{
#define pClient (&(aClient.xclient))
    XEvent aClient;

    if( !hReceiver )
        hReceiver = GetWindow();

    pClient->type           = ClientMessage;
    pClient->display        = pDisp_;
    pClient->window         = hReceiver;
    pClient->message_type   = aEvent;
    pClient->format         = 32;

    if( Data ) for( int i = 0; i < 5; i++ )
        pClient->data.l[i] = Data[i];

    if( osl_acquireMutex( hEventGuard_ ) )
    {
        pClient->send_event     = 2;
        if( pEventQueue_ )
        {
            SalXEvent *pEvent = pEventQueue_;
            while( pEvent->pNext_ )
                pEvent = pEvent->pNext_;

            pEvent->pNext_                      = new SalXEvent;
            pEvent->pNext_->event_              = aClient;
            pEvent->pNext_->pNext_              = NULL;
        }
        else
        {
            ((SalDisplay*)this)->pEventQueue_   = new SalXEvent;
            pEventQueue_->event_                = aClient;
            pEventQueue_->pNext_                = NULL;
        }

        osl_releaseMutex( hEventGuard_ );
    }
    else
        DBG_ASSERT( 1, "SalDisplay::SendEvent !acquireMutex\n" );
#undef pClient
}

BOOL SalDisplay::IsEvent()
{
    if( pEventQueue_ )
        return TRUE;

    if( XEventsQueued( pDisp_, QueuedAlready ) )
        return TRUE;

    XFlush( pDisp_ );
    return FALSE;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalDisplay::Yield( BOOL bWait )
{
    SalXEvent aEvent;
    while( !nStateOfYield_ )
    {
        if( pEventQueue_ )
        {
            nStateOfYield_ = 5;
            if( osl_acquireMutex( hEventGuard_ ) )
            {
                nStateOfYield_ = 6;

                SalXEvent *pEvent = pEventQueue_;

                pEventQueue_    = pEventQueue_->pNext_;

                osl_releaseMutex( hEventGuard_ );

                aEvent.event_   = pEvent->event_;

                delete pEvent;

                break;
            }
            else
                DBG_ASSERT( 1, "SalDisplay::Yield !acquireMutex\n" );
        }

        nStateOfYield_ = 1;

        DBG_ASSERT( XtAppPending( pXLib_->GetAppContext() ), "no pending event" );
        SalData *pSalData       = GetSalData();
        SalYieldMutex* pSalInstYieldMutex   =
            pSalData->pFirstInstance_->maInstData.mpSalYieldMutex;

        DBG_ASSERT( pSalInstYieldMutex->GetThreadId() ==
                   NAMESPACE_VOS(OThread)::getCurrentIdentifier(),
                    "will crash soon since solar mutex not locked in SalDisplay::Yield" );

        // note: alternate input is dispatched by XtAppNextEvent
        XtAppNextEvent( pXLib_->GetAppContext(), &aEvent.event_ );
    }

    nStateOfYield_ = 0;

    BOOL bIgnoreXErrors = pXLib_->GetIgnoreXErrors();

    aEvent.pNext_   = pDispatchStack_;
    pDispatchStack_ = &aEvent;

    Dispatch( &aEvent.event_ );

    pDispatchStack_ = aEvent.pNext_;

#ifdef DBG_UTIL
    if( pXLib_->WasXError() )
    {
        XFlush( pDisp_ );
        PrintEvent( "SalDisplay::Yield (WasXError)", &aEvent.event_ );
    }
#endif

    pXLib_->SetIgnoreXErrors( bIgnoreXErrors );
}

long SalDisplay::Dispatch( XEvent *pEvent )
{
    if( pEvent->type == XLIB_KeyPress || pEvent->type == KeyRelease )
      {
        XLIB_Window aWindow = pEvent->xkey.window;
        SalFrame* pFrame = NULL;
        for( pFrame = GetSalData()->pFirstFrame_;
                pFrame
             && pFrame->maFrameData.GetWindow()      != aWindow
             && pFrame->maFrameData.GetShellWindow() != aWindow;
             pFrame = pFrame->maFrameData.GetNextFrame() )
          ;
        if( pFrame ) {
          XLIB_Window window= pFrame->maFrameData.GetWindow();
          if ( mpInputMethod->FilterEvent( pEvent , window) )
        return 0;
        }
      }
    else
      if ( mpInputMethod->FilterEvent( pEvent, None ) )
        return 0;

    SalInstance* pInstance = GetSalData()->pFirstInstance_;
    if( pInstance->maInstData.mpEventCallback )
    {
        YieldMutexReleaser aReleaser;
        pInstance->maInstData.mpEventCallback( pInstance->maInstData.mpEventInst, pEvent, sizeof( XEvent ) );
    }

    switch( pEvent->type )
    {
        case MotionNotify:
            while( XCheckWindowEvent( pEvent->xany.display,
                                      pEvent->xany.window,
                                      ButtonMotionMask,
                                      pEvent ) )
                ;
            break;

        case MappingNotify:
            if( MappingKeyboard == pEvent->xmapping.request )
                XRefreshKeyboardMapping( &pEvent->xmapping );
            else if( MappingModifier == pEvent->xmapping.request )
                ModifierMapping();
            break;

        default:

            if (   GetKbdExtension()->UseExtension()
                && GetKbdExtension()->GetEventBase() == pEvent->type )
            {
                GetKbdExtension()->Dispatch( pEvent );
                return 1;
            }

#ifdef _XSHM_H_
        {
            BOOL bPrevious = pXLib_->GetIgnoreXErrors();
            pXLib_->SetIgnoreXErrors( TRUE );
            if( pEvent->type == XShmGetEventBase( pDisp_ ) + ShmCompletion )
            {
                Remove( pEvent );
                return 1;
            }
            pXLib_->SetIgnoreXErrors( bPrevious );
        }
#endif
            break;
    }

    SalFrame *pFrame = GetSalData()->pFirstFrame_;

    while( pFrame )
    {
        XLIB_Window aDispatchWindow = pEvent->xany.window;
        if( pFrame->maFrameData.GetWindow() == aDispatchWindow      ||
            pFrame->maFrameData.GetShellWindow() == aDispatchWindow
            )
        {
            return pFrame->maFrameData.Dispatch( pEvent );
        }
        if( pFrame->maFrameData.GetForeignParent() == aDispatchWindow ||
            pFrame->maFrameData.GetForeignTopLevelWindow() == aDispatchWindow )
        {
            pFrame->maFrameData.Dispatch( pEvent );
            break;
        }
        if( pEvent->type == ConfigureNotify && pEvent->xconfigure.window == pFrame->maFrameData.GetStackingWindow() )
        {
            return pFrame->maFrameData.Dispatch( pEvent );
        }
        pFrame = pFrame->maFrameData.GetNextFrame();
    }

    // dispatch to Xt
    XtDispatchEvent( pEvent );

    // dispatch to salobjects
    SalObjectData::Dispatch( pEvent );

    return 0;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalDisplay::PrintEvent( const ByteString &rComment,
                                   XEvent       *pEvent ) const
{
    if( pEvent->type <= MappingNotify )
    {
        fprintf( stderr, "[%s] %s s=%d w=%ld\n",
                 rComment.GetBuffer(),
                 EventNames[pEvent->type],
                 pEvent->xany.send_event,
                 pEvent->xany.window );

        switch( pEvent->type )
        {
            case XLIB_KeyPress:
            case KeyRelease:
                fprintf( stderr, "\t\ts=%d c=%d\n",
                         pEvent->xkey.state,
                         pEvent->xkey.keycode );
                break;

            case ButtonPress:
            case ButtonRelease:
                fprintf( stderr, "\t\ts=%d b=%d x=%d y=%d rx=%d ry=%d\n",
                         pEvent->xbutton.state,
                         pEvent->xbutton.button,
                         pEvent->xbutton.x,
                         pEvent->xbutton.y,
                         pEvent->xbutton.x_root,
                         pEvent->xbutton.y_root );
                break;

            case MotionNotify:
                fprintf( stderr, "\t\ts=%d x=%d y=%d\n",
                         pEvent->xmotion.state,
                         pEvent->xmotion.x,
                         pEvent->xmotion.y );
                break;

            case EnterNotify:
            case LeaveNotify:
                fprintf( stderr, "\t\tm=%d f=%d x=%d y=%d\n",
                         pEvent->xcrossing.mode,
                         pEvent->xcrossing.focus,
                         pEvent->xcrossing.x,
                         pEvent->xcrossing.y );
                break;

            case FocusIn:
            case FocusOut:
                fprintf( stderr, "\t\tm=%d d=%d\n",
                         pEvent->xfocus.mode,
                         pEvent->xfocus.detail );
                break;

            case Expose:
            case GraphicsExpose:
                fprintf( stderr, "\t\tc=%d %d*%d %d+%d\n",
                         pEvent->xexpose.count,
                         pEvent->xexpose.width,
                         pEvent->xexpose.height,
                         pEvent->xexpose.x,
                         pEvent->xexpose.y );
                break;

            case VisibilityNotify:
                fprintf( stderr, "\t\ts=%d\n",
                         pEvent->xvisibility.state );
                break;

            case CreateNotify:
            case DestroyNotify:
                break;

            case MapNotify:
            case UnmapNotify:
                break;

            case ReparentNotify:
                fprintf( stderr, "\t\tp=%d x=%d y=%d\n",
                         pEvent->xreparent.parent,
                         pEvent->xreparent.x,
                         pEvent->xreparent.y );
                break;

            case ConfigureNotify:
                fprintf( stderr, "\t\tb=%d %d*%d %d+%d\n",
                         pEvent->xconfigure.border_width,
                         pEvent->xconfigure.width,
                         pEvent->xconfigure.height,
                         pEvent->xconfigure.x,
                         pEvent->xconfigure.y );
                break;

            case PropertyNotify:
                fprintf( stderr, "\t\ta=%s (0x%X)\n",
                         GetAtomName( pDisp_, pEvent->xproperty.atom ),
                         pEvent->xproperty.atom );
                break;

            case ColormapNotify:
                fprintf( stderr, "\t\tc=%ld n=%d s=%d\n",
                         pEvent->xcolormap.colormap,
                         pEvent->xcolormap.c_new,
                         pEvent->xcolormap.state );
                break;

            case ClientMessage:
                fprintf( stderr, "\t\ta=%s (0x%X) f=%i [0x%lX,0x%lX,0x%lX,0x%lX,0x%lX])\n",
                         GetAtomName( pDisp_, pEvent->xclient.message_type ),
                         pEvent->xclient.message_type,
                         pEvent->xclient.format,
                         pEvent->xclient.data.l[0],
                         pEvent->xclient.data.l[1],
                         pEvent->xclient.data.l[2],
                         pEvent->xclient.data.l[3],
                         pEvent->xclient.data.l[4] );
                break;

            case MappingNotify:
                fprintf( stderr, "\t\tr=%sd\n",
                         MappingModifier == pEvent->xmapping.request
                         ? "MappingModifier"
                         : MappingKeyboard == pEvent->xmapping.request
                           ? "MappingKeyboard"
                           : "MappingPointer" );

                break;
        }
    }
#ifdef _XSHM_H_
    else if( pEvent->type == XShmGetEventBase( pDisp_ ) + ShmCompletion )
    {
#define pCompletionEvent ((XShmCompletionEvent*)pEvent)
        fprintf( stderr, "[%s] %s s=%d d=%ld\n",
                 rComment.GetBuffer(),
                 "ShmCompletion",
                 pCompletionEvent->send_event,
                 pCompletionEvent->drawable );

        fprintf( stderr, "\t\tc=%d.%d s=%ld o=%ld\n",
                 pCompletionEvent->major_code,
                 pCompletionEvent->minor_code,
                 pCompletionEvent->shmseg,
                 pCompletionEvent->offset );
#undef pCompletionEvent
    }
#endif
    else
        fprintf( stderr, "[%s] %d s=%d w=%ld\n",
                 rComment.GetBuffer(),
                 pEvent->type,
                 pEvent->xany.send_event,
                 pEvent->xany.window );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalDisplay::PrintInfo() const
{
    if( IsDisplay() )
    {
        fprintf( stderr, "\n" );
        fprintf( stderr, "Environment\n" );
        fprintf( stderr, "\t$XENVIRONMENT     \t\"%s\"\n",
                 GetEnv( "XENVIRONMENT" ) );
        fprintf( stderr, "\t$DISPLAY          \t\"%s\"\n",
                 GetEnv( "DISPLAY" ) );
        fprintf( stderr, "\t$SAL_VISUAL       \t\"%s\"\n",
                 GetEnv( "SAL_VISUAL" ) );
        fprintf( stderr, "\t$SAL_FONTPATH     \t\"%s\"\n",
                 GetEnv( "SAL_FONTPATH" ) );
        fprintf( stderr, "\t$SAL_NOSEGV       \t\"%s\"\n",
                 GetEnv( "SAL_NOSEGV" ) );
        fprintf( stderr, "\t$SAL_IGNOREXERRORS\t\"%s\"\n",
                 GetEnv( "SAL_IGNOREXERRORS" ) );
        fprintf( stderr, "\t$SAL_PROPERTIES   \t\"%s\"\n",
                 GetEnv( "SAL_PROPERTIES" ) );
        fprintf( stderr, "\t$SAL_WM           \t\"%s\"\n",
                 GetEnv( "SAL_WM" ) );
        fprintf( stderr, "\t$SAL_SYNCHRONIZE  \t\"%s\"\n",
                 GetEnv( "SAL_SYNCHRONIZE" ) );
        fprintf( stderr, "\t$XPPATH           \t\"%s\"\n",
                 GetEnv( "XPPATH" ) );

        char sHostname[ 120 ];
#       ifdef USE_XMU
        XmuGetHostname( sHostname, 120 );
#       else
        gethostname (sHostname, 120 );
#       endif
        fprintf( stderr, "Client\n" );
        fprintf( stderr, "\tHost              \t\"%s\"\n",
                 sHostname );

        fprintf( stderr, "Display\n" );
        fprintf( stderr, "\tHost              \t\"%s\"\n",
                 DisplayString(pDisp_) );
        fprintf( stderr, "\tVendor (Release)  \t\"%s (%d)\"\n",
                 ServerVendor(pDisp_), VendorRelease(pDisp_) );
        fprintf( stderr, "\tProtocol          \t%d.%d\n",
                 ProtocolVersion(pDisp_), ProtocolRevision(pDisp_) );
        fprintf( stderr, "\tScreen (count,def)\t%d (%d,%d)\n",
                 nScreen_, ScreenCount(pDisp_), DefaultScreen(pDisp_) );
        fprintf( stderr, "\tshift ctrl alt    \t%s (0x%X) %s (0x%X) %s (0x%X)\n",
                 KeyStr( nShiftKeySym_ ), nShiftKeySym_,
                 KeyStr( nCtrlKeySym_ ),  nCtrlKeySym_,
                 KeyStr( nMod1KeySym_ ),  nMod1KeySym_ );
        if( XExtendedMaxRequestSize(pDisp_) * 4 )
            fprintf( stderr, "\tXMaxRequestSize   \t%ld %ld [bytes]\n",
                     XMaxRequestSize(pDisp_) * 4, XExtendedMaxRequestSize(pDisp_) * 4 );
        if( GetProperties() != PROPERTY_DEFAULT )
            fprintf( stderr, "\tProperties        \t0x%lX\n", GetProperties() );
        if( eWindowManager_ != otherwm )
            fprintf( stderr, "\tWindowmanager     \t%d\n", eWindowManager_ );
    }
    fprintf( stderr, "Screen\n" );
    fprintf( stderr, "\tResolution/Size   \t%d*%d %d*%d %.1lf\"\n",
             aResolution_.A(), aResolution_.B(),
             aSize_.Width(), aSize_.Height(),
             Hypothenuse( DisplayWidthMM ( pDisp_, nScreen_ ),
                          DisplayHeightMM( pDisp_, nScreen_ ) ) / 25.4 );
    fprintf( stderr, "\tBlack&White       \t%lu %lu\n",
             xColor_->GetBlackPixel(), xColor_->GetWhitePixel() );
    fprintf( stderr, "\tRGB               \t0x%lx 0x%lx 0x%lx\n",
             pVisual_->red_mask, pVisual_->green_mask, pVisual_->blue_mask );
    fprintf( stderr, "\tVisual            \t%d-bit %s ID=0x%x\n",
             pVisual_->GetDepth(),
             VisualClassName[ pVisual_->GetClass() ],
             pVisual_->GetVisualId() );
    if( pVisual_ != pRootVisual_ )
        fprintf( stderr, "\tRoot visual       \t%d-bit %s ID=0x%x\n",
                 pRootVisual_->GetDepth(),
                 VisualClassName[ pRootVisual_->GetClass() ],
                 pRootVisual_->GetVisualId() );
    fprintf( stderr, "\tImages (Shared)   \t0x%lx (%lx)\n",
             nImageDepths_, nSharedImages_ );

    if( nStateOfYield_ || nStateOfSendEvent_ )
    {
        fprintf( stderr, "Thread/Signal\n" );
        fprintf( stderr, "\tNextEvent         \t%d\n", nStateOfYield_ );
        fprintf( stderr, "\tSendEvent         \t%d\n", nStateOfSendEvent_ );
    }
    if( pDispatchStack_ )
    {
        fprintf( stderr, "Event\n" );
        SalXEvent *pEvent = pDispatchStack_;
        while( pEvent )
        {
            PrintEvent( "\t\x08\x08", &pEvent->event_ );
            pEvent = pEvent->pNext_;
        }
    }
}

// -=-= SalICCCM -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

SalICCCM::SalICCCM( SalDisplay *pDisplay )
{
    Display *display = pDisplay->GetDisplay();

    for( int i = 0; i < capacityof( AtomStrings ); i++ )
        (&aWM_Protocols_)[i] = XInternAtom( display, AtomStrings[i], False );
}

// -=-= SalVisual -=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalVisual::SalVisual( const XVisualInfo* pXVI )
{
    *(XVisualInfo*)this = *pXVI;
    if( GetClass() == TrueColor )
    {
        nRedShift_      = sal_Shift( red_mask );
        nGreenShift_    = sal_Shift( green_mask );
        nBlueShift_     = sal_Shift( blue_mask );

        if( GetDepth() == 24 )
            if( red_mask == 0xFF0000 )
                if( green_mask == 0xFF00 )
                    if( blue_mask  == 0xFF )
                        eRGBMode_ = RGB;
                    else
                        eRGBMode_ = other;
                else if( blue_mask  == 0xFF00 )
                    if( green_mask == 0xFF )
                        eRGBMode_ = RBG;
                    else
                        eRGBMode_ = other;
                else
                    eRGBMode_ = other;
            else if( green_mask == 0xFF0000 )
                if( red_mask == 0xFF00 )
                    if( blue_mask  == 0xFF )
                        eRGBMode_ = GRB;
                    else
                        eRGBMode_ = other;
                else if( blue_mask == 0xFF00 )
                    if( red_mask  == 0xFF )
                        eRGBMode_ = GBR;
                    else
                        eRGBMode_ = other;
                else
                    eRGBMode_ = other;
            else if( blue_mask == 0xFF0000 )
                if( red_mask == 0xFF00 )
                    if( green_mask  == 0xFF )
                        eRGBMode_ = BRG;
                    else
                        eRGBMode_ = other;
                else if( green_mask == 0xFF00 )
                    if( red_mask == 0xFF )
                        eRGBMode_ = BGR;
                    else
                        eRGBMode_ = other;
                else
                    eRGBMode_ = other;
            else
                eRGBMode_ = other;
        else
            eRGBMode_ = other;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalVisual::~SalVisual()
{
    if( -1 == screen && -1 == visualid ) delete visual;
}

// Konvertiert die Reihenfolge der Bytes eines Pixel in Bytes eines SalColors
// fuer die 6 XXXA ist das nicht reversibel
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// SalColor is RGB (ABGR) a=0xFF000000, r=0xFF0000, g=0xFF00, b=0xFF

#define SALCOLOR        RGB
#define SALCOLORREVERSE BGR

BOOL SalVisual::Convert( int &n0, int &n1, int &n2, int &n3 )
{
    int n;

    switch( GetMode() )
    {
        case other:
            return FALSE;
            break;
        case SALCOLOR:
            break;
        case SALCOLORREVERSE:
        case RBG:
        case BRG:
        case GBR:
        case GRB:
            return Convert( n0, n1, n2 );
            break;
        case RGBA:
            n  = n0;
            n0 = n1;
            n1 = n2;
            n2 = n3;
            n3 = n;
            break;
        case BGRA:
        case RBGA:
        case BRGA:
        case GBRA:
        case GRBA:
        default:
            fprintf( stderr, "SalVisual::Convert %d\n", GetMode() );
            abort();
            break;
    }
    return TRUE;
}

BOOL SalVisual::Convert( int &n0, int &n1, int &n2 )
{
    int n;

    switch( GetMode() )
    {
        case other:
            return FALSE;
            break;
        case SALCOLOR:
            break;
        case RBG:
            n  = n0;
            n0 = n1;
            n1 = n;
            break;
        case GRB:
            n  = n1;
            n1 = n2;
            n2 = n;
            break;
        case SALCOLORREVERSE:
            n  = n0;
            n0 = n2;
            n2 = n;
            break;
        case BRG:
            n  = n0;
            n0 = n1;
            n1 = n2;
            n2 = n;
            break;
        case GBR:
            n  = n2;
            n2 = n1;
            n1 = n0;
            n0 = n;
            break;
        default:
            fprintf( stderr, "SalVisual::Convert %d\n", GetMode() );
            abort();
            break;
    }
    return TRUE;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalColor SalVisual::GetTCColor( Pixel nPixel ) const
{
    if( SALCOLOR == eRGBMode_ )
        return (SalColor)nPixel;

    if( SALCOLORREVERSE == eRGBMode_ )
        return MAKE_SALCOLOR( (nPixel & 0x0000FF),
                              (nPixel & 0x00FF00) >>  8,
                              (nPixel & 0xFF0000) >> 16);

    Pixel r = nPixel & red_mask;
    Pixel g = nPixel & green_mask;
    Pixel b = nPixel & blue_mask;

    if( other != eRGBMode_ ) // 8+8+8=24
        return MAKE_SALCOLOR( r >> nRedShift_,
                              g >> nGreenShift_,
                              b >> nBlueShift_ );

    if( nRedShift_ > 0 )   r >>= nRedShift_;   else r <<= -nRedShift_;
    if( nGreenShift_ > 0 ) g >>= nGreenShift_; else g <<= -nGreenShift_;
    if( nBlueShift_ > 0 )  b >>= nBlueShift_;  else b <<= -nBlueShift_;

    return MAKE_SALCOLOR( r, g, b );
}

Pixel SalVisual::GetTCPixel( SalColor nSalColor ) const
{
    if( SALCOLOR == eRGBMode_ )
        return (Pixel)nSalColor;

    Pixel r = (Pixel)SALCOLOR_RED( nSalColor );
    Pixel g = (Pixel)SALCOLOR_GREEN( nSalColor );
    Pixel b = (Pixel)SALCOLOR_BLUE( nSalColor );

    if( SALCOLORREVERSE == eRGBMode_ )
        return (b << 16) | (g << 8) | (r);

    if( other != eRGBMode_ ) // 8+8+8=24
        return (r << nRedShift_) | (g << nGreenShift_) | (b << nBlueShift_);

    if( nRedShift_ > 0 )   r <<= nRedShift_;   else r >>= -nRedShift_;
    if( nGreenShift_ > 0 ) g <<= nGreenShift_; else g >>= -nGreenShift_;
    if( nBlueShift_ > 0 )  b <<= nBlueShift_;  else b >>= -nBlueShift_;

    return (r&red_mask) | (g&green_mask) | (b&blue_mask);
}

// -=-= SalColormap -=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalColormap::SalColormap( SalDisplay *pDisplay, Colormap hColormap )
    : pDisplay_( pDisplay ),
      hColormap_( hColormap ),
      pPalette_( NULL ),
      pLookupTable_( NULL )
{
    pVisual_ = pDisplay_->GetVisual();

    if( pVisual_ == pDisplay_->GetRootVisual() )
    {
        nBlackPixel_    = BlackPixel( pDisplay_->GetDisplay(),
                                      pDisplay_->GetScreenNumber() );
        nWhitePixel_    = WhitePixel( pDisplay_->GetDisplay(),
                                      pDisplay_->GetScreenNumber() );
    }
    else
    {
        XColor aColor;

        GetXPixel( aColor, 0x00, 0x00, 0x00 );
        nBlackPixel_ = aColor.pixel;

        GetXPixel( aColor, 0xFF, 0xFF, 0xFF );
        nWhitePixel_ = aColor.pixel;
    }

    nUsed_ = 1 << pVisual_->GetDepth();

    if( pVisual_->GetClass() == PseudoColor )
    {
        XColor aColor;
        int r, g, b;

        // black, white, gray, ~gray = 4
        GetXPixels( aColor, 0xC0, 0xC0, 0xC0 );

        // light colors: 3 * 2 = 6
//      GetXPixels( aColor, 0x00, 0x00, 0x00 );
        GetXPixels( aColor, 0x00, 0x00, 0xFF );
        GetXPixels( aColor, 0x00, 0xFF, 0x00 );
        GetXPixels( aColor, 0x00, 0xFF, 0xFF );
//      GetXPixels( aColor, 0xFF, 0x00, 0x00 );
//      GetXPixels( aColor, 0xFF, 0x00, 0xFF );
//      GetXPixels( aColor, 0xFF, 0xFF, 0x00 );
//      GetXPixels( aColor, 0xFF, 0xFF, 0xFF );

        // standard colors: 7 * 2 = 14
//      GetXPixels( aColor, 0x00, 0x00, 0x00 );
        GetXPixels( aColor, 0x00, 0x00, 0x80 );
        GetXPixels( aColor, 0x00, 0x80, 0x00 );
        GetXPixels( aColor, 0x00, 0x80, 0x80 );
        GetXPixels( aColor, 0x80, 0x00, 0x00 );
        GetXPixels( aColor, 0x80, 0x00, 0x80 );
        GetXPixels( aColor, 0x80, 0x80, 0x00 );
        GetXPixels( aColor, 0x80, 0x80, 0x80 );
        GetXPixels( aColor, 0x00, 0xB8, 0xFF ); // Blau 7

        // cube: 6*6*6 - 8 = 208
        for( r = 0; r < 0x100; r += 0x33 ) // 0x33, 0x66, 0x99, 0xCC, 0xFF
            for( g = 0; g < 0x100; g += 0x33 )
                for( b = 0; b < 0x100; b += 0x33 )
                    GetXPixels( aColor, r, g, b );

        // gray: 16 - 6 = 10
        for( g = 0x11; g < 0xFF; g += 0x11 )
            GetXPixels( aColor, g, g, g );

        // green: 16 - 6 = 10
        for( g = 0x11; g < 0xFF; g += 0x11 )
            GetXPixels( aColor, 0, g, 0 );

        // red: 16 - 6 = 10
        for( r = 0x11; r < 0xFF; r += 0x11 )
            GetXPixels( aColor, r, 0, 0 );

        // blue: 16 - 6 = 10
        for( b = 0x11; b < 0xFF; b += 0x11 )
            GetXPixels( aColor, 0, 0, b );
    }
}

// PseudoColor
SalColormap::SalColormap( const BitmapPalette &rPalette )
    : pDisplay_( GetSalData()->GetCurDisp() ),
      hColormap_( None ),
      pVisual_( NULL ),
      nUsed_( rPalette.GetEntryCount() ),
      nBlackPixel_( 0xFFFFFFFF ),
      nWhitePixel_( 0xFFFFFFFF ),
      pLookupTable_( NULL )
{
    pPalette_ = new SalColor[nUsed_];

    for( int i = 0; i < nUsed_; i++ )
    {
        const BitmapColor &rColor = rPalette[i];
        pPalette_[i] = MAKE_SALCOLOR( rColor.GetRed(),
                                      rColor.GetGreen(),
                                      rColor.GetBlue() );
        if( nBlackPixel_ == 0xFFFFFFFF && SALCOLOR_BLACK == pPalette_[i] )
            nBlackPixel_ = i;
        else if( nWhitePixel_ == 0xFFFFFFFF && SALCOLOR_WHITE == pPalette_[i] )
            nWhitePixel_ = i;
    }
}

// MonoChrome
SalColormap::SalColormap()
    : pDisplay_( GetSalData()->GetCurDisp() ),
      hColormap_( None ),
      pVisual_( NULL ),
      nUsed_( 2 ),
      nBlackPixel_( 0 ),
      nWhitePixel_( 1 ),
      pLookupTable_( NULL )
{
    pPalette_ = new SalColor[nUsed_];

    pPalette_[nBlackPixel_] = SALCOLOR_BLACK;
    pPalette_[nWhitePixel_] = SALCOLOR_WHITE;
}

// TrueColor
SalColormap::SalColormap( USHORT nDepth )
    : pDisplay_( GetSalData()->GetCurDisp() ),
      hColormap_( None ),
      pPalette_( NULL ),
      nUsed_( 1 << nDepth ),
      nWhitePixel_( (1 << nDepth) - 1 ),
      nBlackPixel_( 0x00000000 ),
      pLookupTable_( NULL )

{
    SalVisual *pVisual  = pDisplay_->GetVisual();

    if( pVisual->GetClass() == TrueColor && pVisual->GetDepth() == nDepth )
        pVisual_ = pVisual;
    else
    {
        XVisualInfo aVI;

        if( !XMatchVisualInfo( pDisplay_->GetDisplay(),
                               pDisplay_->GetScreenNumber(),
                               nDepth,
                               TrueColor,
                               &aVI ) )
        {
            aVI.visual          = new Visual();
            aVI.visualid        = (VisualID)-1;
            aVI.screen          = -1;
            aVI.depth           = nDepth;
            aVI.c_class         = TrueColor;
            if( 24 == nDepth ) // 888
            {
                aVI.red_mask        = 0xFF0000;
                aVI.green_mask      = 0x00FF00;
                aVI.blue_mask       = 0x0000FF;
            }
            else if( 16 == nDepth ) // 565
            {
                aVI.red_mask        = 0x00F800;
                aVI.green_mask      = 0x0007E0;
                aVI.blue_mask       = 0x00001F;
            }
            else if( 15 == nDepth ) // 555
            {
                aVI.red_mask        = 0x007C00;
                aVI.green_mask      = 0x0003E0;
                aVI.blue_mask       = 0x00001F;
            }
            else if( 12 == nDepth ) // 444
            {
                aVI.red_mask        = 0x000F00;
                aVI.green_mask      = 0x0000F0;
                aVI.blue_mask       = 0x00000F;
            }
            else if( 8 == nDepth ) // 332
            {
                aVI.red_mask        = 0x0000E0;
                aVI.green_mask      = 0x00001C;
                aVI.blue_mask       = 0x000003;
            }
            else
            {
                aVI.red_mask        = 0x000000;
                aVI.green_mask      = 0x000000;
                aVI.blue_mask       = 0x000000;
            }
            aVI.colormap_size   = 0;
            aVI.bits_per_rgb    = 8;

            aVI.visual->ext_data        = NULL;
            aVI.visual->visualid        = aVI.visualid;
            aVI.visual->c_class         = aVI.c_class;
            aVI.visual->red_mask        = aVI.red_mask;
            aVI.visual->green_mask      = aVI.green_mask;
            aVI.visual->blue_mask       = aVI.blue_mask;
            aVI.visual->bits_per_rgb    = aVI.bits_per_rgb;
            aVI.visual->map_entries     = aVI.colormap_size;
        }

        pVisual_ = new SalVisual( &aVI );
    }
}

SalColormap::~SalColormap()
{
    if( hColormap_
        && pDisplay_->IsDisplay()
        && hColormap_ != DefaultColormap( GetXDisplay(), pDisplay_->GetScreenNumber() ) )
        XFreeColormap( GetXDisplay(), hColormap_ );
    delete pPalette_;
    delete pLookupTable_;
    if( pVisual_ != pDisplay_->GetVisual() )
        delete pVisual_;

#ifdef DBG_UTIL
    hColormap_      = (Colormap)ILLEGAL_POINTER;
    pDisplay_       = (SalDisplay*)ILLEGAL_POINTER;
    pPalette_       = (SalColor*)ILLEGAL_POINTER;
    pLookupTable_   = (USHORT*)ILLEGAL_POINTER;
    pVisual_        = (SalVisual*)ILLEGAL_POINTER;
#endif
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalColormap::SetPalette( const BitmapPalette &rPalette )
{
    if( this != &GetSalData()->GetCurDisp()->GetColormap() )
    {
        nBlackPixel_ = 0xFFFFFFFF;
        nWhitePixel_ = 0xFFFFFFFF;
    }

    if( rPalette.GetEntryCount() > nUsed_ )
    {
        nBlackPixel_ = 0xFFFFFFFF;
        nWhitePixel_ = 0xFFFFFFFF;
        delete pPalette_;
        pPalette_ = new SalColor[rPalette.GetEntryCount()];
        nUsed_ = rPalette.GetEntryCount();
    }

    for( int i = 0; i < rPalette.GetEntryCount(); i++ )
    {
        const BitmapColor &rColor = rPalette[i];
        pPalette_[i] = MAKE_SALCOLOR( rColor.GetRed(),
                                      rColor.GetGreen(),
                                      rColor.GetBlue() );
        if( nBlackPixel_ == 0xFFFFFFFF && SALCOLOR_BLACK == pPalette_[i] )
            nBlackPixel_ = i;
        else if( nWhitePixel_ == 0xFFFFFFFF && SALCOLOR_WHITE == pPalette_[i] )
            nWhitePixel_ = i;
    }
}

void SalColormap::GetPalette()
{
    Pixel i;

    pPalette_ = new SalColor[nUsed_];

    XColor *aColor = new XColor[nUsed_];

    for( i = 0; i < nUsed_; i++ )
    {
        aColor[i].red = aColor[i].green = aColor[i].blue = 0;
        aColor[i].pixel = i;
    }

    XQueryColors( pDisplay_->GetDisplay(), hColormap_, aColor, nUsed_ );

    for( i = 0; i < nUsed_; i++ )
    {
        pPalette_[i] = MAKE_SALCOLOR( aColor[i].red   >> 8,
                                      aColor[i].green >> 8,
                                      aColor[i].blue  >> 8 );
    }

    delete aColor;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static USHORT sal_Lookup( SalColor *pPalette,
                                int r, int g, int b,
                                Pixel nUsed )
{
    USHORT nPixel = 0;
    int    nBest  = ColorDiff( pPalette[0], r, g, b );

    for( USHORT i = 1; i < nUsed; i++ )
    {
        int n = ColorDiff( pPalette[i], r, g, b );

        if( n < nBest )
        {
            if( !n )
                return i;

            nPixel = i;
            nBest  = n;
        }
    }
    return nPixel;
}

void SalColormap::GetLookupTable()
{
    USHORT *p = pLookupTable_ = new USHORT[16*16*16];

    for( int r = 0; r < 256; r += 17 )
        for( int g = 0; g < 256; g += 17 )
            for( int b = 0; b < 256; b += 17 )
                *p++ = sal_Lookup( pPalette_, r, g, b, nUsed_ );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalColor SalColormap::GetColor( Pixel nPixel ) const
{
    if( nBlackPixel_ == nPixel ) return SALCOLOR_BLACK;
    if( nWhitePixel_ == nPixel ) return SALCOLOR_WHITE;

    if( pVisual_ )
    {
        if( pVisual_->GetClass() == TrueColor )
            return pVisual_->GetTCColor( nPixel );

        if( !pPalette_
#ifndef USE_PSPRINT
            && ( hColormap_ || XSalIsPrinter( GetXDisplay() ) )
#else
            && hColormap_
#endif
#ifdef PSEUDOCOLOR12
            && pVisual_->GetDepth() <= 12
#else
            && pVisual_->GetDepth() <= 8
#endif
            && pVisual_->GetClass() == PseudoColor )
            ((SalColormap*)this)->GetPalette();
    }

    if( pPalette_ && nPixel < nUsed_ )
        return pPalette_[nPixel];

#ifndef USE_PSPRINT
    if( !hColormap_ && !XSalIsPrinter( GetXDisplay() ) )
#else
    if( !hColormap_ )
#endif
    {
        DBG_ASSERT( 1, "SalColormap::GetColor() !hColormap_\n" );
        return nPixel;
    }

    // DirectColor, StaticColor, StaticGray, GrayScale
    XColor aColor;

    aColor.pixel = nPixel;

    XQueryColor( pDisplay_->GetDisplay(), hColormap_, &aColor );

    return MAKE_SALCOLOR( aColor.red>>8, aColor.green>>8, aColor.blue>>8 );
}

inline BOOL SalColormap::GetXPixel( XColor &rColor,
                                          int     r,
                                          int     g,
                                          int     b ) const
{
    rColor.red      = r * 257;
    rColor.green    = g * 257;
    rColor.blue     = b * 257;
    return XAllocColor( GetXDisplay(), hColormap_, &rColor );
}

BOOL SalColormap::GetXPixels( XColor &rColor,
                                    int     r,
                                    int     g,
                                    int     b ) const
{
    if( !GetXPixel( rColor, r, g, b ) )
        return FALSE;
    if( rColor.pixel & 1 )
        return TRUE;
    return GetXPixel( rColor, r^0xFF, g^0xFF, b^0xFF );
}

Pixel SalColormap::GetPixel( SalColor nSalColor ) const
{
    if( 0xFFFFFFFF == nSalColor )     return 0;
    if( SALCOLOR_BLACK == nSalColor ) return nBlackPixel_;
    if( SALCOLOR_WHITE == nSalColor ) return nWhitePixel_;

    if( pVisual_ && pVisual_->GetClass() == TrueColor )
        return pVisual_->GetTCPixel( nSalColor );

    if( !pLookupTable_ )
    {
        if( !pPalette_
#ifndef USE_PSPRINT
            && ( hColormap_ || XSalIsPrinter( GetXDisplay() ) )
#else
            && hColormap_
#endif
            && pVisual_
#ifdef PSEUDOCOLOR12
            && pVisual_->GetDepth() <= 12
#else
            && pVisual_->GetDepth() <= 8
#endif
            && pVisual_->GetClass() == PseudoColor ) // what else ???
            ((SalColormap*)this)->GetPalette();

        if( pPalette_ )
            for( Pixel i = 0; i < nUsed_; i++ )
                if( pPalette_[i] == nSalColor )
                    return i;

#ifndef USE_PSPRINT
        if( hColormap_ || XSalIsPrinter( GetXDisplay() ) )
#else
        if( hColormap_ )
#endif
        {
            // DirectColor, StaticColor, StaticGray, GrayScale (PseudoColor)
            XColor aColor;

            if( GetXPixel( aColor,
                           SALCOLOR_RED  ( nSalColor ),
                           SALCOLOR_GREEN( nSalColor ),
                           SALCOLOR_BLUE ( nSalColor ) ) )
            {
                if( pPalette_ && !pPalette_[aColor.pixel] )
                {
                    pPalette_[aColor.pixel] = nSalColor;

                    if( !(aColor.pixel & 1) && !pPalette_[aColor.pixel+1] )
                    {
                        XColor aInversColor;

                        SalColor nInversColor = nSalColor ^ 0xFFFFFF;

                        GetXPixel( aInversColor,
                                   SALCOLOR_RED  ( nInversColor ),
                                   SALCOLOR_GREEN( nInversColor ),
                                   SALCOLOR_BLUE ( nInversColor ) );

                        if( !pPalette_[aInversColor.pixel] )
                            pPalette_[aInversColor.pixel] = nInversColor;
#ifdef DBG_UTIL
                        else
                            fprintf( stderr, "SalColormap::GetPixel() 0x06lx=%d 0x06lx=%d\n",
                                     nSalColor, aColor.pixel,
                                     nInversColor, aInversColor.pixel);
#endif
                    }
                }

                return aColor.pixel;
            }

#ifdef DBG_UTIL
            fprintf( stderr, "SalColormap::GetPixel() !XAllocColor %lx\n",
                     nSalColor );
#endif
        }

        if( !pPalette_ )
        {
            fprintf( stderr, "SalColormap::GetPixel() !pPalette_ %lx\n",
                     nSalColor);
            return nSalColor;
        }

        ((SalColormap*)this)->GetLookupTable();
    }

    // Colormatching ueber Palette
    USHORT r = SALCOLOR_RED  ( nSalColor );
    USHORT g = SALCOLOR_GREEN( nSalColor );
    USHORT b = SALCOLOR_BLUE ( nSalColor );
    return pLookupTable_[ (((r+8)/17) << 8)
                        + (((g+8)/17) << 4)
                        +  ((b+8)/17) ];
}

