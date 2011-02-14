/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: $
 * $Revision: $
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

#ifndef _SD_VIEWOVERLAYMANAGER_HXX
#define _SD_VIEWOVERLAYMANAGER_HXX

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
