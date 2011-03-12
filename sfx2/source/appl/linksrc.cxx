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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <sfx2/linksrc.hxx>
#include <sfx2/lnkbase.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <tools/debug.hxx>
#include <vcl/timer.hxx>
#include <svl/svarray.hxx>


using namespace ::com::sun::star::uno;

namespace sfx2
{

TYPEINIT0( SvLinkSource )

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
    // Secure against beeing destroyed in Handler
    SvLinkSourceRef aAdv( pOwner );
    pOwner->SendDataChanged();
}

static void StartTimer( SvLinkSourceTimer ** ppTimer, SvLinkSource * pOwner,
                        sal_uIntPtr nTimeout )
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
    SvBaseLinkRef       xSink;
    String              aDataMimeType;
    sal_uInt16              nAdviseModes;
    sal_Bool                bIsDataSink;

    SvLinkSource_Entry_Impl( SvBaseLink* pLink, const String& rMimeType,
                                sal_uInt16 nAdvMode )
        : xSink( pLink ), aDataMimeType( rMimeType ),
            nAdviseModes( nAdvMode ), bIsDataSink( sal_True )
    {}

    SvLinkSource_Entry_Impl( SvBaseLink* pLink )
        : xSink( pLink ), nAdviseModes( 0 ), bIsDataSink( sal_False )
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
    sal_uInt16 nPos;
public:
    SvLinkSource_EntryIter_Impl( const SvLinkSource_Array_Impl& rArr );
    ~SvLinkSource_EntryIter_Impl();
    SvLinkSource_Entry_Impl* Curr()
                            { return nPos < aArr.Count() ? aArr[ nPos ] : 0; }
    SvLinkSource_Entry_Impl* Next();
    sal_Bool IsValidCurrValue( SvLinkSource_Entry_Impl* pEntry );
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

sal_Bool SvLinkSource_EntryIter_Impl::IsValidCurrValue( SvLinkSource_Entry_Impl* pEntry )
{
    return ( nPos < aArr.Count() && aArr[nPos] == pEntry && USHRT_MAX != rOrigArr.GetPos( pEntry ) );
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
    String              aDataMimeType;
    SvLinkSourceTimer * pTimer;
    sal_uIntPtr             nTimeout;
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream>
    m_xInputStreamToLoadFrom;
    sal_Bool m_bIsReadOnly;

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


SvLinkSource::StreamToLoadFrom SvLinkSource::getStreamToLoadFrom()
{
    return StreamToLoadFrom(
        pImpl->m_xInputStreamToLoadFrom,
        pImpl->m_bIsReadOnly);
}

void SvLinkSource::setStreamToLoadFrom(const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream,sal_Bool bIsReadOnly )
{
    pImpl->m_xInputStreamToLoadFrom = xInputStream;
    pImpl->m_bIsReadOnly = bIsReadOnly;
}

// #i88291#
void SvLinkSource::clearStreamToLoadFrom()
{
    pImpl->m_xInputStreamToLoadFrom.clear();
}

void  SvLinkSource::Closed()
{
    SvLinkSource_EntryIter_Impl aIter( pImpl->aArr );
    for( SvLinkSource_Entry_Impl* p = aIter.Curr(); p; p = aIter.Next() )
        if( !p->bIsDataSink )
            p->xSink->Closed();
}

sal_uIntPtr SvLinkSource::GetUpdateTimeout() const
{
    return pImpl->nTimeout;
}

void SvLinkSource::SetUpdateTimeout( sal_uIntPtr nTimeout )
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
                GetData( aVal, sDataMimeType, sal_True ) )
            {
                p->xSink->DataChanged( sDataMimeType, aVal );

                if ( !aIter.IsValidCurrValue( p ) )
                    continue;

                if( p->nAdviseModes & ADVISEMODE_ONLYONCE )
                {
                    sal_uInt16 nFndPos = pImpl->aArr.GetPos( p );
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
        StartTimer( &pImpl->pTimer, this, pImpl->nTimeout ); // New timeout
    else
    {
        SvLinkSource_EntryIter_Impl aIter( pImpl->aArr );
        for( SvLinkSource_Entry_ImplPtr p = aIter.Curr(); p; p = aIter.Next() )
            if( p->bIsDataSink )
            {
                Any aVal;
                if( ( p->nAdviseModes & ADVISEMODE_NODATA ) ||
                    GetData( aVal, p->aDataMimeType, sal_True ) )
                {
                    p->xSink->DataChanged( p->aDataMimeType, aVal );

                    if ( !aIter.IsValidCurrValue( p ) )
                        continue;

                    if( p->nAdviseModes & ADVISEMODE_ONLYONCE )
                    {
                        sal_uInt16 nFndPos = pImpl->aArr.GetPos( p );
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
    {   // only when no data was included
        // fire all data to the sink, independent of the requested format
        pImpl->aDataMimeType = rMimeType;
        StartTimer( &pImpl->pTimer, this, pImpl->nTimeout ); // New timeout
    }
    else
    {
        SvLinkSource_EntryIter_Impl aIter( pImpl->aArr );
        for( SvLinkSource_Entry_ImplPtr p = aIter.Curr(); p; p = aIter.Next() )
        {
            if( p->bIsDataSink )
            {
                p->xSink->DataChanged( rMimeType, rVal );

                if ( !aIter.IsValidCurrValue( p ) )
                    continue;

                if( p->nAdviseModes & ADVISEMODE_ONLYONCE )
                {
                    sal_uInt16 nFndPos = pImpl->aArr.GetPos( p );
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
                                    sal_uInt16 nAdviseModes )
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
            sal_uInt16 nFndPos = pImpl->aArr.GetPos( p );
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
            sal_uInt16 nFndPos = pImpl->aArr.GetPos( p );
            if( USHRT_MAX != nFndPos )
                pImpl->aArr.DeleteAndDestroy( nFndPos );
        }
}

sal_Bool SvLinkSource::HasDataLinks( const SvBaseLink* pLink ) const
{
    sal_Bool bRet = sal_False;
    const SvLinkSource_Entry_Impl* p;
    for( sal_uInt16 n = 0, nEnd = pImpl->aArr.Count(); n < nEnd; ++n )
        if( ( p = pImpl->aArr[ n ] )->bIsDataSink &&
            ( !pLink || &p->xSink == pLink ) )
        {
            bRet = sal_True;
            break;
        }
    return bRet;
}

// sal_True => waitinmg for data
sal_Bool SvLinkSource::IsPending() const
{
    return sal_False;
}

// sal_True => data complete loaded
sal_Bool SvLinkSource::IsDataComplete() const
{
    return sal_True;
}

sal_Bool SvLinkSource::Connect( SvBaseLink* )
{
    return sal_True;
}

sal_Bool SvLinkSource::GetData( ::com::sun::star::uno::Any &, const String &, sal_Bool )
{
    return sal_False;
}

void SvLinkSource::Edit( Window *, SvBaseLink *, const Link& )
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
