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

#ifndef INCLUDED_SFX2_INC_INETTBC_HXX
#define INCLUDED_SFX2_INC_INETTBC_HXX

#include <rtl/ustring.hxx>
#include <sfx2/tbxctrl.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <vcl/combobox.hxx>

class SvtURLBox;

class SfxURLToolBoxControl_Impl : public SfxToolBoxControl
{
private:

    SvtURLBox*              GetURLBox() const;
    void                    OpenURL( const OUString& rName, bool bNew ) const;

    DECL_LINK_TYPED(        OpenHdl, SvtURLBox*, void );
    DECL_LINK_TYPED(        SelectHdl, ComboBox&, void );

    struct ExecuteInfo
    {
        css::uno::Reference< css::frame::XDispatch >     xDispatch;
        css::util::URL                                   aTargetURL;
        css::uno::Sequence< css::beans::PropertyValue >  aArgs;
    };

    DECL_STATIC_LINK_TYPED( SfxURLToolBoxControl_Impl, ExecuteHdl_Impl, void*, void );

public:

                            SFX_DECL_TOOLBOX_CONTROL();

                            SfxURLToolBoxControl_Impl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox );
    virtual                 ~SfxURLToolBoxControl_Impl();

    virtual VclPtr<vcl::Window> CreateItemWindow( vcl::Window* pParent ) override;
    virtual void            StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
