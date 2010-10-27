/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CHART2_RES_TITLES_HXX
#define _CHART2_RES_TITLES_HXX

#include "TitleDialogData.hxx"
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

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

//.............................................................................
} //namespace chart
//.............................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
