 /*************************************************************************
 *
 *  $RCSfile: accfootnote.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:39:18 $
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

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _FTNFRM_HXX
#include <ftnfrm.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _PAGEFRM_HXX
//#include <pagefrm.hxx>
#endif
#ifndef _PAGEDESC_HXX
//#include <pagedesc.hxx>
#endif

#ifndef _FLDBAS_HXX
//#include <fldbas.hxx>
#endif
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif
#ifndef _ACCFOOTNOTE_HXX
#include "accfootnote.hxx"
#endif
#ifndef _ACCESS_HRC
#include "access.hrc"
#endif

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::accessibility;
using namespace ::rtl;

const sal_Char sServiceNameFootnote[] = "drafts.com.sun.star.text.AccessibleFootnoteView";
const sal_Char sServiceNameEndnote[] = "drafts.com.sun.star.text.AccessibleEndnoteView";
const sal_Char sImplementationNameFootnote[] = "com.sun.star.comp.Writer.SwAccessibleFootnoteView";
const sal_Char sImplementationNameEndnote[] = "com.sun.star.comp.Writer.SwAccessibleEndnoteView";

SwAccessibleFootnote::SwAccessibleFootnote(
        SwAccessibleMap *pMap,
        sal_Bool bIsEndnote,
        sal_Int32 nFootEndNote,
        const SwFtnFrm *pFtnFrm ) :
    SwAccessibleContext( pMap,
        bIsEndnote ? AccessibleRole::ENDNOTE : AccessibleRole::FOOTNOTE,
        pFtnFrm )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    sal_uInt16 nResId = bIsEndnote ? STR_ACCESS_ENDNOTE_NAME
                                   : STR_ACCESS_FOOTNOTE_NAME;
    OUString sArg( OUString::valueOf( nFootEndNote ) );
    SetName( GetResource( nResId, &sArg ) );
}

SwAccessibleFootnote::~SwAccessibleFootnote()
{
}

OUString SAL_CALL SwAccessibleFootnote::getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    sal_uInt16 nResId = AccessibleRole::ENDNOTE == GetRole()
        ? STR_ACCESS_ENDNOTE_DESC
        : STR_ACCESS_FOOTNOTE_DESC ;

    OUString sArg;
    const SwTxtFtn *pTxtFtn =
        static_cast< const SwFtnFrm *>( GetFrm() )->GetAttr();
    if( pTxtFtn )
    {
        const SwDoc *pDoc = GetMap()->GetShell()->GetDoc();
        sArg = pTxtFtn->GetFtn().GetViewNumStr( *pDoc );
    }

    return GetResource( nResId, &sArg );
}

OUString SAL_CALL SwAccessibleFootnote::getImplementationName()
        throw( RuntimeException )
{
    if( AccessibleRole::ENDNOTE == GetRole() )
        return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationNameEndnote));
    else
        return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationNameFootnote));
}

sal_Bool SAL_CALL SwAccessibleFootnote::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (::com::sun::star::uno::RuntimeException)
{
    if( sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                       sizeof(sAccessibleServiceName)-1 ) )
        return sal_True;
    else if( AccessibleRole::ENDNOTE == GetRole() )
        return sTestServiceName.equalsAsciiL( sServiceNameEndnote, sizeof(sServiceNameEndnote)-1 );
    else
        return sTestServiceName.equalsAsciiL( sServiceNameFootnote, sizeof(sServiceNameFootnote)-1 );

}

Sequence< OUString > SAL_CALL SwAccessibleFootnote::getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    if( AccessibleRole::ENDNOTE == GetRole() )
        pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceNameEndnote) );
    else
        pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceNameFootnote) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

Sequence< sal_Int8 > SAL_CALL SwAccessibleFootnote::getImplementationId()
        throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

sal_Bool SwAccessibleFootnote::IsEndnote( const SwFtnFrm *pFtnFrm )
{
    const SwTxtFtn *pTxtFtn = pFtnFrm ->GetAttr();
    return pTxtFtn && pTxtFtn->GetFtn().IsEndNote() ;
}
