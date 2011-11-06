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


#ifndef _XMLOFF_TXTIMPPR_HXX
#define _XMLOFF_TXTIMPPR_HXX

#include "xmloff/dllapi.h"
#include <xmloff/xmlimppr.hxx>

class XMLFontStylesContext;

class XMLOFF_DLLPUBLIC XMLTextImportPropertyMapper : public SvXMLImportPropertyMapper
{
    sal_Int32 nSizeTypeIndex;
    sal_Int32 nWidthTypeIndex;
    SvXMLImportContextRef xFontDecls;

    void FontFinished(
                XMLPropertyState *pFontFamilyNameState,
                XMLPropertyState *pFontStyleNameState,
                XMLPropertyState *pFontFamilyState,
                XMLPropertyState *pFontPitchState,
                XMLPropertyState *pFontCharsetState ) const;

    void FontDefaultsCheck(
                XMLPropertyState*pFontFamilyName,
                XMLPropertyState* pFontStyleName,
                XMLPropertyState* pFontFamily,
                XMLPropertyState* pFontPitch,
                XMLPropertyState* pFontCharSet,
                XMLPropertyState** ppNewFontStyleName,
                XMLPropertyState** ppNewFontFamily,
                XMLPropertyState** ppNewFontPitch,
                XMLPropertyState** ppNewFontCharSet ) const;

protected:
    virtual sal_Bool handleSpecialItem(
            XMLPropertyState& rProperty,
            ::std::vector< XMLPropertyState >& rProperties,
            const ::rtl::OUString& rValue,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap ) const;
public:
    XMLTextImportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper,
            SvXMLImport& rImport,
            XMLFontStylesContext *pFontDecls = 0 );
    virtual ~XMLTextImportPropertyMapper();

    void SetFontDecls( XMLFontStylesContext *pFontDecls );

    /** This method is called when all attributes have benn processed. It may be used to remove items that are incomplete */
    virtual void finished(
            ::std::vector< XMLPropertyState >& rProperties,
            sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const;
};

#endif  //  _XMLOFF_XMLIMPPR_HXX
