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

#ifndef INCLUDED_SVX_HEXCOLOR_HXX
#define INCLUDED_SVX_HEXCOLOR_HXX

#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/weld.hxx>
#include <sot/exchange.hxx>
#include <sax/tools/converter.hxx>
#include <svx/svxdllapi.h>
#include <tools/color.hxx>

namespace weld {

class SVX_DLLPUBLIC HexColorControl
{
private:
    std::unique_ptr<weld::Entry> m_xEntry;

    DECL_LINK(ImplProcessInputHdl, OUString&, bool);
public:
    HexColorControl(std::unique_ptr<weld::Entry> pEdit);

    void connect_changed(const Link<Entry&, void>& rLink) { m_xEntry->connect_changed(rLink); }

    void SetColor( ::Color nColor );
    ::Color GetColor();
};

}

#endif  // INCLUDED_SVX_HEXCOLOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
