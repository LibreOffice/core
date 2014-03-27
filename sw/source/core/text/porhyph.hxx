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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORHYPH_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORHYPH_HXX

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
    virtual bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const SAL_OVERRIDE;
    virtual bool Format( SwTxtFormatInfo &rInf ) SAL_OVERRIDE;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const SAL_OVERRIDE;

    OUTPUT_OPERATOR_OVERRIDE
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

    virtual bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const SAL_OVERRIDE;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const SAL_OVERRIDE;

    OUTPUT_OPERATOR_OVERRIDE
};

/*************************************************************************
 *                      class SwSoftHyphPortion
 *************************************************************************/

class SwSoftHyphPortion : public SwHyphPortion
{
    bool    bExpand;
    KSHORT  nViewWidth;
    KSHORT  nHyphWidth;

public:
    SwSoftHyphPortion();
    virtual bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const SAL_OVERRIDE;
    virtual SwLinePortion *Compress() SAL_OVERRIDE;
    virtual void Paint( const SwTxtPaintInfo &rInf ) const SAL_OVERRIDE;
    virtual bool Format( SwTxtFormatInfo &rInf ) SAL_OVERRIDE;
    virtual void FormatEOL( SwTxtFormatInfo &rInf ) SAL_OVERRIDE;
    inline void SetExpand( const bool bNew ) { bExpand = bNew; }
    bool IsExpand() const { return bExpand; }

    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const SAL_OVERRIDE;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const SAL_OVERRIDE;

    OUTPUT_OPERATOR_OVERRIDE
};

/*************************************************************************
 *                      class SwSoftHyphStrPortion
 *************************************************************************/

class SwSoftHyphStrPortion : public SwHyphStrPortion
{
public:
    SwSoftHyphStrPortion( const OUString &rStr );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const SAL_OVERRIDE;
    OUTPUT_OPERATOR_OVERRIDE
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
