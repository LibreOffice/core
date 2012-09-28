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

#include "buildlistcontext.hxx"
#include <rtl/ustring.hxx>
#include "oox/helper/attributelist.hxx"

namespace oox { namespace ppt {

    BuildListContext::BuildListContext( FragmentHandler2& rParent )
        : FragmentHandler2( rParent )
        , mbInBldGraphic( false )
        ,   mbBuildAsOne( false )
    {
    }

    BuildListContext::~BuildListContext( )
    {
    }

    void BuildListContext::onEndElement()
    {
        switch( getCurrentElement() )
        {
        case PPT_TOKEN( bldGraphic ):
            mbInBldGraphic = false;
            break;
        default:
            break;
        }
    }

    ::oox::core::ContextHandlerRef BuildListContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
    {
        switch( aElementToken )
        {
        case PPT_TOKEN( bldAsOne ):
            if( mbInBldGraphic )
            {
                mbBuildAsOne = true;
            }
            return this;
        case PPT_TOKEN( bldSub ):
            if( mbInBldGraphic )
            {
            }
            return this;
        case PPT_TOKEN( bldGraphic ):
        {
            mbInBldGraphic = true;
            OUString sShapeId = rAttribs.getString( XML_spid, OUString() );
// TODO
//      bool uiExpand = rAttribs.getBool( XML_uiExpand, true );
                /* this is unsigned */
//      sal_uInt32 nGroupId =  rAttribs.getUnsignedInteger( XML_grpId, 0 );
        return this;
        }
        case A_TOKEN( bldDgm ):
        case A_TOKEN( bldOleChart ):
        case A_TOKEN( bldP ):
            return this;
        default:
            break;
        }

        return this;
    }


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
