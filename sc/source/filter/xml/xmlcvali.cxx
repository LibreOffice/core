/*************************************************************************
 *
 *  $RCSfile: xmlcvali.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sab $ $Date: 2000-10-12 06:35:57 $
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

#include "xmlcvali.hxx"
#include "xmlimprt.hxx"
#include "xmlconti.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlkywd.hxx>

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLContentValidationsContext::ScXMLContentValidationsContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    // here are no attributes
}

ScXMLContentValidationsContext::~ScXMLContentValidationsContext()
{
}

SvXMLImportContext *ScXMLContentValidationsContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationsElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_CONTENT_VALIDATION:
            pContext = new ScXMLContentValidationContext( GetScImport(), nPrefix, rLName, xAttrList);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLContentValidationsContext::EndElement()
{
}

ScXMLContentValidationContext::ScXMLContentValidationContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sName(),
    sHelpTitle(),
    sHelpMessage(),
    sErrorTitle(),
    sErrorMessage(),
    sErrorMessageType(),
    sCondition(),
    sBaseCellAddress(),
    bAllowEmptyCell(sal_True),
    bDisplayHelp(sal_False),
    bDisplayError(sal_False)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_CONTENT_VALIDATION_NAME:
                sName = sValue;
            break;
            case XML_TOK_CONTENT_VALIDATION_CONDITION:
                sCondition = sValue;
            break;
            case XML_TOK_CONTENT_VALIDATION_BASE_CELL_ADDRESS:
                sBaseCellAddress = sValue;
            break;
            case XML_TOK_CONTENT_VALIDATION_ALLOW_EMPTY_CELL:
                if (sValue.compareToAscii(sXML_false) == 0)
                    bAllowEmptyCell = sal_True;
            break;
        }
    }
}

ScXMLContentValidationContext::~ScXMLContentValidationContext()
{
}

SvXMLImportContext *ScXMLContentValidationContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_CONTENT_VALIDATION_ELEM_HELP_MESSAGE:
            pContext = new ScXMLHelpMessageContext( GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
        case XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MESSAGE:
            pContext = new ScXMLErrorMessageContext( GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
        case XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MACRO:
            pContext = new ScXMLErrorMacroContext( GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLContentValidationContext::EndElement()
{
}

void ScXMLContentValidationContext::SetHelpMessage(const rtl::OUString& sTitle, const rtl::OUString& sMessage, const sal_Bool bDisplay)
{
    sHelpTitle = sTitle;
    sHelpMessage = sMessage;
    bDisplayHelp = bDisplay;
}

void ScXMLContentValidationContext::SetErrorMessage(const rtl::OUString& sTitle, const rtl::OUString& sMessage,
    const rtl::OUString& sMessageType, const sal_Bool bDisplay)
{
    sErrorTitle = sTitle;
    sErrorMessage = sMessage;
    sErrorMessageType = sMessageType;
    bDisplayError = bDisplay;
}

void ScXMLContentValidationContext::SetErrorMacro(const rtl::OUString& sName, const sal_Bool bExecute)
{
    sErrorTitle = sName;
    sErrorMessageType = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("macro"));
    bDisplayError = bExecute;
}

ScXMLHelpMessageContext::ScXMLHelpMessageContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLContentValidationContext* pTempValidationContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sTitle(),
    sMessage(),
    nParagraphCount(0),
    bDisplay(sal_False)
{
    pValidationContext = pTempValidationContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationHelpMessageAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_HELP_MESSAGE_ATTR_TITLE:
                sTitle = sValue;
            break;
            case XML_TOK_HELP_MESSAGE_ATTR_DISPLAY:
                if (sValue.compareToAscii(sXML_true) == 0)
                    bDisplay = sal_True;
            break;
        }
    }
}

ScXMLHelpMessageContext::~ScXMLHelpMessageContext()
{
}

SvXMLImportContext *ScXMLHelpMessageContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationMessageElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_P:
        {
            if(nParagraphCount)
                sMessage += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
            nParagraphCount++;
            pContext = new ScXMLContentContext( GetScImport(), nPrefix, rLName, xAttrList, sMessage);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLHelpMessageContext::EndElement()
{
    pValidationContext->SetHelpMessage(sTitle, sMessage, bDisplay);
}

ScXMLErrorMessageContext::ScXMLErrorMessageContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLContentValidationContext* pTempValidationContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sTitle(),
    sMessage(),
    sMessageType(),
    nParagraphCount(0),
    bDisplay(sal_False)
{
    pValidationContext = pTempValidationContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationErrorMessageAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_ERROR_MESSAGE_ATTR_TITLE:
                sTitle = sValue;
            break;
            case XML_TOK_ERROR_MESSAGE_ATTR_MESSAGE_TYPE:
                sMessageType = sValue;
            break;
            case XML_TOK_ERROR_MESSAGE_ATTR_DISPLAY:
                if (sValue.compareToAscii(sXML_true) == 0)
                    bDisplay = sal_True;
            break;
        }
    }
}

ScXMLErrorMessageContext::~ScXMLErrorMessageContext()
{
}

SvXMLImportContext *ScXMLErrorMessageContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationMessageElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_P:
        {
            if(nParagraphCount)
                sMessage += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
            nParagraphCount++;
            pContext = new ScXMLContentContext( GetScImport(), nPrefix, rLName, xAttrList, sMessage);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLErrorMessageContext::EndElement()
{
    pValidationContext->SetErrorMessage(sTitle, sMessage, sMessageType, bDisplay);
}

ScXMLErrorMacroContext::ScXMLErrorMacroContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLContentValidationContext* pTempValidationContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sName(),
    bExecute(sal_False)
{
    pValidationContext = pTempValidationContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationErrorMacroAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_ERROR_MACRO_ATTR_NAME:
                sName = sValue;
            break;
            case XML_TOK_ERROR_MACRO_ATTR_EXECUTE:
                if (sValue.compareToAscii(sXML_true) == 0)
                    bExecute = sal_True;
            break;
        }
    }
}

ScXMLErrorMacroContext::~ScXMLErrorMacroContext()
{
}

SvXMLImportContext *ScXMLErrorMacroContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLErrorMacroContext::EndElement()
{
    pValidationContext->SetErrorMacro(sName, bExecute);
}
