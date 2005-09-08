/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tbxalign.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:18:30 $
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
#ifndef _TBXALIGN_HXX
#define _TBXALIGN_HXX

#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif


class SVX_DLLPUBLIC SvxTbxCtlAlign: public SfxToolBoxControl
{
public:
    virtual ::sal_Bool SAL_CALL opensSubToolbar() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSubToolbarName() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL functionSelected( const ::rtl::OUString& aCommand ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateImage() throw (::com::sun::star::uno::RuntimeException);

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();

            SFX_DECL_TOOLBOX_CONTROL();

            SvxTbxCtlAlign( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
            ~SvxTbxCtlAlign() {}
private:
    rtl::OUString m_aSubTbName;
    rtl::OUString m_aSubTbResName;
    rtl::OUString m_aCommand;
};
#endif
