/*************************************************************************
 *
 *  $RCSfile: accmap.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: mib $ $Date: 2002-03-21 12:56:53 $
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
#ifndef _ACCMAP_HXX
#define _ACCMAP_HXX

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessible.hpp>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

class Rectangle;
class SwFrm;
class SwRootFrm;
class SwAccessibleContext;
class SwAccessibleContextMap_Impl;
class SwAccessibleEventList_Impl;
class SwAccessibleEventMap_Impl;
struct SwAccessibleEvent_Impl;
class SwRect;
class ViewShell;

#define ACC_STATE_EDITABLE 0x01
#define ACC_STATE_OPAQUE 0x02
#define ACC_STATE_CARET 0x80

#define ACC_STATE_MASK 0x7F

class SwAccessibleMap
{
    ::vos::OMutex aMutex;
    ::vos::OMutex aEventMutex;
    SwAccessibleContextMap_Impl *pMap;
    SwAccessibleEventList_Impl *pEvents;
    SwAccessibleEventMap_Impl *pEventMap;
    ViewShell *pVSh;
    sal_Int32 nPara;
    sal_Int32 nFootnote;
    sal_Int32 nEndnote;

    static void FireEvent( const SwAccessibleEvent_Impl& rEvent );
    void AppendEvent( const SwAccessibleEvent_Impl& rEvent );

    void InvalidateCaretPosition(
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessible>& rAcc );

public:

    SwAccessibleMap( ViewShell *pSh );
    ~SwAccessibleMap();

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible> GetDocumentView();

    ::vos::ORef < SwAccessibleContext > GetContextImpl(
                                                 const SwFrm *pFrm,
                                                sal_Bool bCreate = sal_True );
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible> GetContext(
                                                 const SwFrm *pFrm,
                                                sal_Bool bCreate = sal_True );

    ViewShell *GetShell() { return pVSh; }

    void RemoveContext( const SwFrm *pFrm );

    void DisposeFrm( const SwFrm *pFrm );

    void MoveFrm( const SwFrm *pFrm, const SwRect& rOldFrm );

    void InvalidateFrmContent( const SwFrm *pFrm );

    void InvalidateCaretPosition( const SwFrm *pFrm );

    void SetCaretContext(
        const ::vos::ORef < SwAccessibleContext >& rCaretContext );

    void InvalidateStates( sal_uInt8 nStates );

    void FireEvents();
};

#endif
