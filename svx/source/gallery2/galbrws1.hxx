/*************************************************************************
 *
 *  $RCSfile: galbrws1.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-14 13:23:43 $
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

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/menu.hxx>
#include "galbrws.hxx"

// -----------------------
// - GalleryThemeListBox -
// -----------------------

class GalleryThemeListBox : public ListBox
{
private:

protected:

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
struct ExchangeData;

class GalleryBrowser1 : public Control, SfxListener
{
    friend class GalleryThemeListBox;

private:

    PushButton              maNewTheme;
    GalleryThemeListBox*    mpThemes;
    Gallery*                mpGallery;

    void                    ImplAdjustControls();
    ULONG                   ImplInsertThemeEntry( const GalleryThemeEntry* pEntry );
    void                    ImplFillExchangeData( const GalleryTheme* pThm, ExchangeData& rData );

    // Control
    virtual void            Resize();

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            DECL_LINK( ClickNewThemeHdl, void* );
                            DECL_LINK( SelectThemeHdl, void* );
                            DECL_LINK( ShowContextMenuHdl, void* );
                            DECL_LINK( PopupMenuHdl, Menu* );

public:

                            GalleryBrowser1( GalleryBrowser* pParent, const ResId& rResId, Gallery* pGallery );
                            ~GalleryBrowser1();

    void                    SelectTheme( const String& rThemeName ) { mpThemes->SelectEntry( rThemeName ); SelectThemeHdl( NULL ); }
    void                    SelectTheme( ULONG nThemePos ) { mpThemes->SelectEntryPos( nThemePos ); SelectThemeHdl( NULL ); }
    String                  GetSelectedTheme() { return mpThemes->GetEntryCount() ? mpThemes->GetEntry( mpThemes->GetSelectEntryPos() ) : String(); }

    void                    ShowContextMenu();
};
