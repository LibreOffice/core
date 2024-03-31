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

#include <rtl/ustring.hxx>

#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include "texttabstoplistcontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::xml::sax;

namespace oox::drawingml {

        TextTabStopListContext::TextTabStopListContext( ContextHandler2Helper const & rParent, std::vector< TabStop >  & aTabList )
            : ContextHandler2( rParent )
            , maTabList( aTabList )
        {
        }

        TextTabStopListContext::~TextTabStopListContext()
        {
        }

        ContextHandlerRef TextTabStopListContext::onCreateContext( ::sal_Int32 aElement, const AttributeList& rAttribs )
        {
            switch( aElement )
            {
            case A_TOKEN( tab ):
            {
                OUString sValue;
                TabStop aTabStop;
                sValue = rAttribs.getStringDefaulted( XML_pos );
                if( !sValue.isEmpty() )
                {
                    aTabStop.Position = GetCoordinate( sValue );
                }
                sal_Int32 aToken = rAttribs.getToken( XML_algn, 0 );
                if( aToken != 0 )
                {
                    aTabStop.Alignment = GetTabAlign( aToken );
                }
                maTabList.push_back(aTabStop);
                break;
            }
            default:
                break;
            }
            return this;
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
