/*************************************************************************
 *
 *  $RCSfile: accfrmobj.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mib $ $Date: 2002-04-05 12:05:18 $
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
#ifndef _ACCFRMOBJ_HXX
#define _ACCFRMOBJ_HXX

#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif

class SwFrmOrObj
{
    const SdrObject *pObj;
    const SwFrm *pFrm;

    inline Init( const SdrObject *pO );
    inline Init( const SwFrm *pF );

public:

    inline SwFrmOrObj();
    inline SwFrmOrObj( const SdrObject *pO );
    inline SwFrmOrObj( const SwFrm *pF );
    inline SwFrmOrObj( const SwFrmOrObj& r );

    inline SwFrmOrObj& operator=( const SwFrmOrObj& r );
    inline SwFrmOrObj& operator=( const SdrObject *pO );
    inline SwFrmOrObj& operator=( const SwFrm *pF );

    inline sal_Bool operator==( const SwFrmOrObj& r ) const;
    inline sal_Bool operator==( const SdrObject *pO ) const;
    inline sal_Bool operator==( const SwFrm *pF ) const;

    inline sal_Bool IsValid() const;

    inline const SdrObject *GetSdrObject() const;
    inline const SwFrm *GetSwFrm() const;

    inline sal_Bool IsAccessible() const;
    inline sal_Bool IsBoundAsChar() const;
    inline SwRect GetBox() const;
    inline SwRect GetBounds() const;
};

inline SwFrmOrObj::Init( const SdrObject *pO )
{
    pObj = pO;
    pFrm = pObj && pObj->IsWriterFlyFrame()
                ? static_cast < const SwVirtFlyDrawObj * >( pObj )->GetFlyFrm()
                : 0;
}

inline SwFrmOrObj::Init( const SwFrm *pF )
{
    pFrm = pF;
    pObj = pFrm && pFrm->IsFlyFrm()
                ? static_cast < const SwFlyFrm * >( pFrm )->GetVirtDrawObj()
                : 0;
}

inline SwFrmOrObj::SwFrmOrObj() :
    pObj( 0 ), pFrm( 0 )
{}

inline SwFrmOrObj::SwFrmOrObj( const SdrObject *pO )
{
    Init( pO );
}

inline SwFrmOrObj::SwFrmOrObj( const SwFrm *pF )
{
    Init( pF );
}

inline SwFrmOrObj::SwFrmOrObj( const SwFrmOrObj& r ) :
    pObj( r.pObj ), pFrm( r.pFrm )
{}

inline SwFrmOrObj& SwFrmOrObj::operator=( const SwFrmOrObj& r )
{
    pObj = r.pObj;
    pFrm = r.pFrm;
}

inline SwFrmOrObj& SwFrmOrObj::operator=( const SdrObject *pO )
{
    Init( pO );
    return *this;
}

inline SwFrmOrObj& SwFrmOrObj::operator=( const SwFrm *pF )
{
    Init( pF );
    return *this;
}

inline sal_Bool SwFrmOrObj::operator==( const SwFrmOrObj& r ) const
{
    return pObj == r.pObj && pFrm == r.pFrm;
}

inline sal_Bool SwFrmOrObj::operator==( const SdrObject *pO ) const
{
    return pObj == pO;
}

inline sal_Bool SwFrmOrObj::operator==( const SwFrm *pF ) const
{
    return pFrm == pF;
}

inline sal_Bool SwFrmOrObj::IsValid() const
{
    return pObj != 0 || pFrm != 0;
}

inline const SdrObject *SwFrmOrObj::GetSdrObject() const
{
    return pObj;
}

inline const SwFrm *SwFrmOrObj::GetSwFrm() const
{
    return pFrm;
}

inline sal_Bool SwFrmOrObj::IsAccessible() const
{
    // currently only SwFrms are accessible
    return pFrm && pFrm->IsAccessibleFrm();
}

inline sal_Bool SwFrmOrObj::IsBoundAsChar() const
{
    // currently only SwFrms are accessible
    return pFrm && pFrm->IsFlyFrm() &&
           static_cast< const SwFlyFrm *>(pFrm)->IsFlyInCntFrm();
}

inline SwRect SwFrmOrObj::GetBox() const
{
    if( pFrm )
        return pFrm->Frm();
    else if( pObj )
        return SwRect( pObj->GetBoundRect() );
}

inline SwRect SwFrmOrObj::GetBounds() const
{
    if( pFrm )
        return pFrm->PaintArea();
    else if( pObj )
        return SwRect( pObj->GetBoundRect() );
}


#endif
