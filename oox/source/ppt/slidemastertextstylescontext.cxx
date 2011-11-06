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



#include "oox/drawingml/textliststyle.hxx"
#include "oox/drawingml/textliststylecontext.hxx"
#include "oox/ppt/slidemastertextstylescontext.hxx"

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

SlideMasterTextStylesContext::SlideMasterTextStylesContext( ContextHandler& rParent, SlidePersistPtr pSlidePersistPtr )
: ContextHandler( rParent )
, mpSlidePersistPtr( pSlidePersistPtr )
{
}

SlideMasterTextStylesContext::~SlideMasterTextStylesContext()
{
}

Reference< XFastContextHandler > SlideMasterTextStylesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& /* xAttribs */ ) throw (SAXException, RuntimeException)
{
    oox::drawingml::TextListStylePtr aTextListStylePtr;
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case PPT_TOKEN( titleStyle ):
        {
            aTextListStylePtr = mpSlidePersistPtr->getTitleTextStyle();
            break;
        }
        case PPT_TOKEN( bodyStyle ):
        {
            aTextListStylePtr = mpSlidePersistPtr->getBodyTextStyle();
            break;
        }
        case PPT_TOKEN( notesStyle ):
        {
            aTextListStylePtr = mpSlidePersistPtr->getNotesTextStyle();
            break;
        }
        case PPT_TOKEN( otherStyle ):
        {
            aTextListStylePtr = mpSlidePersistPtr->getOtherTextStyle();
            break;
        }
    }
    if ( aTextListStylePtr )            // sj: the master list style is the last instance of from where properties
    {                                   // are obtained. i got some documents without having the textsize set at
        for ( int i = 0; i < 9; i++ )   // any point, the master reference application is using 18pt then
            aTextListStylePtr->getListStyle()[ i ]->getTextCharacterProperties().moHeight = 1800;
        xRet.set( new oox::drawingml::TextListStyleContext( *this, *aTextListStylePtr ) );
    }
    if( !xRet.is() )
        xRet.set( this );

    return xRet;
}

} }
