/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inettbc.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:58:37 $
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

#ifndef _SFX_INETTBC_HXX
#define _SFX_INETTBC_HXX

// includes *****************************************************************
#include <tools/string.hxx>
#include <tools/urlobj.hxx>

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX
#include <svtools/acceleratorexecute.hxx>
#endif

#if _SOLAR__PRIVATE

#include "tbxctrl.hxx"
class SvtURLBox;

class SfxURLToolBoxControl_Impl : public SfxToolBoxControl
{
private:
    ::svt::AcceleratorExecute*  pAccExec;

    SvtURLBox*              GetURLBox() const;
    void                    OpenURL( const String& rName, BOOL bNew ) const;

    DECL_LINK(              OpenHdl, void* );
    DECL_LINK(              SelectHdl, void* );
    DECL_LINK( WindowEventListener, VclSimpleEvent* );

    struct ExecuteInfo
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
        ::com::sun::star::util::URL                                                aTargetURL;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >  aArgs;
    };

    DECL_STATIC_LINK( SfxURLToolBoxControl_Impl, ExecuteHdl_Impl, ExecuteInfo* );

public:

                            SFX_DECL_TOOLBOX_CONTROL();

                            SfxURLToolBoxControl_Impl( USHORT nSlotId, USHORT nId, ToolBox& rBox );
    virtual                 ~SfxURLToolBoxControl_Impl();

    virtual Window*         CreateItemWindow( Window* pParent );
    virtual void            StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
};

class SfxCancelToolBoxControl_Impl : public SfxToolBoxControl
{
public:

                            SFX_DECL_TOOLBOX_CONTROL();

                            SfxCancelToolBoxControl_Impl( USHORT nSlotId, USHORT nId, ToolBox& rBox );

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
};

#endif

#endif

