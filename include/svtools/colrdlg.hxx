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

#ifndef INCLUDED_SVTOOLS_COLRDLG_HXX
#define INCLUDED_SVTOOLS_COLRDLG_HXX

#include <svtools/svtdllapi.h>

#include <tools/color.hxx>

class Window;


// - ColorDialog -


namespace svtools
{
    // SELECT is the default
    enum ColorPickerMode { ColorPickerMode_SELECT = 0, ColorPickerMode_ADD = 1, ColorPickerMode_MODIFY = 2 };
}

class SVT_DLLPUBLIC SvColorDialog
{
public:
    SvColorDialog( ::Window* pParent );
    virtual ~SvColorDialog();

    void            SetColor( const Color& rColor );
    const Color&    GetColor() const;

    void            SetMode( sal_Int16 eMode );

    virtual short   Execute();

private:
    Window*    mpParent;
    Color    maColor;
    sal_Int16 meMode;
};

#endif // INCLUDED_SVTOOLS_COLRDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
