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
#ifndef INCLUDED_SW_SOURCE_UI_INC_COLEX_HXX
#define INCLUDED_SW_SOURCE_UI_INC_COLEX_HXX

#include <svx/pagectrl.hxx>
#include <editeng/paperinf.hxx>
#include "swdllapi.h"
#include <fmtclds.hxx>
#include "frmatr.hxx"

class SwColMgr;
class SfxItemSet;

class SW_DLLPUBLIC SwPageExample : public SvxPageWindow
{
public:
    SwPageExample(Window* pPar)
        : SvxPageWindow(pPar)
    {
        SetSize(SvxPaperInfo::GetPaperSize(PAPER_A4));
    }

    void UpdateExample( const SfxItemSet& rSet );
};

class SwTextGridItem;

class SW_DLLPUBLIC SwPageGridExample : public SwPageExample
{
    SwTextGridItem*     pGridItem;
    bool            m_bVertical;
protected:
    virtual void DrawPage( const Point& rPoint,
                           const bool bSecond,
                           const bool bEnabled );
public:
    SwPageGridExample(Window* pPar) :
                                SwPageExample(pPar),
                                pGridItem(0),
                                m_bVertical(false){}

    ~SwPageGridExample();
    void UpdateExample( const SfxItemSet& rSet );
};

class SW_DLLPUBLIC SwColExample : public SwPageExample
{
    SwColMgr*   pColMgr;

    using SwPageExample::UpdateExample;

protected:
    virtual void DrawPage( const Point& rPoint,
                           const bool bSecond,
                           const bool bEnabled );

public:
    SwColExample(Window* pPar)
        : SwPageExample(pPar)
        , pColMgr(0)
    {
    }

    void UpdateExample( const SfxItemSet& rSet, SwColMgr* pMgr  )
    {
        pColMgr = pMgr;
        SwPageExample::UpdateExample(rSet);
    }
};

class SW_DLLPUBLIC SwColumnOnlyExample : public Window
{
private:
    Size        m_aWinSize;

    Size        m_aFrmSize;
    SwFmtCol    m_aCols;

protected:
    virtual void Paint( const Rectangle& rRect );

public:
    SwColumnOnlyExample(Window*);

    void        SetColumns(const SwFmtCol& rCol);

    virtual Size GetOptimalSize() const;
};

#endif // INCLUDED_SW_SOURCE_UI_INC_COLEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
