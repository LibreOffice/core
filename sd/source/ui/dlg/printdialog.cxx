/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: printdialog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:30:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include <boost/scoped_ptr.hpp>

#include "sdresid.hxx"
#include "printdialog.hxx"
#include "printdialog.hrc"
#include "optsitem.hxx"
#include "res_bmp.hrc"

class SdPrintDialogImpl : public ModalDialog
{
public:
    SdPrintDialogImpl()
    : ModalDialog( 0, SdResId( DLG_PRINTDIALOG ) )
    {
    }

    virtual ~SdPrintDialogImpl()
    {
        FreeResource();
    }
};

/**
        aOptions.GetOptionsPrint().SetDraw( aCbxDraw.IsChecked() );
        aOptions.GetOptionsPrint().SetNotes( aCbxNotes.IsChecked() );
        aOptions.GetOptionsPrint().SetHandout( aCbxHandout.IsChecked() );
        aOptions.GetOptionsPrint().SetOutline( aCbxOutline.IsChecked() );

        aCbxDraw.Check(              pPrintOpts->GetOptionsPrint().IsDraw() );
        aCbxNotes.Check(             pPrintOpts->GetOptionsPrint().IsNotes() );
        aCbxHandout.Check(           pPrintOpts->GetOptionsPrint().IsHandout() );
        aCbxOutline.Check(           pPrintOpts->GetOptionsPrint().IsOutline() );

    only for impress!!
*/

SdPrintDialog* SdPrintDialog::Create( Window* pWindow, bool bImpress )
{
    return new SdPrintDialog( pWindow, bImpress );
}

SdPrintDialog::SdPrintDialog( Window* pWindow, bool bImpress )
: PrintDialog( pWindow, false )
, mbImpress( bImpress )
{
    if( bImpress )
    {
        mpControls.resize( PRINTDLG_CONTROLCOUNT );

        boost::scoped_ptr< SdPrintDialogImpl > aImplDlg( new SdPrintDialogImpl() );

        USHORT nChildCount = GetChildCount();

        mpControls[FL_PRINTCONTENT-1].reset( new FixedLine( this, SdResId( FL_PRINTCONTENT ) ) );
        mpControls[FT_CONTENT-1].reset( new FixedText( this, SdResId( FT_CONTENT ) ) );
        mpControls[CB_CONTENT-1].reset( new ListBox( this, SdResId( CB_CONTENT ) ) );
        mpControls[FT_SLIDESPERPAGE-1].reset( new FixedText( this, SdResId( FT_SLIDESPERPAGE ) ) );
        mpControls[CB_SLIDESPERPAGE-1].reset( new ListBox( this, SdResId( CB_SLIDESPERPAGE ) ) );
        mpControls[FT_ORDER-1].reset( new FixedText( this, SdResId( FT_ORDER ) ) );
        mpControls[RBT_HORIZONTAL-1].reset( new RadioButton( this, SdResId( RBT_HORIZONTAL ) ) );
        mpControls[RBT_VERTICAL-1].reset( new RadioButton( this, SdResId( RBT_VERTICAL ) ) );
        mpControls[IMG_PREVIEW-1].reset( new FixedImage( this, SdResId( IMG_PREVIEW ) ) );

        sal_Int32 nHeight = LogicToPixel( Size( 0, 71 ), MAP_APPFONT ).Height();

        sal_Int32 nInsertY = nHeight;

        Window* pNextControl = 0;

        // find second fixed line
        USHORT nChild = 0;
        USHORT nFixedLine = 2;
        while( nChildCount-- )
        {
            pNextControl = GetChild( nChild++ );
            if( dynamic_cast< FixedLine* >( pNextControl ) )
            {
                if( --nFixedLine == 0 )
                {
                    nInsertY = pNextControl->GetPosPixel().Y();
                    nChild--;
                    nChildCount++;
                    break;
                }
            }
        }

        // move lower controls down
        std::vector< Window* > aLowerControls;
        while( nChildCount-- )
        {
            Window* pControl = GetChild( nChild++ );
            if( pControl )
            {
                Point aPos( pControl->GetPosPixel() );
                aPos.Y() += nHeight;
                pControl->SetPosPixel(aPos);
                aLowerControls.push_back(pControl);
            }
        }

        // positionate new controls
        for( int i = 0; i < PRINTDLG_CONTROLCOUNT; i++ )
        {
            Control* pControl = mpControls[i].get();
            if( pControl )
            {
                Point aPos( pControl->GetPosPixel() );
                aPos.Y() += nInsertY;
                pControl->SetPosPixel(aPos);
                pControl->SetZOrder( pNextControl, WINDOW_ZORDER_BEFOR );
            }
        }

        Size aDialogSize( GetSizePixel() );
        aDialogSize.Height() += nHeight;
        SetSizePixel( aDialogSize );
    }
}

