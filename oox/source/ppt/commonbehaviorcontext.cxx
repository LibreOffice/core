/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commonbehaviorcontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:36:45 $
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

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include <comphelper/processfactory.hxx>
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

    CommonBehaviorContext::CommonBehaviorContext( const FragmentHandlerRef& xHandler,
                                                                                                const Reference< XFastAttributeList >& xAttribs,
                                                                                                const TimeNodePtr & pNode )
        : TimeNodeContext( xHandler, NMSP_PPT|XML_cBhvr, xAttribs, pNode )
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
            xRet.set( new CommonTimeNodeContext( getHandler(), aElementToken, xAttribs, mpNode ) );
            break;
        case NMSP_PPT|XML_tgtEl:
            xRet.set( new TimeTargetElementContext( getHandler(), mpNode->getTarget() ) );
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
