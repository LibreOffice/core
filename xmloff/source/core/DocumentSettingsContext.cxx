/*************************************************************************
 *
 *  $RCSfile: DocumentSettingsContext.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-22 09:09:00 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifndef _XMLOFF_DOCUMENTSETTINGSCONTEXT_HXX
#include "DocumentSettingsContext.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef __SGI_STL_LIST
#include <list>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

using namespace com::sun::star;

//------------------------------------------------------------------

class XMLMyList
{
    std::list<beans::PropertyValue> aProps;
    sal_uInt32                      nCount;
public:
    XMLMyList() : nCount(0) {}
    ~XMLMyList() {}

    void push_back(beans::PropertyValue& aProp) { aProps.push_back(aProp); nCount++; }
    uno::Sequence<beans::PropertyValue> GetSequence();
    uno::Reference<container::XNameContainer> GetNameContainer();
    uno::Reference<container::XIndexContainer> GetIndexContainer();
};

uno::Sequence<beans::PropertyValue> XMLMyList::GetSequence()
{
    uno::Sequence<beans::PropertyValue> aSeq;
    if(nCount)
    {
        DBG_ASSERT(nCount == aProps.size(), "wrong count of PropertyValue");
        aSeq.realloc(nCount);
        beans::PropertyValue* pProps = aSeq.getArray();
        std::list<beans::PropertyValue>::iterator aItr = aProps.begin();
        while (aItr != aProps.end())
        {
            *pProps = *aItr;
            pProps++;
            aItr++;
        }
    }
    return aSeq;
}

uno::Reference<container::XNameContainer> XMLMyList::GetNameContainer()
{
    uno::Reference<container::XNameContainer> xNameContainer;
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory =
                                        comphelper::getProcessServiceFactory();
    DBG_ASSERT( xServiceFactory.is(), "got no service manager" );
    if( xServiceFactory.is() )
    {
        rtl::OUString sName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.NamedPropertyValues"));
        xNameContainer = uno::Reference<container::XNameContainer>(xServiceFactory->createInstance(sName), uno::UNO_QUERY);
        if (xNameContainer.is())
        {
            std::list<beans::PropertyValue>::iterator aItr = aProps.begin();
            while (aItr != aProps.end())
            {
                xNameContainer->insertByName(aItr->Name, aItr->Value);
                aItr++;
            }
        }
    }
    return xNameContainer;
}

uno::Reference<container::XIndexContainer> XMLMyList::GetIndexContainer()
{
    uno::Reference<container::XIndexContainer> xIndexContainer;
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory =
                                        comphelper::getProcessServiceFactory();
    DBG_ASSERT( xServiceFactory.is(), "got no service manager" );
    if( xServiceFactory.is() )
    {
        rtl::OUString sName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.IndexedPropertyValues"));
        xIndexContainer = uno::Reference<container::XIndexContainer>(xServiceFactory->createInstance(sName), uno::UNO_QUERY);
        if (xIndexContainer.is())
        {
            std::list<beans::PropertyValue>::iterator aItr = aProps.begin();
            sal_uInt32 i(0);
            while (aItr != aProps.end())
            {
                xIndexContainer->insertByIndex(i, aItr->Value);
                aItr++;
                i++;
            }
        }
    }
    return xIndexContainer;
}

//=============================================================================

class XMLConfigBaseContext : public SvXMLImportContext
{
protected:
    XMLMyList                   aProps;
    beans::PropertyValue        aProp;
    com::sun::star::uno::Any&   rAny;
    XMLConfigBaseContext*       pBaseContext;
public:
    XMLConfigBaseContext(SvXMLImport& rImport, USHORT nPrfx, const rtl::OUString& rLName,
                                    com::sun::star::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext);
    virtual ~XMLConfigBaseContext();

    void AddPropertyValue() { aProps.push_back(aProp); }
};

//=============================================================================

class XMLConfigItemContext : public SvXMLImportContext
{
    rtl::OUString               sType;
    rtl::OUString               sValue;
    com::sun::star::uno::Any&   rAny;
    XMLConfigBaseContext*       pBaseContext;

public:
    XMLConfigItemContext(SvXMLImport& rImport, USHORT nPrfx, const rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                    com::sun::star::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext);
    virtual ~XMLConfigItemContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void EndElement();
};

//=============================================================================

class XMLConfigItemSetContext : public XMLConfigBaseContext
{
public:
    XMLConfigItemSetContext(SvXMLImport& rImport, USHORT nPrfx, const rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                    com::sun::star::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext);
    virtual ~XMLConfigItemSetContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

//=============================================================================

class XMLConfigItemMapNamedContext : public XMLConfigBaseContext
{
public:
    XMLConfigItemMapNamedContext(SvXMLImport& rImport, USHORT nPrfx, const rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                    com::sun::star::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext);
    virtual ~XMLConfigItemMapNamedContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

//=============================================================================

class XMLConfigItemMapIndexedContext : public XMLConfigBaseContext
{
public:
    XMLConfigItemMapIndexedContext(SvXMLImport& rImport, USHORT nPrfx,
                                    const rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                    com::sun::star::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext);
    virtual ~XMLConfigItemMapIndexedContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

//=============================================================================

SvXMLImportContext *CreateSettingsContext(SvXMLImport& rImport, USHORT nPrefix,
                        const rtl::OUString& rLocalName,
                        const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                        beans::PropertyValue& rProp, XMLConfigBaseContext* pBaseContext)
{
    SvXMLImportContext *pContext = 0;

    rProp.Name = rtl::OUString();
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_CONFIG)
        {
            if (aLocalName.compareToAscii(sXML_name) == 0)
                rProp.Name = sValue;
        }
    }

    if (nPrefix == XML_NAMESPACE_CONFIG)
    {
        if (rLocalName.compareToAscii(sXML_config_item) == 0)
            pContext = new XMLConfigItemContext(rImport, nPrefix, rLocalName, xAttrList, rProp.Value, pBaseContext);
        else if((rLocalName.compareToAscii(sXML_config_item_set) == 0) ||
                (rLocalName.compareToAscii(sXML_config_item_map_entry) == 0))
            pContext = new XMLConfigItemSetContext(rImport, nPrefix, rLocalName, xAttrList, rProp.Value, pBaseContext);
        else if(rLocalName.compareToAscii(sXML_config_item_map_named) == 0)
            pContext = new XMLConfigItemMapNamedContext(rImport, nPrefix, rLocalName, xAttrList, rProp.Value, pBaseContext);
        else if(rLocalName.compareToAscii(sXML_config_item_map_indexed) == 0)
            pContext = new XMLConfigItemMapIndexedContext(rImport, nPrefix, rLocalName, xAttrList, rProp.Value, pBaseContext);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( rImport, nPrefix, rLocalName );

    return pContext;
}

//=============================================================================

XMLDocumentSettingsContext::XMLDocumentSettingsContext(SvXMLImport& rImport, USHORT nPrfx, const rtl::OUString& rLName,
                    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
    : SvXMLImportContext( rImport, nPrfx, rLName )
{
    // here are no attributes
}

XMLDocumentSettingsContext::~XMLDocumentSettingsContext()
{
}

SvXMLImportContext *XMLDocumentSettingsContext::CreateChildContext( USHORT nPrefix,
                                     const rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;
    rtl::OUString sName;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_CONFIG)
        {
            if (aLocalName.compareToAscii(sXML_name) == 0)
                sName = sValue;
        }
    }

    if (nPrefix == XML_NAMESPACE_CONFIG)
    {
        if (rLocalName.compareToAscii(sXML_config_item_set) == 0)
        {
            if (sName.compareToAscii(sXML_view_settings) == 0)
                pContext = new XMLConfigItemSetContext(GetImport(), nPrefix, rLocalName, xAttrList, aViewProps, NULL);
            else if (sName.compareToAscii(sXML_configuration_settings) == 0)
                pContext = new XMLConfigItemSetContext(GetImport(), nPrefix, rLocalName, xAttrList, aConfigProps, NULL);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void XMLDocumentSettingsContext::EndElement()
{
    uno::Sequence<beans::PropertyValue> aSeqViewProps;
    if (aViewProps >>= aSeqViewProps)
        GetImport().SetViewSettings(aSeqViewProps);
    uno::Sequence<beans::PropertyValue> aSeqConfigProps;
    if (aConfigProps >>= aSeqConfigProps)
        GetImport().SetConfigurationSettings(aSeqConfigProps);
}

//=============================================================================

XMLConfigBaseContext::XMLConfigBaseContext(SvXMLImport& rImport, USHORT nPrfx,
        const rtl::OUString& rLName, com::sun::star::uno::Any& rTempAny,
        XMLConfigBaseContext* pTempBaseContext)
    : SvXMLImportContext( rImport, nPrfx, rLName ),
    rAny(rTempAny),
    aProps(),
    aProp(),
    pBaseContext(pTempBaseContext)
{
}

XMLConfigBaseContext::~XMLConfigBaseContext()
{
}

//=============================================================================

XMLConfigItemSetContext::XMLConfigItemSetContext(SvXMLImport& rImport, USHORT nPrfx,
                                    const rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                    com::sun::star::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext)
    : XMLConfigBaseContext( rImport, nPrfx, rLName, rAny, pBaseContext )
{
    // here are no attributes
}

XMLConfigItemSetContext::~XMLConfigItemSetContext()
{
}

SvXMLImportContext *XMLConfigItemSetContext::CreateChildContext( USHORT nPrefix,
                                     const rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    return CreateSettingsContext(GetImport(), nPrefix, rLocalName, xAttrList, aProp, this);
}

void XMLConfigItemSetContext::EndElement()
{
    rAny <<= aProps.GetSequence();
    if (pBaseContext)
        pBaseContext->AddPropertyValue();
}

//=============================================================================

XMLConfigItemContext::XMLConfigItemContext(SvXMLImport& rImport, USHORT nPrfx, const rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                    com::sun::star::uno::Any& rTempAny,
                                    XMLConfigBaseContext* pTempBaseContext)
    : SvXMLImportContext(rImport, nPrfx, rLName),
    rAny(rTempAny),
    pBaseContext(pTempBaseContext),
    sType(),
    sValue()
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_CONFIG)
        {
            if (aLocalName.compareToAscii(sXML_type) == 0)
                sType = sValue;
        }
    }
}

XMLConfigItemContext::~XMLConfigItemContext()
{
}

SvXMLImportContext *XMLConfigItemContext::CreateChildContext( USHORT nPrefix,
                                                    const rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
    return pContext;
}

void XMLConfigItemContext::Characters( const ::rtl::OUString& rChars )
{
    sValue += rChars;
}

void XMLConfigItemContext::EndElement()
{
    if (pBaseContext)
    {
        if (sType.compareToAscii(sXML_boolean) == 0)
        {
            sal_Bool bValue(sal_False);
            if (sValue.compareToAscii(sXML_true) == 0)
                bValue = sal_True;
            rAny <<= bValue;
        }
        else if (sType.compareToAscii(sXML_short) == 0)
        {
            sal_Int32 nValue(0);
            SvXMLUnitConverter::convertNumber(nValue, sValue);
            rAny <<= static_cast<sal_Int16>(nValue);
        }
        else if (sType.compareToAscii(sXML_int) == 0)
        {
            sal_Int32 nValue(0);
            SvXMLUnitConverter::convertNumber(nValue, sValue);
            rAny <<= nValue;
        }
        else if (sType.compareToAscii(sXML_long) == 0)
        {
            sal_Int64 nValue(sValue.toInt64());
            rAny <<= nValue;
        }
        else if (sType.compareToAscii(sXML_double) == 0)
        {
            double fValue(0.0);
            SvXMLUnitConverter::convertDouble(fValue, sValue);
            rAny <<= fValue;
        }
        else if (sType.compareToAscii(sXML_string) == 0)
        {
            rAny <<= sValue;
        }
        else
            DBG_ERROR("wrong type");
        pBaseContext->AddPropertyValue();
    }
    else
        DBG_ERROR("no BaseContext");
}

//=============================================================================

XMLConfigItemMapNamedContext::XMLConfigItemMapNamedContext(SvXMLImport& rImport, USHORT nPrfx, const rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                    com::sun::star::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext)
    : XMLConfigBaseContext(rImport, nPrfx, rLName, rAny, pBaseContext)
{
}

XMLConfigItemMapNamedContext::~XMLConfigItemMapNamedContext()
{
}

SvXMLImportContext *XMLConfigItemMapNamedContext::CreateChildContext( USHORT nPrefix,
                                                    const rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    return CreateSettingsContext(GetImport(), nPrefix, rLocalName, xAttrList, aProp, this);
}

void XMLConfigItemMapNamedContext::EndElement()
{
    if (pBaseContext)
    {
        rAny <<= aProps.GetNameContainer();
        pBaseContext->AddPropertyValue();
    }
    else
        DBG_ERROR("no BaseContext");
}

//=============================================================================

XMLConfigItemMapIndexedContext::XMLConfigItemMapIndexedContext(SvXMLImport& rImport, USHORT nPrfx,
                                    const rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                    com::sun::star::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext)
    : XMLConfigBaseContext(rImport, nPrfx, rLName, rAny, pBaseContext)
{
}

XMLConfigItemMapIndexedContext::~XMLConfigItemMapIndexedContext()
{
}

SvXMLImportContext *XMLConfigItemMapIndexedContext::CreateChildContext( USHORT nPrefix,
                                                    const rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    return CreateSettingsContext(GetImport(), nPrefix, rLocalName, xAttrList, aProp, this);
}

void XMLConfigItemMapIndexedContext::EndElement()
{
    if (pBaseContext)
    {
        rAny <<= aProps.GetIndexContainer();
        pBaseContext->AddPropertyValue();
    }
    else
        DBG_ERROR("no BaseContext");
}

