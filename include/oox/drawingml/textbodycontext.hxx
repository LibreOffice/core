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

#ifndef OOX_DRAWINGML_TEXTBODYCONTEXT_HXX
#define OOX_DRAWINGML_TEXTBODYCONTEXT_HXX

#include <com/sun/star/text/XText.hpp>

#include "oox/drawingml/textbody.hxx"
#include "oox/drawingml/textrun.hxx"
#include "oox/core/contexthandler.hxx"

namespace oox { namespace drawingml {

class TextBodyContext : public ::oox::core::ContextHandler
{
public:
    TextBodyContext( ::oox::core::ContextHandler& rParent, TextBody& rTextBody );

    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    TextBody&           mrTextBody;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > mxText;
};

// CT_RegularTextRun
class RegularTextRunContext : public ::oox::core::ContextHandler
{
public:
    RegularTextRunContext( ::oox::core::ContextHandler& rParent, TextRunPtr pRunPtr );

    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const OUString& aChars ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    TextRunPtr          mpRunPtr;
    bool                mbIsInText;
};

} }

#endif  //  OOX_DRAWINGML_TEXTBODYCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
