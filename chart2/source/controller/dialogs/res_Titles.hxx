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
#ifndef _CHART2_RES_TITLES_HXX
#define _CHART2_RES_TITLES_HXX

#include "TitleDialogData.hxx"
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>

namespace chart
{

class TitleResources
{
public:
    TitleResources( Window* pParent, bool bShowSecondaryAxesTitle );
    virtual ~TitleResources();

    void writeToResources( const TitleDialogData& rInput );
    void readFromResources( TitleDialogData& rOutput );

    void SetUpdateDataHdl( const Link& rLink );
    bool IsModified();
    void ClearModifyFlag();

private:
    FixedText           m_aFT_Main;
    Edit                m_aEd_Main;
    FixedText           m_aFT_Sub;
    Edit                m_aEd_Sub;

    FixedLine           m_aFL_Axes;
    FixedText           m_aFT_XAxis;
    Edit                m_aEd_XAxis;
    FixedText           m_aFT_YAxis;
    Edit                m_aEd_YAxis;
    FixedText           m_aFT_ZAxis;
    Edit                m_aEd_ZAxis;

    FixedLine           m_aFL_SecondaryAxes;
    FixedText           m_aFT_SecondaryXAxis;
    Edit                m_aEd_SecondaryXAxis;
    FixedText           m_aFT_SecondaryYAxis;
    Edit                m_aEd_SecondaryYAxis;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
