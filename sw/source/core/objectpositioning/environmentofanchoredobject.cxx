/*************************************************************************
 *
 *  $RCSfile: environmentofanchoredobject.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-28 13:42:54 $
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
#ifndef _ENVIRONMENTOFANCHOREDOBJECT
#include <environmentofanchoredobject.hxx>
#endif

#ifndef _FRAME_HXX
#include <frame.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif

using namespace objectpositioning;

SwEnvironmentOfAnchoredObject::SwEnvironmentOfAnchoredObject(
                                                const bool   _bFollowTextFlow )
    : mbFollowTextFlow( _bFollowTextFlow )
{}

SwEnvironmentOfAnchoredObject::~SwEnvironmentOfAnchoredObject()
{}

/** determine environment layout frame for possible horizontal object positions

    OD 05.11.2003

    @author OD
*/
const SwLayoutFrm& SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrm(
                                            const SwFrm& _rHoriOrientFrm,
                                            const bool _bForPageAlignment ) const
{
    const SwFrm* pHoriEnvironmentLayFrm = &_rHoriOrientFrm;

    if ( !mbFollowTextFlow )
    {
        if ( _bForPageAlignment )
        {
            while ( !pHoriEnvironmentLayFrm->IsCellFrm() &&
                    !pHoriEnvironmentLayFrm->IsPageFrm() )
            {
                pHoriEnvironmentLayFrm =
                    pHoriEnvironmentLayFrm->IsFlyFrm()
                    ? static_cast<const SwFlyFrm*>(pHoriEnvironmentLayFrm)->GetAnchorFrm()
                    : pHoriEnvironmentLayFrm->GetUpper();
                ASSERT( pHoriEnvironmentLayFrm,
                        "SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrm(..) - no page|cell frame found" );
            }
        }
        else
        {
            pHoriEnvironmentLayFrm = _rHoriOrientFrm.FindPageFrm();
        }
    }
    else
    {
        while ( !pHoriEnvironmentLayFrm->IsCellFrm() &&
                !pHoriEnvironmentLayFrm->IsFlyFrm() &&
                !pHoriEnvironmentLayFrm->IsPageFrm() )
        {
            pHoriEnvironmentLayFrm = pHoriEnvironmentLayFrm->GetUpper();
            ASSERT( pHoriEnvironmentLayFrm,
                    "SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrm(..) - no page|fly|cell frame found" );
        }
    }

    ASSERT( pHoriEnvironmentLayFrm->ISA(SwLayoutFrm),
                "SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrm(..) - found frame isn't a layout frame" );

    return static_cast<const SwLayoutFrm&>(*pHoriEnvironmentLayFrm);
}

/** determine environment layout frame for possible vertical object positions

    OD 05.11.2003

    @author OD
*/
const SwLayoutFrm& SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrm(
                                            const SwFrm& _rVertOrientFrm,
                                            const bool _bForPageAlignment ) const
{
    const SwFrm* pVertEnvironmentLayFrm = &_rVertOrientFrm;

    if ( !mbFollowTextFlow )
    {
        if ( _bForPageAlignment )
        {
            while ( !pVertEnvironmentLayFrm->IsCellFrm() &&
                    !pVertEnvironmentLayFrm->IsPageFrm() )
            {
                pVertEnvironmentLayFrm =
                    pVertEnvironmentLayFrm->IsFlyFrm()
                    ? static_cast<const SwFlyFrm*>(pVertEnvironmentLayFrm)->GetAnchorFrm()
                    : pVertEnvironmentLayFrm->GetUpper();
                ASSERT( pVertEnvironmentLayFrm,
                        "SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrm(..) - proposed frame not found" );
            }
        }
        else
        {
            pVertEnvironmentLayFrm = _rVertOrientFrm.FindPageFrm();
        }
    }
    else
    {
        while ( !pVertEnvironmentLayFrm->IsCellFrm() &&
                !pVertEnvironmentLayFrm->IsFlyFrm() &&
                !pVertEnvironmentLayFrm->IsHeaderFrm() &&
                !pVertEnvironmentLayFrm->IsFooterFrm() &&
                !pVertEnvironmentLayFrm->IsFtnFrm() &&
                !pVertEnvironmentLayFrm->IsPageBodyFrm() &&
                !pVertEnvironmentLayFrm->IsPageFrm() )
        {
            pVertEnvironmentLayFrm = pVertEnvironmentLayFrm->GetUpper();
            ASSERT( pVertEnvironmentLayFrm,
                    "SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrm(..) - proposed frame not found" );
        }
    }

    ASSERT( pVertEnvironmentLayFrm->ISA(SwLayoutFrm),
                "SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrm(..) - found frame isn't a layout frame" );

    return static_cast<const SwLayoutFrm&>(*pVertEnvironmentLayFrm);
}
