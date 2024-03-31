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

#include <drawingml/textliststyle.hxx>
#include <drawingml/textliststylecontext.hxx>
#include <oox/ppt/slidemastertextstylescontext.hxx>
#include <oox/token/namespaces.hxx>
#include <utility>

using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;

namespace oox::ppt {

SlideMasterTextStylesContext::SlideMasterTextStylesContext( FragmentHandler2 const & rParent, SlidePersistPtr pSlidePersistPtr )
: FragmentHandler2( rParent )
, mpSlidePersistPtr(std::move( pSlidePersistPtr ))
{
}

SlideMasterTextStylesContext::~SlideMasterTextStylesContext()
{
}

::oox::core::ContextHandlerRef SlideMasterTextStylesContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& /*rAttribs*/ )
{
    oox::drawingml::TextListStylePtr aTextListStylePtr;
    switch( aElementToken )
    {
        case PPT_TOKEN( titleStyle ):
        {
            aTextListStylePtr = mpSlidePersistPtr->getTitleTextStyle();
            break;
        }
        case PPT_TOKEN( bodyStyle ):
        {
            aTextListStylePtr = mpSlidePersistPtr->getBodyTextStyle();
            break;
        }
        case PPT_TOKEN( notesStyle ):
        {
            aTextListStylePtr = mpSlidePersistPtr->getNotesTextStyle();
            break;
        }
        case PPT_TOKEN( otherStyle ):
        {
            aTextListStylePtr = mpSlidePersistPtr->getOtherTextStyle();
            break;
        }
    }
    if ( aTextListStylePtr )            // sj: the master list style is the last instance of from where properties
    {                                   // are obtained. i got some documents without having the textsize set at
        for ( int i = 0; i < 9; i++ )   // any point, the master reference application is using 18pt then
            aTextListStylePtr->getListStyle()[ i ].getTextCharacterProperties().moHeight = 1800;
        return new oox::drawingml::TextListStyleContext( *this, *aTextListStylePtr );
    }

    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
