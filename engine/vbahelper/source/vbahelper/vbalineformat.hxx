/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#ifndef INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBALINEFORMAT_HXX
#define INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBALINEFORMAT_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/msforms/XLineFormat.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ov::msforms::XLineFormat > ScVbaLineFormat_BASE;

class ScVbaLineFormat : public ScVbaLineFormat_BASE
{
private:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
    sal_Int32 m_nLineDashStyle;
    double m_nLineWeight;
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
    static sal_Int32 convertLineStartEndNameToArrowheadStyle( std::u16string_view sLineName );
    /// @throws cpo::uno::RuntimeException
    static OUString convertArrowheadStyleToLineStartEndName( sal_Int32 nArrowheadStyle );
public:
    ScVbaLineFormat( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< cpo::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape >& xShape );

    // Attributes
    virtual sal_Int32 getBeginArrowheadStyle() override;
    virtual void setBeginArrowheadStyle( sal_Int32 _beginarrowheadstyle ) override;
    virtual sal_Int32 getBeginArrowheadLength() override;
    virtual void setBeginArrowheadLength( sal_Int32 _beginarrowheadlength ) override;
    virtual sal_Int32 getBeginArrowheadWidth() override;
    virtual void setBeginArrowheadWidth( sal_Int32 _beginarrowheadwidth ) override;
    virtual sal_Int32 getEndArrowheadStylel() override;
    virtual void setEndArrowheadStylel( sal_Int32 _endarrowheadstylel ) override;
    virtual sal_Int32 getEndArrowheadLength() override;
    virtual void setEndArrowheadLength( sal_Int32 _endarrowheadlength ) override;
    virtual sal_Int32 getEndArrowheadWidth() override;
    virtual void setEndArrowheadWidth( sal_Int32 _endarrowheadwidth ) override;
    virtual double getWeight() override;
    virtual void setWeight( double _weight ) override;
    virtual bool getVisible() override;
    virtual void setVisible( bool _visible ) override;
    virtual double getTransparency() override;
    virtual void setTransparency( double _transparency ) override;
    virtual sal_Int16 getStyle() override;
    virtual void setStyle( sal_Int16 _style ) override;
    virtual sal_Int32 getDashStyle() override;
    virtual void setDashStyle( sal_Int32 _dashstyle ) override;

    // Methods
    virtual css::uno::Reference< ov::msforms::XColorFormat > BackColor() override;
    virtual css::uno::Reference< ov::msforms::XColorFormat > ForeColor() override;
};

#endif // INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBALINEFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
