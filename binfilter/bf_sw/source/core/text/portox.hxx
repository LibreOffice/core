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

#ifndef _PORTOX_HXX
#define _PORTOX_HXX

#include "portxt.hxx"

namespace binfilter {

/*************************************************************************
 *                      class SwToxPortion
 *************************************************************************/

class SwToxPortion : public SwTxtPortion
{
public:
    inline	SwToxPortion(){	SetWhichPor( POR_TOX ); }
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwIsoToxPortion
 *************************************************************************/

class SwIsoToxPortion : public SwToxPortion
{
    KSHORT nViewWidth;

public:
            SwIsoToxPortion();
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual SwLinePortion *Compress();

    // Accessibility: pass information about this portion to the PortionHandler

    OUTPUT_OPERATOR
};

/*************************************************************************
 *					inline - Implementations
 *************************************************************************/

CLASSIO( SwToxPortion )
CLASSIO( SwIsoToxPortion )


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
