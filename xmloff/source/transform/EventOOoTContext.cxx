/*************************************************************************
 *
 *  $RCSfile: EventOOoTContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:49:07 $
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

#ifndef _XMLOFF_EVENTOOOTCONTEXT_HXX
#include "EventOOoTContext.hxx"
#endif
#ifndef _XMLOFF_EVENTMAP_HXX
#include "EventMap.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_ACTIONMAPTYPESOOo_HXX
#include "ActionMapTypesOOo.hxx"
#endif
#ifndef _XMLOFF_ATTRTRANSFORMERACTION_HXX
#include "AttrTransformerAction.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#include "TransformerActions.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <hash_map>

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

class XMLTransformerOOoEventMap_Impl:
    public ::std::hash_map< ::rtl::OUString, NameKey_Impl,
                            ::rtl::OUStringHash, ::comphelper::UStringEqual >
{
public:
    XMLTransformerOOoEventMap_Impl( XMLTransformerEventMapEntry *pInit );
    ~XMLTransformerOOoEventMap_Impl();
};

XMLTransformerOOoEventMap_Impl::XMLTransformerOOoEventMap_Impl( XMLTransformerEventMapEntry *pInit )
{
    if( pInit )
    {
        XMLTransformerOOoEventMap_Impl::key_type aKey;
        XMLTransformerOOoEventMap_Impl::data_type aData;
        while( pInit->m_pOOoName )
        {
            aKey = OUString::createFromAscii(pInit->m_pOOoName);

            OSL_ENSURE( find( aKey ) == end(), "duplicate event map entry" );

            aData.m_nPrefix = pInit->m_nOASISPrefix;
            aData.m_aLocalName = OUString::createFromAscii(pInit->m_pOASISName);

            XMLTransformerOOoEventMap_Impl::value_type aVal( aKey, aData );

            insert( aVal );
            ++pInit;
        }
    }
}

XMLTransformerOOoEventMap_Impl::~XMLTransformerOOoEventMap_Impl()
{
}

// -----------------------------------------------------------------------------

TYPEINIT1( XMLEventOOoTransformerContext, XMLPersAttrListTContext );

XMLEventOOoTransformerContext::XMLEventOOoTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_Bool bPersistent ) :
    XMLPersAttrListTContext( rImp, rQName,
        rImp.GetNamespaceMap().GetKeyByAttrName( rQName ), XML_EVENT_LISTENER ),
    m_bPersistent( bPersistent )
{
}

XMLEventOOoTransformerContext::~XMLEventOOoTransformerContext()
{
}

XMLTransformerOOoEventMap_Impl
    *XMLEventOOoTransformerContext::CreateEventMap()
{
    return new XMLTransformerOOoEventMap_Impl( aTransformerEventMap );
}

void XMLEventOOoTransformerContext::FlushEventMap(
        XMLTransformerOOoEventMap_Impl *p )
{
    delete p;
}

sal_uInt16 XMLEventOOoTransformerContext::GetEventName(
        const OUString& rName,
        OUString& rNewName,
           XMLTransformerOOoEventMap_Impl& rMap )
{
    XMLTransformerOOoEventMap_Impl::key_type aKey( rName );
    XMLTransformerOOoEventMap_Impl::const_iterator aIter = rMap.find( aKey );
    if( aIter == rMap.end() )
    {
        rNewName = rName;
        return XML_NAMESPACE_UNKNOWN;
    }
    else
    {
        rNewName = (*aIter).second.m_aLocalName;
        return (*aIter).second.m_nPrefix;
    }
}


void XMLEventOOoTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OOO_EVENT_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    OUString aLocation, aMacroName;
    sal_Int16 nMacroName = -1;
    Reference< XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = 0;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                 &aLocalName );
        XMLTransformerActions::key_type aKey( nPrefix, aLocalName );
        XMLTransformerActions::const_iterator aIter =
            pActions->find( aKey );
        if( !(aIter == pActions->end() ) )
        {
            if( !pMutableAttrList )
            {
                pMutableAttrList =
                        new XMLMutableAttributeList( xAttrList );
                xAttrList = pMutableAttrList;
            }
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_HREF:
                // TODO
                break;
            case XML_ATACTION_EVENT_NAME:
                pMutableAttrList->SetValueByIndex( i,
                               GetTransformer().GetEventName( rAttrValue ) );
                break;
            case XML_ATACTION_ADD_NAMESPACE_PREFIX:
                {
                    OUString aAttrValue( rAttrValue );
                    sal_uInt16 nValPrefix =
                        static_cast<sal_uInt16>((*aIter).second.m_nParam1);
                    if( GetTransformer().AddNamespacePrefix( aAttrValue,
                                                             nValPrefix ) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_MACRO_LOCATION:
                aLocation = rAttrValue;
                pMutableAttrList->RemoveAttributeByIndex( i );
                --i;
                --nAttrCount;
                break;
            case XML_ATACTION_MACRO_NAME:
                aMacroName = rAttrValue;
                nMacroName = i;
                break;
            case XML_ATACTION_COPY:
                break;
            default:
                OSL_ENSURE( !this, "unknown action" );
                break;
            }
        }
    }

    if( nMacroName != -1 && aLocation.getLength() > 0 )
    {
        if( !IsXMLToken( aLocation, XML_APPLICATION ) )
            aLocation = GetXMLToken( XML_DOCUMENT );
        OUStringBuffer sTmp( aLocation.getLength() + aMacroName.getLength() + 1 );
        sTmp = aLocation;
        sTmp.append( sal_Unicode( ':' ) );
        sTmp.append( aMacroName );
        pMutableAttrList->SetValueByIndex( nMacroName,
                                           sTmp.makeStringAndClear() );
    }

    if( m_bPersistent )
        XMLPersAttrListTContext::StartElement( xAttrList );
    else
        GetTransformer().GetDocHandler()->startElement( GetExportQName(),
                                                        xAttrList );
}

void XMLEventOOoTransformerContext::EndElement()
{
    if( m_bPersistent )
        XMLPersAttrListTContext::EndElement();
    else
        GetTransformer().GetDocHandler()->endElement( GetExportQName() );
}

sal_Bool XMLEventOOoTransformerContext::IsPersistent() const
{
    return m_bPersistent;
}
