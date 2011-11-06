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



#ifndef XML_SETTINGS_EXPORT_CONTEXT_HXX
#define XML_SETTINGS_EXPORT_CONTEXT_HXX

/** === begin UNO includes === **/
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
/** === end UNO includes === **/

#include "xmloff/xmltoken.hxx"

//........................................................................
namespace xmloff
{
//........................................................................

    //====================================================================
    //= XMLExporter
    //====================================================================
    class SAL_NO_VTABLE XMLSettingsExportContext
    {
    public:
        virtual void    AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName,
                                      const ::rtl::OUString& i_rValue ) = 0;
        virtual void    AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName,
                                      enum ::xmloff::token::XMLTokenEnum i_eValue ) = 0;

        virtual void    StartElement( enum ::xmloff::token::XMLTokenEnum i_eName,
                                      const sal_Bool i_bIgnoreWhitespace ) = 0;
        virtual void    EndElement(   const sal_Bool i_bIgnoreWhitespace ) = 0;

        virtual void    Characters( const ::rtl::OUString& i_rCharacters ) = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        GetServiceFactory() const = 0;

    };

//........................................................................
} // namespace xmloff
//........................................................................

#endif // XML_SETTINGS_EXPORT_CONTEXT_HXX
