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



#ifndef _XMLOFF_ENUMPROPERTYHANDLER_HXX
#define _XMLOFF_ENUMPROPERTYHANDLER_HXX

#include <xmloff/xmlprhdl.hxx>
#include <com/sun/star/uno/Type.h>
#include <xmloff/xmlement.hxx>

/**
    PropertyHandler for a generic xml enumeration type:
*/
class XMLOFF_DLLPUBLIC XMLEnumPropertyHdl : public XMLPropertyHandler
{
private:
    const SvXMLEnumMapEntry* mpEnumMap;
    const ::com::sun::star::uno::Type & mrType;

public:
    XMLEnumPropertyHdl( const SvXMLEnumMapEntry* pEnumMap, const ::com::sun::star::uno::Type & rType ) : mpEnumMap( pEnumMap ), mrType( rType ) {}
    virtual ~XMLEnumPropertyHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

#endif      // _XMLOFF_ENUMPROPERTYHANDLER_HXX
