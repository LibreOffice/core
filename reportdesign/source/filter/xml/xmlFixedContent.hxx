/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef RPT_XMLFIXEDCONTENT_HXX
#define RPT_XMLFIXEDCONTENT_HXX

#include "xmlReportElementBase.hxx"
#include <com/sun/star/drawing/XShapes.hpp>


namespace rptxml
{
    class ORptFilter;
    class OXMLCell;
    class OXMLFixedContent : public OXMLReportElementBase
    {
        ::rtl::OUString     m_sPageText; // page count and page number
        ::rtl::OUString     m_sLabel;
        OXMLCell&           m_rCell;
        OXMLFixedContent*   m_pInP; // if set than we are in text-p element
        bool                m_bFormattedField;

    protected:
        virtual SvXMLImportContext* _CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
        OXMLFixedContent(const OXMLFixedContent&);
        void operator =(const OXMLFixedContent&);
    public:

        OXMLFixedContent( ORptFilter& rImport, sal_uInt16 nPrfx,
                    const ::rtl::OUString& rLName
                    ,OXMLCell& _rCell
                    ,OXMLTable* _pContainer
                    ,OXMLFixedContent* _pInP = NULL);
        virtual ~OXMLFixedContent();

        // This method is called for all characters that are contained in the
        // current element. The default is to ignore them.
        virtual void Characters( const ::rtl::OUString& rChars );

        virtual void EndElement();
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLFIXEDCONTENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
