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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_RES_TITLES_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_RES_TITLES_HXX

#include "TitleDialogData.hxx"
#include <vcl/builder.hxx>
#include <vcl/fixed.hxx>

namespace chart
{

class TitleResources
{
public:
    TitleResources(VclBuilderContainer& rParent, bool bShowSecondaryAxesTitle);
    virtual ~TitleResources();

    void writeToResources( const TitleDialogData& rInput );
    void readFromResources( TitleDialogData& rOutput );

    void SetUpdateDataHdl( const Link<Edit&,void>& rLink );
    bool IsModified();
    void ClearModifyFlag();

private:
    VclPtr<FixedText> m_pFT_Main;
    VclPtr<FixedText> m_pFT_Sub;
    VclPtr<Edit> m_pEd_Main;
    VclPtr<Edit> m_pEd_Sub;

    VclPtr<FixedText> m_pFT_XAxis;
    VclPtr<FixedText> m_pFT_YAxis;
    VclPtr<FixedText> m_pFT_ZAxis;
    VclPtr<Edit> m_pEd_XAxis;
    VclPtr<Edit> m_pEd_YAxis;
    VclPtr<Edit> m_pEd_ZAxis;

    VclPtr<FixedText> m_pFT_SecondaryXAxis;
    VclPtr<FixedText> m_pFT_SecondaryYAxis;
    VclPtr<Edit> m_pEd_SecondaryXAxis;
    VclPtr<Edit> m_pEd_SecondaryYAxis;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
