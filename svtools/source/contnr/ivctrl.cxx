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
#include "precompiled_svtools.hxx"

#include <svtools/ivctrl.hxx>
#include "imivctl.hxx"
#include <vcl/bitmapex.hxx>
#include <vcl/controllayout.hxx>
#include <vcl/mnemonic.hxx>
using namespace ::com::sun::star::accessibility;

/*****************************************************************************
|
| class : SvxIconChoiceCtrlEntry
|
\*****************************************************************************/

SvxIconChoiceCtrlEntry::SvxIconChoiceCtrlEntry( const String& rText, const Image& rImage, sal_uInt16 _nFlags )
{
    aText = rText;
    aImage = rImage;
    aImageHC = rImage;
    pUserData = NULL;

    nFlags = _nFlags;
    eTextMode = IcnShowTextShort;
    pblink = 0;
    pflink = 0;
}

SvxIconChoiceCtrlEntry::SvxIconChoiceCtrlEntry( const String& rText, const Image& rImage, const Image& rImageHC, sal_uInt16 _nFlags )
{
    aText = rText;
    aImage = rImage;
    aImageHC = rImageHC;
    pUserData = NULL;

    nFlags = _nFlags;
    eTextMode = IcnShowTextShort;
    pblink = 0;
    pflink = 0;
}

SvxIconChoiceCtrlEntry::SvxIconChoiceCtrlEntry( sal_uInt16 _nFlags )
{
    pUserData = NULL;

    nFlags = _nFlags;
    eTextMode = IcnShowTextShort;
    pblink = 0;
    pflink = 0;
}

void SvxIconChoiceCtrlEntry::SetMoved( sal_Bool bMoved )
{
    if( bMoved )
        nFlags |= ICNVIEW_FLAG_POS_MOVED;
    else
        nFlags &= ~ICNVIEW_FLAG_POS_MOVED;
}

void SvxIconChoiceCtrlEntry::LockPos( sal_Bool bLock )
{
    if( bLock )
        nFlags |= ICNVIEW_FLAG_POS_LOCKED;
    else
        nFlags &= ~ICNVIEW_FLAG_POS_LOCKED;
}

/*sal_Unicode SvxIconChoiceCtrlEntry::GetMnemonicChar() const
{
    sal_Unicode cChar = 0;
    xub_StrLen nPos = aText.Search( '~' );
    if ( nPos != STRING_NOTFOUND && nPos < ( aText.Len() ) - 1 )
        cChar = aText.GetChar( nPos + 1 );
    return cChar;
}*/

String SvxIconChoiceCtrlEntry::GetDisplayText() const
{
    return MnemonicGenerator::EraseAllMnemonicChars( aText );
}

// ----------------------------------------------------------------------------

SvxIconChoiceCtrlColumnInfo::SvxIconChoiceCtrlColumnInfo( const SvxIconChoiceCtrlColumnInfo& rInfo )
    : aColText( rInfo.aColText ), aColImage( rInfo.aColImage )
{
    nWidth = rInfo.nWidth;
    eAlignment = rInfo.eAlignment;
    nSubItem = rInfo.nSubItem;
}

/*****************************************************************************
|
| class : SvtIconChoiceCtrl
|
\*****************************************************************************/

SvtIconChoiceCtrl::SvtIconChoiceCtrl( Window* pParent, WinBits nWinStyle ) :

     // WB_CLIPCHILDREN an, da ScrollBars auf dem Fenster liegen!
    Control( pParent, nWinStyle | WB_CLIPCHILDREN ),

    _pCurKeyEvent   ( NULL ),
    _pImp           ( new SvxIconChoiceCtrl_Impl( this, nWinStyle ) ),
    _bAutoFontColor ( sal_False )

{
    SetLineColor();
    _pImp->SetGrid( Size( 100, 70 ) );
    _pImp->InitSettings();
    _pImp->SetPositionMode( IcnViewPositionModeAutoArrange );
}

SvtIconChoiceCtrl::SvtIconChoiceCtrl( Window* pParent, const ResId& rResId ) :

    Control( pParent, rResId ),

    _pCurKeyEvent   ( NULL ),
    _pImp           ( new SvxIconChoiceCtrl_Impl( this, WB_BORDER ) ),
    _bAutoFontColor ( sal_False )

