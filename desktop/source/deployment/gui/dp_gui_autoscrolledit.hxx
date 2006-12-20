/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_autoscrolledit.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 14:22:02 $
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
#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_AUTOSCROLLEDIT_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_AUTOSCROLLEDIT_HXX

#ifndef _SVTOOLS_SVMEDIT2_HXX
#include "svtools/svmedit2.hxx"
#endif
#ifndef _SFXLSTNER_HXX
#include "svtools/lstner.hxx"
#endif

namespace dp_gui {

/** This control shows automatically the vertical scroll bar if text is inserted,
    that does not fit into the text area. In the resource one uses MultiLineEdit
    and needs to set VScroll = TRUE
*/
class AutoScrollEdit : public ExtMultiLineEdit, public SfxListener
{
public:
    AutoScrollEdit( Window* pParent, const ResId& rResId );
    ~AutoScrollEdit();

    using ExtMultiLineEdit::Notify;
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

} // namespace dp_gui

#endif
