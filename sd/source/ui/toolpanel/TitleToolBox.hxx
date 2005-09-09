/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TitleToolBox.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:35:57 $
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

#ifndef SD_TOOLBOX_TITLETOOLBOX_HXX
#define SD_TOOLBOX_TITLETOOLBOX_HXX

#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif

class PushButton;

namespace sd { namespace toolpanel {

/** Copied from vcl/menu.cxx
// To get the transparent mouse-over look, the TitleToolBox is actually a toolbox
// overload DataChange to handle style changes correctly
*/
class TitleToolBox
    : public ToolBox
{
public:
    enum ToolBoxId {
        TBID_DOCUMENT_CLOSE = 1,
        TBID_PANEL_MENU = 2,
        TBID_TRIANGLE_RIGHT = 3,
        TBID_TRIANGLE_DOWN = 4,
        TBID_TEST
    };

    TitleToolBox (::Window* pParent, WinBits nStyle = 0);

    void AddItem (ToolBoxId aId);

    void DataChanged (const DataChangedEvent& rDCEvt);

private:
    Image maImage;
    Image maImageHC;
    Image maTriangleRight;
    Image maTriangleDown;

    long lastSize;
};

} } // end of namespace ::sd::toolbox

#endif
