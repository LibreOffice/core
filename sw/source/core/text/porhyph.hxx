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

class SwHyphPortion : public SwExpandPortion
{
public:
    SwHyphPortion()
    {
        SetWhichPor( POR_HYPH );
    }
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;

    OUTPUT_OPERATOR_OVERRIDE
};

class SwHyphStrPortion : public SwHyphPortion
{
    OUString aExpand;
public:
    explicit SwHyphStrPortion(const OUString &rStr)
        : aExpand(rStr + "-")
    {
        SetWhichPor( POR_HYPHSTR );
    }

    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;

    OUTPUT_OPERATOR_OVERRIDE
};

class SwSoftHyphPortion : public SwHyphPortion
{
    bool    bExpand;
    sal_uInt16  nViewWidth;
    sal_uInt16  nHyphWidth;

public:
    SwSoftHyphPortion();
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    virtual SwLinePortion *Compress() override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual void FormatEOL( SwTextFormatInfo &rInf ) override;
    inline void SetExpand( const bool bNew ) { bExpand = bNew; }
    bool IsExpand() const { return bExpand; }

    virtual sal_uInt16 GetViewWidth( const SwTextSizeInfo &rInf ) const override;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;

    OUTPUT_OPERATOR_OVERRIDE
};

class SwSoftHyphStrPortion : public SwHyphStrPortion
{
public:
    explicit SwSoftHyphStrPortion( const OUString &rStr );
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    OUTPUT_OPERATOR_OVERRIDE
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
