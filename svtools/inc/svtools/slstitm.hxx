/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slstitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:34:12 $
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
#ifndef _SFXSLSTITM_HXX
#define _SFXSLSTITM_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _RTTI_HXX //autogen
#include <tools/rtti.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

class SfxImpStringList;

class SVL_DLLPUBLIC SfxStringListItem : public SfxPoolItem
{
protected:
    SfxImpStringList*   pImp;

public:
    TYPEINFO();

    SfxStringListItem();
    SfxStringListItem( USHORT nWhich, const List* pList=NULL );
    SfxStringListItem( USHORT nWhich, SvStream& rStream );
    SfxStringListItem( const SfxStringListItem& rItem );
    ~SfxStringListItem();

    List *                  GetList();

    const List *            GetList() const
    { return SAL_CONST_CAST(SfxStringListItem *, this)->GetList(); }

#ifndef TF_POOLABLE
    virtual int             IsPoolable() const;
#endif

    // String-Separator: \n
    virtual void            SetString( const XubString& );
    virtual XubString       GetString();

    void                    SetStringList( const com::sun::star::uno::Sequence< rtl::OUString >& rList );
    void                    GetStringList( com::sun::star::uno::Sequence< rtl::OUString >& rList ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream &, USHORT nVersion ) const;
    virtual SvStream&       Store( SvStream &, USHORT nItemVersion ) const;
    void                    Sort( BOOL bAscending = TRUE, List* pParallelList = 0 );

    virtual BOOL            PutValue  ( const com::sun::star::uno::Any& rVal,
                                         BYTE nMemberId = 0 );
    virtual BOOL            QueryValue( com::sun::star::uno::Any& rVal,
                                         BYTE nMemberId = 0 ) const;
};
#endif