SdPrintDialog::~SdPrintDialog()
{
}

void SdPrintDialog::Init( const SdOptionsPrintItem* pPrintOpts )
{
    if( mbImpress )
    {
        bool bDraw = true, bHandout = false, bNotes = false, bOutline = false;
        sal_uInt16 nPagesPerHandout = 6;
        bool bHandoutHorizontal = true;

        if( pPrintOpts )
        {
            const SdOptionsPrint& rOpts = pPrintOpts->GetOptionsPrint();
            bDraw = rOpts.IsDraw();
            bHandout = rOpts.IsHandout();
            bNotes = rOpts.IsNotes();
            bOutline = rOpts.IsOutline();
            nPagesPerHandout = rOpts.GetHandoutPages();
            bHandoutHorizontal = rOpts.IsHandoutHorizontal();
        }

        sal_uInt16 nPos = 0;
        if( !bDraw )
        {
            if( bHandout )
                nPos = 1;
            else if( bNotes )
                nPos = 2;
            else if( bOutline )
                nPos = 3;
        }
        static_cast<ListBox*>( mpControls[CB_CONTENT-1].get() )->SelectEntryPos( nPos );

        switch( nPagesPerHandout )
        {
        case 1: nPos = 0; break;
        case 2: nPos = 1; break;
        case 3: nPos = 2; break;
        case 4: nPos = 3; break;
        case 5:
        case 6: nPos = 4; break;
        default:nPos = 5; break;
        }
        static_cast<ListBox*>( mpControls[CB_SLIDESPERPAGE-1].get() )->SelectEntryPos( nPos );


        static_cast<RadioButton*>( mpControls[RBT_HORIZONTAL-1].get() )->Check( bHandoutHorizontal == true ) ;
        static_cast<RadioButton*>( mpControls[RBT_VERTICAL-1].get() )->Check( bHandoutHorizontal == false ) ;

        LoadPreviewImages();

        Image* pImage = mpPreviews[0].get();
        if( pImage )
        {
            FixedImage* pPreviewImage = static_cast<FixedImage*>( mpControls[IMG_PREVIEW-1].get() );

            Point aPos( pPreviewImage->GetPosPixel() );
            pPreviewImage->SetSizePixel( pImage->GetSizePixel() );
            aPos.X() -= pImage->GetSizePixel().Width();

            pPreviewImage->SetPosPixel( aPos );
        }

        UpdateStates();

        Link aLink( LINK(this,SdPrintDialog,UpdateStatesHdl) );
        static_cast<ListBox*>( mpControls[CB_CONTENT-1].get() )->SetSelectHdl( aLink );
        static_cast<ListBox*>( mpControls[CB_SLIDESPERPAGE-1].get() )->SetSelectHdl( aLink );
    }
}

