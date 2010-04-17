/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _ACCFRMOBJ_HXX
#define _ACCFRMOBJ_HXX
#include <flyfrm.hxx>
#include <pagefrm.hxx>
#include <dflyobj.hxx>
#include <swtable.hxx>

class SwFrmOrObj
{
    const SdrObject *pObj;
    const SwFrm *pFrm;

    inline void Init( const SdrObject *pO );
    inline void Init( const SwFrm *pF );

public:

    inline SwFrmOrObj();
    inline SwFrmOrObj( const SdrObject *pO );
    inline SwFrmOrObj( const SwFrm *pF );
    inline SwFrmOrObj( const SwFrm *pF, const SdrObject *pO );
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

    sal_Bool IsAccessible( sal_Bool bPagePreview ) const;
    sal_Bool IsBoundAsChar() const;
    inline sal_Bool IsVisibleChildrenOnly() const;
    inline SwRect GetBox() const;
    inline SwRect GetBounds() const;
};

inline void SwFrmOrObj::Init( const SdrObject *pO )
{
    pObj = pO;
    // #110094#-1
    pFrm = pObj && pObj->ISA(SwVirtFlyDrawObj)
                ? static_cast < const SwVirtFlyDrawObj * >( pObj )->GetFlyFrm()
                : 0;
}

inline void SwFrmOrObj::Init( const SwFrm *pF )
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

inline SwFrmOrObj::SwFrmOrObj( const SwFrm *pF, const SdrObject *pO )
{
    if( pF )
        Init( pF );
    else
        Init( pO );
    ASSERT( (!pF || pF == pFrm) && (!pO || pO == pObj),
            "invalid frame/object combination" );

}

inline SwFrmOrObj::SwFrmOrObj( const SwFrmOrObj& r ) :
    pObj( r.pObj ), pFrm( r.pFrm )
{}

inline SwFrmOrObj& SwFrmOrObj::operator=( const SwFrmOrObj& r )
{
    pObj = r.pObj;
    pFrm = r.pFrm;
    return *this;
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

inline sal_Bool SwFrmOrObj::IsVisibleChildrenOnly() const
{
    return !pFrm || pFrm->IsRootFrm() ||
           !( pFrm->IsTabFrm() || pFrm->IsInTab() ||
              ( IsBoundAsChar() &&
                static_cast<const SwFlyFrm*>(pFrm)->GetAnchorFrm()->IsInTab()) );
}

inline SwRect SwFrmOrObj::GetBox() const
{
    if( pFrm )
    {
        if( pFrm->IsPageFrm() &&
            static_cast< const SwPageFrm * >( pFrm )->IsEmptyPage() )
        {
            SwRect aBox( pFrm->Frm().Left(), pFrm->Frm().Top()-1, 1, 1 );
            return aBox;
        }
        else if ( pFrm->IsTabFrm() )
        {
            SwRect aBox( pFrm->Frm() );
            aBox.Intersection( pFrm->GetUpper()->Frm() );
            return aBox;
        }
        else
            return pFrm->Frm();
    }
    else if( pObj )
        return SwRect( pObj->GetCurrentBoundRect() );
    else
        return SwRect();
}

inline SwRect SwFrmOrObj::GetBounds() const
{
    if( pFrm )
    {
        if( pFrm->IsPageFrm() &&
            static_cast< const SwPageFrm * >( pFrm )->IsEmptyPage() )
        {
            SwRect aBox( pFrm->Frm().Left(), pFrm->Frm().Top()-1, 0, 0 );
            return aBox;
        }
        else
            return pFrm->PaintArea();
    }
    else if( pObj )
        return SwRect( pObj->GetCurrentBoundRect() );
    return SwRect();
}


#endif
