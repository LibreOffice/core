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

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimate.hpp>

#include "oox/core/namespaces.hxx"
#include "oox/core/fragmenthandler.hxx"

#include "commonbehaviorcontext.hxx"
#include "commontimenodecontext.hxx"
#include "timetargetelementcontext.hxx"
#include "pptfilterhelpers.hxx"
#include "tokens.hxx"

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
        : TimeNodeContext( rParent, NMSP_PPT|XML_cBhvr, xAttribs, pNode )
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
        case NMSP_PPT|XML_cBhvr:
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
        case NMSP_PPT|XML_attrNameLst:
            mbInAttrList = false;
            break;
        case NMSP_PPT|XML_attrName:
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
        case NMSP_PPT|XML_cTn:
            xRet.set( new CommonTimeNodeContext( *this, aElementToken, xAttribs, mpNode ) );
            break;
        case NMSP_PPT|XML_tgtEl:
            xRet.set( new TimeTargetElementContext( *this, mpNode->getTarget() ) );
            break;
        case NMSP_PPT|XML_attrNameLst:
            mbInAttrList = true;
            break;
        case NMSP_PPT|XML_attrName:
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
