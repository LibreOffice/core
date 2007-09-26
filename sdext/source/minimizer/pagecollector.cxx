 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pagecollector.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2007-09-26 13:00:23 $
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

#ifndef PAGECOLLECTOR_HXX
#include "pagecollector.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_XPRESENTATIONPAGE_HPP_
#include <com/sun/star/presentation/XPresentationPage.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XMASTERPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XMASTERPAGETARGET_HPP_
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_XCUSTOMPRESENTATIONSUPPLIER_HPP_
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::presentation;

void PageCollector::CollectCustomShowPages( const com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rxModel, const rtl::OUString& rCustomShowName, std::vector< Reference< XDrawPage > >& rUsedPageList )
{
    try
    {
        Reference< XCustomPresentationSupplier > aXCPSup( rxModel, UNO_QUERY_THROW );
        Reference< XNameContainer > aXCont( aXCPSup->getCustomPresentations() );
        if ( aXCont.is() )
        {
            // creating a list of every page that is used within our customshow
            Sequence< OUString> aNameSeq( aXCont->getElementNames() );
            const OUString* pUString = aNameSeq.getArray();
            sal_Int32 i, nCount = aNameSeq.getLength();
            for ( i = 0; i < nCount; i++ )
            {
                if ( pUString[ i ] == rCustomShowName )
                {
                    Reference< container::XIndexContainer > aXIC( aXCont->getByName( pUString[ i ] ), UNO_QUERY_THROW );
                    sal_Int32 j, nSlideCount = aXIC->getCount();
                    for ( j = 0; j < nSlideCount; j++ )
                    {
                        Reference< XDrawPage > xDrawPage( aXIC->getByIndex( j ), UNO_QUERY_THROW );
                        std::vector< Reference< XDrawPage > >::iterator aIter( rUsedPageList.begin() );
                        std::vector< Reference< XDrawPage > >::iterator aEnd( rUsedPageList.end() );
                        while( aIter != aEnd )
                        {
                            if ( *aIter == xDrawPage )
                                break;
                            aIter++;
                        }
                        if ( aIter == aEnd )
                            rUsedPageList.push_back( xDrawPage );
                    }
                }
            }
        }
    }
    catch( Exception& )
    {

    }
}

void PageCollector::CollectNonCustomShowPages( const com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rxModel, const rtl::OUString& rCustomShowName, std::vector< Reference< XDrawPage > >& rNonUsedPageList )
{
    try
    {
        std::vector< Reference< XDrawPage > > vUsedPageList;
        PageCollector::CollectCustomShowPages( rxModel, rCustomShowName, vUsedPageList );
        if ( vUsedPageList.size() )
        {
            Reference< XDrawPagesSupplier > xDrawPagesSupplier( rxModel, UNO_QUERY_THROW );
            Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
            for ( sal_Int32 j = 0; j < xDrawPages->getCount(); j++ )
            {
                Reference< XDrawPage > xDrawPage( xDrawPages->getByIndex( j ), UNO_QUERY_THROW );
                std::vector< Reference< XDrawPage > >::iterator aIter( vUsedPageList.begin() );
                std::vector< Reference< XDrawPage > >::iterator aEnd( vUsedPageList.end() );
                while( aIter != aEnd )
                {
                    if ( *aIter == xDrawPage )
                        break;
                    aIter++;
                }
                if ( aIter == aEnd )
                    rNonUsedPageList.push_back( xDrawPage );
            }
        }
    }
    catch( Exception& )
    {
    }
}


void PageCollector::CollectMasterPages( const Reference< XModel >& rxModel, std::vector< PageCollector::MasterPageEntity >& rMasterPageList )
{
    typedef std::vector< MasterPageEntity > MasterPageList;
    typedef MasterPageList::iterator MasterPageIter;

    try
    {
        // generating list of all master pages
        Reference< XMasterPagesSupplier > xMasterPagesSupplier( rxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xMasterPages( xMasterPagesSupplier->getMasterPages(), UNO_QUERY_THROW );
        for ( sal_Int32 i = 0; i < xMasterPages->getCount(); i++ )
        {
            Reference< XDrawPage > xMasterPage( xMasterPages->getByIndex( i ), UNO_QUERY_THROW );
            MasterPageIter aIter( rMasterPageList.begin() );
            MasterPageIter aEnd ( rMasterPageList.end() );
            while( aIter != aEnd )
            {
                if ( aIter->xMasterPage == xMasterPage )
                    break;
                aIter++;
            }
            if ( aIter == aEnd )
            {
                MasterPageEntity aMasterPageEntity;
                aMasterPageEntity.xMasterPage = xMasterPage;
                aMasterPageEntity.bUsed = sal_False;
                rMasterPageList.push_back( aMasterPageEntity );
            }
        }

        // mark masterpages which are referenced by drawpages
        Reference< XDrawPagesSupplier > xDrawPagesSupplier( rxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
        for ( sal_Int32 j = 0; j < xDrawPages->getCount(); j++ )
        {
            Reference< XMasterPageTarget > xMasterPageTarget( xDrawPages->getByIndex( j ), UNO_QUERY_THROW );
            Reference< XDrawPage > xMasterPage( xMasterPageTarget->getMasterPage(), UNO_QUERY_THROW );
            MasterPageIter aIter( rMasterPageList.begin() );
            MasterPageIter aEnd ( rMasterPageList.end() );
            while( aIter != aEnd )
            {
                if ( aIter->xMasterPage == xMasterPage )
                {
                    aIter->bUsed = sal_True;
                    break;
                }
                aIter++;
            }
            if ( aIter == aEnd )
                throw uno::RuntimeException();
        }
    }
    catch( Exception& )
    {
    }
}

