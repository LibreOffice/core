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

#include "animvariantcontext.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include <osl/diagnose.h>

#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>

#include "oox/helper/attributelist.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "pptfilterhelpers.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

    AnimVariantContext::AnimVariantContext( FragmentHandler2& rParent, sal_Int32 aElement, Any & aValue )
        : FragmentHandler2( rParent )
            , mnElement( aElement )
            , maValue( aValue )
    {
    }

    AnimVariantContext::~AnimVariantContext( ) throw( )
    {
    }

    void AnimVariantContext::onEndElement()
    {
        if( isCurrentElement( mnElement ) && maColor.isUsed() )
        {
            maValue = makeAny( maColor.getColor( getFilter().getGraphicHelper() ) );
        }
    }


    ContextHandlerRef AnimVariantContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
    {
        switch( aElementToken )
        {
        case PPT_TOKEN( boolVal ):
        {
            bool val = rAttribs.getBool( XML_val, false );
            maValue = makeAny( val );
            return this;
        }
        case PPT_TOKEN( clrVal ):
            return new ::oox::drawingml::ColorContext( *this, maColor );
            // we'll defer setting the Any until the end.
        case PPT_TOKEN( fltVal ):
        {
            double val = rAttribs.getDouble( XML_val, 0.0 );
            maValue = makeAny( val );
            return this;
        }
        case PPT_TOKEN( intVal ):
        {
            sal_Int32 val = rAttribs.getInteger( XML_val, 0 );
            maValue = makeAny( val );
            return this;
        }
        case PPT_TOKEN( strVal ):
        {
            OUString val = rAttribs.getString( XML_val, OUString() );
            convertMeasure( val ); // ignore success or failure if it fails, use as is
            maValue = makeAny( val );
            return this;
        }
        default:
            break;
        }

        return this;
    }



} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
