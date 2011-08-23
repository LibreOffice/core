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

#ifndef _SVTOOLS_IMGDEF_HXX
#define _SVTOOLS_IMGDEF_HXX

namespace binfilter
{

enum SfxSymbolsSize
{
    SFX_SYMBOLS_SIZE_SMALL,
    SFX_SYMBOLS_SIZE_LARGE,
    SFX_SYMBOLS_SIZE_AUTO
};

enum SfxSymbolsStyle
{
    SFX_SYMBOLS_STYLE_AUTO,
    SFX_SYMBOLS_STYLE_DEFAULT,
    SFX_SYMBOLS_STYLE_HICONTRAST,
    SFX_SYMBOLS_STYLE_INDUSTRIAL,
    SFX_SYMBOLS_STYLE_CRYSTAL,
    SFX_SYMBOLS_STYLE_TANGO
};

#define SFX_TOOLBOX_CHANGESYMBOLSET		0x0001
#define SFX_TOOLBOX_CHANGEOUTSTYLE  	0x0002
#define SFX_TOOLBOX_CHANGEBUTTONTYPE	0x0004

}

#endif // _SVTOOLS_IMGDEF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
