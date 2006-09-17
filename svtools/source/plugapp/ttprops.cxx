/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ttprops.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:22:52 $
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
#include "precompiled_svtools.hxx"

#ifndef _SVTOOLS_TTPROPS_HXX // handmade
#include "ttprops.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif
#ifndef _RTTI_HXX // manual
#include <tools/rtti.hxx>
#endif

TYPEINIT1( TTProperties, ApplicationProperty )

BOOL TTProperties::RequestProperty( USHORT nRequest )
{
    if ( (( nRequest & TT_PR_ONCE ) == 0) || (nDonePRs & (nRequest & 0x0ff)) == 0 )
    {
        nActualPR = nRequest;
        nDonePRs |= nRequest;
        GetpApp()->Property( *this );
        return nActualPR == 0;
    }
    return TRUE;
}


BOOL TTProperties::GetSlots()
{
    RequestProperty( TT_PR_SLOTS );
    return HasSlots();
}

USHORT TTProperties::ExecuteFunction( USHORT nSID, SfxPoolItem** ppArgs, USHORT nMode )
{
    mnSID = nSID;
    mppArgs = ppArgs;
    mnMode = nMode;
    RequestProperty( TT_PR_DISPATCHER );
    mppArgs = NULL;
    return nActualPR;
}

BOOL TTProperties::Img( Bitmap *pBmp )
{
    BOOL bRet;
    mpBmp = pBmp;
    bRet = RequestProperty( TT_PR_IMG );
    mpBmp = NULL;
    return bRet;
}

SvtResId TTProperties::GetSvtResId( USHORT nId )
{
    return SvtResId( nId );
}

