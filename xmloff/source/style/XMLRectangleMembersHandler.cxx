/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HDL_
#include <com/sun/star/awt/Rectangle.hdl>
#endif
#include "XMLRectangleMembersHandler.hxx"
#include <xmloff/xmltypes.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


XMLRectangleMembersHdl::XMLRectangleMembersHdl( sal_Int32 nType )
: mnType( nType )
{
}

XMLRectangleMembersHdl::~XMLRectangleMembersHdl()
{
}

sal_Bool XMLRectangleMembersHdl::importXML(
    const OUString& rStrImpValue,
    Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    awt::Rectangle aRect( 0, 0, 0, 0 );
    if( rValue.hasValue() )
        rValue >>= aRect;

    sal_Int32 nValue;

    if( rUnitConverter.convertMeasure( nValue, rStrImpValue ) )
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
        return sal_True;
    }

    return sal_False;
}

sal_Bool XMLRectangleMembersHdl::exportXML(
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

    rtl::OUStringBuffer sBuffer;
    rUnitConverter.convertMeasure( sBuffer, nValue );
    rStrExpValue = sBuffer.makeStringAndClear();
    return sal_True;
}

