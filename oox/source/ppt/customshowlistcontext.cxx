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

#include <oox/ppt/customshowlistcontext.hxx>

#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <sax/fastattribs.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox::ppt {

namespace {

class CustomShowContext : public ::oox::core::FragmentHandler2
{
    std::vector< CustomShow >& mrCustomShowList;

public:
    CustomShowContext( ::oox::core::FragmentHandler2 const & rParent,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttribs,
            std::vector< CustomShow >& rCustomShowList );

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override;
};

}

CustomShowContext::CustomShowContext( FragmentHandler2 const & rParent,
    const Reference< XFastAttributeList >& rxAttribs,
        std::vector< CustomShow >& rCustomShowList )
: FragmentHandler2( rParent )
, mrCustomShowList( rCustomShowList )
{
    CustomShow aCustomShow;
    aCustomShow.maCustomShowName = rxAttribs->getOptionalValue( XML_name );
    aCustomShow.mnId = rxAttribs->getOptionalValue( XML_id );
    mrCustomShowList.push_back(aCustomShow);
}

::oox::core::ContextHandlerRef CustomShowContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case PPT_TOKEN( sld ) :
            mrCustomShowList.back().maSldLst.push_back(
                getRelations()
                    .getRelationFromRelId(rAttribs.getStringDefaulted(R_TOKEN(id)))
                    ->maTarget);
            return this;
        default:
        break;
    }

    return this;
}

CustomShowListContext::CustomShowListContext( FragmentHandler2 const & rParent,
    std::vector< CustomShow >& rCustomShowList )
: FragmentHandler2( rParent )
, mrCustomShowList( rCustomShowList )
{
}

CustomShowListContext::~CustomShowListContext( )
{
}

::oox::core::ContextHandlerRef CustomShowListContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case PPT_TOKEN( custShow ) :
        {
            return new CustomShowContext( *this, rAttribs.getFastAttributeList(), mrCustomShowList );
        }
        default:
        break;
    }

    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
