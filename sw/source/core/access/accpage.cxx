 /*************************************************************************
 *
 *  $RCSfile: accpage.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2002-05-22 11:38:22 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _ACCPAGE_HXX
#include "accpage.hxx"
#endif

#ifndef _ACCESS_HRC
#include "access.hrc"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::drafts::com::sun::star::accessibility::XAccessibleContext;


SwAccessiblePage::SwAccessiblePage( SwAccessibleMap* pMap,
                                    const SwPageFrm *pFrame ) :
    SwAccessibleContext( pMap, 0, pFrame )
{
    DBG_ASSERT( pFrame != NULL, "need frame" );
    DBG_ASSERT( pMap != NULL, "need map" );
}

SwAccessiblePage::SwAccessiblePage( SwAccessibleMap* pMap,
                                    const SwFrm *pFrame ) :
    SwAccessibleContext( pMap, 0, pFrame )
{
    DBG_ASSERT( pFrame != NULL, "need frame" );
    DBG_ASSERT( pMap != NULL, "need map" );
    DBG_ASSERT( pFrame->IsPageFrm(), "need page frame" );
}

SwAccessiblePage::~SwAccessiblePage()
{
}

OUString SwAccessiblePage::getImplementationName( )
    throw( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SwAccessiblePage" ) );
}

sal_Bool SwAccessiblePage::supportsService( const OUString& rServiceName)
    throw( RuntimeException )
{
    return rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "bla" ) );
}

Sequence<OUString> SwAccessiblePage::getSupportedServiceNames( )
    throw( RuntimeException )
{
    Sequence<OUString> aSeq( 1 );
    aSeq[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "bla" ) );
    return aSeq;
}


OUString SwAccessiblePage::GetPageResource( sal_uInt16 nResId )
{
    OUString sPage = OUString::valueOf(
        static_cast<sal_Int32>(
            static_cast<const SwPageFrm*>( GetFrm() )->GetPhyPageNum() ) );
    return SwAccessibleContext::GetResource( nResId, &sPage );
}

OUString SwAccessiblePage::getAccessibleDescription( )
    throw( RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleContext );

    return GetPageResource( STR_ACCESS_PAGE_DESC );
}

OUString SwAccessiblePage::getAccessibleName( )
    throw( RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleContext );

    return GetPageResource( STR_ACCESS_PAGE_NAME );
}
