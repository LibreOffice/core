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

#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "headerfootercontext.hxx"
#include <oox/ppt/layoutfragmenthandler.hxx>
#include <oox/drawingml/shapegroupcontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <utility>

using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;

namespace oox { namespace ppt {

// CT_SlideLayout

LayoutFragmentHandler::LayoutFragmentHandler(XmlFilterBase& rFilter, const OUString& rFragmentPath,
                                             const SlidePersistPtr& pMasterPersistPtr)
    : SlideFragmentHandler(rFilter, rFragmentPath, std::move(pMasterPersistPtr), Layout)
{
}

LayoutFragmentHandler::~LayoutFragmentHandler()
{

}

ContextHandlerRef LayoutFragmentHandler::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case PPT_TOKEN( sldLayout ):        // CT_SlideLayout
        {
            mpSlidePersistPtr->setLayoutValueToken( rAttribs.getToken( XML_type, 0 ) ); // CT_SlideLayoutType

            OptValue< bool > aShowMasterShapes = rAttribs.getBool( XML_showMasterSp );
            if( aShowMasterShapes.has() && !aShowMasterShapes.get() ) {
                mpSlidePersistPtr->hideShapesAsMasterShapes();
            }
            break;
        }
        case PPT_TOKEN( hf ):               // CT_HeaderFooter
            return new HeaderFooterContext( *this, rAttribs, mpSlidePersistPtr->getHeaderFooter() );
        default:
            return SlideFragmentHandler::onCreateContext( aElementToken, rAttribs );
    }
    return this;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
