/*************************************************************************
 *
 *  $RCSfile: inspagob.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:33 $
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

#pragma hdrstop

#include "inspagob.hxx"

#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "viewshel.hxx"
#include "inspagob.hrc"


/*************************************************************************
|*
|*  Ctor
|*
\************************************************************************/

SdInsertPagesObjsDlg::SdInsertPagesObjsDlg( Window* pWindow,
                                            const SdDrawDocument* pInDoc,
                                            SfxMedium* pSfxMedium,
                                            const String& rFileName ) :
            ModalDialog     ( pWindow, SdResId( DLG_INSERT_PAGES_OBJS ) ),
        aLbTree         ( this, SdResId( LB_TREE ) ),
        aBtnOk          ( this, SdResId( BTN_OK ) ),
        aBtnCancel      ( this, SdResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, SdResId( BTN_HELP ) ),
        aCbxLink        ( this, SdResId( CBX_LINK ) ),
        aCbxMasters     ( this, SdResId( CBX_CHECK_MASTERS ) ),
        pDoc            ( pInDoc ),
        pMedium         ( pSfxMedium ),
        rName           ( rFileName )
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
        aLbTree.Fill( pDoc, pMedium, rName );
    }
    else
    {
        Color aColor( COL_WHITE );
        Bitmap aBitmap( SdResId( BMP_DOC_TEXT ) );
        Image aImgText( aBitmap, aColor );
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
            //return( aLbTree.GetBookmarkList( nType ) );
            return( NULL ); // #37350#
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

IMPL_LINK( SdInsertPagesObjsDlg, SelectObjectHdl, void *, p )
{
    if( aLbTree.IsOleSelected() ||
        aLbTree.IsGraphicSelected() )
        aCbxLink.Disable();
    else
        aCbxLink.Enable();

    return( 0 );
}


