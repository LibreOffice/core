/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <bf_so3/linksrc.hxx>
#include "bf_so3/lnkbase.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <tools/debug.hxx>
#include <vcl/timer.hxx>
#include <bf_svtools/svarray.hxx>


using namespace ::com::sun::star::uno;

namespace binfilter
{

TYPEINIT0( SvLinkSource )

/************** class SvLinkSourceTimer *********************************/
class SvLinkSourceTimer : public Timer
{
    SvLinkSource *  pOwner;
    virtual void    Timeout();
public:
            SvLinkSourceTimer( SvLinkSource * pOwn );
};

SvLinkSourceTimer::SvLinkSourceTimer( SvLinkSource * pOwn )
    : pOwner( pOwn )
{
}

void SvLinkSourceTimer::Timeout()
{
    // sicher gegen zerstoeren im Handler
    SvLinkSourceRef aAdv( pOwner );
    pOwner->SendDataChanged();
}

static void StartTimer( SvLinkSourceTimer ** ppTimer, SvLinkSource * pOwner,
                        ULONG nTimeout )
{
    if( !*ppTimer )
    {
        *ppTimer = new SvLinkSourceTimer( pOwner );
        (*ppTimer)->SetTimeout( nTimeout );
        (*ppTimer)->Start();
    }
}


struct SvLinkSource_Entry_Impl
{
    SvBaseLinkRef		xSink;
    String				aDataMimeType;
    USHORT				nAdviseModes;
    BOOL 				bIsDataSink;

    SvLinkSource_Entry_Impl( SvBaseLink* pLink, const String& rMimeType,
                                USHORT nAdvMode )
        : xSink( pLink ), aDataMimeType( rMimeType ),
            nAdviseModes( nAdvMode ), bIsDataSink( TRUE )
    {}

    SvLinkSource_Entry_Impl( SvBaseLink* pLink )
        : xSink( pLink ), nAdviseModes( 0 ), bIsDataSink( FALSE )
    {}

    ~SvLinkSource_Entry_Impl();
};

SvLinkSource_Entry_Impl::~SvLinkSource_Entry_Impl()
{
}

typedef SvLinkSource_Entry_Impl* SvLinkSource_Entry_ImplPtr;
SV_DECL_PTRARR_DEL( SvLinkSource_Array_Impl, SvLinkSource_Entry_ImplPtr, 4, 4 )
SV_IMPL_PTRARR( SvLinkSource_Array_Impl, SvLinkSource_Entry_ImplPtr );

class SvLinkSource_EntryIter_Impl
{
    SvLinkSource_Array_Impl aArr;
    const SvLinkSource_Array_Impl& rOrigArr;
    USHORT nPos;
public:
    SvLinkSource_EntryIter_Impl( const SvLinkSource_Array_Impl& rArr );
    ~SvLinkSource_EntryIter_Impl();
    SvLinkSource_Entry_Impl* Curr()
                            { return nPos < aArr.Count() ? aArr[ nPos ] : 0; }
    SvLinkSource_Entry_Impl* Next();
};

SvLinkSource_EntryIter_Impl::SvLinkSource_EntryIter_Impl(
        const SvLinkSource_Array_Impl& rArr )
    : rOrigArr( rArr ), nPos( 0 )
{
    aArr.Insert( &rArr, 0 );
}
SvLinkSource_EntryIter_Impl::~SvLinkSource_EntryIter_Impl()
{
    aArr.Remove( 0, aArr.Count() );
}

SvLinkSource_Entry_Impl* SvLinkSource_EntryIter_Impl::Next()
{
    SvLinkSource_Entry_ImplPtr pRet = 0;
    if( nPos + 1 < aArr.Count() )
    {
        ++nPos;
        if( rOrigArr.Count() == aArr.Count() &&
            rOrigArr[ nPos ] == aArr[ nPos ] )
            pRet = aArr[ nPos ];
        else
        {
            // then we must search the current (or the next) in the orig
            do {
                pRet = aArr[ nPos ];
                if( USHRT_MAX != rOrigArr.GetPos( pRet ))
                    break;
                pRet = 0;
                ++nPos;
            } while( nPos < aArr.Count() );

            if( nPos >= aArr.Count() )
                pRet = 0;
        }
    }
    return pRet;
}

struct SvLinkSource_Impl
{
    SvLinkSource_Array_Impl aArr;
    String				aDataMimeType;
    SvLinkSourceTimer *	pTimer;
    ULONG				nTimeout;

