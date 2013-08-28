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

#ifndef _SFX_INETTBC_HXX
#define _SFX_INETTBC_HXX

#include <tools/string.hxx>

#include <vcl/combobox.hxx>
#include <svtools/acceleratorexecute.hxx>

#include <sfx2/tbxctrl.hxx>

class SvtURLBox;

class SfxURLToolBoxControl_Impl : public SfxToolBoxControl
{
private:
    ::svt::AcceleratorExecute*  pAccExec;

    SvtURLBox*              GetURLBox() const;
    void                    OpenURL( const OUString& rName, sal_Bool bNew ) const;

    DECL_LINK(              OpenHdl, void* );
    DECL_LINK(              SelectHdl, void* );

    struct ExecuteInfo
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
        ::com::sun::star::util::URL                                                aTargetURL;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >  aArgs;
    };

    DECL_STATIC_LINK( SfxURLToolBoxControl_Impl, ExecuteHdl_Impl, ExecuteInfo* );

public:

                            SFX_DECL_TOOLBOX_CONTROL();

                            SfxURLToolBoxControl_Impl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox );
    virtual                 ~SfxURLToolBoxControl_Impl();

    virtual Window*         CreateItemWindow( Window* pParent );
    virtual void            StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
