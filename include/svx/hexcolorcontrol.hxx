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

#include <svx/svxdllapi.h>
#include <tools/color.hxx>
#include <tools/link.hxx>

namespace weld { class Entry; }

struct ImplSVEvent;

namespace weld {

class SVX_DLLPUBLIC HexColorControl
{
private:
    std::unique_ptr<weld::Entry> m_xEntry;
    Link<weld::Entry&, void> m_aModifyHdl;
    ImplSVEvent* m_nAsyncModifyEvent;

    DECL_STATIC_LINK(HexColorControl, ImplProcessInputHdl, OUString&, bool);
    DECL_LINK(ImplProcessModifyHdl, weld::Entry&, void);
    DECL_LINK(OnAsyncModifyHdl, void*, void);
public:
    HexColorControl(std::unique_ptr<weld::Entry> pEdit);
    ~HexColorControl();

    void connect_changed(const Link<weld::Entry&, void>& rLink) { m_aModifyHdl = rLink; }

    void SetColor( ::Color nColor );
    ::Color GetColor() const;
};

}

#endif  // INCLUDED_SVX_HEXCOLOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
