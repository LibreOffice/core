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

#include <osl/diagnose.h>

#include "oox/drawingml/table/tablebackgroundstylecontext.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/helper/attributelist.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::rtl::OUString;

namespace oox { namespace drawingml { namespace table {

TableBackgroundStyleContext::TableBackgroundStyleContext( ContextHandler& rParent, TableStyle& rTableStyle )
: ContextHandler( rParent )
, mrTableStyle( rTableStyle )
{
}

TableBackgroundStyleContext::~TableBackgroundStyleContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
TableBackgroundStyleContext::createFastChildContext( ::sal_Int32 aElementToken, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs )
    throw ( xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< xml::sax::XFastContextHandler > xRet;

    AttributeList aAttribs( xAttribs );
    switch( aElementToken )
    {
        // EG_ThemeableFillStyle (choice)
        case A_TOKEN( fill ):       // CT_FillProperties
            {
                boost::shared_ptr< FillProperties >& rxFillProperties = mrTableStyle.getBackgroundFillProperties();
                rxFillProperties.reset( new FillProperties );
                xRet.set( new FillPropertiesContext( *this, *rxFillProperties ) );
            }
            break;
        case A_TOKEN( fillRef ):    // CT_StyleMatrixReference
            {
                ShapeStyleRef& rStyleRef = mrTableStyle.getBackgroundFillStyleRef();
                rStyleRef.mnThemedIdx = aAttribs.getInteger( XML_idx, 0 );
                xRet.set( new ColorContext( *this, rStyleRef.maPhClr ) );
            }
            break;

        // EG_ThemeableEffectStyle (choice)
        case A_TOKEN( effect ):     // CT_EffectProperties
            break;
        case A_TOKEN( effectRef ):  // CT_StyleMatrixReference
            break;
    }
    if( !xRet.is() )
    {
        uno::Reference<XFastContextHandler> xTmp(this);
        xRet.set( xTmp );
    }
    return xRet;
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
