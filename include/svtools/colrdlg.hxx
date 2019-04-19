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

#include <svtools/svtdllapi.h>
#include <tools/color.hxx>

namespace weld { class Window; }

namespace svtools
{
    // Select is the default.
    // These values must match the constants used in ColorPickerDialog in cui/source/dialogs/colorpicker.cxx
    enum class ColorPickerMode { Select = 0, Modify = 2 };
}

class SVT_DLLPUBLIC SvColorDialog final
{
public:
    SvColorDialog();
    ~SvColorDialog();

    void            SetColor( const Color& rColor );
    const Color&    GetColor() const { return maColor;}

    void            SetMode( svtools::ColorPickerMode eMode );

    short           Execute(weld::Window* pParent);

private:
    Color               maColor;
    svtools::ColorPickerMode meMode;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
