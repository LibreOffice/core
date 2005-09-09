/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gluectrl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:42:43 $
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

#ifndef SD_GLUECTRL_HXX
#define SD_GLUECTRL_HXX

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif

/*************************************************************************
|*
|* GluePointEscDirLB
|*
\************************************************************************/
class GlueEscDirLB : public ListBox
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
public:
                GlueEscDirLB( Window* pParent,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
                 ~GlueEscDirLB();

    virtual void Select();

    void         Fill();
};

/*************************************************************************
|*
|* Toolbox-Controller fuer Klebepunkte-Austrittsrichtung
|*
\************************************************************************/

class SdTbxCtlGlueEscDir: public SfxToolBoxControl
{
private:
    UINT16  GetEscDirPos( UINT16 nEscDir );

public:
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
                                const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );

            SFX_DECL_TOOLBOX_CONTROL();

            SdTbxCtlGlueEscDir( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
            ~SdTbxCtlGlueEscDir() {}
};

#endif // SD_GLUECTRL_HXX

