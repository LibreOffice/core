/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mediatoolbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:25:15 $
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

#ifndef _AVMEDIA_MEDIATOOLBOX_HXX
#define _AVMEDIA_MEDIATOOLBOX_HXX

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif

namespace avmedia
{

// -------------------------------
// - SvxGrafFilterToolBoxControl -
// -------------------------------

class MediaItem;

class MediaToolBoxControl : public SfxToolBoxControl
{
     friend class MediaToolBoxControl_Impl;

public:

                                SFX_DECL_TOOLBOX_CONTROL();

                                MediaToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbX );
                                ~MediaToolBoxControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual Window*             CreateItemWindow( Window* pParent );

private:

    void                        implUpdateMediaControl();
    void                        implExecuteMediaControl( const MediaItem& rItem );
};

}

#endif // _AVMEDIA_MEDIATOOLBOX_HXX
