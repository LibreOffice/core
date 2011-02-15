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
#ifndef _SWCLI_HXX
#define _SWCLI_HXX
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <svtools/embedhlp.hxx>

#ifndef _SFX_CLIENTSH_HXX //autogen
#include <sfx2/ipclient.hxx>
#endif

class SwView;
class SwEditWin;

class SwOleClient : public SfxInPlaceClient
{
    sal_Bool bInDoVerb;
    sal_Bool bOldCheckForOLEInCaption;

    virtual void ObjectAreaChanged();
    virtual void RequestNewObjectArea( Rectangle& );
    virtual void ViewChanged();
    virtual void MakeVisible();

public:
    SwOleClient( SwView *pView, SwEditWin *pWin, const svt::EmbeddedObjectRef& );

    void SetInDoVerb( sal_Bool bFlag )              { bInDoVerb = bFlag; }

    sal_Bool IsCheckForOLEInCaption() const         { return bOldCheckForOLEInCaption; }

    virtual void FormatChanged();
};

#endif
