/*************************************************************************
 *
 *  $RCSfile: TitleToolBox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:39:58 $
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
