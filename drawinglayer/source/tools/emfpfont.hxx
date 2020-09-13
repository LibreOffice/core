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

#include "emfphelperdata.hxx"

namespace emfplushelper
{
    const sal_uInt32 FontStyleBold = 0x00000001;
    const sal_uInt32 FontStyleItalic = 0x00000002;
    const sal_uInt32 FontStyleUnderline = 0x00000004;
    const sal_uInt32 FontStyleStrikeout = 0x00000008;


    struct EMFPFont : public EMFPObject
    {
        float emSize;
        sal_uInt32 sizeUnit;
        sal_Int32 fontFlags;
        OUString family;

        void Read(SvMemoryStream &s);

        bool Bold()      const { return fontFlags & FontStyleBold; }
        bool Italic()    const { return fontFlags & FontStyleItalic; }
        bool Underline() const { return fontFlags & FontStyleUnderline; }
        bool Strikeout() const { return fontFlags & FontStyleStrikeout; }
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
