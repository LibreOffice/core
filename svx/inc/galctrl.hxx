/*************************************************************************
 *
 *  $RCSfile: galctrl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 08:57:57 $
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

#ifndef _SVX_GALCTRL_HXX_
#define _SVX_GALCTRL_HXX_

#include <vcl/dialog.hxx>
#include <vcl/graph.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menu.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <vcl/sound.hxx>
#include <svtools/slstitm.hxx>
#include <svtools/transfer.hxx>
#include <svtools/valueset.hxx>
#include <svtools/brwbox.hxx>
#include <goodies/grfmgr.hxx>
#include "galmisc.hxx"

// ------------
// - Forwards -
// ------------

class GalleryTheme;
class GalleryBrowser2;

// ------------------
// - GalleryPreview -
// ------------------

class GalleryPreview : public Window, public DropTargetHelper, public DragSourceHelper
{
private:

    GraphicObject       aGraphicObj;
    Rectangle           aPreviewRect;
    Sound               aSound;
    GalleryTheme*       mpTheme;

    BOOL                ImplGetGraphicCenterRect( const Graphic& rGraphic, Rectangle& rResultRect ) const;
    void                InitSettings();

    // Window
    virtual void        Paint(const Rectangle& rRect);
    virtual void        MouseButtonDown(const MouseEvent& rMEvt);
    virtual void        Command(const CommandEvent& rCEvt);
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );


    // DropTargetHelper
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

    // DragSourceHelper
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

                        DECL_LINK( MenuSelectHdl, Menu* );
                        DECL_LINK( SoundEndHdl, Sound* );

public:

                        GalleryPreview( GalleryBrowser2* pParent, GalleryTheme* pTheme );
                        GalleryPreview( Window* pParent, const ResId& rResId  );
                        ~GalleryPreview();

    void                SetGraphic( const Graphic& rGraphic ) { aGraphicObj.SetGraphic( rGraphic ); }
    void                PreviewMedia( const INetURLObject& rURL );
};

// -------------------
// - GalleryIconView -
// -------------------

class GalleryIconView : public ValueSet, public DropTargetHelper, public DragSourceHelper
{
private:

    GalleryTheme*       mpTheme;

    void                InitSettings();

    // ValueSet
    virtual void        UserDraw( const UserDrawEvent& rUDEvt );

    // Window
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    // DropTargetHelper
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

    // DragSourceHelper
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

public:

                        GalleryIconView( GalleryBrowser2* pParent, GalleryTheme* pTheme );
                        ~GalleryIconView();
};

// -------------------
// - GalleryListView -
// -------------------

class GalleryListView : public BrowseBox
{
private:

    Link                maSelectHdl;
    GalleryTheme*       mpTheme;
    ULONG               mnCurRow;
    BOOL                mbInit;

    void                InitSettings();

    // BrowseBox
    virtual BOOL        SeekRow( long nRow );
    virtual void        PaintField( OutputDevice& rDev, const Rectangle& rRect, USHORT nColumnId ) const;
    virtual void        DoubleClick( const BrowserMouseEvent& rEvt );
    virtual void        Select();
    virtual sal_Int8    AcceptDrop( const BrowserAcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const BrowserExecuteDropEvent& rEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );

    // Window
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    // DragSourceHelper
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

public:

                        GalleryListView( GalleryBrowser2* pParent, GalleryTheme* pTheme );
                        ~GalleryListView();

    void                SetSelectHdl( const Link& rSelectHdl ) { maSelectHdl = rSelectHdl; }

    /** GetCellText returns the text at the given position
        @param  _nRow
            the number of the row
        @param  _nColId
            the ID of the column
        @return
            the text out of the cell
    */
    virtual String  GetCellText(long _nRow, USHORT _nColId) const;

    // from IAccessibleTableProvider
    virtual Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex);
    virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint);
};

#endif // _SVX_GALCTRL_HXX_
