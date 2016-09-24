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

#ifndef INCLUDED_AVMEDIA_MEDIATOOLBOX_HXX
#define INCLUDED_AVMEDIA_MEDIATOOLBOX_HXX

#include <svl/lstner.hxx>
#include <sfx2/tbxctrl.hxx>
#include <avmedia/avmediadllapi.h>

namespace avmedia
{

class MediaItem;

class AVMEDIA_DLLPUBLIC MediaToolBoxControl : public SfxToolBoxControl
{
     friend class MediaToolBoxControl_Impl;

public:

                                SFX_DECL_TOOLBOX_CONTROL();

                                MediaToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbX );
                                virtual ~MediaToolBoxControl() override;

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;
    virtual VclPtr<vcl::Window> CreateItemWindow( vcl::Window* pParent ) override;

private:

    AVMEDIA_DLLPRIVATE void implUpdateMediaControl();
    AVMEDIA_DLLPRIVATE void implExecuteMediaControl( const MediaItem& rItem );
};

}

#endif // INCLUDED_AVMEDIA_MEDIATOOLBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
