/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SV_COLRDLG_HXX
#define _SV_COLRDLG_HXX

#include "svtools/svtdllapi.h"

#include <tools/color.hxx>

class Window;

// ---------------
// - ColorDialog -
// ---------------

namespace svtools
{
    // SELECT is the default
    enum ColorPickerMode { ColorPickerMode_SELECT = 0, ColorPickerMode_ADD = 1, ColorPickerMode_MODIFY = 2 };
}

class SVT_DLLPUBLIC SvColorDialog
{
public:
    SvColorDialog( ::Window* pParent );

    void            SetColor( const Color& rColor );
    const Color&    GetColor() const;

    void            SetMode( sal_Int16 eMode );

    virtual short   Execute();

private:
    Window*    mpParent;
    Color    maColor;
    sal_Int16 meMode;
};

#endif  // _SV_COLRDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
