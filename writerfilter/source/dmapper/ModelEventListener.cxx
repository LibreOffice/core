/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ModelEventListener.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:52:21 $
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
#include <ModelEventListener.hxx>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;


/*-- 22.11.2007 08:40:22---------------------------------------------------

  -----------------------------------------------------------------------*/
ModelEventListener::ModelEventListener()
{
}
/*-- 22.11.2007 08:40:22---------------------------------------------------

  -----------------------------------------------------------------------*/
ModelEventListener::~ModelEventListener()
{
}
/*-- 22.11.2007 08:40:22---------------------------------------------------

  -----------------------------------------------------------------------*/
void ModelEventListener::notifyEvent( const document::EventObject& rEvent ) throw (uno::RuntimeException)
{
    if( rEvent.EventName.equalsAscii("OnFocus"))
    {
        try
        {
            uno::Reference< text::XDocumentIndexesSupplier> xIndexesSupplier( rEvent.Source, uno::UNO_QUERY );
            //remove listener
            uno::Reference<document::XEventBroadcaster>(rEvent.Source, uno::UNO_QUERY )->removeEventListener(
            uno::Reference<document::XEventListener>(this));

            uno::Reference< container::XIndexAccess > xIndexes = xIndexesSupplier->getDocumentIndexes();

            sal_Int32 nIndexes = xIndexes->getCount();
            for( sal_Int32 nIndex = 0; nIndex < nIndexes; ++nIndex)
            {
                uno::Reference< text::XDocumentIndex> xIndex( xIndexes->getByIndex( nIndex ), uno::UNO_QUERY );
                xIndex->update();
            }
        }
        catch( const uno::Exception& rEx )
        {
            (void)rEx;
            OSL_ENSURE( false, "exception while updating indexes" );
        }
    }
}
/*-- 22.11.2007 08:40:22---------------------------------------------------

  -----------------------------------------------------------------------*/
void ModelEventListener::disposing( const lang::EventObject& rEvent ) throw (uno::RuntimeException)
{
    try
    {
        uno::Reference<document::XEventBroadcaster>(rEvent.Source, uno::UNO_QUERY )->removeEventListener(
            uno::Reference<document::XEventListener>(this));
    }
    catch( const uno::Exception& )
    {
    }
}

} //namespace dmapper
} //namespace writerfilter
