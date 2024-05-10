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

#include <sal/config.h>

#include <string_view>

#include <drawingml/textfield.hxx>

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextField.hpp>

#include <o3tl/string_view.hxx>
#include <oox/helper/helper.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <drawingml/textparagraphproperties.hxx>
#include <drawingml/textcharacterproperties.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <editeng/flditem.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

namespace oox::drawingml {

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
                                                            const Reference< XModel > & xModel, std::u16string_view sType )
{
    Reference< XInterface > xIface;
    Reference< XMultiServiceFactory > xFactory( xModel, UNO_QUERY_THROW );
    if( o3tl::starts_with(sType, u"datetime"))
    {
        auto p = sType.substr(8);
        try
        {
            if (o3tl::starts_with(p, u"'"))
            {
                xIface = xFactory->createInstance( u"com.sun.star.text.TextField.Custom"_ustr );
                aFields.emplace_back( xIface, UNO_QUERY );
                return;
            }

            SvxDateFormat eDateFormat = TextField::getLODateFormat(sType);
            if (eDateFormat != SvxDateFormat::AppDefault)
            {
                xIface = xFactory->createInstance( u"com.sun.star.text.TextField.DateTime"_ustr );
                aFields.emplace_back( xIface, UNO_QUERY );
                Reference< XPropertySet > xProps( xIface, UNO_QUERY_THROW );
                xProps->setPropertyValue(u"NumberFormat"_ustr, Any(static_cast<sal_Int32>(eDateFormat)));
                xProps->setPropertyValue(u"IsDate"_ustr, Any(true));
                xProps->setPropertyValue(u"IsFixed"_ustr, Any(false));
            }

            SvxTimeFormat eTimeFormat = TextField::getLOTimeFormat(sType);
            if (eTimeFormat != SvxTimeFormat::AppDefault)
            {
                xIface = xFactory->createInstance( u"com.sun.star.text.TextField.DateTime"_ustr );
                aFields.emplace_back( xIface, UNO_QUERY );
                Reference< XPropertySet > xProps( xIface, UNO_QUERY_THROW );
                xProps->setPropertyValue(u"NumberFormat"_ustr, Any(static_cast<sal_Int32>(eTimeFormat)));
                xProps->setPropertyValue(u"IsDate"_ustr, Any(false));
                xProps->setPropertyValue(u"IsFixed"_ustr, Any(false));
            }
        }
        catch(const Exception &)
        {
            TOOLS_WARN_EXCEPTION("oox", "");
        }
    }
    else if ( sType == u"slidenum" )
    {
        xIface = xFactory->createInstance( u"com.sun.star.text.TextField.PageNumber"_ustr );
        aFields.emplace_back( xIface, UNO_QUERY );
    }
    else if ( sType == u"slidecount" )
    {
        xIface = xFactory->createInstance( u"com.sun.star.text.TextField.PageCount"_ustr );
        aFields.emplace_back( xIface, UNO_QUERY );
    }
    else if ( sType == u"slidename" )
    {
        xIface = xFactory->createInstance( u"com.sun.star.text.TextField.PageName"_ustr );
        aFields.emplace_back( xIface, uno::UNO_QUERY );
    }
    else if ( o3tl::starts_with(sType, u"file") )
    {
        int idx = o3tl::toInt32(sType.substr(4));
        xIface = xFactory->createInstance( u"com.sun.star.text.TextField.FileName"_ustr );
        aFields.emplace_back( xIface, UNO_QUERY );
        Reference< XPropertySet > xProps( xIface, UNO_QUERY_THROW );

        switch( idx )
        {
            case 1: // Path
                xProps->setPropertyValue(u"FileFormat"_ustr, Any(sal_Int16(1)));
                break;
            case 2: // File name without extension
                xProps->setPropertyValue(u"FileFormat"_ustr, Any(sal_Int16(2)));
                break;
            case 3: // File name with extension
                xProps->setPropertyValue(u"FileFormat"_ustr, Any(sal_Int16(3)));
                break;
            default: // Path/File name
                xProps->setPropertyValue(u"FileFormat"_ustr, Any(sal_Int16(0)));
        }
    }
    else if( sType == u"author" )
    {
        xIface = xFactory->createInstance( u"com.sun.star.text.TextField.Author"_ustr );
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
        if ( aTextCharacterProps.moHeight.has_value() )
            nCharHeight = aTextCharacterProps.moHeight.value();
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
                        xText->insertString( xAt, u" "_ustr, false );
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
        TOOLS_WARN_EXCEPTION("oox", "OOX:  TextField::insertAt()");
    }

    return nCharHeight;
}

SvxDateFormat TextField::getLODateFormat(std::u16string_view rDateTimeType)
{
    auto aDateTimeNum = rDateTimeType.substr(8);

    if( aDateTimeNum.empty() ) // "datetime"
        return SvxDateFormat::StdSmall;

    int nDateTimeNum = o3tl::toInt32(aDateTimeNum);

    switch( nDateTimeNum )
    {
    case 1: // Date dd/mm/yyyy
    case 8: // DateTime dd/mm/yyyy H:MM PM
    case 9: // DateTime dd/mm/yyyy H:MM:SS PM
        return SvxDateFormat::B;
    case 2: // Date Day, Month dd, yyyy
        return SvxDateFormat::StdBig;
    case 3: // Date dd Month yyyy
    case 4: // Date Month dd, yyyy - no exact map
    case 6: // Date Month yy - no exact map
        return SvxDateFormat::D;
    case 5: // Date dd-Mon-yy - no exact map
    case 7: // Date Mon-yy - no exact map
        return SvxDateFormat::C;
    case 10: // Time H:MM - not a date format
    case 11: // Time H:MM:SS - not a date format
    case 12: // Time H:MM PM - not a date format
    case 13: // Time H:MM:SS PM - not a date format
    default:
        return SvxDateFormat::AppDefault;
    }
}

SvxTimeFormat TextField::getLOTimeFormat(std::u16string_view rDateTimeType)
{
    auto aDateTimeNum = rDateTimeType.substr(8);
    int nDateTimeNum = o3tl::toInt32(aDateTimeNum);

    switch( nDateTimeNum )
    {
    case 8: // DateTime dd/mm/yyyy H:MM PM
    case 12: // Time H:MM PM
        return SvxTimeFormat::HH12_MM;
    case 9: // DateTime dd/mm/yyyy H:MM:SS PM
    case 13: // Time H:MM:SS PM
        return SvxTimeFormat::HH12_MM_SS;
    case 10: // Time H:MM
        return SvxTimeFormat::HH24_MM;
    case 11: // Time H:MM:SS
        return SvxTimeFormat::Standard;
    case 1: // Date dd/mm/yyyy
    case 2: // Date Day, Month dd, yyyy
    case 3: // Date dd Month yyyy
    case 4: // Date Month dd, yyyy
    case 5: // Date dd-Mon-yy
    case 6: // Date Month yy
    case 7: // Date Mon-yy
    default:
        return SvxTimeFormat::AppDefault;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
