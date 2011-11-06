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



#include "timeanimvaluecontext.hxx"

#include "animvariantcontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

    TimeAnimValueListContext::TimeAnimValueListContext( ContextHandler& rParent,
                const Reference< XFastAttributeList >& /*xAttribs*/,
                TimeAnimationValueList & aTavList )
        : ContextHandler( rParent )
            , maTavList( aTavList )
            , mbInValue( false )
    {
    }


    TimeAnimValueListContext::~TimeAnimValueListContext( )
    {
    }


    void SAL_CALL TimeAnimValueListContext::endFastElement( sal_Int32 aElement )
        throw ( SAXException, RuntimeException)
    {
        if( aElement == PPT_TOKEN( tav ) )
        {
            mbInValue = false;
        }
    }


    Reference< XFastContextHandler > SAL_CALL TimeAnimValueListContext::createFastChildContext( ::sal_Int32 aElementToken,
                                                                                                                                                                                            const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
    {
        Reference< XFastContextHandler > xRet;

        switch ( aElementToken )
        {
        case PPT_TOKEN( tav ):
        {
            mbInValue = true;
            TimeAnimationValue val;
            val.msFormula = xAttribs->getOptionalValue( XML_fmla );
            val.msTime =  xAttribs->getOptionalValue( XML_tm );
            maTavList.push_back( val );
            break;
        }
        case PPT_TOKEN( val ):
            if( mbInValue )
            {
                // CT_TLAnimVariant
                xRet.set( new AnimVariantContext( *this, aElementToken, maTavList.back().maValue ) );
            }
            break;
        default:
            break;
        }

        if( !xRet.is() )
            xRet.set( this );

        return xRet;
    }


} }