bool SdPrintDialog::Fill( SdOptionsPrintItem* pPrintOpts )
{
    bool bChanges = false;
    if( mbImpress && pPrintOpts )
    {
        SdOptionsPrint& rOpts = pPrintOpts->GetOptionsPrint();

        BOOL bDraw = FALSE, bHandout = FALSE, bNotes = FALSE, bOutline = FALSE;
        switch( static_cast<ListBox*>( mpControls[CB_CONTENT-1].get() )->GetSelectEntryPos() )
        {
        case 1: bHandout = TRUE; break;
        case 2: bNotes = TRUE; break;
        case 3: bOutline = TRUE; break;
        default: bDraw = TRUE; break;
        }
        if( rOpts.IsDraw() != bDraw )
        {
            bChanges = true;
            rOpts.SetDraw( bDraw );
        }

        if( rOpts.IsNotes() != bNotes )
        {
            bChanges = true;
            rOpts.SetNotes( bNotes );
        }

        if( rOpts.IsHandout() != bHandout )
        {
            bChanges = true;
            rOpts.SetHandout( bHandout );
        }

        if( rOpts.IsOutline() != bOutline )
        {
            bChanges = true;
            rOpts.SetOutline( bOutline );
        }

        sal_uInt16 nSlidePerHandout = 6;
        switch( static_cast<ListBox*>( mpControls[CB_SLIDESPERPAGE-1].get() )->GetSelectEntryPos() )
        {
        case 0: nSlidePerHandout = 1; break;
        case 1: nSlidePerHandout = 2; break;
        case 2: nSlidePerHandout = 3; break;
        case 3: nSlidePerHandout = 4; break;
        case 5: nSlidePerHandout = 9; break;
        }

        if( rOpts.GetHandoutPages() != nSlidePerHandout )
        {
            bChanges = true;
            rOpts.SetHandoutPages( nSlidePerHandout );
        }

        const BOOL bHorizontal = static_cast<RadioButton*>( mpControls[RBT_HORIZONTAL-1].get() )->IsChecked();
        if( rOpts.IsHandoutHorizontal() != bHorizontal )
        {
            bChanges = true;
            rOpts.SetHandoutHorizontal( bHorizontal );
        }
    }
    return bChanges;
}

static const sal_uInt16 PREVIEW_COUNT = 6;

void SdPrintDialog::UpdateStates()
{
    const sal_uInt16 nSlidesPerPagePos = static_cast<ListBox*>( mpControls[CB_SLIDESPERPAGE-1].get() )->GetSelectEntryPos();
    bool bHandout = static_cast<ListBox*>( mpControls[CB_CONTENT-1].get() )->GetSelectEntryPos() == 1;
    bool bOrder = bHandout && ( nSlidesPerPagePos >= 3);

    static_cast<Control*>( mpControls[FT_SLIDESPERPAGE-1].get() )->Enable( bHandout );
    static_cast<Control*>( mpControls[CB_SLIDESPERPAGE-1].get() )->Enable( bHandout );
    static_cast<Control*>( mpControls[FT_ORDER-1].get() )->Enable( bHandout && bOrder );
    static_cast<Control*>( mpControls[RBT_HORIZONTAL-1].get() )->Enable( bHandout && bOrder );
    static_cast<Control*>( mpControls[RBT_VERTICAL-1].get() )->Enable( bHandout && bOrder );

    FixedImage* pPreviewImage = static_cast<FixedImage*>( mpControls[IMG_PREVIEW-1].get() );

    if( pPreviewImage )
    {
        pPreviewImage->Show( bHandout );
        if( bHandout )
        {
            if( nSlidesPerPagePos < PREVIEW_COUNT)
            {
                if( mpPreviews[nSlidesPerPagePos].get() )
                    pPreviewImage->SetModeImage( *mpPreviews[nSlidesPerPagePos].get(), BMP_COLOR_NORMAL );

                if( mpPreviewsHC[nSlidesPerPagePos].get() )
                    pPreviewImage->SetModeImage( *mpPreviewsHC[nSlidesPerPagePos].get(), BMP_COLOR_HIGHCONTRAST );
            }
        }
    }
}

IMPL_LINK( SdPrintDialog, UpdateStatesHdl, void*, EMPTYARG )
{
    UpdateStates();
    return 0;
}

void SdPrintDialog::LoadPreviewImages()
{
    static const sal_uInt16 aResId[2*PREVIEW_COUNT] = { BMP_FOILH_01, BMP_FOILH_01_H, BMP_FOILH_02, BMP_FOILH_02_H, BMP_FOILH_03, BMP_FOILH_03_H, BMP_FOILH_04, BMP_FOILH_04_H, BMP_FOILH_06, BMP_FOILH_06_H, BMP_FOILH_09, BMP_FOILH_09_H };

    mpPreviews.resize( PREVIEW_COUNT );
    mpPreviewsHC.resize( PREVIEW_COUNT );

    const sal_uInt16* pResId( aResId );
    for( sal_uInt16 i = 0; i < PREVIEW_COUNT; i++ )
    {
        mpPreviews[i].reset( new Image( Bitmap( SdResId( *pResId++ ) ) ) );
        mpPreviewsHC[i].reset( new Image( Bitmap(  SdResId( *pResId++ ) ) ) );
    }
}

