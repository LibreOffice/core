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



#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimate.hpp>

#include "oox/core/fragmenthandler.hxx"

#include "commonbehaviorcontext.hxx"
#include "commontimenodecontext.hxx"
#include "timetargetelementcontext.hxx"
#include "pptfilterhelpers.hxx"

#include <string.h>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::animations;

namespace oox { namespace ppt {

    CommonBehaviorContext::CommonBehaviorContext( ContextHandler& rParent,
            const Reference< XFastAttributeList >& xAttribs,
            const TimeNodePtr & pNode )
        : TimeNodeContext( rParent, PPT_TOKEN( cBhvr ), xAttribs, pNode )
            , mbInAttrList( false )
            , mbIsInAttrName( false )
    {
    }


    CommonBehaviorContext::~CommonBehaviorContext( ) throw( )
    {
    }



    void SAL_CALL CommonBehaviorContext::endFastElement( sal_Int32 aElement )
        throw ( SAXException, RuntimeException)
    {
        switch( aElement )
        {
        case PPT_TOKEN( cBhvr ):
        {
            if( !maAttributes.empty() )
            {
                OUStringBuffer sAttributes;
                std::list< Attribute >::const_iterator iter;
                for(iter = maAttributes.begin(); iter != maAttributes.end(); iter++)
                {
                    if( sAttributes.getLength() )
                    {
                        sAttributes.appendAscii( ";" );
                    }
                    sAttributes.append( iter->name );
                }
                OUString sTmp( sAttributes.makeStringAndClear() );
                mpNode->getNodeProperties()[ NP_ATTRIBUTENAME ] = makeAny( sTmp );
            }
            break;
        }
        case PPT_TOKEN( attrNameLst ):
            mbInAttrList = false;
            break;
        case PPT_TOKEN( attrName ):
            if( mbIsInAttrName )
            {
                const ImplAttributeNameConversion *attrConv = gImplConversionList;
                while( attrConv->mpMSName != NULL )
                {
                    if(msCurrentAttribute.compareToAscii( attrConv->mpMSName ) == 0 )
                    {
                        Attribute attr;
                        attr.name = ::rtl::OUString::intern( attrConv->mpAPIName,
                                                             strlen(attrConv->mpAPIName),
                                                             RTL_TEXTENCODING_ASCII_US );
                        attr.type = attrConv->meAttribute;
                        maAttributes.push_back( attr );
                        OSL_TRACE( "OOX: attrName is %s -> %s",
                                   OUSTRING_TO_CSTR( msCurrentAttribute ),
                                   attrConv->mpAPIName );
                        break;
                    }
                    attrConv++;
                }
                mbIsInAttrName = false;
            }
            break;
        default:
            break;
        }
    }


    void CommonBehaviorContext::characters( const OUString& aChars )
        throw( SAXException, RuntimeException )
    {
        if( mbIsInAttrName )
        {
            msCurrentAttribute += aChars;
        }
    }


    Reference< XFastContextHandler > SAL_CALL CommonBehaviorContext::createFastChildContext( ::sal_Int32 aElementToken,
                                                                                                                                                                                     const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
    {
        Reference< XFastContextHandler > xRet;

        switch ( aElementToken )
        {
        case PPT_TOKEN( cTn ):
            xRet.set( new CommonTimeNodeContext( *this, aElementToken, xAttribs, mpNode ) );
            break;
        case PPT_TOKEN( tgtEl ):
            xRet.set( new TimeTargetElementContext( *this, mpNode->getTarget() ) );
            break;
        case PPT_TOKEN( attrNameLst ):
            mbInAttrList = true;
            break;
        case PPT_TOKEN( attrName ):
        {
            if( mbInAttrList )
            {
                mbIsInAttrName = true;
                msCurrentAttribute = OUString();
            }
            else
            {
                OSL_TRACE( "OOX: Attribute Name outside an Attribute List" );
            }
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