{
    SetLineColor();
    _pImp->SetGrid( Size( 100, 70 ) );
    _pImp->InitSettings();
    _pImp->SetPositionMode( IcnViewPositionModeAutoArrange );
}

SvtIconChoiceCtrl::~SvtIconChoiceCtrl()
{
    _pImp->CallEventListeners( VCLEVENT_OBJECT_DYING );
    delete _pImp;
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::InsertEntry( sal_uLong nPos, const Point* pPos, sal_uInt16 nFlags  )
{
    SvxIconChoiceCtrlEntry* pEntry = new SvxIconChoiceCtrlEntry( nFlags );
    _pImp->InsertEntry( pEntry, nPos, pPos );
    return pEntry;
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::InsertEntry( const String& rText, const Image& rImage, sal_uLong nPos, const Point* pPos, sal_uInt16 nFlags  )
{
    SvxIconChoiceCtrlEntry* pEntry = new SvxIconChoiceCtrlEntry( rText, rImage, nFlags);

    _pImp->InsertEntry( pEntry, nPos, pPos );

    return pEntry;
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::InsertEntry( const String& rText, const Image& rImage, const Image& rImageHC, sal_uLong nPos, const Point* pPos, sal_uInt16 nFlags  )
{
    SvxIconChoiceCtrlEntry* pEntry = new SvxIconChoiceCtrlEntry( rText, rImage, rImageHC, nFlags);

    _pImp->InsertEntry( pEntry, nPos, pPos );

    return pEntry;
}

sal_Bool SvtIconChoiceCtrl::EditedEntry( SvxIconChoiceCtrlEntry*, const XubString&, sal_Bool )
{
    return sal_True;
}
sal_Bool SvtIconChoiceCtrl::EditingEntry( SvxIconChoiceCtrlEntry* )
{
    return sal_True;
}
void SvtIconChoiceCtrl::DrawEntryImage( SvxIconChoiceCtrlEntry* pEntry, const Point& rPos, OutputDevice& rDev )
{
    rDev.DrawImage( rPos, GetSettings().GetStyleSettings().GetHighContrastMode() ? pEntry->GetImageHC() : pEntry->GetImage() );
}
String SvtIconChoiceCtrl::GetEntryText( SvxIconChoiceCtrlEntry* pEntry, sal_Bool )
{
    return pEntry->GetText();
}
sal_Bool SvtIconChoiceCtrl::HasBackground() const
{
    return sal_False;
}
sal_Bool SvtIconChoiceCtrl::HasFont() const
{
    return sal_False;
}
sal_Bool SvtIconChoiceCtrl::HasFontTextColor() const
{
    return sal_True;
}
sal_Bool SvtIconChoiceCtrl::HasFontFillColor() const
{
    return sal_True;
}

void SvtIconChoiceCtrl::Paint( const Rectangle& rRect )
{
    _pImp->Paint( rRect );
}

void SvtIconChoiceCtrl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( !_pImp->MouseButtonDown( rMEvt ) )
        Control::MouseButtonDown( rMEvt );
}

void SvtIconChoiceCtrl::MouseButtonUp( const MouseEvent& rMEvt )
{
    if( !_pImp->MouseButtonUp( rMEvt ) )
        Control::MouseButtonUp( rMEvt );
}

void SvtIconChoiceCtrl::MouseMove( const MouseEvent& rMEvt )
{
    if( !_pImp->MouseMove( rMEvt ) )
        Control::MouseMove( rMEvt );
}
void SvtIconChoiceCtrl::ArrangeIcons()
{
    if ( GetStyle() & WB_ALIGN_TOP )
    {
        Size aFullSize;
        Rectangle aEntryRect;

        for ( sal_uLong i = 0; i < GetEntryCount(); i++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = GetEntry ( i );
            aEntryRect = _pImp->GetEntryBoundRect ( pEntry );

            aFullSize.setWidth ( aFullSize.getWidth()+aEntryRect.GetWidth() );
        }

        _pImp->Arrange ( sal_False, aFullSize.getWidth() );
    }
    else if ( GetStyle() & WB_ALIGN_LEFT )
    {
        Size aFullSize;
        Rectangle aEntryRect;

        for ( sal_uLong i = 0; i < GetEntryCount(); i++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = GetEntry ( i );
            aEntryRect = _pImp->GetEntryBoundRect ( pEntry );

            aFullSize.setHeight ( aFullSize.getHeight()+aEntryRect.GetHeight() );
        }

        _pImp->Arrange ( sal_False, 0, aFullSize.getHeight() );
    }
    else
    {
        _pImp->Arrange();
    }
    _pImp->Arrange( sal_False, 0, 1000 );
}
void SvtIconChoiceCtrl::Resize()
{
    _pImp->Resize();
    Control::Resize();
}

Point SvtIconChoiceCtrl::GetLogicPos( const Point& rPosPixel ) const
{
    Point aPos( rPosPixel );
    aPos -= GetMapMode().GetOrigin();
    return aPos;
}

Point SvtIconChoiceCtrl::GetPixelPos( const Point& rPosLogic ) const
{
    Point aPos( rPosLogic );
    aPos += GetMapMode().GetOrigin();
    return aPos;
}

void SvtIconChoiceCtrl::DocumentRectChanged()
{
    _aDocRectChangedHdl.Call( this );
}

void SvtIconChoiceCtrl::VisibleRectChanged()
{
    _aVisRectChangedHdl.Call( this );
}

void SvtIconChoiceCtrl::GetFocus()
{
    _pImp->GetFocus();
    Control::GetFocus();
    sal_uLong nPos;
    SvxIconChoiceCtrlEntry* pSelectedEntry = GetSelectedEntry ( nPos );
    if ( pSelectedEntry )
        _pImp->CallEventListeners( VCLEVENT_LISTBOX_SELECT, pSelectedEntry );
}

void SvtIconChoiceCtrl::LoseFocus()
{
    _pImp->LoseFocus();
    Control::LoseFocus();
}

void SvtIconChoiceCtrl::SetUpdateMode( sal_Bool bUpdate )
{
    Control::SetUpdateMode( bUpdate );
    _pImp->SetUpdateMode( bUpdate );
}
void SvtIconChoiceCtrl::SetFont( const Font& rFont )
{
    if( rFont != GetFont() )
    {
        Control::SetFont( rFont );
        _pImp->FontModified();
    }
}

void SvtIconChoiceCtrl::SetPointFont( const Font& rFont )
{
    if( rFont != GetPointFont() )
    {
        Control::SetPointFont( rFont );
        _pImp->FontModified();
    }
}
SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetEntry( const Point& rPixPos, sal_Bool bHit ) const
{
    Point aPos( rPixPos );
    aPos -= GetMapMode().GetOrigin();
    return ((SvtIconChoiceCtrl*)this)->_pImp->GetEntry( aPos, bHit );
}

void SvtIconChoiceCtrl::SetStyle( WinBits nWinStyle )
{
    _pImp->SetStyle( nWinStyle );
}

WinBits SvtIconChoiceCtrl::GetStyle() const
{
    return _pImp->GetStyle();
}
void SvtIconChoiceCtrl::Command( const CommandEvent& rCEvt )
{
    _pImp->Command( rCEvt );
}

void SvtIconChoiceCtrl::SetEntryTextMode( SvxIconChoiceCtrlTextMode eMode, SvxIconChoiceCtrlEntry* pEntry )
{
    _pImp->SetEntryTextMode( eMode, pEntry );
}

SvxIconChoiceCtrlTextMode SvtIconChoiceCtrl::GetEntryTextMode( const SvxIconChoiceCtrlEntry* pEntry ) const
{
    return _pImp->GetEntryTextModeSmart( pEntry );
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetNextEntry( const Point& rPixPos, SvxIconChoiceCtrlEntry* pCurEntry, sal_Bool  ) const
{
    Point aPos( rPixPos );
    aPos -= GetMapMode().GetOrigin();
    return ((SvtIconChoiceCtrl*)this)->_pImp->GetNextEntry( aPos, pCurEntry );
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetPrevEntry( const Point& rPixPos, SvxIconChoiceCtrlEntry* pCurEntry, sal_Bool  ) const
{
    Point aPos( rPixPos );
    aPos -= GetMapMode().GetOrigin();
    return ((SvtIconChoiceCtrl*)this)->_pImp->GetPrevEntry( aPos, pCurEntry );
}
sal_uLong SvtIconChoiceCtrl::GetEntryCount() const
{
    return _pImp->GetEntryCount();
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetEntry( sal_uLong nPos ) const
{
    return _pImp->GetEntry( nPos );
}

void SvtIconChoiceCtrl::CreateAutoMnemonics( MnemonicGenerator& _rUsedMnemonics )
{
    _pImp->CreateAutoMnemonics( &_rUsedMnemonics );
}

void SvtIconChoiceCtrl::CreateAutoMnemonics( void )
{
    _pImp->CreateAutoMnemonics();
}

void SvtIconChoiceCtrl::RemoveEntry( SvxIconChoiceCtrlEntry* pEntry )
{
    _pImp->RemoveEntry( pEntry );
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetSelectedEntry( sal_uLong& rPos ) const
{
    return _pImp->GetFirstSelectedEntry( rPos );
}

void SvtIconChoiceCtrl::ClickIcon()
{
    sal_uLong nPos;
    GetSelectedEntry ( nPos );
    _aClickIconHdl.Call( this );
}
sal_Bool SvtIconChoiceCtrl::IsEntryEditing() const
{
    return _pImp->IsEntryEditing();
}

sal_Bool SvtIconChoiceCtrl::SetChoiceWithCursor ( sal_Bool bDo )
{
    return _pImp->SetChoiceWithCursor (bDo);
}

void SvtIconChoiceCtrl::KeyInput( const KeyEvent& rKEvt )
{
    sal_Bool bKeyUsed = DoKeyInput( rKEvt );
    if ( !bKeyUsed )
    {
        _pCurKeyEvent = (KeyEvent*)&rKEvt;
        Control::KeyInput( rKEvt );
        _pCurKeyEvent = NULL;
    }
}
sal_Bool SvtIconChoiceCtrl::DoKeyInput( const KeyEvent& rKEvt )
{
    // unter OS/2 bekommen wir auch beim Editieren Key-Up/Down
    if( IsEntryEditing() )
        return sal_True;
    _pCurKeyEvent = (KeyEvent*)&rKEvt;
    sal_Bool bHandled = _pImp->KeyInput( rKEvt );
    _pCurKeyEvent = NULL;
    return bHandled;
}
sal_uLong SvtIconChoiceCtrl::GetEntryListPos( SvxIconChoiceCtrlEntry* pEntry ) const
{
    return _pImp->GetEntryListPos( pEntry );
}
SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetCursor( ) const
{
    return _pImp->GetCurEntry( );
}
void SvtIconChoiceCtrl::SetCursor( SvxIconChoiceCtrlEntry* pEntry )
{
    _pImp->SetCursor( pEntry );
}
void SvtIconChoiceCtrl::InvalidateEntry( SvxIconChoiceCtrlEntry* pEntry )
{
    _pImp->InvalidateEntry( pEntry );
}
void SvtIconChoiceCtrl::Clear()
{
    _pImp->Clear();
}
void SvtIconChoiceCtrl::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );
}


void SvtIconChoiceCtrl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ((rDCEvt.GetType() == DATACHANGED_SETTINGS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         (rDCEvt.GetType() == DATACHANGED_FONTS) ) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        _pImp->InitSettings();
        Invalidate(INVALIDATE_NOCHILDREN);
    }
    else
        Control::DataChanged( rDCEvt );
}

void SvtIconChoiceCtrl::SetBackground( const Wallpaper& rPaper )
{
    if( rPaper != GetBackground() )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Wallpaper aEmpty;
        if( rPaper == aEmpty )
            Control::SetBackground( rStyleSettings.GetFieldColor() );
        else
        {
            Wallpaper aBackground( rPaper );
            // HACK, da Hintergrund sonst transparent sein koennte
            if( !aBackground.IsBitmap() )
                aBackground.SetStyle( WALLPAPER_TILE );

            WallpaperStyle eStyle = aBackground.GetStyle();
            Color aBack( aBackground.GetColor());
            Color aTrans( COL_TRANSPARENT );
            if( aBack == aTrans  && (
                (!aBackground.IsBitmap() ||
                    aBackground.GetBitmap().IsTransparent() ||
                    (eStyle != WALLPAPER_TILE && eStyle != WALLPAPER_SCALE))))
            {
                aBackground.SetColor( rStyleSettings.GetFieldColor() );
            }
            if( aBackground.IsScrollable() )
            {
                Rectangle aRect;
                aRect.SetSize( Size(32765, 32765) );
                aBackground.SetRect( aRect );
            }
            else
            {
                Rectangle aRect( _pImp->GetOutputRect() );
                aBackground.SetRect( aRect );
            }
            Control::SetBackground( aBackground );
        }

        // bei hart attributierter Textfarbe keine 'Automatik', die eine
        // lesbare Textfarbe einstellt.
        Font aFont( GetFont() );
        aFont.SetColor( rStyleSettings.GetFieldTextColor() );
        SetFont( aFont );

        Invalidate(INVALIDATE_NOCHILDREN);
    }
}

void SvtIconChoiceCtrl::Flush()
{
    _pImp->Flush();
}

void SvtIconChoiceCtrl::RequestHelp( const HelpEvent& rHEvt )
{
    if ( !_pImp->RequestHelp( rHEvt ) )
        Control::RequestHelp( rHEvt );
}

void SvtIconChoiceCtrl::SetSelectionMode( SelectionMode eMode )
{
    _pImp->SetSelectionMode( eMode );
}

sal_Bool SvtIconChoiceCtrl::HandleShortCutKey( const KeyEvent& r )
{
    return _pImp->HandleShortCutKey( r );
}

Rectangle SvtIconChoiceCtrl::GetBoundingBox( SvxIconChoiceCtrlEntry* pEntry ) const
{
    return _pImp->GetEntryBoundRect( pEntry );
}

void SvtIconChoiceCtrl::FillLayoutData() const
{
    CreateLayoutData();

    SvtIconChoiceCtrl* pNonConstMe = const_cast< SvtIconChoiceCtrl* >( this );

    // loop through all entries
    sal_uInt16 nCount = (sal_uInt16)GetEntryCount();
    sal_uInt16 nPos = 0;
    while ( nPos < nCount )
    {
        SvxIconChoiceCtrlEntry* pEntry = GetEntry( nPos );

        Point aPos = _pImp->GetEntryBoundRect( pEntry ).TopLeft();
        String sEntryText = pEntry->GetDisplayText( );
        Rectangle aTextRect = _pImp->CalcTextRect( pEntry, &aPos, sal_False, &sEntryText );

        sal_Bool bLargeIconMode = WB_ICON == ( _pImp->GetStyle() & ( VIEWMODE_MASK ) );
        sal_uInt16 nTextPaintFlags = bLargeIconMode ? PAINTFLAG_HOR_CENTERED : PAINTFLAG_VER_CENTERED;

        _pImp->PaintItem( aTextRect, IcnViewFieldTypeText, pEntry, nTextPaintFlags, pNonConstMe, &sEntryText, GetLayoutData() );

        ++nPos;
    }
}

Rectangle SvtIconChoiceCtrl::GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const
{
    Rectangle aRect;

    Pair aEntryCharacterRange = GetLineStartEnd( _nEntryPos );
    if ( aEntryCharacterRange.A() + _nCharacterIndex < aEntryCharacterRange.B() )
    {
        aRect = GetCharacterBounds( aEntryCharacterRange.A() + _nCharacterIndex );
    }

    return aRect;
}

void SvtIconChoiceCtrl::SetNoSelection()
{
    _pImp->SetNoSelection();
}

void SvtIconChoiceCtrl::CallImplEventListeners(sal_uLong nEvent, void* pData)
{
    CallEventListeners(nEvent, pData);
}
::com::sun::star::uno::Reference< XAccessible > SvtIconChoiceCtrl::CreateAccessible()
{
    Window* pParent = GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "SvTreeListBox::CreateAccessible - accessible parent not found" );

    ::com::sun::star::uno::Reference< XAccessible > xAccessible;
    if ( pParent )
    {
        ::com::sun::star::uno::Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if ( xAccParent.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xTemp(GetComponentInterface());
            xAccessible = _pImp->GetAccessibleFactory().createAccessibleIconChoiceCtrl( *this, xAccParent );
        }
    }
    return xAccessible;
}

