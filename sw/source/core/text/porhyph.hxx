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
#ifndef _PORHYPH_HXX
#define _PORHYPH_HXX

#include "porexp.hxx"

/*************************************************************************
 *                      class SwHyphPortion
 *************************************************************************/

class SwHyphPortion : public SwExpandPortion
{
public:
    SwHyphPortion()
    {
        SetWhichPor( POR_HYPH );
    }
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwHyphStrPortion
 *************************************************************************/

class SwHyphStrPortion : public SwHyphPortion
{
    OUString aExpand;
public:
    SwHyphStrPortion(const OUString &rStr)
        : aExpand(rStr + "-")
    {
        SetWhichPor( POR_HYPHSTR );
    }

    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwSoftHyphPortion
 *************************************************************************/

class SwSoftHyphPortion : public SwHyphPortion
{
    sal_Bool    bExpand;
    KSHORT  nViewWidth;
    KSHORT  nHyphWidth;

public:
    SwSoftHyphPortion();
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const;
    virtual SwLinePortion *Compress();
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
    inline void SetExpand( const sal_Bool bNew ) { bExpand = bNew; }
    sal_Bool IsExpand() const { return bExpand; }

    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwSoftHyphStrPortion
 *************************************************************************/

class SwSoftHyphStrPortion : public SwHyphStrPortion
{
public:
    SwSoftHyphStrPortion( const OUString &rStr );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    OUTPUT_OPERATOR
};

CLASSIO( SwHyphPortion )
CLASSIO( SwHyphStrPortion )
CLASSIO( SwSoftHyphPortion )
CLASSIO( SwSoftHyphStrPortion )


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
