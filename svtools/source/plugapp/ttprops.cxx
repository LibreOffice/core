/*************************************************************************
 *
 *  $RCSfile: ttprops.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

