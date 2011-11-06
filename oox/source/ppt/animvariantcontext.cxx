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



#include "animvariantcontext.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include <osl/diagnose.h>

#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>

#include "oox/helper/attributelist.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "pptfilterhelpers.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

    AnimVariantContext::AnimVariantContext( ContextHandler& rParent, sal_Int32 aElement, Any & aValue )
        : ContextHandler( rParent )
            , mnElement( aElement )
            , maValue( aValue )
    {
    }

    AnimVariantContext::~AnimVariantContext( ) throw( )
    {
    }

    void SAL_CALL AnimVariantContext::endFastElement( sal_Int32 aElement )
        throw ( SAXException, RuntimeException)
    {
        if( ( aElement == mnElement ) && maColor.isUsed() )
        {
            maValue = makeAny( maColor.getColor( getFilter().getGraphicHelper() ) );
        }
    }


    Reference< XFastContextHandler >
    SAL_CALL AnimVariantContext::createFastChildContext( ::sal_Int32 aElementToken,
                             const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
    {
        Reference< XFastContextHandler > xRet;
        AttributeList attribs(xAttribs);

        switch( aElementToken )
        {
        case PPT_TOKEN( boolVal ):
        {
            bool val = attribs.getBool( XML_val, false );
            maValue = makeAny( val );
            break;
        }
        case PPT_TOKEN( clrVal ):
            xRet.set( new ::oox::drawingml::ColorContext( *this, maColor ) );
            // we'll defer setting the Any until the end.
            break;
        case PPT_TOKEN( fltVal ):
        {
            double val = attribs.getDouble( XML_val, 0.0 );
            maValue = makeAny( val );
            break;
        }
        case PPT_TOKEN( intVal ):
        {
            sal_Int32 val = attribs.getInteger( XML_val, 0 );
            maValue = makeAny( val );
            break;
        }
        case PPT_TOKEN( strVal ):
        {
            OUString val = attribs.getString( XML_val, OUString() );
            convertMeasure( val ); // ignore success or failure if it fails, use as is
            maValue = makeAny( val );
            break;
        }
        default:
            break;
        }

        if( !xRet.is() )
            xRet.set( this );

        return xRet;
    }



} }
