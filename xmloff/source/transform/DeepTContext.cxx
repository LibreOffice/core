/*************************************************************************
 *
 *  $RCSfile: DeepTContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:47:03 $
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

#ifndef _XMLOFF_DEEPTCONTEXT_HXX
#include "DeepTContext.hxx"
#endif
#ifndef _XMLOFF_FLATTTCONTEXT_HXX
#include "FlatTContext.hxx"
#endif
#ifndef _XMLOFF_EVENTOOOTCONTEXT_HXX
#include "EventOOoTContext.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#include "TransformerActions.hxx"
#endif
#ifndef _XMLOFF_ELEMTRANSFORMERACTION_HXX
#include "ElemTransformerAction.hxx"
#endif
#ifndef _XMLOFF_PERSMIXEDCONTENTTCONTEXT_HXX
#include "PersMixedContentTContext.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

TYPEINIT1( XMLPersElemContentTContext, XMLPersAttrListTContext );

void XMLPersElemContentTContext::AddContent( XMLTransformerContext *pContext )
{
    OSL_ENSURE( pContext && pContext->IsPersistent(),
                "non-persistent context" );
    XMLTransformerContextVector::value_type aVal( pContext );
    m_aChildContexts.push_back( aVal );
}

XMLPersElemContentTContext::XMLPersElemContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLPersAttrListTContext( rImp, rQName )
{
}

XMLPersElemContentTContext::XMLPersElemContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
       sal_uInt16 nActionMap ) :
    XMLPersAttrListTContext( rImp, rQName, nActionMap )
{
}

XMLPersElemContentTContext::XMLPersElemContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken ) :
    XMLPersAttrListTContext( rImp, rQName, nPrefix, eToken )
{
}

XMLPersElemContentTContext::XMLPersElemContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
       sal_uInt16 nActionMap ) :
    XMLPersAttrListTContext( rImp, rQName, nPrefix, eToken, nActionMap )
{
}

XMLPersElemContentTContext::~XMLPersElemContentTContext()
{
}

XMLTransformerContext *XMLPersElemContentTContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& xAttrList )
{
    XMLTransformerContext *pContext = 0;

    XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
    XMLTransformerActions::const_iterator aIter =
        GetTransformer().GetElemActions().find( aKey );

    if( !(aIter == GetTransformer().GetElemActions().end()) )
    {
        switch( (*aIter).second.m_nActionType )
        {
        case XML_ETACTION_COPY:
            pContext = new XMLPersMixedContentTContext( GetTransformer(),
                                                       rQName );
            break;
        case XML_ETACTION_COPY_TEXT:
            pContext = new XMLPersMixedContentTContext( GetTransformer(),
                                                       rQName );
            break;
        case XML_ETACTION_RENAME_ELEM:
            pContext = new XMLPersMixedContentTContext( GetTransformer(), rQName,
                    (*aIter).second.GetQNamePrefixFromParam1(),
                    (*aIter).second.GetQNameTokenFromParam1() );
            break;
        case XML_ETACTION_RENAME_ELEM_PROC_ATTRS:
            pContext = new XMLPersMixedContentTContext( GetTransformer(), rQName,
                    (*aIter).second.GetQNamePrefixFromParam1(),
                    (*aIter).second.GetQNameTokenFromParam1(),
                       static_cast< sal_uInt16 >( (*aIter).second.m_nParam2 ) );
            break;
        case XML_ETACTION_PROC_ATTRS:
            pContext = new XMLPersMixedContentTContext( GetTransformer(), rQName,
                       static_cast< sal_uInt16 >( (*aIter).second.m_nParam1 ) );
            break;
        default:
            pContext = GetTransformer().CreateUserDefinedContext(
                            (*aIter).second, rQName, sal_True );
            OSL_ENSURE( pContext && pContext->IsPersistent(),
                        "unknown or not persistent action" );
            if( pContext && !pContext->IsPersistent() )
            {
                delete pContext;
                pContext = 0;
            }
            break;
        }
    }

    // default is copying
    if( !pContext )
        pContext = new XMLPersMixedContentTContext( GetTransformer(), rQName );
    XMLTransformerContextVector::value_type aVal( pContext );
    m_aChildContexts.push_back( aVal );

    return pContext;
}

void XMLPersElemContentTContext::ExportContent()
{
    XMLTransformerContextVector::iterator aIter = m_aChildContexts.begin();

    for( ; aIter != m_aChildContexts.end(); ++aIter )
    {
        (*aIter)->Export();
    }
}

