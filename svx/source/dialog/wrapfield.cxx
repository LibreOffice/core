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

#include "svx/wrapfield.hxx"
#include <vcl/builder.hxx>

namespace svx {

WrapField::WrapField( Window* pParent, WinBits nBits ) :
    NumericField( pParent, nBits )
{
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeWrapField(Window *pParent, VclBuilder::stringmap &)
{
    return new WrapField(pParent, WB_SPIN|WB_REPEAT|WB_BORDER|WB_TABSTOP);
}

void WrapField::Up()
{
    SetValue( ((GetValue() + GetSpinSize() - GetMin()) % (GetMax() + 1)) + GetMin() );
    GetUpHdl().Call( this );
}

void WrapField::Down()
{
    SetValue( ((GetValue() - GetSpinSize() + GetMax() + 1 - GetMin()) % (GetMax() + 1)) + GetMin() );
    GetDownHdl().Call( this );
}



} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
