/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif


#include "inspagob.hxx"

#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "ViewShell.hxx"
#include "inspagob.hrc"


/*************************************************************************
|*
|*  Ctor
|*
\************************************************************************/

SdInsertPagesObjsDlg::SdInsertPagesObjsDlg(
    ::Window* pWindow,
    const SdDrawDocument* pInDoc,
    SfxMedium* pSfxMedium,
    const String& rFileName )
    : ModalDialog     ( pWindow, SdResId( DLG_INSERT_PAGES_OBJS ) ),
      aLbTree         ( this, SdResId( LB_TREE ) ),
      aCbxLink        ( this, SdResId( CBX_LINK ) ),
      aCbxMasters     ( this, SdResId( CBX_CHECK_MASTERS ) ),
      aBtnOk          ( this, SdResId( BTN_OK ) ),
      aBtnCancel      ( this, SdResId( BTN_CANCEL ) ),
      aBtnHelp        ( this, SdResId( BTN_HELP ) ),
      pMedium       ( pSfxMedium ),
      mpDoc         ( pInDoc ),
      rName         ( rFileName )
{
    FreeResource();

    aLbTree.SetViewFrame( ( (SdDrawDocument*) pInDoc )->GetDocSh()->GetViewShell()->GetViewFrame() );

    aLbTree.SetSelectHdl( LINK( this, SdInsertPagesObjsDlg, SelectObjectHdl ) );

    // Text wird eingefuegt
    if( !pMedium )
        SetText( String( SdResId( STR_INSERT_TEXT ) ) );

    Reset();
}

/*************************************************************************
|*
|*  Dtor
|*
\************************************************************************/

SdInsertPagesObjsDlg::~SdInsertPagesObjsDlg()
{
}

/*************************************************************************
|*
|*  Fuellt die TreeLB in Abhaengigkeit des Mediums. Ist kein Medium
|*  vorhanden, handelt es sich um ein Text- und kein Drawdokument
|*
\************************************************************************/

void SdInsertPagesObjsDlg::Reset()
{
    if( pMedium )
    {
        aLbTree.SetSelectionMode( MULTIPLE_SELECTION );

        // transfer ownership of Medium
        aLbTree.Fill( mpDoc, pMedium, rName );
    }
    else
    {
        Color aColor( COL_WHITE );
        Bitmap aBmpText( SdResId( BMP_DOC_TEXT ) );
        Image aImgText( aBmpText, aColor );
        aLbTree.InsertEntry( rName, aImgText, aImgText );
    }

    aCbxMasters.Check( TRUE );
}

/*************************************************************************
|*
|*  Liefert die Liste zurueck
|*  nType == 0 -> Seiten
|*  nType == 1 -> Objekte
|*
\************************************************************************/

List* SdInsertPagesObjsDlg::GetList( USHORT nType )
{
    // Bei Draw-Dokumenten muss bei der Selektion des Dokumentes NULL
    // zurueckgegeben werden
    if( pMedium )
    {
        // Um zu gewaehrleisten, dass die Bookmarks geoeffnet sind
        // (Wenn gesamtes Dokument ausgewaehlt wurde)
        aLbTree.GetBookmarkDoc();

        // Wenn das Dokument (mit-)selektiert oder nichst selektiert ist,
        // wird das gesamte Dokument (und nicht mehr!) eingefuegt.
        if( aLbTree.GetSelectionCount() == 0 ||
            ( aLbTree.IsSelected( aLbTree.First() ) ) )
            return( NULL );
    }
    return( aLbTree.GetSelectEntryList( nType ) );
}

/*************************************************************************
|*
|*  Ist Verknuepfung gechecked
|*
\************************************************************************/

BOOL SdInsertPagesObjsDlg::IsLink()
{
    return( aCbxLink.IsChecked() );
}

/*************************************************************************
|*
|*  Ist Verknuepfung gechecked
|*
\************************************************************************/

BOOL SdInsertPagesObjsDlg::IsRemoveUnnessesaryMasterPages() const
{
    return( aCbxMasters.IsChecked() );
}

/*************************************************************************
|*
|* Enabled und selektiert Endfarben-LB
|*
\************************************************************************/

IMPL_LINK( SdInsertPagesObjsDlg, SelectObjectHdl, void *, EMPTYARG )
{
    if( aLbTree.IsLinkableSelected() )
        aCbxLink.Enable();
    else
        aCbxLink.Disable();

    return( 0 );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
