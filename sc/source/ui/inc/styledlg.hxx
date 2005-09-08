/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: styledlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:53:29 $
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

#ifndef SC_STYLEDLG_HXX
#define SC_STYLEDLG_HXX

#ifndef _SFX_HXX
#endif

#ifndef _SFX_STYLEDLG_HXX //autogen
#include <sfx2/styledlg.hxx>
#endif

//==================================================================

class SfxStyleSheetBase;

class ScStyleDlg : public SfxStyleDialog
{
public:
                ScStyleDlg( Window*             pParent,
                            SfxStyleSheetBase&  rStyleBase,
                            USHORT              nRscId );
                ~ScStyleDlg();

protected:
    virtual void                PageCreated( USHORT nPageId, SfxTabPage& rTabPage );
    virtual const SfxItemSet*   GetRefreshedSet();

private:
    USHORT nDlgRsc;
};


#endif // SC_STYLEDLG_HXX


