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
#ifndef _PORREF_HXX
#define _PORREF_HXX

#include "portxt.hxx"

/*************************************************************************
 *                      class SwRefPortion
 *************************************************************************/

class SwRefPortion : public SwTxtPortion
{
public:
    inline  SwRefPortion(){ SetWhichPor( POR_REF ); }
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwIsoRefPortion
 *************************************************************************/

class SwIsoRefPortion : public SwRefPortion
{
    KSHORT nViewWidth;

public:
            SwIsoRefPortion();
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual SwLinePortion *Compress();
    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  inline - Implementations
 *************************************************************************/

CLASSIO( SwRefPortion )
CLASSIO( SwIsoRefPortion )


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
