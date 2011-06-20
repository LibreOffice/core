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
#ifndef _CHART2_TP_TITLEROTATION_HXX
#define _CHART2_TP_TITLEROTATION_HXX

// header for SfxTabPage
#include <sfx2/tabdlg.hxx>
#include <svx/dialcontrol.hxx>
#include <svx/wrapfield.hxx>
#include <svx/orienthelper.hxx>
#include <vcl/fixed.hxx>
#include "TextDirectionListBox.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

class SchAlignmentTabPage : public SfxTabPage
{
private:
    FixedLine               aFlAlign;
    svx::DialControl        aCtrlDial;
    FixedText               aFtRotate;
    svx::WrapField          aNfRotate;
    TriStateBox             aCbStacked;
    svx::OrientationHelper  aOrientHlp;
    FixedText               aFtTextDirection;
    TextDirectionListBox    aLbTextDirection;

public:
    SchAlignmentTabPage(Window* pParent, const SfxItemSet& rInAttrs, bool bWithRotation = true);
    virtual ~SchAlignmentTabPage();

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rInAttrs);
    static SfxTabPage* CreateWithoutRotation(Window* pParent, const SfxItemSet& rInAttrs);
    virtual sal_Bool FillItemSet(SfxItemSet& rOutAttrs);
    virtual void Reset(const SfxItemSet& rInAttrs);
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
