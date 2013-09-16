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

#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/awt/Rectangle.hpp>
#include "XMLRectangleMembersHandler.hxx"
#include <xmloff/xmltypes.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


XMLRectangleMembersHdl::XMLRectangleMembersHdl( sal_Int32 nType )
: mnType( nType )
{
}

XMLRectangleMembersHdl::~XMLRectangleMembersHdl()
{
}

bool XMLRectangleMembersHdl::importXML(
    const OUString& rStrImpValue,
    Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    awt::Rectangle aRect( 0, 0, 0, 0 );
    if( rValue.hasValue() )
        rValue >>= aRect;

    sal_Int32 nValue;

    if (rUnitConverter.convertMeasureToCore( nValue, rStrImpValue ))
    {
        switch( mnType )
        {
            case XML_TYPE_RECTANGLE_LEFT :
                aRect.X = nValue;
                break;
            case XML_TYPE_RECTANGLE_TOP :
                aRect.Y = nValue;
                break;
            case XML_TYPE_RECTANGLE_WIDTH :
                aRect.Width = nValue;
                break;
            case XML_TYPE_RECTANGLE_HEIGHT :
                aRect.Height = nValue;
                break;
        }

        rValue <<= aRect;
        return true;
    }

    return false;
}

bool XMLRectangleMembersHdl::exportXML(
    OUString& rStrExpValue,
    const Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    awt::Rectangle aRect( 0, 0, 0, 0 );
    rValue >>= aRect;

    sal_Int32 nValue;

    switch( mnType )
    {
        case XML_TYPE_RECTANGLE_LEFT :
            nValue = aRect.X;
            break;
        case XML_TYPE_RECTANGLE_TOP :
            nValue = aRect.Y;
            break;
        case XML_TYPE_RECTANGLE_WIDTH :
            nValue = aRect.Width;
            break;
        case XML_TYPE_RECTANGLE_HEIGHT :
            nValue = aRect.Height;
            break;
        default:
            nValue = 0;  // TODO What value should this be?
            break;
    }

    OUStringBuffer sBuffer;
    rUnitConverter.convertMeasureToXML( sBuffer, nValue );
    rStrExpValue = sBuffer.makeStringAndClear();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
