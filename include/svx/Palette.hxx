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
#ifndef INCLUDED_SVX_PALETTE_HXX
#define INCLUDED_SVX_PALETTE_HXX

#include <svx/SvxColorValueSet.hxx>
#include <svx/xtable.hxx>
#include <rtl/ustring.hxx>

class Color;

typedef std::pair<Color, OUString> NamedColor;

class SVX_DLLPUBLIC Palette
{
public:
    virtual ~Palette();

    virtual const OUString&     GetName() = 0;
    virtual const OUString&     GetPath() = 0;
    virtual void                LoadColorSet( SvxColorValueSet& rColorSet ) = 0;

    virtual bool                IsValid() = 0;
};

#endif // INCLUDED_SVX_PALETTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
