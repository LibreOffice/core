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

#include <sal/config.h>

#include <string_view>

#include "porexp.hxx"

class SwHyphPortion : public SwExpandPortion
{
public:
    SwHyphPortion()
    {
        SetWhichPor( PortionType::Hyphen );
    }
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;

    void dumpAsXml(xmlTextWriterPtr pWriter, const OUString& rText,
                   TextFrameIndex& nOffset) const override;
};

class SwHyphStrPortion : public SwHyphPortion
{
    OUString m_aExpand;
public:
    explicit SwHyphStrPortion(std::u16string_view rStr)
        : m_aExpand(OUString::Concat(rStr) + "-")
    {
        SetWhichPor( PortionType::HyphenStr );
    }

    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;
};

class SwSoftHyphPortion : public SwHyphPortion
{
    bool    m_bExpand;
    SwTwips m_nViewWidth;

public:
    SwSoftHyphPortion();
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    virtual SwLinePortion *Compress() override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual void FormatEOL( SwTextFormatInfo &rInf ) override;
    void SetExpand( const bool bNew ) { m_bExpand = bNew; }
    bool IsExpand() const { return m_bExpand; }

    virtual SwTwips GetViewWidth(const SwTextSizeInfo& rInf) const override;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;
};

class SwSoftHyphStrPortion : public SwHyphStrPortion
{
public:
    explicit SwSoftHyphStrPortion( std::u16string_view rStr );
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
