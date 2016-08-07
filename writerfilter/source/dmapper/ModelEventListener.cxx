/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    catch( const uno::Exception& e)
    {
        (void) e;
    }
}

} //namespace dmapper
} //namespace writerfilter
