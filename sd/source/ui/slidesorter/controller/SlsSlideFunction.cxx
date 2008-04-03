/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsSlideFunction.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:30:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        rSlideSorter.GetView().GetWindow(),
        &rSlideSorter.GetView(),
        rSlideSorter.GetModel().GetDocument(),
        rRequest)
{
}

FunctionReference SlideFunction::Create( SlideSorter& rSlideSorter, SfxRequest& rRequest )
{
    FunctionReference xFunc( new SlideFunction( rSlideSorter, rRequest ) );
    return xFunc;
}

void SlideFunction::ScrollStart (void)
{
}

void SlideFunction::ScrollEnd (void)
{
}

BOOL SlideFunction::MouseMove(const MouseEvent& )
{
    return FALSE;
}

BOOL SlideFunction::MouseButtonUp(const MouseEvent& )
{
    return FALSE;

}

BOOL SlideFunction::MouseButtonDown(const MouseEvent& )
{
    return FALSE;
}

} } } // end of namespace ::sd::slidesorter::controller
