 /*************************************************************************
 *
 *  $RCSfile: accnotextframe.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 13:37:29 $
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


#pragma hdrstop

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif

#ifndef _ACCNOTEXTFRAME_HXX
#include "accnotextframe.hxx"
#endif

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::accessibility;
using namespace ::rtl;

const SwNoTxtNode *SwAccessibleNoTextFrame::GetNoTxtNode() const
{
    const SwNoTxtNode *pNd  = 0;
    const SwFlyFrm *pFlyFrm = static_cast< const SwFlyFrm *>( GetFrm() );
    if( pFlyFrm->Lower() && pFlyFrm->Lower()->IsNoTxtFrm() )
    {
        const SwCntntFrm *pCntFrm =
            static_cast<const SwCntntFrm *>( pFlyFrm->Lower() );
        pNd = pCntFrm->GetNode()->GetNoTxtNode();
    }

    return pNd;
}

SwAccessibleNoTextFrame::SwAccessibleNoTextFrame(
        SwAccessibleMap *pMap, sal_Int16 nRole,
        const SwFlyFrm *pFlyFrm ) :
    SwAccessibleFrameBase( pMap, nRole, pFlyFrm ),
    aDepend( this, const_cast < SwNoTxtNode * >( GetNoTxtNode() ) )
{
    const SwNoTxtNode *pNd = GetNoTxtNode();
    if( pNd )
        sDesc = OUString( pNd->GetAlternateText() );
    if( !sDesc.getLength() )
        sDesc = GetName();
}

SwAccessibleNoTextFrame::~SwAccessibleNoTextFrame()
{
}

void SwAccessibleNoTextFrame::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    SwAccessibleFrameBase::Modify( pOld, pNew );

    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
    const SwNoTxtNode *pNd = GetNoTxtNode();
    ASSERT( pNd == aDepend.GetRegisteredIn(), "invalid frame" );
    switch( nWhich )
    {
    case RES_NAME_CHANGED:
        if( pNd->GetAlternateText().Len() )
            break;
    case RES_ALT_TEXT_CHANGED:
        if( pNd && GetFrm() )
        {
            OUString sOldDesc( sDesc );

            const String& rDesc = pNd->GetAlternateText();
            sDesc = rDesc;
            if( !sDesc.getLength() )
                sDesc = GetName();

            if( sDesc != sOldDesc )
            {
                AccessibleEventObject aEvent;
                aEvent.EventId = AccessibleEventId::ACCESSIBLE_DESCRIPTION_EVENT;
                aEvent.OldValue <<= sOldDesc;
                aEvent.NewValue <<= sDesc;
                FireAccessibleEvent( aEvent );
            }
        }
        break;
        /*
    case RES_OBJECTDYING:
        if( aDepend.GetRegisteredIn() ==
                static_cast< SwModify *>( static_cast< SwPtrMsgPoolItem * >( pOld )->pObject ) )
            const_cast < SwModify *>( aDepend.GetRegisteredIn()->Remove( aDepend );
        break;

    case RES_FMT_CHG:
        if( static_cast< SwFmtChg * >(pNew)->pChangedFmt == GetRegisteredIn() &&
            static_cast< SwFmtChg * >(pOld)->pChangedFmt->IsFmtInDTOR() )
            pRegisteredIn->Remove( this );
        break;
    */
    }
}

void SwAccessibleNoTextFrame::Dispose( sal_Bool bRecursive )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( aDepend.GetRegisteredIn() )
        const_cast < SwModify *>( aDepend.GetRegisteredIn() )->Remove( &aDepend );

    SwAccessibleFrameBase::Dispose( bRecursive );
}

OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    return sDesc;
}



//
// XInterface
//

Any SAL_CALL SwAccessibleNoTextFrame::queryInterface(
    const ::com::sun::star::uno::Type& aType )
    throw (RuntimeException)
{
    if( aType ==
        ::getCppuType( static_cast<Reference<XAccessibleImage>*>( NULL ) ) )
    {
        Reference<XAccessibleImage> xImage = this;
        Any aAny;
        aAny <<= xImage;
        return aAny;
    }
    else
        return SwAccessibleContext::queryInterface( aType );
}


//====== XTypeProvider ====================================================
Sequence< Type > SAL_CALL SwAccessibleNoTextFrame::getTypes() throw(RuntimeException)
{
    Sequence< ::com::sun::star::uno::Type > aTypes( SwAccessibleFrameBase::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    aTypes.realloc( nIndex + 1 );

    ::com::sun::star::uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex] = ::getCppuType( static_cast< Reference< XAccessibleImage > * >( 0 ) );

    return aTypes;
}


//
// XAccessibleImage
//

// implementation of the XAccessibleImage methods is a no-brainer, as
// all releveant information is already accessible through other
// methods. So we just delegate to those.

OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageDescription()
    throw ( RuntimeException )
{
    return getAccessibleDescription();
}

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageHeight(  )
    throw ( RuntimeException )
{
    return getSize().Height;
}

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageWidth(  )
    throw ( RuntimeException )
{
    return getSize().Width;
}
