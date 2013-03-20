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

#pragma once
#if 1

#include <vcl/image.hxx>
#include <svl/lstner.hxx>
#include "EventMultiplexer.hxx"
#include "ViewShellBase.hxx"
#include "res_bmp.hrc"

namespace sd
{

typedef std::vector< rtl::Reference< SmartTag > > ViewTagVector;

class ViewOverlayManager : public SfxListener
{
public:
    ViewOverlayManager( ViewShellBase& rViewShellBase );
    virtual ~ViewOverlayManager();

    void onZoomChanged();
    void UpdateTags();

    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent*);
    DECL_LINK(UpdateTagsHdl, void *);

    bool CreateTags();
    bool DisposeTags();

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

private:
    ViewShellBase& mrBase;
    sal_uLong mnUpdateTagsEvent;

    ViewTagVector   maTagVector;
};

}

#endif // _SD_VIEWOVERLAYMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
