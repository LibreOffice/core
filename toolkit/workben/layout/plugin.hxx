/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
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
#ifndef _LAYOUT_PLUGIN_HXX
#define _LAYOUT_PLUGIN_HXX

#include <layout/layout.hxx>
#include <layout/layout-pre.hxx>

namespace svx {
class DialControl;
}

class PluginDialog : public ModalDialog
{
private:
    FixedImage aHeaderImage;
    FixedText aHeaderText;
    FixedLine aHeaderLine;
    Plugin aPlugin;
    svx::DialControl& aDialControl;
    OKButton aOKBtn;
    CancelButton aCancelBtn;
    HelpButton aHelpBtn;

public:
    PluginDialog( Window* pParent );
    ~PluginDialog();
};

#include <layout/layout-post.hxx>

#endif /* _LAYOUT_PLUGIN_HXX */

