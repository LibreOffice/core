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


#include "SlideSorterViewShellBase.hxx"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#include "strings.hrc"
#include "framework/FrameworkHelper.hxx"

namespace sd {

class DrawDocShell;

TYPEINIT1(SlideSorterViewShellBase, ViewShellBase);

// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new SlideSorterViewShellBase object has been constructed.

SfxViewFactory* SlideSorterViewShellBase::pFactory;
SfxViewShell* SlideSorterViewShellBase::CreateInstance (
    SfxViewFrame *pFrame, SfxViewShell *pOldView)
{
    SlideSorterViewShellBase* pBase = new SlideSorterViewShellBase(pFrame, pOldView);
    pBase->LateInit(framework::FrameworkHelper::msSlideSorterURL);
    return pBase;
}




void SlideSorterViewShellBase::RegisterFactory( sal_uInt16 nPrio )
{
    pFactory = new SfxViewFactory(&CreateInstance,nPrio,"SlideSorter");
    InitFactory();
}




void SlideSorterViewShellBase::InitFactory()
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}







SlideSorterViewShellBase::SlideSorterViewShellBase (
    SfxViewFrame* _pFrame,
    SfxViewShell* pOldShell)
    : ImpressViewShellBase (_pFrame, pOldShell)
{
}




SlideSorterViewShellBase::~SlideSorterViewShellBase (void)
{
}




} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
