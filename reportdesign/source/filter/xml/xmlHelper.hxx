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
#ifndef RPT_XMLHELPER_HXX
#define RPT_XMLHELPER_HXX

#include <xmloff/xmlprmap.hxx>
#include <xmloff/contextid.hxx>
#include <xmloff/controlpropertyhdl.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <memory>

#define CTF_RPT_NUMBERFORMAT                    (XML_DB_CTF_START + 1)

#define XML_STYLE_FAMILY_REPORT_ID              700
#define XML_STYLE_FAMILY_REPORT_NAME            "report-element"
#define XML_STYLE_FAMILY_REPORT_PREFIX          "rptelem"


class SvXMLImport;
class SvXMLExport;
class SvXMLStylesContext;
class SvXMLTokenMap;
namespace rptxml
{
    class OPropertyHandlerFactory : public ::xmloff::OControlPropertyHandlerFactory
    {
        OPropertyHandlerFactory(const OPropertyHandlerFactory&);
        void operator =(const OPropertyHandlerFactory&);
    protected:
        mutable ::std::auto_ptr<XMLConstantsPropertyHandler>    m_pDisplayHandler;
        mutable ::std::auto_ptr<XMLPropertyHandler>             m_pTextAlignHandler;
    public:
        OPropertyHandlerFactory();
        virtual ~OPropertyHandlerFactory();

        virtual const XMLPropertyHandler* GetPropertyHandler(sal_Int32 _nType) const;
    };

    class OXMLHelper
    {
    public:
        static UniReference < XMLPropertySetMapper > GetCellStylePropertyMap(bool _bOldFormat = false);

        static const SvXMLEnumMapEntry* GetReportPrintOptions();
        static const SvXMLEnumMapEntry* GetForceNewPageOptions();
        static const SvXMLEnumMapEntry* GetKeepTogetherOptions();
        static const SvXMLEnumMapEntry* GetCommandTypeOptions();
        static const SvXMLEnumMapEntry* GetImageScaleOptions();

        static const XMLPropertyMapEntry* GetTableStyleProps();
        static const XMLPropertyMapEntry* GetColumnStyleProps();

        static const XMLPropertyMapEntry* GetRowStyleProps();

        static void copyStyleElements(const bool _bOld,const ::rtl::OUString& _sStyleName,const SvXMLStylesContext* _pAutoStyles,const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet>& _xProp);
        static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet> createBorderPropertySet();

        static SvXMLTokenMap* GetReportElemTokenMap();
        static SvXMLTokenMap* GetSubDocumentElemTokenMap();

    };
// -----------------------------------------------------------------------------
} // rptxml
// -----------------------------------------------------------------------------
#endif // RPT_XMLHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
