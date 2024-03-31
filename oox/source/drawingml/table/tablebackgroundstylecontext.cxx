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


#include <drawingml/table/tablebackgroundstylecontext.hxx>
#include <drawingml/misccontexts.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace oox::drawingml::table {

TableBackgroundStyleContext::TableBackgroundStyleContext( ContextHandler2Helper const & rParent, TableStyle& rTableStyle )
: ContextHandler2( rParent )
, mrTableStyle( rTableStyle )
{
}

TableBackgroundStyleContext::~TableBackgroundStyleContext()
{
}

ContextHandlerRef
TableBackgroundStyleContext::onCreateContext( ::sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        // EG_ThemeableFillStyle (choice)
        case A_TOKEN( fill ):       // CT_FillProperties
            {
                std::shared_ptr< FillProperties >& rxFillProperties = mrTableStyle.getBackgroundFillProperties();
                rxFillProperties = std::make_shared<FillProperties>();
                return new FillPropertiesContext( *this, *rxFillProperties );
            }
        case A_TOKEN( fillRef ):    // CT_StyleMatrixReference
            {
                ShapeStyleRef& rStyleRef = mrTableStyle.getBackgroundFillStyleRef();
                rStyleRef.mnThemedIdx = rAttribs.getInteger( XML_idx, 0 );
                return new ColorContext( *this, rStyleRef.maPhClr );
            }
        // EG_ThemeableEffectStyle (choice)
        case A_TOKEN( effect ):     // CT_EffectProperties
            break;
        case A_TOKEN( effectRef ):  // CT_StyleMatrixReference
            break;
    }

    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
