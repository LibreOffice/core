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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLHELPER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLHELPER_HXX

#include <xmloff/xmlprmap.hxx>
#include <xmloff/contextid.hxx>
#include <xmloff/controlpropertyhdl.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <memory>

#define CTF_RPT_NUMBERFORMAT                    (XML_DB_CTF_START + 1)



class SvXMLStylesContext;
class SvXMLTokenMap;
namespace rptxml
{
    class OPropertyHandlerFactory : public ::xmloff::OControlPropertyHandlerFactory
    {
        OPropertyHandlerFactory(const OPropertyHandlerFactory&) = delete;
        void operator =(const OPropertyHandlerFactory&) = delete;
    protected:
        mutable ::std::unique_ptr<XMLConstantsPropertyHandler>    m_pDisplayHandler;
        mutable ::std::unique_ptr<XMLPropertyHandler>             m_pTextAlignHandler;
    public:
        OPropertyHandlerFactory();
        virtual ~OPropertyHandlerFactory();

        virtual const XMLPropertyHandler* GetPropertyHandler(sal_Int32 _nType) const SAL_OVERRIDE;
    };

    class OXMLHelper
    {
    public:
        static rtl::Reference < XMLPropertySetMapper > GetCellStylePropertyMap(bool _bOldFormat, bool bForExport);

        static const SvXMLEnumMapEntry* GetReportPrintOptions();
        static const SvXMLEnumMapEntry* GetForceNewPageOptions();
        static const SvXMLEnumMapEntry* GetKeepTogetherOptions();
        static const SvXMLEnumMapEntry* GetCommandTypeOptions();
        static const SvXMLEnumMapEntry* GetImageScaleOptions();

        static const XMLPropertyMapEntry* GetTableStyleProps();
        static const XMLPropertyMapEntry* GetColumnStyleProps();

        static const XMLPropertyMapEntry* GetRowStyleProps();

        static void copyStyleElements(const bool _bOld,const OUString& _sStyleName,const SvXMLStylesContext* _pAutoStyles,const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet>& _xProp);
        static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet> createBorderPropertySet();

        static SvXMLTokenMap* GetReportElemTokenMap();
        static SvXMLTokenMap* GetSubDocumentElemTokenMap();

    };

} // rptxml

#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
