/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swframeposstrings.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:41:53 $
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

#ifndef _SVXSWFRAMEPOSSTRINGS_HXX
#include <swframeposstrings.hxx>
#endif
#ifndef _TOOLS_RC_HXX
#include <tools/rc.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVX_DIALMGR_HXX
#include <dialmgr.hxx>
#endif
#include <dialogs.hrc>

class SvxSwFramePosString_Impl : public Resource
{
    friend class SvxSwFramePosString;
    String aStrings[SvxSwFramePosString::STR_MAX];
public:
    SvxSwFramePosString_Impl();
};
SvxSwFramePosString_Impl::SvxSwFramePosString_Impl() :
    Resource(SVX_RES(RID_SVXSW_FRAMEPOSITIONS))
{
    for(USHORT i = 0; i < SvxSwFramePosString::STR_MAX; i++)
    {
        //string ids have to start at 1
        aStrings[i] = String(ResId(i + 1));
    }
    FreeResource();
}
/*-- 04.03.2004 13:14:48---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxSwFramePosString::SvxSwFramePosString() :
    pImpl(new SvxSwFramePosString_Impl)
{
}
/*-- 04.03.2004 13:14:48---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxSwFramePosString::~SvxSwFramePosString()
{
    delete pImpl;
}
/*-- 04.03.2004 13:14:48---------------------------------------------------

  -----------------------------------------------------------------------*/
const String& SvxSwFramePosString::GetString(StringId eId)
{
    DBG_ASSERT(eId >= 0 && eId < STR_MAX, "invalid StringId")
    if(!(eId >= 0 && eId < STR_MAX))
        eId = LEFT;
    return pImpl->aStrings[eId];
}
