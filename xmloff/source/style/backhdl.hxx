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



#ifndef _XMLOFF_PROPERTYHANDLER_BRUSHTYPES_HXX
#define _XMLOFF_PROPERTYHANDLER_BRUSHTYPES_HXX

#include <xmloff/xmlprhdl.hxx>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>

/**
    PropertyHandler for the XML-data-type:
*/
class XMLBackGraphicPositionPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLBackGraphicPositionPropHdl();

    /// TabStops will be imported/exported as XML-Elements. So the Import/Export-work must be done at another place.
    using XMLPropertyHandler::importXML;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;

private:
    void MergeXMLHoriPos( ::com::sun::star::style::GraphicLocation& ePos, ::com::sun::star::style::GraphicLocation eHori ) const;
    void MergeXMLVertPos( ::com::sun::star::style::GraphicLocation& ePos, ::com::sun::star::style::GraphicLocation eVert ) const;
};

#endif      // _XMLOFF_PROPERTYHANDLER_BRUSHTYPES_HXX
