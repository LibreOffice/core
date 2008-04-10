/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TitleToolBox.hxx,v $
 * $Revision: 1.4 $
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

#ifndef SD_TOOLBOX_TITLETOOLBOX_HXX
#define SD_TOOLBOX_TITLETOOLBOX_HXX

#include <vcl/toolbox.hxx>

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
