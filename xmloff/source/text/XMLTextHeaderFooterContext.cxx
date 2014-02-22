/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XRelativeTextContentRemove.hpp>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include "XMLTextHeaderFooterContext.hxx"
#include <xmloff/XMLTextTableContext.hxx>
#include <xmloff/xmlimp.hxx>



using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;


TYPEINIT1( XMLTextHeaderFooterContext, SvXMLImportContext );

XMLTextHeaderFooterContext::XMLTextHeaderFooterContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                       const OUString& rLName,
                       const uno::Reference<
                            xml::sax::XAttributeList > &,
                        const Reference < XPropertySet > & rPageStylePropSet,
                       sal_Bool bFooter, sal_Bool bLft, sal_Bool bFrst ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xPropSet( rPageStylePropSet ),
    sOn( bFooter ? OUString("FooterIsOn") : OUString("HeaderIsOn") ),
    sShareContent( bFooter ? OUString("FooterIsShared") : OUString("HeaderIsShared") ),
    sShareContentFirst( "FirstIsShared" ),
    sText( bFooter ? OUString("FooterText") : OUString("HeaderText") ),
    sTextFirst(bFooter ? OUString("FooterTextFirst") : OUString("HeaderTextFirst")),
    sTextLeft( bFooter ?  OUString("FooterTextLeft") : OUString("HeaderTextLeft") ),
    bInsertContent( sal_True ),
    bLeft( bLft ),
    bFirst( bFrst )
{
    
    if( bLeft || bFirst )
    {
        Any aAny;

        aAny = xPropSet->getPropertyValue( sOn );
        sal_Bool bOn = *(sal_Bool *)aAny.getValue();

        if( bOn )
        {
            if (bLeft)
            {
                aAny = xPropSet->getPropertyValue( sShareContent );
                sal_Bool bShared = *(sal_Bool *)aAny.getValue();
                if( bShared )
                {
                    
                    bShared = sal_False;
                    aAny.setValue( &bShared, ::getBooleanCppuType() );
                    xPropSet->setPropertyValue( sShareContent, aAny );
                }
            }
            if (bFirst)
            {
                aAny = xPropSet->getPropertyValue( sShareContentFirst );
                sal_Bool bSharedFirst = aAny.has<sal_Bool>() && *(sal_Bool *)aAny.getValue();
                if( bSharedFirst )
                {
                    
                    bSharedFirst = sal_False;
                    aAny.setValue( &bSharedFirst, ::getBooleanCppuType() );
                    xPropSet->setPropertyValue( sShareContentFirst, aAny );
                }
            }
        }
        else
        {
            
            
            bInsertContent = sal_False;
        }
    }
}

XMLTextHeaderFooterContext::~XMLTextHeaderFooterContext()
{
}

SvXMLImportContext *XMLTextHeaderFooterContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    if( bInsertContent )
    {
        if( !xOldTextCursor.is() )
        {
            sal_Bool bRemoveContent = sal_True;
            Any aAny;
            if( bLeft || bFirst )
            {
                
                
                if (bLeft)
                    aAny = xPropSet->getPropertyValue( sTextLeft );
                else
                    aAny = xPropSet->getPropertyValue( sTextFirst );
            }
            else
            {
                aAny = xPropSet->getPropertyValue( sOn );
                sal_Bool bOn = *(sal_Bool *)aAny.getValue();

                if( !bOn )
                {
                    
                    bOn = sal_True;
                    aAny.setValue( &bOn, ::getBooleanCppuType() );
                    xPropSet->setPropertyValue( sOn, aAny );

                    
                    
                    bRemoveContent = sal_False;
                }

                
                aAny = xPropSet->getPropertyValue( sShareContent );
                sal_Bool bShared = *(sal_Bool *)aAny.getValue();
                if( !bShared )
                {
                    bShared = sal_True;
                    aAny.setValue( &bShared, ::getBooleanCppuType() );
                    xPropSet->setPropertyValue( sShareContent, aAny );
                }

                aAny = xPropSet->getPropertyValue( sText );
            }

            Reference < XText > xText;
            aAny >>= xText;

            if( bRemoveContent )
            {
                OUString aText;
                xText->setString( aText );
            }

            UniReference < XMLTextImportHelper > xTxtImport =
                GetImport().GetTextImport();

            xOldTextCursor = xTxtImport->GetCursor();
            xTxtImport->SetCursor( xText->createTextCursor() );
        }

        pContext =
            GetImport().GetTextImport()->CreateTextChildContext(
                GetImport(), nPrefix, rLocalName, xAttrList,
                XML_TEXT_TYPE_HEADER_FOOTER );
    }
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void XMLTextHeaderFooterContext::EndElement()
{
    if( xOldTextCursor.is() )
    {
        GetImport().GetTextImport()->DeleteParagraph();
        GetImport().GetTextImport()->SetCursor( xOldTextCursor );
    }
    else if( !bLeft )
    {
        
        
        sal_Bool bOn = sal_False;
        Any aAny;
        aAny.setValue( &bOn, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( sOn, aAny );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
