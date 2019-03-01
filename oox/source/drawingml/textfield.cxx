/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <drawingml/textfield.hxx>

#include <list>

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <sal/log.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextField.hpp>

#include <oox/helper/helper.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <drawingml/textparagraphproperties.hxx>
#include <drawingml/textcharacterproperties.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

namespace oox { namespace drawingml {

TextField::TextField()
{
}

namespace {

/** instantiate the textfields. Because of semantics difference between
 * OpenXML and OpenOffice, some OpenXML field might cause two fields to be created.
 * @param aFields the created fields. The list is empty if no field has been created.
 * @param xModel the model
 * @param sType the OpenXML field type.
 */
void lclCreateTextFields( std::vector< Reference< XTextField > > & aFields,
                                                            const Reference< XModel > & xModel, const OUString & sType )
{
    Reference< XInterface > xIface;
    Reference< XMultiServiceFactory > xFactory( xModel, UNO_QUERY_THROW );
    if( sType.startsWith("datetime"))
    {
        OString s = OUStringToOString( sType, RTL_TEXTENCODING_UTF8);
        OString p( s.pData->buffer + 8 );
        try
        {
            bool bIsDate = true;
            int idx = p.toInt32();
            sal_uInt16 nNumFmt;
            xIface = xFactory->createInstance( "com.sun.star.text.TextField.DateTime" );
            aFields.emplace_back( xIface, UNO_QUERY );
            Reference< XPropertySet > xProps( xIface, UNO_QUERY_THROW );

            // here we should format the field properly. waiting after #i81091.
            switch( idx )
            {
            case 1: // Date dd/mm/yyyy
                // this is the default format...
                nNumFmt = 5;
                xProps->setPropertyValue("NumberFormat", makeAny(nNumFmt));
                break;
            case 2: // Date Day, Month dd, yyyy
                break;
            case 3: // Date dd Month yyyy
                nNumFmt = 3;
                xProps->setPropertyValue("NumberFormat", makeAny(nNumFmt));
                break;
            case 4: // Date Month dd, yyyy
                break;
            case 5: // Date dd-Mon-yy
                break;
            case 6: // Date Month yy
                break;
            case 7: // Date Mon-yy
                break;
            case 8: // DateTime dd/mm/yyyy H:MM PM
                lclCreateTextFields( aFields, xModel, "datetime12" );
                break;
            case 9: // DateTime dd/mm/yy H:MM:SS PM
                lclCreateTextFields( aFields, xModel, "datetime13" );
                break;
            case 10: // Time H:MM
                bIsDate = false;
                nNumFmt = 3;
                xProps->setPropertyValue("NumberFormat", makeAny(nNumFmt));
                break;
            case 11: // Time H:MM:SS
                bIsDate = false;
                // this is the default format
                nNumFmt = 2;
                xProps->setPropertyValue("NumberFormat", makeAny(nNumFmt));
                break;
            case 12: // Time H:MM PM
                bIsDate = false;
                nNumFmt = 6;
                xProps->setPropertyValue("NumberFormat", makeAny(nNumFmt));
                break;
            case 13: // Time H:MM:SS PM
                bIsDate = false;
                nNumFmt = 7;
                xProps->setPropertyValue("NumberFormat", makeAny(nNumFmt));
                break;
            default:
                nNumFmt = 2;
                xProps->setPropertyValue("NumberFormat", makeAny(nNumFmt));
            }
            xProps->setPropertyValue( "IsDate", makeAny( bIsDate ) );
            xProps->setPropertyValue( "IsFixed", makeAny( false ) );
        }
        catch(Exception & e)
        {
            SAL_WARN("oox",  e );
        }
    }
    else if ( sType == "slidenum" )
    {
        xIface = xFactory->createInstance( "com.sun.star.text.TextField.PageNumber" );
        aFields.emplace_back( xIface, UNO_QUERY );
    }
    else if ( sType == "slidecount" )
    {
        xIface = xFactory->createInstance( "com.sun.star.text.TextField.PageCount" );
        aFields.emplace_back( xIface, UNO_QUERY );
    }
    else if ( sType == "slidename" )
    {
        xIface = xFactory->createInstance( "com.sun.star.text.TextField.PageName" );
        aFields.emplace_back( xIface, uno::UNO_QUERY );
    }
    else if ( sType.startsWith("file") )
    {
        OString s = OUStringToOString( sType, RTL_TEXTENCODING_UTF8);
        OString p( s.pData->buffer + 4 );
        int idx = p.toInt32();
        xIface = xFactory->createInstance( "com.sun.star.text.TextField.FileName" );
        aFields.emplace_back( xIface, UNO_QUERY );
        Reference< XPropertySet > xProps( xIface, UNO_QUERY_THROW );

        switch( idx )
        {
            case 1: // Path
                xProps->setPropertyValue("FileFormat", makeAny<sal_Int16>(1));
                break;
            case 2: // File name without extension
                xProps->setPropertyValue("FileFormat", makeAny<sal_Int16>(2));
                break;
            case 3: // File name with extension
                xProps->setPropertyValue("FileFormat", makeAny<sal_Int16>(3));
                break;
            default: // Path/File name
                xProps->setPropertyValue("FileFormat", makeAny<sal_Int16>(0));
        }
    }
    else if( sType == "author" )
    {
        xIface = xFactory->createInstance( "com.sun.star.text.TextField.Author" );
        aFields.emplace_back( xIface, UNO_QUERY );
    }
}

} // namespace

sal_Int32 TextField::insertAt(
        const ::oox::core::XmlFilterBase& rFilterBase,
        const Reference < XText > & xText,
        const Reference < XTextCursor > &xAt,
        const TextCharacterProperties& rTextCharacterStyle,
        float /*nDefaultCharHeight*/) const
{
    sal_Int32 nCharHeight = 0;
    try
    {
        PropertyMap aioBulletList;
        Reference< XPropertySet > xProps( xAt, UNO_QUERY);
        PropertySet aPropSet( xProps );

        maTextParagraphProperties.pushToPropSet( &rFilterBase, xProps, aioBulletList, nullptr, true, 18 );

        TextCharacterProperties aTextCharacterProps( rTextCharacterStyle );
        aTextCharacterProps.assignUsed( maTextParagraphProperties.getTextCharacterProperties() );
        aTextCharacterProps.assignUsed( getTextCharacterProperties() );
        if ( aTextCharacterProps.moHeight.has() )
            nCharHeight = aTextCharacterProps.moHeight.get();
        aTextCharacterProps.pushToPropSet( aPropSet, rFilterBase );

        std::vector< Reference< XTextField > > fields;
        lclCreateTextFields( fields, rFilterBase.getModel(), msType );
        if( !fields.empty() )
        {
            bool bFirst = true;
            for (auto const& field : fields)
            {
                if( field.is() )
                {
                    Reference< XTextContent > xContent( field, UNO_QUERY);
                    if( bFirst)
                    {
                        bFirst = false;
                    }
                    else
                    {
                        xText->insertString( xAt, " ", false );
                    }
                    xText->insertTextContent( xAt, xContent, false );
                }
            }
        }
        else
        {
            xText->insertString( xAt, getText(), false );
        }
    }
    catch( const Exception&  )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("oox", "OOX:  TextField::insertAt() exception " << exceptionToString(ex));
    }

    return nCharHeight;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
