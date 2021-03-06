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

#include <svl/style.hxx>

#include "scdllapi.h"

class ScStyleSheetPool;

class SAL_DLLPUBLIC_RTTI ScStyleSheet final : public SfxStyleSheet
{
friend class ScStyleSheetPool;

public:

    enum class Usage
    {
        UNKNOWN,
        USED,
        NOTUSED
    };

                        ScStyleSheet( const ScStyleSheet& rStyle );

    virtual bool        SetParent        ( const OUString& rParentName ) override;
    SC_DLLPUBLIC void ResetParent();
    SC_DLLPUBLIC virtual SfxItemSet& GetItemSet() override;
    virtual bool        IsUsed           () const override;
    virtual bool        HasFollowSupport () const override;
    virtual bool        HasParentSupport () const override;

    virtual bool SetName(const OUString& rNewName, bool bReindexNow = true) override;

    void                SetUsage( ScStyleSheet::Usage eUse ) const { eUsage = eUse; }
    ScStyleSheet::Usage GetUsage() const { return eUsage; }

    /// Fix for expensive dynamic_cast
    virtual bool isScStyleSheet() const override { return true; }
private:
    virtual             ~ScStyleSheet() override;

                ScStyleSheet( const OUString&   rName,
                              const ScStyleSheetPool& rPool,
                              SfxStyleFamily    eFamily,
                              SfxStyleSearchBits nMask );

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    mutable ScStyleSheet::Usage eUsage;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
