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

#include "headerfootercontext.hxx"
#include <oox/helper/attributelist.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;

namespace oox::ppt {

    HeaderFooterContext::HeaderFooterContext( FragmentHandler2 const & rParent,
        const AttributeList& rAttribs, HeaderFooter& rHeaderFooter )
        : FragmentHandler2( rParent )
    {
        if ( rAttribs.hasAttribute( XML_sldNum ) )
        {
            rHeaderFooter.mbSlideNumber = rAttribs.getBool( XML_sldNum, true );
        }
        if ( rAttribs.hasAttribute( XML_hdr ) )
        {
            rHeaderFooter.mbHeader = rAttribs.getBool( XML_hdr, true );
        }
        if ( rAttribs.hasAttribute( XML_ftr ) )
        {
            rHeaderFooter.mbFooter = rAttribs.getBool( XML_ftr, true );
        }
        if ( rAttribs.hasAttribute( XML_dt ) )
        {
            rHeaderFooter.mbDateTime = rAttribs.getBool( XML_dt, true );
        }
    }

    HeaderFooterContext::~HeaderFooterContext( )
    {
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
