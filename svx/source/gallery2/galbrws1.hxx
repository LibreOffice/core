/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: galbrws1.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:37:03 $
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

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/menu.hxx>
#include <svtools/lstner.hxx>
#include <vector>
#include "galbrws.hxx"

// -----------------
// - GalleryButton -
// -----------------

class GalleryButton : public PushButton
{
private:

    virtual void    KeyInput( const KeyEvent& rKEvt );

public:

                    GalleryButton( GalleryBrowser1* pParent, WinBits nWinBits );
                    ~GalleryButton();
};

// -----------------------
// - GalleryThemeListBox -
// -----------------------

class GalleryThemeListBox : public ListBox
{
protected:

    void            InitSettings();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

public:

                    GalleryThemeListBox( GalleryBrowser1* pParent, WinBits nWinBits );
                    ~GalleryThemeListBox();
};

// -------------------
// - GalleryBrowser1 -
// -------------------

class Gallery;
class GalleryThemeEntry;
class GalleryTheme;
class VclAbstractDialog2;
struct ExchangeData;
class SfxItemSet;

class GalleryBrowser1 : public Control, SfxListener
{
    friend class GalleryBrowser;
    friend class GalleryThemeListBox;
    using Control::Notify;
    using Window::KeyInput;

private:

    GalleryButton           maNewTheme;
    GalleryThemeListBox*    mpThemes;
    Gallery*                mpGallery;
    ExchangeData*           mpExchangeData;
    SfxItemSet*             mpThemePropsDlgItemSet;

    Image                   aImgNormal;
    Image                   aImgDefault;
    Image                   aImgReadOnly;
    Image                   aImgImported;

    void                    ImplAdjustControls();
    ULONG                   ImplInsertThemeEntry( const GalleryThemeEntry* pEntry );
    void                    ImplFillExchangeData( const GalleryTheme* pThm, ExchangeData& rData );
    ::std::vector< USHORT > ImplGetExecuteVector();
    void                    ImplExecute( USHORT nId );
    void                    ImplGalleryThemeProperties( const String & rThemeName, bool bCreateNew );
    void                    ImplEndGalleryThemeProperties( VclAbstractDialog2* pDialog, bool bCreateNew );

    // Control
    virtual void            Resize();
    virtual void            GetFocus();

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            DECL_LINK( ClickNewThemeHdl, void* );
                            DECL_LINK( SelectThemeHdl, void* );
                            DECL_LINK( ShowContextMenuHdl, void* );
                            DECL_LINK( PopupMenuHdl, Menu* );
                            DECL_LINK( EndNewThemePropertiesDlgHdl, VclAbstractDialog2* );
                            DECL_LINK( EndThemePropertiesDlgHdl, VclAbstractDialog2* );
                            DECL_LINK( DestroyThemePropertiesDlgHdl, VclAbstractDialog2* );

public:

                            GalleryBrowser1( GalleryBrowser* pParent, const ResId& rResId, Gallery* pGallery );
                            ~GalleryBrowser1();

    void                    SelectTheme( const String& rThemeName ) { mpThemes->SelectEntry( rThemeName ); SelectThemeHdl( NULL ); }
    void                    SelectTheme( ULONG nThemePos ) { mpThemes->SelectEntryPos( (USHORT) nThemePos ); SelectThemeHdl( NULL ); }
    String                  GetSelectedTheme() { return mpThemes->GetEntryCount() ? mpThemes->GetEntry( mpThemes->GetSelectEntryPos() ) : String(); }

    void                    ShowContextMenu();
    BOOL                    KeyInput( const KeyEvent& rKEvt, Window* pWindow );
};
