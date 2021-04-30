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

#pragma once

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlprmap.hxx>

#include <map>
#include <memory>


namespace dbaxml
{

using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::xml::sax;


class ODBFilter : public SvXMLImport
{
public:
    typedef std::map< OUString, Sequence<PropertyValue> > TPropertyNameMap;
private:
    TPropertyNameMap                                m_aQuerySettings;
    TPropertyNameMap                                m_aTablesSettings;
    std::vector< css::beans::PropertyValue>         m_aInfoSequence;

    mutable rtl::Reference < XMLPropertySetMapper >   m_xTableStylesPropertySetMapper;
    mutable rtl::Reference < XMLPropertySetMapper >   m_xColumnStylesPropertySetMapper;
    mutable rtl::Reference < XMLPropertySetMapper >   m_xCellStylesPropertySetMapper;
    Reference<XPropertySet>                         m_xDataSource;
    bool                                            m_bNewFormat;

    /// @throws RuntimeException
    bool                            implImport( const Sequence< PropertyValue >& rDescriptor );

    /** fills the map with the Properties
        @param  _rValue
            The Any where the sequence resists in.
        @param  _rMap
            The map to fill.
    */
    static void fillPropertyMap(const Any& _rValue,TPropertyNameMap& _rMap);

public:
    SvXMLImportContext* CreateStylesContext( bool bIsAutoStyle );

protected:
    // SvXMLImport
    virtual SvXMLImportContext *CreateFastContext(sal_Int32 Element,
        const ::css::uno::Reference< ::css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual ~ODBFilter()  noexcept override;
public:

    explicit ODBFilter( const Reference< XComponentContext >& _rxContext );

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) override;

    /// @throws css::uno::RuntimeException
    static OUString getImplementationName_Static();

    // helper class
    virtual void SetViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;
    virtual void SetConfigurationSettings(const css::uno::Sequence<css::beans::PropertyValue>& aConfigProps) override;

    const Reference<XPropertySet>& getDataSource() const { return m_xDataSource; }

    const TPropertyNameMap& getQuerySettings() const { return m_aQuerySettings;}

    rtl::Reference < XMLPropertySetMapper > const & GetTableStylesPropertySetMapper() const;
    rtl::Reference < XMLPropertySetMapper > const & GetColumnStylesPropertySetMapper() const;
    rtl::Reference < XMLPropertySetMapper > const & GetCellStylesPropertySetMapper() const;

    /** add an Info to the sequence which will be appended to the data source
        @param  _rInfo The property to append.
    */
    void addInfo(const css::beans::PropertyValue& _rInfo)
    {
        m_aInfoSequence.push_back(_rInfo);
    }

    void setPropertyInfo();

    bool isNewFormat() const { return m_bNewFormat; }
    void setNewFormat(bool _bNewFormat) { m_bNewFormat = _bNewFormat; }
};

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
