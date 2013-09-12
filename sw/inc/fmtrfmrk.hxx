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
#ifndef _FMTRFMRK_HXX
#define _FMTRFMRK_HXX


#include <tools/string.hxx>
#include <svl/poolitem.hxx>

class SwTxtRefMark;

// ATT_REFMARK

class SwFmtRefMark : public SfxPoolItem
{
    friend class SwTxtRefMark;
    SwTxtRefMark* pTxtAttr;

    /// Protected CopyCtor.
    SwFmtRefMark& operator=(const SwFmtRefMark& rRefMark);
    OUString aRefName;

public:
    SwFmtRefMark( const OUString& rTxt );
    SwFmtRefMark( const SwFmtRefMark& rRefMark );
    ~SwFmtRefMark( );

    /// "Pure virtual methods" of SfxPoolItem.
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    const SwTxtRefMark *GetTxtRefMark() const   { return pTxtAttr; }
    SwTxtRefMark *GetTxtRefMark()               { return pTxtAttr; }

    inline       OUString &GetRefName()       { return aRefName; }
    inline const OUString &GetRefName() const { return aRefName; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
