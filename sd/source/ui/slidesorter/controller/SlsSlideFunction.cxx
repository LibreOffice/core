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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "controller/SlsSlideFunction.hxx"

#include "SlideSorter.hxx"
#include "controller/SlideSorterController.hxx"
#include "view/SlideSorterView.hxx"
#include "model/SlideSorterModel.hxx"


namespace sd { namespace slidesorter { namespace controller {

TYPEINIT1(SlideFunction, FuPoor);


SlideFunction::SlideFunction (
    SlideSorter& rSlideSorter,
    SfxRequest& rRequest)
    : FuPoor (
        rSlideSorter.GetViewShell(),
        rSlideSorter.GetContentWindow().get(),
        &rSlideSorter.GetView(),
        rSlideSorter.GetModel().GetDocument(),
        rRequest)
{
}

void SlideFunction::ScrollStart (void)
{
}

void SlideFunction::ScrollEnd (void)
{
}

sal_Bool SlideFunction::MouseMove(const MouseEvent& )
{
    return sal_False;
}

sal_Bool SlideFunction::MouseButtonUp(const MouseEvent& )
{
    return sal_False;

}

sal_Bool SlideFunction::MouseButtonDown(const MouseEvent& )
{
    return sal_False;
}

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
