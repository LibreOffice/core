/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: toolbox.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 16:16:27 $
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

#ifndef _SV_TOOLBOX_H
#define _SV_TOOLBOX_H

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif

#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif

#ifndef _VCL_CONTROLLAYOUT_HXX
#include <vcl/controllayout.hxx>
#endif

#include <vector>

#define TB_DROPDOWNARROWWIDTH   11

#define TB_MENUBUTTON_SIZE      12
#define TB_MENUBUTTON_OFFSET    2

#define TB_SMALLIMAGESIZE       16

#define TB_LARGEIMAGESIZE            26
#define TB_LARGEIMAGESIZE_INDUSTRIAL 24
#define TB_LARGEIMAGESIZE_CRYSTAL    22

class Window;

// ----------------
// - ImplToolItem -
// ----------------

struct ImplToolItem
{
    Window*             mpWindow;
    void*               mpUserData;
    Image               maImage;
    Image               maHighImage;
    long                mnImageAngle;
    bool                mbMirrorMode;
    XubString           maText;
    XubString           maQuickHelpText;
    XubString           maHelpText;
    String              maCommandStr;
    ULONG               mnHelpId;
    Rectangle           maRect;
    Rectangle           maCalcRect;
    // the overall horizontal item size, including one or more of [image size + textlength + dropdown arrow]
    Size                maItemSize;
    long                mnSepSize;
    long                mnDropDownArrowWidth;
    ToolBoxItemType     meType;
    ToolBoxItemBits     mnBits;
    TriState            meState;
    USHORT              mnId;
    BOOL                mbEnabled:1,
                        mbVisible:1,
                        mbEmptyBtn:1,
                        mbShowWindow:1,
                        mbBreak:1,
                        mbVisibleText:1;    // indicates if text will definitely be drawn, influences dropdown pos

                        ImplToolItem();
                        ImplToolItem( USHORT nItemId, const Image& rImage,
                                      ToolBoxItemBits nItemBits );
                        ImplToolItem( USHORT nItemId, const XubString& rTxt,
                                      ToolBoxItemBits nItemBits );
                        ImplToolItem( USHORT nItemId, const Image& rImage,
                                      const XubString& rTxt,
                                      ToolBoxItemBits nItemBits );
                        ~ImplToolItem();

    ImplToolItem( const ImplToolItem& );
    ImplToolItem& operator=(const ImplToolItem&);

    // returns the size of a item, taking toolbox orientation into account
    // the default size is the precomputed size for standard items
    // ie those that are just ordinary buttons (no windows or text etc.)
    // bCheckMaxWidth indicates that item windows must not exceed maxWidth in which case they will be painted as buttons
    Size                GetSize( BOOL bHorz, BOOL bCheckMaxWidth, long maxWidth, const Size& rDefaultSize );

    // only useful for buttons: returns if the text or image part or both can be drawn according to current button drawing style
    void DetermineButtonDrawStyle( ButtonType eButtonType, BOOL& rbImage, BOOL& rbText ) const;

    // returns the rectangle which contains the drop down arrow
    // or an empty rect if there is none
    // bHorz denotes the toolbox alignment
    Rectangle   GetDropDownRect( BOOL bHorz ) const;

    // returns TRUE if the toolbar item is currently clipped, which can happen for docked toolbars
    BOOL IsClipped() const;
};

namespace vcl
{

struct ToolBoxLayoutData : public ControlLayoutData
{
    std::vector< USHORT >               m_aLineItemIds;
    std::vector< USHORT >               m_aLineItemPositions;
};

}

struct ImplToolBoxPrivateData
{
    vcl::ToolBoxLayoutData*         m_pLayoutData;
    std::vector< ImplToolItem >     m_aItems;

    ImplToolBoxPrivateData();
    ~ImplToolBoxPrivateData();

    void ImplClearLayoutData() { delete m_pLayoutData; m_pLayoutData = NULL; }

    // called when dropdown items are clicked
    Link    maDropdownClickHdl;
    Timer   maDropdownTimer; // for opening dropdown items on "long click"

    // large or small buttons ?
    ToolBoxButtonSize   meButtonSize;

    // the optional custom menu
    PopupMenu*  mpMenu;
    USHORT      maMenuType;
    ULONG       mnEventId;

    // called when menu button is clicked and before the popup menu is executed
    Link        maMenuButtonHdl;

    // a dummy item representing the custom menu button
    ImplToolItem   maMenubuttonItem;
    long           mnMenuButtonWidth;

    Wallpaper   maDisplayBackground;

    BOOL    mbIsLocked:1,           // keeps last lock state from ImplDockingWindowWrapper
            mbAssumeDocked:1,       // only used during calculations to override current floating/popup mode
            mbAssumeFloating:1,
            mbAssumePopupMode:1,
            mbKeyInputDisabled:1,   // no KEY input if all items disabled, closing/docking will be allowed though
            mbIsPaintLocked:1,      // don't allow paints
            mbMenubuttonSelected:1, // menu button is highlighted
            mbPageScroll:1,         // determines if we scroll a page at a time
            mbNativeButtons:1,      // system supports native toolbar buttons
            mbWillUsePopupMode:1,   // this toolbox will be opened in popup mode
            mbDropDownByKeyboard:1; // tells whether a dropdown was started by key input
};


#endif  // _SV_TOOLBOX_H
