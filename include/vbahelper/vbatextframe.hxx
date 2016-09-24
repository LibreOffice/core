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
#ifndef INCLUDED_VBAHELPER_VBATEXTFRAME_HXX
#define INCLUDED_VBAHELPER_VBATEXTFRAME_HXX

#include <exception>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <ooo/vba/msforms/XTextFrame.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbadllapi.h>
#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbahelperinterface.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace drawing { class XShape; }
    namespace uno { class XComponentContext; }
} } }

namespace ooo { namespace vba {
    class XHelperInterface;
} }

typedef InheritedHelperInterfaceWeakImpl< ov::msforms::XTextFrame > VbaTextFrame_BASE;

class VBAHELPER_DLLPUBLIC VbaTextFrame : public VbaTextFrame_BASE
{
protected:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
protected:
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
    void setAsMSObehavior();
    sal_Int32 getMargin( const OUString& sMarginType );
    void setMargin( const OUString& sMarginType, float fMargin );
public:
    VbaTextFrame( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext , css::uno::Reference< css::drawing::XShape > const & xShape);
    virtual ~VbaTextFrame() override {}
    // Attributes
    virtual sal_Bool SAL_CALL getAutoSize() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setAutoSize( sal_Bool _autosize ) throw (css::uno::RuntimeException, std::exception) override;
    virtual float SAL_CALL getMarginBottom() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMarginBottom( float _marginbottom ) throw (css::uno::RuntimeException, std::exception) override;
    virtual float SAL_CALL getMarginTop() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMarginTop( float _margintop ) throw (css::uno::RuntimeException, std::exception) override;
    virtual float SAL_CALL getMarginLeft() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMarginLeft( float _marginleft ) throw (css::uno::RuntimeException, std::exception) override;
    virtual float SAL_CALL getMarginRight() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMarginRight( float _marginright ) throw (css::uno::RuntimeException, std::exception) override;

    // Methods
    virtual css::uno::Any SAL_CALL Characters(  ) throw (css::uno::RuntimeException, std::exception) override;

};

#endif//SC_ INCLUDED_VBAHELPER_VBATEXTFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
