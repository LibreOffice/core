/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_TBXCUSTOMSHAPES_HXX
#define INCLUDED_SVX_TBXCUSTOMSHAPES_HXX

#include <sfx2/tbxctrl.hxx>
#include <svx/svxdllapi.h>

/*************************************************************************
|*
|* SvxTbxCtlCustomShapes
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxTbxCtlCustomShapes : public SfxToolBoxControl
{
public:
    using SfxToolBoxControl::Select;
    virtual void                Select( bool bMod1 = false ) SAL_OVERRIDE;
    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState ) SAL_OVERRIDE;
    virtual SfxPopupWindowType  GetPopupWindowType() const SAL_OVERRIDE;
    virtual SfxPopupWindow*     CreatePopupWindow() SAL_OVERRIDE;

            SFX_DECL_TOOLBOX_CONTROL();

            SvxTbxCtlCustomShapes( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
            ~SvxTbxCtlCustomShapes() {}

    //interface XSubToolbarController:
    virtual ::sal_Bool SAL_CALL opensSubToolbar() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getSubToolbarName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL functionSelected( const OUString& aCommand ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL updateImage() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    OUString       m_aSubTbName;
    OUString       m_aSubTbxResName;
    OUString       m_aCommand;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
