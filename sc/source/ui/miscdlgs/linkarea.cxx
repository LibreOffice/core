/*************************************************************************
 *
 *  $RCSfile: linkarea.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:24:04 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/waitobj.hxx>

#include "linkarea.hxx"
#include "linkarea.hrc"
#include "scresid.hxx"
#include "sc.hrc"
#include "rangeutl.hxx"
#include "docsh.hxx"
#include "tablink.hxx"

//==================================================================

ScLinkedAreaDlg::ScLinkedAreaDlg( Window* pParent ) :
    ModalDialog ( pParent, ScResId( RID_SCDLG_LINKAREA ) ),
    //
    aFlLocation ( this, ScResId( FL_LOCATION ) ),
    aCbUrl      ( this, ScResId( CB_URL ) ),
    aBtnBrowse  ( this, ScResId( BTN_BROWSE ) ),
    aTxtHint    ( this, ScResId( FT_HINT ) ),
    aFtRanges   ( this, ScResId( FT_RANGES ) ),
    aLbRanges   ( this, ScResId( LB_RANGES ) ),
    aBtnReload  ( this, ScResId( BTN_RELOAD ) ),
    aNfDelay    ( this, ScResId( NF_DELAY ) ),
    aFtSeconds  ( this, ScResId( FT_SECONDS ) ),
    aBtnOk      ( this, ScResId( BTN_OK ) ),
    aBtnCancel  ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp    ( this, ScResId( BTN_HELP ) ),
    //
    pSourceShell( NULL )
{
    FreeResource();

    aCbUrl.SetHelpId( HID_SCDLG_LINKAREAURL );  // SvtURLBox ctor always sets SID_OPENURL
    aCbUrl.SetSelectHdl( LINK( this, ScLinkedAreaDlg, FileHdl ) );
    aBtnBrowse.SetClickHdl( LINK( this, ScLinkedAreaDlg, BrowseHdl ) );
    aLbRanges.SetSelectHdl( LINK( this, ScLinkedAreaDlg, RangeHdl ) );
    aBtnReload.SetClickHdl( LINK( this, ScLinkedAreaDlg, ReloadHdl ) );
    UpdateEnable();
}

ScLinkedAreaDlg::~ScLinkedAreaDlg()
{
    // pSourceShell is deleted by aSourceRef
}

short ScLinkedAreaDlg::Execute()
{
    // set parent for file dialog or filter options

    Window* pOldDefParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );

    short nRet = ModalDialog::Execute();

    Application::SetDefDialogParent( pOldDefParent );

    return nRet;
}

IMPL_LINK( ScLinkedAreaDlg, BrowseHdl, PushButton*, EMPTYARG )
{
    //  dialog parent has been set in execute

    SfxApplication* pApp = SFX_APP();
    SfxMedium* pMed = pApp->InsertDocumentDialog( 0, String::CreateFromAscii( ScDocShell::Factory().GetShortName() ) );

    if ( pMed )
    {
        WaitObject aWait( this );

        // #92296# replace HTML filter with DataQuery filter
        const String aHTMLFilterName( RTL_CONSTASCII_USTRINGPARAM( "HTML (StarCalc)" ) );
        const String aWebQFilterName( RTL_CONSTASCII_USTRINGPARAM( "calc_HTML_WebQuery" ) );

        const SfxFilter* pFilter = pMed->GetFilter();
        if( pFilter && (pFilter->GetFilterName() == aHTMLFilterName) )
        {
            const SfxFilter* pNewFilter = pApp->GetFilter( ScDocShell::Factory(), aWebQFilterName );
            if( pNewFilter )
                pMed->SetFilter( pNewFilter );
        }

        //  ERRCTX_SFX_OPENDOC -> "Fehler beim Laden des Dokumentes"
        SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, pMed->GetName() );

        if (pSourceShell)
            pSourceShell->DoClose();        // deleted when assigning aSourceRef

        pMed->UseInteractionHandler( TRUE );    // to enable the filter options dialog

        pSourceShell = new ScDocShell;
        aSourceRef = pSourceShell;
        pSourceShell->DoLoad( pMed );

        ULONG nErr = pSourceShell->GetErrorCode();
        if (nErr)
            ErrorHandler::HandleError( nErr );              // including warnings

        if ( !pSourceShell->GetError() )                    // only errors
        {
            //aCbUrl.SetText( pSourceShell->GetTitle( SFX_TITLE_FULLNAME ) );
            aCbUrl.SetText( pMed->GetName() );
        }
        else
        {
            pSourceShell->DoClose();
            pSourceShell = NULL;
            aSourceRef.Clear();

            aCbUrl.SetText( EMPTY_STRING );
        }
    }

    UpdateSourceRanges();
    UpdateEnable();
    return 0;
}

IMPL_LINK( ScLinkedAreaDlg, FileHdl, ComboBox*, EMPTYARG )
{
    String aEntered = aCbUrl.GetURL();
    if (pSourceShell)
    {
        SfxMedium* pMed = pSourceShell->GetMedium();
        if ( pMed->GetName() == aEntered )
        {
            //  already loaded - nothing to do
            return 0;
        }
    }

    String aFilter;
    String aOptions;
    //  get filter name by looking at the file content (bWithContent = TRUE)
    ScDocumentLoader::GetFilterName( aEntered, aFilter, aOptions, TRUE );

    LoadDocument( aEntered, aFilter, aOptions );

    UpdateSourceRanges();
    UpdateEnable();
    return 0;
}

void ScLinkedAreaDlg::LoadDocument( const String& rFile, const String& rFilter, const String& rOptions )
{
    if ( pSourceShell )
    {
        //  unload old document
        pSourceShell->DoClose();
        pSourceShell = NULL;
        aSourceRef.Clear();
    }

    if ( rFile.Len() )
    {
        WaitObject aWait( this );

        String aNewFilter = rFilter;
        String aNewOptions = rOptions;

        SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, rFile );

        ScDocumentLoader aLoader( rFile, aNewFilter, aNewOptions, 0, TRUE );    // with interaction
        pSourceShell = aLoader.GetDocShell();
        if ( pSourceShell )
        {
            ULONG nErr = pSourceShell->GetErrorCode();
            if (nErr)
                ErrorHandler::HandleError( nErr );      // including warnings

            aSourceRef = pSourceShell;
            aLoader.ReleaseDocRef();    // don't call DoClose in DocLoader dtor
        }
    }
}

void ScLinkedAreaDlg::InitFromOldLink( const String& rFile, const String& rFilter,
                                        const String& rOptions, const String& rSource,
                                        ULONG nRefresh )
{
    LoadDocument( rFile, rFilter, rOptions );
    if (pSourceShell)
    {
        SfxMedium* pMed = pSourceShell->GetMedium();
        aCbUrl.SetText( pMed->GetName() );
    }
    else
        aCbUrl.SetText( EMPTY_STRING );

    UpdateSourceRanges();

    xub_StrLen nRangeCount = rSource.GetTokenCount();
    for ( xub_StrLen i=0; i<nRangeCount; i++ )
    {
        String aRange = rSource.GetToken(i);
        aLbRanges.SelectEntry( aRange );
    }

    BOOL bDoRefresh = ( nRefresh != 0 );
    aBtnReload.Check( bDoRefresh );
    if (bDoRefresh)
        aNfDelay.SetValue( nRefresh );

    UpdateEnable();
}

IMPL_LINK( ScLinkedAreaDlg, RangeHdl, MultiListBox*, EMPTYARG )
{
    UpdateEnable();
    return 0;
}

IMPL_LINK( ScLinkedAreaDlg, ReloadHdl, CheckBox*, EMPTYARG )
{
    UpdateEnable();
    return 0;
}

void ScLinkedAreaDlg::UpdateSourceRanges()
{
    aLbRanges.SetUpdateMode( FALSE );

    aLbRanges.Clear();
    if ( pSourceShell )
    {
        ScAreaNameIterator aIter( pSourceShell->GetDocument() );
        ScRange aDummy;
        String aName;
        while ( aIter.Next( aName, aDummy ) )
            aLbRanges.InsertEntry( aName );
    }

    aLbRanges.SetUpdateMode( TRUE );

    if ( aLbRanges.GetEntryCount() == 1 )
        aLbRanges.SelectEntryPos(0);
}

void ScLinkedAreaDlg::UpdateEnable()
{
    BOOL bEnable = ( pSourceShell && aLbRanges.GetSelectEntryCount() );
    aBtnOk.Enable( bEnable );

    BOOL bReload = aBtnReload.IsChecked();
    aNfDelay.Enable( bReload );
    aFtSeconds.Enable( bReload );
}

String ScLinkedAreaDlg::GetURL()
{
    if (pSourceShell)
    {
        SfxMedium* pMed = pSourceShell->GetMedium();
        return pMed->GetName();
    }
    return EMPTY_STRING;
}

String ScLinkedAreaDlg::GetFilter()
{
    if (pSourceShell)
    {
        SfxMedium* pMed = pSourceShell->GetMedium();
        return pMed->GetFilter()->GetFilterName();
    }
    return EMPTY_STRING;
}

String ScLinkedAreaDlg::GetOptions()
{
    if (pSourceShell)
    {
        SfxMedium* pMed = pSourceShell->GetMedium();
        return ScDocumentLoader::GetOptions( *pMed );
    }
    return EMPTY_STRING;
}

String ScLinkedAreaDlg::GetSource()
{
    String aSource;
    USHORT nCount = aLbRanges.GetSelectEntryCount();
    for (USHORT i=0; i<nCount; i++)
    {
        if (i > 0)
            aSource.Append( (sal_Unicode) ';' );
        aSource.Append( aLbRanges.GetSelectEntry( i ) );
    }
    return aSource;
}

ULONG ScLinkedAreaDlg::GetRefresh()
{
    if ( aBtnReload.IsChecked() )
        return aNfDelay.GetValue();
    else
        return 0;   // disabled
}

