/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DeepTContext.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:22:22 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

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
        const Reference< XAttributeList >& )
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
        case XML_ETACTION_RENAME_ELEM_ADD_PROC_ATTR:
            {
                XMLPersMixedContentTContext *pMC =
                    new XMLPersMixedContentTContext( GetTransformer(), rQName,
                        (*aIter).second.GetQNamePrefixFromParam1(),
                        (*aIter).second.GetQNameTokenFromParam1(),
                        static_cast< sal_uInt16 >(
                            (*aIter).second.m_nParam3  >> 16 ) );
                pMC->AddAttribute(
                    (*aIter).second.GetQNamePrefixFromParam2(),
                    (*aIter).second.GetQNameTokenFromParam2(),
                       static_cast< ::xmloff::token::XMLTokenEnum >(
                        (*aIter).second.m_nParam3 & 0xffff ) );
                pContext = pMC;
            }
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

