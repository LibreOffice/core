/*************************************************************************
 *
 *  $RCSfile: impgrf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:09 $
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

// include ------------------------------------------------------------------

#include <limits.h>             // USHRT_MAX

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _BIGINT_HXX
#include <tools/bigint.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINIMGR_HXX
#include <svtools/iniman.hxx>
#endif
#ifndef _SFX_INIMGR_HXX
#include <sfx2/inimgr.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX //autogen
#include <vcl/waitobj.hxx>
#endif
#pragma hdrstop

#include <ucbhelper/content.hxx>

#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif

using namespace ::ucb;
using namespace com::sun::star::uno;

#include "dialogs.hrc"
#include "impgrf.hrc"

#define _SVX_IMPGRF_CXX
#include "impgrf.hxx"

#include "dialmgr.hxx"
#include "svxerr.hxx"
#include "helpid.hrc"

// defines ---------------------------------------------------------------

#define IMPGRF_INIKEY_ASLINK        "ImportGraphicAsLink"
#define IMPGRF_INIKEY_PREVIEW       "ImportGraphicPreview"

#ifdef MAC
#define IMPGRF_GRAPHIC_FILTER_FILE  "Filterlist"
#else
#define IMPGRF_GRAPHIC_FILTER_FILE  "install.ini"
#endif

#define IMPGRF_GRAPHIC_OPTIONS_FILE "fltopt.ini"

// C-Funktion ------------------------------------------------------------

USHORT GetImportFormatCount( GraphicFilter& rFlt )
{
    if ( rFlt.GetImportFormatCount() )
        return rFlt.GetImportFormatCount();
    else
        return 4;
}

// -----------------------------------------------------------------------

String GetImportFormatName( GraphicFilter& rFlt,
                            USHORT nFormat, String pFmtStrs[] )
{
    if ( rFlt.GetImportFormatCount() )
        return rFlt.GetImportFormatName( nFormat );
    else
        return pFmtStrs[STR_FLT_BMP + nFormat].GetToken( 0, ',' );
}

// -----------------------------------------------------------------------

String GetImportFormatWildcard( GraphicFilter& rFlt,
                                USHORT nFormat, String pFmtStrs[] )
{
    if ( rFlt.GetImportFormatCount() )
        return rFlt.GetImportWildcard( nFormat );
    else
        return pFmtStrs[STR_FLT_BMP + nFormat].GetToken( 1, ',' );
}

// -----------------------------------------------------------------------

String GetImportFormatOSType( GraphicFilter& rFlt, USHORT nFormat, String pFmtStrs[] )
{
    String aOSType;

    if ( rFlt.GetImportFormatCount() )
        aOSType = rFlt.GetImportFormatType( nFormat );
    else
    {
#ifdef MAC
        aOSType = pFmtStrs[STR_FLT_BMP + nFormat].GetToken( 2, ',' );
#endif
    }
    return aOSType;
}

// -----------------------------------------------------------------------

GraphicFilter* DialogsResMgr::GetGrfFilter_Impl()
{
    if( !pGrapicFilter )
    {
        pGrapicFilter = new GraphicFilter;
        ::FillFilter( *pGrapicFilter );
    }
    const Link aLink;
    pGrapicFilter->SetStartFilterHdl( aLink );
    pGrapicFilter->SetEndFilterHdl( aLink );
    pGrapicFilter->SetUpdatePercentHdl( aLink );
    return pGrapicFilter;
}

// -----------------------------------------------------------------------

GraphicFilter* GetGrfFilter()
{
    return (*(DialogsResMgr**)GetAppData(SHL_SVX))->GetGrfFilter_Impl();
}

// -----------------------------------------------------------------------

USHORT FillFilter( GraphicFilter& rFilter )
{
    ResMgr* pMgr = DIALOG_MGR();
    SfxIniManager* pIniMgr = SFX_INIMANAGER();
    String aModulesPath( pIniMgr->Get( SFX_KEY_MODULES_PATH ) );
    String aFullConfigPath;

    for ( xub_StrLen i = 0, nCount = aModulesPath.GetTokenCount(); i < nCount; i++ )
    {
        INetURLObject aToken( aModulesPath.GetToken( i ), INET_PROT_FILE );
        aToken.insertName( DEFINE_CONST_UNICODE(IMPGRF_GRAPHIC_FILTER_FILE) );

        if ( aFullConfigPath.Len() )
            aFullConfigPath += sal_Unicode(';');

        aFullConfigPath += aToken.getFSysPath( INetURLObject::FSYS_DETECT );;
    }

    rFilter.SetConfigPath( aFullConfigPath );

    INetURLObject aFilterPath( pIniMgr->Get( SFX_KEY_FILTER_PATH ), INET_PROT_FILE );
    rFilter.SetFilterPath( aFilterPath.getFSysPath( INetURLObject::FSYS_DETECT ) );

    INetURLObject aFltOptFile( pIniMgr->Get( SFX_KEY_USERCONFIG_PATH ), INET_PROT_FILE );
    aFltOptFile.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( IMPGRF_GRAPHIC_OPTIONS_FILE ) ) );
    rFilter.SetOptionsConfigPath( aFltOptFile );

    return rFilter.GetImportFormatCount();
}

sal_Bool IsFolder_Impl( const String& rURL )
{
    sal_Bool bRet = sal_False;
    try
    {
        Content aCnt( rURL, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        bRet = aCnt.isFolder();
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( ContentCreationException& )
    {
        DBG_ERRORFILE( "ContentCreationException" );
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
    }

    return bRet;
}

// -----------------------------------------------------------------------

#pragma optimize( "", off )

int LoadGraphic( const String &rPath, const String &rFilterName,
                 Graphic& rGraphic, GraphicFilter* pFilter,
                 USHORT* pDeterminedFormat )
{
    if ( !pFilter )
        pFilter = ::GetGrfFilter();

    const int nFilter = rFilterName.Len() && pFilter->GetImportFormatCount()
                    ? pFilter->GetImportFormatNumber( rFilterName )
                    : GRFILTER_FORMAT_DONTKNOW;

    SfxMedium* pMed = 0;

    // dann teste mal auf File-Protokoll:
    SvStream* pStream = NULL;
    INetURLObject aURL( rPath );

    if ( aURL.HasError() || INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( rPath );
    }
    else if ( INET_PROT_FILE != aURL.GetProtocol() )
    {
        // z.Z. nur auf die aktuelle DocShell
        pMed = new SfxMedium( rPath, STREAM_READ, TRUE );
        pMed->SetTransferPriority( SFX_TFPRIO_SYNCHRON );
        pMed->DownLoad();
        pStream = pMed->GetInStream();
    }
    int nRes = GRFILTER_OK;

    if ( !pStream )
        nRes = pFilter->ImportGraphic( rGraphic, aURL, nFilter, pDeterminedFormat );
    else
        nRes = pFilter->ImportGraphic( rGraphic, rPath, *pStream,
                                       nFilter, pDeterminedFormat );

#ifndef PRODUCT
    if( nRes )
    {
        if( pMed )
        {
            DBG_WARNING3( "GrafikFehler [%d] - [%s] URL[%s]",
                            nRes,
                            pMed->GetPhysicalName().GetBuffer(),
                            rPath.GetBuffer() );
        }
        else
        {
            DBG_WARNING2( "GrafikFehler [%d] - [%s]", nRes, rPath.GetBuffer() );
        }
    }
#endif

    if ( pMed )
        delete pMed;
    return nRes;
}

#pragma optimize( "", on )

// -----------------------------------------------------------------------

BOOL IsGraphicError_Impl( const USHORT nError, USHORT& rErrInfo )
{
    // default: format error
    rErrInfo = nError ? STR_GRFILTER_FORMATERROR : 0;

    switch ( nError )
    {
        case GRFILTER_OPENERROR:
            rErrInfo = STR_GRFILTER_OPENERROR;
            break;
        case GRFILTER_IOERROR:
            rErrInfo = STR_GRFILTER_IOERROR;
            break;
        case GRFILTER_FORMATERROR:
            rErrInfo = STR_GRFILTER_FORMATERROR;
            break;
        case GRFILTER_VERSIONERROR:
            rErrInfo = STR_GRFILTER_VERSIONERROR;
            break;
        case GRFILTER_FILTERERROR:
            rErrInfo = STR_GRFILTER_FILTERERROR;
            break;
    }
    return (BOOL)nError;
}

// struct SvxImportGraphicRes_Impl ---------------------------------------

struct SvxImportGraphicRes_Impl : public Resource
{
    SvxImportGraphicRes_Impl( USHORT nResId );
    ~SvxImportGraphicRes_Impl();

    String*     pStrings;
    BOOL        bSaveAs;
};

// -----------------------------------------------------------------------

SvxImportGraphicRes_Impl::SvxImportGraphicRes_Impl( USHORT nResId ) :

    Resource( SVX_RES( nResId ) ),

    pStrings( 0 ),
    bSaveAs( FALSE )

{
    USHORT nCount = STR_IMPORTGRAPHIC_COUNT+1;
    pStrings = new String[nCount];
    pStrings[0] = String();           // ein Dummy

    for ( USHORT i = 1; i < nCount; ++i )
        pStrings[i] = String( ResId(i) );
    FreeResource();
}

// -----------------------------------------------------------------------

SvxImportGraphicRes_Impl::~SvxImportGraphicRes_Impl()
{
    __DELETE(STR_IMPORTGRAPHIC_COUNT)pStrings;
}

// class SvxGraphicHdl_Impl ----------------------------------------------

class SvxGraphicHdl_Impl
{
public:
    SvxGraphicHdl_Impl();
    ~SvxGraphicHdl_Impl();

    void            GraphicPreview( SvxImportGraphicDialog* pDlg );

private:
friend class SvxImportGraphicDialog;

    Graphic*        pPreviewGrf;
    String          aPreviewPath;

    int             InsertGraphic( const String& rPath, const String& rFilter );
};

// class SvxGraphicPrevWin_Impl ------------------------------------------

class SvxGraphicPrevWin_Impl : public Window
{
public:
    SvxGraphicPrevWin_Impl( Window *pParent, WinBits nBits );
    ~SvxGraphicPrevWin_Impl();

    void        SetGraphic( Graphic* pPict, const String& rFile );
    Graphic*    CreateTempGraphic( Graphic* pPict );
    void        SetMessage( const String& rMessage );

private:
friend class SvxImportGraphicDialog;

    Graphic*    pTempGraphic;
    Graphic*    pGraphic;   // Grafik ...
    String      aMessage;   // oder Fehlermeldung
    String      aFile;      // Dateiname

    void        MouseButtonDown( const MouseEvent& );
    void        Paint( const Rectangle& );
};

// -----------------------------------------------------------------------

SvxGraphicPrevWin_Impl::SvxGraphicPrevWin_Impl( Window* pParent, WinBits nBits ) :

    Window( pParent, nBits ),

    pTempGraphic( NULL ),
    pGraphic    ( NULL )

{
    // Transparenter Font f"ur Message-Ausgabe
    Font aFont( GetFont() );
    aFont.SetTransparent( TRUE );
    SetFont( aFont );
}

// -----------------------------------------------------------------------

void SvxGraphicPrevWin_Impl::MouseButtonDown( const MouseEvent& )
{
    SvxImportGraphicDialog *pDlg = (SvxImportGraphicDialog*)GetParent();

    if ( pDlg->pPreviewBox )
    {
        pDlg->pPreviewBox->Check();
        ( (Link&)pDlg->pPreviewBox->GetClickHdl() ).Call( pDlg->pPreviewBox );
    }
}

// -----------------------------------------------------------------------

void SvxGraphicPrevWin_Impl::Paint( const Rectangle& )
{
    Size aOutSizePix = GetOutputSizePixel();

    if( pGraphic )
    {
        Size aNewSizePix( LogicToPixel( pGraphic->GetPrefSize(),
                                        pGraphic->GetPrefMapMode() ) );

        if( aNewSizePix.Height() && aOutSizePix.Height() )
        {
            Point           aPos;
            const double    fGrfWH = (double) aNewSizePix.Width() / aNewSizePix.Height();
            const double    fWinWH = (double) aOutSizePix.Width() / aOutSizePix.Height();

            // Bitmap an Fenstergroesse anpassen
            if( fGrfWH < fWinWH )
            {
                aNewSizePix.Width() = (long) ( aOutSizePix.Height() * fGrfWH );
                aNewSizePix.Height() = aOutSizePix.Height();
            }
            else
            {
                aNewSizePix.Width() = aOutSizePix.Width();
                aNewSizePix.Height()= (long) ( aOutSizePix.Width() / fGrfWH );
            }

            WaitObject aWaitPtr( this );

            aPos.X() = ( aOutSizePix.Width()  - aNewSizePix.Width() ) >> 1;
            aPos.Y() = ( aOutSizePix.Height() - aNewSizePix.Height() ) >> 1;

            if( pGraphic->IsAnimated() )
                pGraphic->StartAnimation( this, aPos, aNewSizePix );
            else
                pGraphic->Draw( this, aPos, aNewSizePix );
        }
    }
    else
    {
        SetLineColor( Color( COL_BLACK ) );

        if ( aMessage.Len() )
        {
            // Fehlermeldung zeilenweise ausgeben
            UniString aStrSpace( sal_Unicode( ' ' ) ), aStrChar( sal_Unicode( 'X' ) );
            xub_StrLen i,
                       nCount  = aMessage.GetTokenCount( sal_Unicode( ' ' ) ),
                       x       = 2,
                       y       = 2,
                       nSpaceW = GetTextWidth( aStrSpace ),
                       nCharH  = GetTextHeight();
            for ( i = 0; i < nCount; i++ )
            {
                String aWord = aMessage.GetToken( i, sal_Unicode( ' ' ) );
                long nWordW = GetTextWidth( aWord );
                if ( x != 2 && x + nWordW > aOutSizePix.Width() - 2 )
                {
                    x  = 2;
                    y += nCharH;
                }
                DrawText( Point( x, y ), aWord );
                x += nWordW + nSpaceW;
            }
        }
        else
        {
            // Kreuz malen
            DrawLine( Point( 0, 0),
                      Point( aOutSizePix.Width()  - 1,
                             aOutSizePix.Height() - 1 ) );
            DrawLine( Point( 0, aOutSizePix.Height() - 1),
                      Point( aOutSizePix.Width() - 1, 0 ) );
        }
    }

    // Zeichnen abgeschlossen
    ( (SvxImportGraphicDialog*)GetParent() )->SetPreviewing( FALSE );
}

// -----------------------------------------------------------------------

void SvxGraphicPrevWin_Impl::SetGraphic( Graphic* pPict, const String& rFile )
{
    delete pGraphic;
    pGraphic = pPict;

    if ( pGraphic )
    {
        aMessage.Erase();
        aFile = rFile;
    }
    else
        aFile.Erase();

    Invalidate();
}

// -----------------------------------------------------------------------

Graphic* SvxGraphicPrevWin_Impl::CreateTempGraphic( Graphic* pPict )
{
    DELETEZ(pTempGraphic);

    if ( pPict )
        pTempGraphic = new Graphic( *pPict );
    return pTempGraphic;
}

// -----------------------------------------------------------------------

void SvxGraphicPrevWin_Impl::SetMessage( const String& rMessage )
{
    delete pGraphic;
    pGraphic = 0;
    aMessage = rMessage;
}

// -----------------------------------------------------------------------

SvxGraphicPrevWin_Impl::~SvxGraphicPrevWin_Impl()
{
    delete pTempGraphic;
    delete pGraphic;
}

// class SvxImportGraphicDialog ------------------------------------------

SvxImportGraphicDialog::SvxImportGraphicDialog
(
    Window*         pParent,
    const String&   rTitle,
    const USHORT    nEnable,
    WinBits         nFlags
) :
    SfxFileDialog( pParent, nFlags ),

    pMedium             ( 0 ),
    pStandardButton     ( 0 ),
    pInternetButton     ( 0 ),
    pPropertiesButton   ( 0 ),
    pFilterButton       ( 0 ),
    pLinkBox            ( 0 ),
    pPreviewBox         ( 0 ),
    bPreviewing         ( FALSE )

{
    if (nEnable & ENABLE_EMPTY_FILENAMES)
        EnableEmptyFilename();

    Construct_Impl( rTitle, nEnable );
    pResImpl->bSaveAs = WB_SAVEAS == (nFlags&WB_SAVEAS);
}

//-------------------------------------------------------------------------

void SvxImportGraphicDialog::Construct_Impl( const String &rTitle, USHORT nEnable )
{
    pResImpl = new SvxImportGraphicRes_Impl( RID_SVXRES_IMPORTGRAPHIC );

    SetFilterSelectHdl(
        LINK( this, SvxImportGraphicDialog, FilterSelectHdl_Impl ) );

    // Titel
    SetText( rTitle );

    // Ggf. "Eigenschaften" und "Verkn"upfung"
    SfxApplication* pSfxApp = SFX_APP();
    SfxIniManager* pIniMgr = SFX_INIMANAGER();

    if ( ( ( ENABLE_LINK & nEnable ) && ( ENABLE_PROPERTY & nEnable ) ) ||
         ( ENABLE_PROP_WITHOUTLINK & nEnable ) )
    {
        pPropertiesButton = new PushButton( this );
        pPropertiesButton->SetText( pResImpl->pStrings[STR_PROPERTIES] );
        pPropertiesButton->SetClickHdl(
            LINK( this, SvxImportGraphicDialog, PropertiesHdl_Impl ) );
        pPropertiesButton->SetHelpId( HID_IMPGRF_BTN_PROPERTIES );
        AddControl( pPropertiesButton );
        pPropertiesButton->Show();
    }

    // FilterButton jetzt immer
    pFilterButton = new PushButton( this );
    pFilterButton->SetText( pResImpl->pStrings[STR_FILTER] );
    pFilterButton->SetClickHdl( LINK( this, SvxImportGraphicDialog, FilterHdl_Impl ) );
    pFilterButton->SetHelpId( HID_IMPGRF_BTN_FILTER );
    AddControl( pFilterButton );
    pFilterButton->Show();

    if ( ENABLE_LINK & nEnable )
    {
        BOOL bLink = FALSE;
        const SfxPoolItem* pLink = pSfxApp->GetItem( SID_IMPORT_GRAPH_LINK );

        if ( pLink )
            bLink = ( (const SfxBoolItem*)pLink )->GetValue();
        else
            bLink = (BOOL)pIniMgr->Get( SFX_GROUP_COMMON, UniString::CreateFromAscii(
                           RTL_CONSTASCII_STRINGPARAM( IMPGRF_INIKEY_ASLINK ) ) ).ToInt32();
        pLinkBox = new CheckBox( this );
        pLinkBox->SetText( pResImpl->pStrings[STR_LINK] );
        pLinkBox->Check( bLink );
        pLinkBox->SetHelpId( HID_IMPGRF_CB_LINK );
        AddControl( pLinkBox );
        pLinkBox->Show();
    }

    // static Werte besorgen
    const SfxPoolItem* pPrevItem = pSfxApp->GetItem( SID_IMPORT_GRAPH_PREVIEW );
    BOOL bShowPreview = FALSE;

    if ( pPrevItem )
        bShowPreview = ( (SfxBoolItem*)pPrevItem )->GetValue();
    else
        bShowPreview = (BOOL)pIniMgr->Get( SFX_GROUP_COMMON, UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM( IMPGRF_INIKEY_PREVIEW ) ) ).ToInt32();

    // "Vorschau"
    pPreviewBox = new CheckBox( this );
    pPreviewBox->SetText( pResImpl->pStrings[STR_PREVIEW] );
    pPreviewBox->SetClickHdl( LINK( this, SvxImportGraphicDialog, PreviewHdl_Impl ) );
    pPreviewBox->Check( bShowPreview );
    pPreviewBox->SetHelpId( HID_IMPGRF_CB_PREVIEW );
    AddControl( pPreviewBox );
    pPreviewBox->Show();

    // Preview-Fenster erst hier erzeugen
    pPrevWin = new SvxGraphicPrevWin_Impl( this, WinBits( WB_BORDER ) );
    pPrevWin->SetBackground( Wallpaper( Color( COL_WHITE ) ) );
    AddControl( pPrevWin );
    pPrevWin->Show();

    const SfxStringItem* pPathItem =
        (const SfxStringItem*)pSfxApp->GetItem( SID_IMPORT_GRAPH_LASTPATH );
    const SfxStringItem* pFilterItem =
        (const SfxStringItem*)pSfxApp->GetItem( SID_IMPORT_GRAPH_LASTFILTER );

    // Filter ermitteln
    GraphicFilter*  pGraphicFilter = GetGrfFilter();
    USHORT          i, nCount = pGraphicFilter->GetImportFormatCount();

    // Filter "Alle"
    String aExtensions;

    for ( i = 0; i < nCount; i++ )
    {
        String aWildcard =
            ::GetImportFormatWildcard( *pGraphicFilter, i, pResImpl->pStrings );

        if ( aExtensions.Search( aWildcard ) == STRING_NOTFOUND )
        {
            if ( aExtensions.Len() )
                aExtensions += sal_Unicode(';');
            aExtensions += aWildcard;
        }
    }
#if defined(WIN) || defined(WNT)
    if ( aExtensions.Len() < 240 )
        AddFilter( pResImpl->pStrings[STR_IMPORT_ALL], aExtensions );
    else
        AddFilter( SVX_RESSTR( RID_SVXSTR_ALL_FILES ),
                   UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.*" ) ) );
#else
    AddFilter( pResImpl->pStrings[STR_IMPORT_ALL], aExtensions );
#endif

    // Filter eintragen

    for ( i = 0; i < nCount; i++ )
    {
        String aName =
            ::GetImportFormatName( *pGraphicFilter, i, pResImpl->pStrings );
        String aWildcard =
            ::GetImportFormatWildcard( *pGraphicFilter, i, pResImpl->pStrings );
        String aOSType =
            ::GetImportFormatOSType( *pGraphicFilter, i, pResImpl->pStrings );
        AddFilter( aName, aWildcard, aOSType );
    }

    // Pfad und Filter setzen
    aStartPath = pIniMgr->Get( SFX_KEY_GRAPHICS_PATH );
    FASTBOOL bGrfPath = ( aStartPath.Len() > 0 );
    if ( !bGrfPath )
        aStartPath = pIniMgr->Get( SFX_KEY_WORK_PATH );
    SetStandardDir( aStartPath );
    String aLastPath;
    if ( pPathItem )
      aLastPath = pPathItem->GetValue();

    if ( aLastPath.Len() )
        SetPath( aLastPath, TRUE );
    else
    {
        String aPath =  aStartPath;

        if ( bGrfPath )
        {
            INetURLObject aTemp( aPath, INET_PROT_FILE );
            aTemp.setFinalSlash();
            aPath = aTemp.getFSysPath( INetURLObject::FSYS_DETECT );
        }
        SetPath( aPath, TRUE );
    }

    String aLastFilter;

    if ( pFilterItem )
        aLastFilter = pFilterItem->GetValue();

    if ( aLastFilter.Len() )
        SetCurFilter( aLastFilter );
    else
        SetCurFilter( pResImpl->pStrings[STR_IMPORT_ALL] );

    // aPreviewTimer
    aPrevTimer.SetTimeout( 500 );
    aPrevTimer.SetTimeoutHdl(
        LINK( this, SvxImportGraphicDialog, TimeOutHdl_Impl ) );

    FilterSelectHdl_Impl( NULL );
}

// -----------------------------------------------------------------------

SvxImportGraphicDialog::~SvxImportGraphicDialog()
{
    ReleaseOwnerShip( pPrevWin );
    ReleaseOwnerShip( pPropertiesButton );
    ReleaseOwnerShip( pLinkBox );
    ReleaseOwnerShip( pPreviewBox );
    ReleaseOwnerShip( pStandardButton );
    ReleaseOwnerShip( pInternetButton );
    ReleaseOwnerShip( pFilterButton );
    delete pPrevWin;
    delete pPropertiesButton;
    delete pLinkBox;
    delete pPreviewBox;
    delete pStandardButton;
    delete pInternetButton;
    delete pFilterButton;
    delete pResImpl;
    delete pMedium;
}

// -----------------------------------------------------------------------

void SvxImportGraphicDialog::FileSelect()
{
    // falls User neu selektiert hat, dann die Internetselektion vergessen
    String aSelPath = SfxFileDialog::GetPath();
    sal_Bool bFolder = IsFolder_Impl( aSelPath );

    if ( bFolder || aSelPath == aCurrPath )
        // keine Datei selektiert, sondern Verzeichnis gewechselt
        // oder gleiche Datei selektiert
        return;
    else
        aCurrPath = aSelPath;

    // falls gew"unscht, Preview antriggern
    if ( pPreviewBox && pPreviewBox->IsChecked() )
        aPrevTimer.Start();
}

// -----------------------------------------------------------------------

BOOL SvxImportGraphicDialog::IsURL() const
{
    return FALSE;
}

// -----------------------------------------------------------------------

short SvxImportGraphicDialog::Execute()
{
    short  nRet;
    USHORT nFound = 0;
    SfxApplication* pSfxApp = SFX_APP();
    GraphicFilter*  pGraphicFilter = GetGrfFilter();

    do
    {
        InitSize( UniString::CreateFromAscii(
                  RTL_CONSTASCII_STRINGPARAM( "ImpGrfDialogSize" ) ) );
        nRet = SfxFileDialog::Execute();
        String aGrfPath = GetPath();

        if ( nRet == RET_OK && aGrfPath.Len())
        {
            INetURLObject aObj( aGrfPath );

            // zuletzt verwendeten Filter merken
            pSfxApp->PutItem( SfxStringItem( SID_IMPORT_GRAPH_LASTFILTER, GetCurFilter() ) );

            // fall File-System,

            if ( !IsURL() && INET_PROT_FILE == aObj.GetProtocol() )
            {
                // dann den zuletzt verwendeten Pfad merken
                INetURLObject aTemp = aObj;
                aTemp.removeSegment();
                aTemp.setFinalSlash();
                pSfxApp->PutItem( SfxStringItem( SID_IMPORT_GRAPH_LASTPATH,
                                                 aTemp.getFSysPath( INetURLObject::FSYS_DETECT ) ) );
            }

            // merken, ob der Benutzer Linken und Preview eingeschaltet hat
            BOOL bLink = pLinkBox ? pLinkBox->IsChecked() : FALSE;
            pSfxApp->PutItem( SfxBoolItem( SID_IMPORT_GRAPH_LINK, bLink ) );
            sal_Unicode cTemp = bLink ? '1' : '0';
            SFX_INIMANAGER()->Set( String( cTemp ), SFX_GROUP_COMMON, UniString::CreateFromAscii(
                                   RTL_CONSTASCII_STRINGPARAM( IMPGRF_INIKEY_ASLINK ) ) );
            cTemp = pPreviewBox && pPreviewBox->IsChecked() ? '1' : '0';
            SFX_INIMANAGER()->Set( String( cTemp ), SFX_GROUP_COMMON, UniString::CreateFromAscii(
                                   RTL_CONSTASCII_STRINGPARAM( IMPGRF_INIKEY_PREVIEW ) ) );

            // Open?
            if ( !pResImpl->bSaveAs )
            {
                // pr"uefen, ob die Grafik geladen werden kann
                nFound = USHRT_MAX;
                SvStream* pStream = NULL;
                SfxMedium* pMed = NULL;

                if ( INET_PROT_FILE != aObj.GetProtocol() )
                {
                    pMed = new SfxMedium( aObj.GetMainURL(), STREAM_READ, TRUE );
                    pMed->SetTransferPriority( SFX_TFPRIO_SYNCHRON );
                    pMed->DownLoad();
                    pStream = pMed->GetInStream();
                }
                USHORT nImpRet = 0;
                USHORT nRetFormat = 0;
                USHORT nFormatNum = pGraphicFilter->
                            GetImportFormatNumber( GetCurFilter() );

                if ( !pStream )
                    nImpRet = pGraphicFilter->CanImportGraphic( aObj, nFormatNum, &nRetFormat );
                else
                    nImpRet = pGraphicFilter->CanImportGraphic( aObj.GetMainURL(), *pStream, nFormatNum, &nRetFormat );

                if ( GRFILTER_OK != nImpRet )
                {
                    if ( !pStream )
                        nImpRet = pGraphicFilter->CanImportGraphic( aObj, GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
                    else
                        nImpRet = pGraphicFilter->CanImportGraphic( aObj.GetMainURL(), *pStream,
                                        GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
                }

                if ( GRFILTER_OK == nImpRet )
                    nFound = nRetFormat;

                // konnte sie nicht geladen werden?
                if ( nFound == USHRT_MAX )
                {
                    // Fehlermeldung
                    Window *pParent = GetParent();

                    if ( !pParent->IsReallyVisible() )
                        pParent = NULL;

                    USHORT nErrInfo;
                    IsGraphicError_Impl( nImpRet, nErrInfo );
                    InfoBox aInfoBox( pParent, pResImpl->pStrings[nErrInfo] );
                    aInfoBox.Execute();
                }
                else
                {
                    // passenden Filter setzen
                    String sFmt( ::GetImportFormatName(
                        *pGraphicFilter, nFound, pResImpl->pStrings ) );
                    SetCurFilter( sFmt );
                }

                if ( pPrevWin && pPrevWin->pGraphic &&
                     pPrevWin->pGraphic->IsAnimated() )
                    pPrevWin->pGraphic->StopAnimation( pPrevWin );
            }
        }
    }
    while ( nRet == RET_OK && nFound == USHRT_MAX );
    return nRet;
}

// -----------------------------------------------------------------------

void SvxImportGraphicDialog::SetPath( const String& rPath, BOOL bDir, BOOL bLink )
{
    // Leere Verzeichnisse ignorieren
    if ( bDir && !rPath.Len() )
        return;

    // Bei Pfaden ggf. Delimiter anhaengen
    String aPath( rPath );

    if ( !bDir && pLinkBox )
        pLinkBox->Check( bLink );

    if ( !aPath.Len() )
        return;

    INetURLObject aTmp;
    aTmp.SetSmartURL( rPath );

    if ( aTmp.GetProtocol() != INET_PROT_FILE )
    {
        SFX_APP()->PutItem( SfxStringItem( SID_IMPORT_GRAPH_LASTURL, rPath ) );
        return;
    }
    else if ( bDir )
    {
        aTmp.setFinalSlash();
        aPath = aTmp.getFSysPath( INetURLObject::FSYS_DETECT );
    }

    if ( !bDir || IsFolder_Impl( aTmp.GetMainURL() ) )
        SfxFileDialog::SetPath( aPath );
    else
    {
        String aTmp( pResImpl->pStrings[STR_PATH_NOT_FOUND1] );
        aTmp += aPath;
        aTmp += pResImpl->pStrings[STR_PATH_NOT_FOUND2];
        Window* pBoxParent;

        if ( IsInExecute() )
            pBoxParent = this;
        else
            pBoxParent = GetParent();
        InfoBox( pBoxParent, aTmp ).Execute();
    }
}

// -----------------------------------------------------------------------

Graphic* SvxImportGraphicDialog::GetGraphic() const
{
    Graphic* pGraphic = NULL;
    FASTBOOL bForcePreview = FALSE;

    if ( ( !pPrevWin || !pPrevWin->pGraphic ) && pPreviewBox )
    {
        // wenn die Grafik noch nicht geladen ist (Vorschau ausgeschaltet),
        // dann die Vorschau anschalten und dadurch die Grafik laden
        if ( !pPreviewBox->IsChecked() )
        {
            pPreviewBox->Check( TRUE );
            bForcePreview = TRUE;
        }
        ( (SvxImportGraphicDialog*)this )->PreviewHdl_Impl( NULL );
    }

    if ( pPrevWin )
    {
        INetURLObject aFile( pPrevWin->aFile );
        INetURLObject aCurFile( GetPath() );
        if ( aFile == aCurFile )
        {
            pGraphic = pPrevWin->pGraphic;

            // falls Grafik gelinkt eingef"ugt werden soll, native Link zur"ucksetzen
            if ( pGraphic && AsLink() )
                pGraphic->SetLink( GfxLink() );
        }
    }

    if ( bForcePreview && pPrevWin )
    {
        pGraphic = pPrevWin->CreateTempGraphic( pGraphic );
        pPreviewBox->Check( FALSE );
        ( (SvxImportGraphicDialog*)this )->PreviewHdl_Impl( (Button*)-1 );
        pPreviewBox->Enable();
    }

    return pGraphic;
}

// -----------------------------------------------------------------------

String SvxImportGraphicDialog::GetPath() const
{
    return SfxFileDialog::GetPath();
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxImportGraphicDialog, StandardHdl_Impl, Button *, EMPTYARG )
{
    SetPath( aStartPath, TRUE );
    return 0;
}
IMPL_LINK_INLINE_END( SvxImportGraphicDialog, StandardHdl_Impl, Button *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxImportGraphicDialog, PropertiesHdl_Impl, Button *, EMPTYARG )
{
    aPropertyLink.Call( this );
    return 0;
}
IMPL_LINK_INLINE_END( SvxImportGraphicDialog, PropertiesHdl_Impl, Button *, EMPTYARG )

// -----------------------------------------------------------------------

#ifdef WNT
#pragma optimize( "", off )
#endif

IMPL_LINK( SvxImportGraphicDialog, FilterHdl_Impl, Button *, EMPTYARG )
{
    GraphicFilter* pGraphicFilter = GetGrfFilter();
    USHORT nFormat = pGraphicFilter->GetImportFormatNumber( GetCurFilter() );

    if ( pGraphicFilter->DoImportDialog( this, nFormat ) )
        PreviewHdl_Impl( 0 );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxImportGraphicDialog, PreviewHdl_Impl, Button *, pButton )
{
    BOOL bPreview = FALSE;

    if ( pPreviewBox && pPreviewBox->IsChecked() )
    {
        if ( !bPreviewing )
        {
            WaitObject aWaitPtr( this );
            bPreview = TRUE;
            SvxGraphicHdl_Impl aGrfImp;
            aGrfImp.GraphicPreview( this );
        }
        else
            aPrevTimer.Start();
    }
    else
    {
        String aEmpty;
        bPreview = FALSE;
        pPrevWin->SetMessage( aEmpty );
        pPrevWin->SetGraphic( 0, aEmpty );
        // wieder zur"ucksetzen, damit das Kreuz auch richtig gemalt wird
        pPrevWin->SetMapMode( MAP_PIXEL );
    }

    if ( IsInExecute() )
        // wenn Preview "uber GetGraphic() eingeschaltet wurde, nicht merken
        SFX_APP()->PutItem( SfxBoolItem( SID_IMPORT_GRAPH_PREVIEW, bPreview ) );

    return 0;
}

#ifdef WNT
#pragma optimize( "", on )
#endif

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxImportGraphicDialog, TimeOutHdl_Impl, Timer *, EMPTYARG )
{
    PreviewHdl_Impl( 0 );
    return 0;
}
IMPL_LINK_INLINE_END( SvxImportGraphicDialog, TimeOutHdl_Impl, Timer *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK( SvxImportGraphicDialog, FilterSelectHdl_Impl, void*, EMPTYARG )
{
    if ( !pFilterButton )
        return 0;

    GraphicFilter* pGraphicFilter = GetGrfFilter();
    BOOL bEnable = pGraphicFilter->HasImportDialog(
        pGraphicFilter->GetImportFormatNumber( GetCurFilter() ) );

    if ( bEnable )
        pFilterButton->Enable();
    else
        pFilterButton->Disable();
    return 1;
}

// -----------------------------------------------------------------------

long SvxImportGraphicDialog::OK()
{
    if ( !bPreviewing )
        aPrevTimer.Stop();
    return (long)!bPreviewing;
}

// -----------------------------------------------------------------------

void SvxImportGraphicDialog::SetPreviewing( BOOL bPrev )
{
    bPreviewing = bPrev;

    if ( pPreviewBox )
    {
        if ( !bPreviewing )
            pPreviewBox->Enable();
        else
            pPreviewBox->Disable();
    }
}

// SvxGraphicHdl_Impl ----------------------------------------------------

SvxGraphicHdl_Impl::SvxGraphicHdl_Impl()
{
    pPreviewGrf = NULL;
}

// -----------------------------------------------------------------------

SvxGraphicHdl_Impl::~SvxGraphicHdl_Impl()
{
}

// -----------------------------------------------------------------------

void SvxGraphicHdl_Impl::GraphicPreview( SvxImportGraphicDialog* pDlg )
{
    SvxImportGraphicRes_Impl* pResImpl =
        new SvxImportGraphicRes_Impl( RID_SVXRES_IMPORTGRAPHIC );

    // Grafik einlesen
    aPreviewPath = pDlg->GetPath();
    SvStream* pStream = NULL;
    SfxMedium* pMed = NULL;
    INetURLObject aUrl( aPreviewPath );

    if ( aUrl.HasError() || INET_PROT_NOT_VALID == aUrl.GetProtocol() )
    {
        aUrl.SetSmartProtocol( INET_PROT_FILE );
        aUrl.SetSmartURL( aPreviewPath );
    }
    else if ( INET_PROT_FILE != aUrl.GetProtocol() )
    {
        pMed = new SfxMedium( aPreviewPath, STREAM_READ, TRUE );
        pMed->SetTransferPriority( SFX_TFPRIO_SYNCHRON );
        pMed->DownLoad();       // nur mal das Medium anfassen (DownLoaden)
        pStream = pMed->GetInStream();
    }

    if ( !pStream && IsFolder_Impl( aPreviewPath ) )
    {
        aPreviewPath.Erase();
        delete pResImpl;
        delete pMed;
        return;
    }

    // OK in FileDialog verweigern
    pDlg->SetPreviewing( TRUE );

    // Filter erkennen
    GraphicFilter& rGrf = pDlg->GetFilter();
    USHORT nRetFormat = GRFILTER_FORMAT_DONTKNOW;
    USHORT nFormatNum = rGrf.GetImportFormatNumber( pDlg->GetCurFilter() );
    USHORT nImpRet = GRFILTER_OK;

    if ( !pStream )
        nImpRet = rGrf.CanImportGraphic( aUrl, nFormatNum, &nRetFormat );
    else
        nImpRet = rGrf.CanImportGraphic( aPreviewPath, *pStream,
                                         nFormatNum, &nRetFormat );

    if ( GRFILTER_OK != nImpRet )
    {
        if ( !pStream )
            nImpRet = rGrf.CanImportGraphic( aUrl, GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
        else
            nImpRet = rGrf.CanImportGraphic( aPreviewPath, *pStream, GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
    }
    String sFmt( rGrf.GetImportFormatName( nRetFormat ) );
    USHORT nError = InsertGraphic( aPreviewPath, sFmt );

    // Format ist ungleich Current Filter, jetzt mit AutoDetection
    if( nError == GRFILTER_FORMATERROR )
        nError = InsertGraphic( aPreviewPath, String() );

    USHORT nErrInfo;
    BOOL bError = IsGraphicError_Impl( nError, nErrInfo );

    if ( bError )
    {
        pDlg->GetPreviewWindow().SetMessage( pResImpl->pStrings[nErrInfo] );
        pPreviewGrf = 0;
    }
    pDlg->GetPreviewWindow().SetGraphic( pPreviewGrf, aPreviewPath );
    delete pResImpl;
    delete pMed;
}

// -----------------------------------------------------------------------

int SvxGraphicHdl_Impl::InsertGraphic( const String &rPath, const String &rFilter )
{
    Graphic* pImpGrf = new Graphic;

    int nRes = LoadGraphic( rPath, rFilter, *pImpGrf );

    if ( nRes )
    {
        delete pImpGrf;
        pImpGrf = 0;
    }
    pPreviewGrf = pImpGrf;
    return nRes;
}

