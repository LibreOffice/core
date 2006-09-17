/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoedsrc.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:17:08 $
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
#include "precompiled_svx.hxx"

#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif

#include "unoedsrc.hxx"


//------------------------------------------------------------------------

void SvxEditSource::addRange( SvxUnoTextRangeBase* )
{
}

//------------------------------------------------------------------------

void SvxEditSource::removeRange( SvxUnoTextRangeBase* )
{
}

//------------------------------------------------------------------------

const SvxUnoTextRangeBaseList& SvxEditSource::getRanges() const
{
    static SvxUnoTextRangeBaseList gList;
    return gList;
}

//------------------------------------------------------------------------

SvxTextForwarder::~SvxTextForwarder()
{
}

//------------------------------------------------------------------------

SvxViewForwarder::~SvxViewForwarder()
{
}

//------------------------------------------------------------------------

SvxEditSource::~SvxEditSource()
{
}

SvxViewForwarder* SvxEditSource::GetViewForwarder()
{
    return NULL;
}

SvxEditViewForwarder* SvxEditSource::GetEditViewForwarder( sal_Bool )
{
    return NULL;
}

SfxBroadcaster& SvxEditSource::GetBroadcaster() const
{
    DBG_ERROR("SvxEditSource::GetBroadcaster called for implementation missing this feature!");

    static SfxBroadcaster aBroadcaster;

    return aBroadcaster;
}
