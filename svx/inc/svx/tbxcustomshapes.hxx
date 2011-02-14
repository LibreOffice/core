/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _SVX_TBXCUSTOMSHAPES_HXX
#define _SVX_TBXCUSTOMSHAPES_HXX

#include <sfx2/tbxctrl.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* SvxTbxCtlCustomShapes
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxTbxCtlCustomShapes : public SfxToolBoxControl
{
public:
    using SfxToolBoxControl::Select;
    virtual void                Select( sal_Bool bMod1 = sal_False );
    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();

            SFX_DECL_TOOLBOX_CONTROL();

            SvxTbxCtlCustomShapes( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
            ~SvxTbxCtlCustomShapes() {}

    //interface XSubToolbarController:
    virtual ::sal_Bool SAL_CALL opensSubToolbar() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSubToolbarName() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL functionSelected( const ::rtl::OUString& aCommand ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateImage() throw (::com::sun::star::uno::RuntimeException);

private:
    rtl::OUString       m_aSubTbName;
    rtl::OUString       m_aSubTbxResName;
    rtl::OUString       m_aCommand;
};


#endif

