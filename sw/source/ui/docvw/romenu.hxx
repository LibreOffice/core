/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: romenu.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:41:11 $
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
#ifndef _ROMENU_HXX
#define _ROMENU_HXX


#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif

class SwView;
class SfxDispatcher;
class SvxBrushItem;
class ImageMap;
class INetImage;

class SwReadOnlyPopup : public PopupMenu
{
          SwView &rView;
    const SvxBrushItem *pItem;
    const Point &rDocPos;
                Graphic aGraphic;
    String      sURL,
                sTargetFrameName,
                sDescription,
                sGrfName;
    List        aThemeList;
    BOOL        bGrfToGalleryAsLnk;
    ImageMap*   pImageMap;
    INetImage*  pTargetURL;

    void Check( USHORT nMID, USHORT nSID, SfxDispatcher &rDis );
    String SaveGraphic( USHORT nId );

public:
    SwReadOnlyPopup( const Point &rDPos, SwView &rV );
    ~SwReadOnlyPopup();

    using PopupMenu::Execute;
    void Execute( Window* pWin, const Point &rPPos );
    void Execute( Window* pWin, USHORT nId );
};


#endif

