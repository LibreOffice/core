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

#include "headerfootercontext.hxx"
#include "oox/helper/attributelist.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

    HeaderFooterContext::HeaderFooterContext( FragmentHandler2& rParent,
        const AttributeList& rAttribs, HeaderFooter& rHeaderFooter )
        : FragmentHandler2( rParent )
    {
        if ( rAttribs.hasAttribute( XML_sldNum ) )
        {
            rHeaderFooter.mbSlideNumber = rAttribs.getBool( XML_sldNum, sal_True );
        }
        if ( rAttribs.hasAttribute( XML_hdr ) )
        {
            rHeaderFooter.mbHeader = rAttribs.getBool( XML_hdr, sal_True );
        }
        if ( rAttribs.hasAttribute( XML_ftr ) )
        {
            rHeaderFooter.mbFooter = rAttribs.getBool( XML_ftr, sal_True );
        }
        if ( rAttribs.hasAttribute( XML_dt ) )
        {
            rHeaderFooter.mbDateTime = rAttribs.getBool( XML_dt, sal_True );
        }
    }

    HeaderFooterContext::~HeaderFooterContext( )
    {
    }

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