    SvLinkSource_Impl() : pTimer( 0 ), nTimeout( 3000 ) {}
    ~SvLinkSource_Impl();

    void Closed();
};

SvLinkSource_Impl::~SvLinkSource_Impl()
{
    delete pTimer;
}

SvLinkSource::SvLinkSource()
     : pImpl( new SvLinkSource_Impl )
{
}

SvLinkSource::~SvLinkSource()
{
    delete pImpl;
}

void  SvLinkSource::Closed()
{
    SvLinkSource_EntryIter_Impl aIter( pImpl->aArr );
    for( SvLinkSource_Entry_Impl* p = aIter.Curr(); p; p = aIter.Next() )
        if( !p->bIsDataSink )
            p->xSink->Closed();
}

void SvLinkSource::SetUpdateTimeout( ULONG nTimeout )
{
    pImpl->nTimeout = nTimeout;
    if( pImpl->pTimer )
        pImpl->pTimer->SetTimeout( nTimeout );
}

void SvLinkSource::SendDataChanged()
{
    SvLinkSource_EntryIter_Impl aIter( pImpl->aArr );
    for( SvLinkSource_Entry_ImplPtr p = aIter.Curr(); p; p = aIter.Next() )
    {
        if( p->bIsDataSink )
        {
            String sDataMimeType( pImpl->aDataMimeType );
            if( !sDataMimeType.Len() )
                sDataMimeType = p->aDataMimeType;

            Any aVal;
            if( ( p->nAdviseModes & ADVISEMODE_NODATA ) ||
                GetData( aVal, sDataMimeType, TRUE ) )
            {
                p->xSink->DataChanged( sDataMimeType, aVal );

                if( p->nAdviseModes & ADVISEMODE_ONLYONCE )
                {
                    USHORT nFndPos = pImpl->aArr.GetPos( p );
                    if( USHRT_MAX != nFndPos )
                        pImpl->aArr.DeleteAndDestroy( nFndPos );
                }

            }
        }
    }
    if( pImpl->pTimer )
    {
        delete pImpl->pTimer;
        pImpl->pTimer = NULL;
    }
    pImpl->aDataMimeType.Erase();
}

void SvLinkSource::NotifyDataChanged()
{
    if( pImpl->nTimeout )
        StartTimer( &pImpl->pTimer, this, pImpl->nTimeout ); // Timeout neu
    else
    {
        SvLinkSource_EntryIter_Impl aIter( pImpl->aArr );
        for( SvLinkSource_Entry_ImplPtr p = aIter.Curr(); p; p = aIter.Next() )
            if( p->bIsDataSink )
            {
                Any aVal;
                if( ( p->nAdviseModes & ADVISEMODE_NODATA ) ||
                    GetData( aVal, p->aDataMimeType, TRUE ) )
                {
                    p->xSink->DataChanged( p->aDataMimeType, aVal );

                    if( p->nAdviseModes & ADVISEMODE_ONLYONCE )
                    {
                        USHORT nFndPos = pImpl->aArr.GetPos( p );
                        if( USHRT_MAX != nFndPos )
                            pImpl->aArr.DeleteAndDestroy( nFndPos );
                    }
                }
            }

            if( pImpl->pTimer )
            {
                delete pImpl->pTimer;
                pImpl->pTimer = NULL;
            }
    }
}

// notify the sink, the mime type is not
// a selection criterion
void SvLinkSource::DataChanged( const String & rMimeType,
                                const ::com::sun::star::uno::Any & rVal )
{
    if( pImpl->nTimeout && !rVal.hasValue() )
    {   // nur wenn keine Daten mitgegeben wurden
        // fire all data to the sink, independent of the requested format
        pImpl->aDataMimeType = rMimeType;
        StartTimer( &pImpl->pTimer, this, pImpl->nTimeout ); // Timeout neu
    }
    else
    {
        SvLinkSource_EntryIter_Impl aIter( pImpl->aArr );
        for( SvLinkSource_Entry_ImplPtr p = aIter.Curr(); p; p = aIter.Next() )
        {
            if( p->bIsDataSink )
            {
                p->xSink->DataChanged( rMimeType, rVal );

                if( p->nAdviseModes & ADVISEMODE_ONLYONCE )
                {
                    USHORT nFndPos = pImpl->aArr.GetPos( p );
                    if( USHRT_MAX != nFndPos )
                        pImpl->aArr.DeleteAndDestroy( nFndPos );
                }
            }
        }

        if( pImpl->pTimer )
        {
            delete pImpl->pTimer;
            pImpl->pTimer = NULL;
        }
    }
}


// only one link is correct
void SvLinkSource::AddDataAdvise( SvBaseLink * pLink, const String& rMimeType,
                                    USHORT nAdviseModes )
{
    SvLinkSource_Entry_ImplPtr pNew = new SvLinkSource_Entry_Impl(
                    pLink, rMimeType, nAdviseModes );
    pImpl->aArr.Insert( pNew, pImpl->aArr.Count() );
}

void SvLinkSource::RemoveAllDataAdvise( SvBaseLink * pLink )
{
    SvLinkSource_EntryIter_Impl aIter( pImpl->aArr );
    for( SvLinkSource_Entry_ImplPtr p = aIter.Curr(); p; p = aIter.Next() )
        if( p->bIsDataSink && &p->xSink == pLink )
        {
            USHORT nFndPos = pImpl->aArr.GetPos( p );
            if( USHRT_MAX != nFndPos )
                pImpl->aArr.DeleteAndDestroy( nFndPos );
        }
}

// only one link is correct
void SvLinkSource::AddConnectAdvise( SvBaseLink * pLink )
{
    SvLinkSource_Entry_ImplPtr pNew = new SvLinkSource_Entry_Impl( pLink );
    pImpl->aArr.Insert( pNew, pImpl->aArr.Count() );
}

void SvLinkSource::RemoveConnectAdvise( SvBaseLink * pLink )
{
    SvLinkSource_EntryIter_Impl aIter( pImpl->aArr );
    for( SvLinkSource_Entry_ImplPtr p = aIter.Curr(); p; p = aIter.Next() )
        if( !p->bIsDataSink && &p->xSink == pLink )
        {
            USHORT nFndPos = pImpl->aArr.GetPos( p );
            if( USHRT_MAX != nFndPos )
                pImpl->aArr.DeleteAndDestroy( nFndPos );
        }
}

BOOL SvLinkSource::HasDataLinks( const SvBaseLink* pLink ) const
{
    BOOL bRet = FALSE;
    const SvLinkSource_Entry_Impl* p;
    for( USHORT n = 0, nEnd = pImpl->aArr.Count(); n < nEnd; ++n )
        if( ( p = pImpl->aArr[ n ] )->bIsDataSink &&
            ( !pLink || &p->xSink == pLink ) )
        {
            bRet = TRUE;
            break;
        }
    return bRet;
}

// TRUE => waitinmg for data
BOOL SvLinkSource::IsPending() const
{
    return FALSE;
}

// TRUE => data complete loaded
BOOL SvLinkSource::IsDataComplete() const
{
    return TRUE;
}

BOOL SvLinkSource::Connect( SvBaseLink * )
{
    return TRUE;
}

BOOL SvLinkSource::GetData( ::com::sun::star::uno::Any & ,
                        const String & , BOOL )
{
    return FALSE;
}

String SvLinkSource::Edit( Window *, SvBaseLink * )
{
    return String();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
