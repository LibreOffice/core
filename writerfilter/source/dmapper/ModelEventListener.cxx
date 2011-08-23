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
