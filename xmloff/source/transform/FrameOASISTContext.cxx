/*************************************************************************
 *
 *  $RCSfile: FrameOASISTContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:51:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_FRAMEOASISTCONTEXT_HXX
#include "FrameOASISTContext.hxx"
#endif
#ifndef _XMLOFF_IGNORETCONTEXT_HXX
#include "IgnoreTContext.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_ACTIONMAPTYPESOASIS_HXX
#include "ActionMapTypesOASIS.hxx"
#endif
#ifndef _XMLOFF_ELEMTRANSFORMERACTION_HXX
#include "ElemTransformerAction.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#include "TransformerActions.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLFrameOASISTransformerContext, XMLTransformerContext );

XMLFrameOASISTransformerContext::XMLFrameOASISTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLFrameOASISTransformerContext::~XMLFrameOASISTransformerContext()
{
}

void XMLFrameOASISTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    m_xAttrList = new XMLMutableAttributeList( rAttrList, sal_True );
}

XMLTransformerContext *XMLFrameOASISTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext *pContext = 0;

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OASIS_FRAME_ELEM_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );
    XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
    XMLTransformerActions::const_iterator aIter = pActions->find( aKey );

    if( !(aIter == pActions->end()) )
    {
        switch( (*aIter).second.m_nActionType )
        {
        case XML_ETACTION_COPY:
            if( !m_aElemQName.getLength() )
            {
                pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                            rQName,
                                                            sal_False, sal_False );
                m_aElemQName = rQName;
                static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                    ->AppendAttributeList( rAttrList );
                GetTransformer().ProcessAttrList( m_xAttrList,
                                                  OASIS_SHAPE_ACTIONS,
                                                     sal_False  );
                GetTransformer().GetDocHandler()->startElement( m_aElemQName,
                                                                m_xAttrList );
            }
            else
            {
                pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                            rQName,
                                                            sal_True, sal_True );
            }
            break;
        default:
            OSL_ENSURE( !this, "unknown action" );
            break;
        }
    }

    // default is copying
    if( !pContext )
        pContext = XMLTransformerContext::CreateChildContext( nPrefix,
                                                              rLocalName,
                                                              rQName,
                                                              rAttrList );

    return pContext;
}

void XMLFrameOASISTransformerContext::EndElement()
{
    GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

void XMLFrameOASISTransformerContext::Characters( const OUString& rChars )
{
    // ignore
    if( m_aElemQName.getLength() )
        XMLTransformerContext::Characters( rChars );
}
