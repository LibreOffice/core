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

#include "timeanimvaluecontext.hxx"

#include "animvariantcontext.hxx"

#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;

namespace oox::ppt {

    TimeAnimValueListContext::TimeAnimValueListContext( FragmentHandler2 const & rParent,
                TimeAnimationValueList & aTavList )
        : FragmentHandler2( rParent )
            , maTavList( aTavList )
            , mbInValue( false )
    {
    }

    TimeAnimValueListContext::~TimeAnimValueListContext( )
    {
    }

    void TimeAnimValueListContext::onEndElement()
    {
        if( isCurrentElement( PPT_TOKEN( tav ) ) )
        {
            mbInValue = false;
        }
    }

    ::oox::core::ContextHandlerRef TimeAnimValueListContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
    {

        switch ( aElementToken )
        {
        case PPT_TOKEN( tav ):
        {
            mbInValue = true;
            TimeAnimationValue val;
            val.msFormula = rAttribs.getStringDefaulted( XML_fmla);
            val.msTime =  rAttribs.getStringDefaulted( XML_tm);
            maTavList.push_back( val );
            return this;
        }
        case PPT_TOKEN( val ):
            if( mbInValue )
            {
                // CT_TLAnimVariant
                return new AnimVariantContext( *this, aElementToken, maTavList.back().maValue );
            }
            break;
        default:
            break;
        }

        return this;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
