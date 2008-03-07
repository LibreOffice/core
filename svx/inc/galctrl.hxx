/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: galctrl.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:36:03 $
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

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// ------------
// - Forwards -
// ------------

class GalleryTheme;
class GalleryBrowser2;

// ------------------
// - GalleryPreview -
// ------------------

class SVX_DLLPUBLIC GalleryPreview : public Window, public DropTargetHelper, public DragSourceHelper
{
private:

    GraphicObject       aGraphicObj;
    Rectangle           aPreviewRect;
    GalleryTheme*       mpTheme;

    SVX_DLLPRIVATE BOOL             ImplGetGraphicCenterRect( const Graphic& rGraphic, Rectangle& rResultRect ) const;
    SVX_DLLPRIVATE void                InitSettings();

    // Window
    SVX_DLLPRIVATE virtual void         Paint(const Rectangle& rRect);
    SVX_DLLPRIVATE virtual void         MouseButtonDown(const MouseEvent& rMEvt);
    SVX_DLLPRIVATE virtual void     Command(const CommandEvent& rCEvt);
    SVX_DLLPRIVATE virtual void     KeyInput( const KeyEvent& rKEvt );
    SVX_DLLPRIVATE virtual void        DataChanged( const DataChangedEvent& rDCEvt );


    // DropTargetHelper
    SVX_DLLPRIVATE virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );
    SVX_DLLPRIVATE virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );

    // DragSourceHelper
    SVX_DLLPRIVATE virtual void     StartDrag( sal_Int8 nAction, const Point& rPosPixel );

                        DECL_LINK( MenuSelectHdl, Menu* );

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
    using ValueSet::StartDrag;

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
    using BrowseBox::AcceptDrop;
    using BrowseBox::ExecuteDrop;

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
