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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLFIXEDCONTENT_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLFIXEDCONTENT_HXX

#include "xmlReportElementBase.hxx"
#include <com/sun/star/drawing/XShapes.hpp>


namespace rptxml
{
    class ORptFilter;
    class OXMLCell;
    class OXMLFixedContent : public OXMLReportElementBase
    {
        OUString     m_sPageText; // page count and page number
        OUString     m_sLabel;
        OXMLCell&           m_rCell;
        OXMLFixedContent*   m_pInP; // if set than we are in text-p element
        bool                m_bFormattedField;

    protected:
        virtual SvXMLImportContext* _CreateChildContext( sal_uInt16 nPrefix,
                    const OUString& rLocalName,
                    const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;
    private:
        OXMLFixedContent(const OXMLFixedContent&) = delete;
        OXMLFixedContent& operator =(const OXMLFixedContent&) = delete;
    public:

        OXMLFixedContent( ORptFilter& rImport, sal_uInt16 nPrfx,
                    const OUString& rLName
                    ,OXMLCell& _rCell
                    ,OXMLTable* _pContainer
                    ,OXMLFixedContent* _pInP = NULL);
        virtual ~OXMLFixedContent();

        // This method is called for all characters that are contained in the
        // current element. The default is to ignore them.
        virtual void Characters( const OUString& rChars ) override;

        virtual void EndElement() override;
    };

} // namespace rptxml


#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLFIXEDCONTENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
