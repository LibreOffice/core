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
#ifndef INCLUDED_SW_INC_FMTFLCNT_HXX
#define INCLUDED_SW_INC_FMTFLCNT_HXX

#include <svl/poolitem.hxx>

class SwFrmFmt;
class SwTxtFlyCnt;
// ATT_FLYCNT

class SwFmtFlyCnt : public SfxPoolItem
{
    friend class SwTxtFlyCnt;
    SwTxtFlyCnt* pTxtAttr;
    SwFrmFmt* pFmt; ///< My Fly/DrawFrame-format.
    /// protected CopyCtor
    SwFmtFlyCnt& operator=(const SwFmtFlyCnt& rFlyCnt);

public:
    SwFmtFlyCnt( SwFrmFmt *pFrmFmt );
    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;

    inline SwFrmFmt *GetFrmFmt() const { return pFmt; }
    /// For Undo: delete the FlyFrmFormat "logically"; it is kept in Undo-object.
    inline void SetFlyFmt( SwFrmFmt* pNew = 0 )   { pFmt = pNew; }

    const SwTxtFlyCnt *GetTxtFlyCnt() const { return pTxtAttr; }
          SwTxtFlyCnt *GetTxtFlyCnt()       { return pTxtAttr; }

    bool Sw3ioExportAllowed() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
