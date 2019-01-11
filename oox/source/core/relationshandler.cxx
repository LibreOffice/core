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

#include <sal/config.h>

#include <string_view>

#include <oox/core/relationshandler.hxx>

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

namespace oox {
namespace core {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace {

/*  Build path to relations file from passed fragment path, e.g.:
        'path/path/file.xml'    -> 'path/path/_rels/file.xml.rels'
        'file.xml'              -> '_rels/file.xml.rels'
        ''                      -> '_rels/.rels'
 */
OUString lclGetRelationsPath( const OUString& rFragmentPath )
{
    sal_Int32 nPathLen = ::std::max< sal_Int32 >( rFragmentPath.lastIndexOf( '/' ) + 1, 0 );
    return
        OUStringBuffer( rFragmentPath.copy( 0, nPathLen ) ).    // file path including slash
        append( "_rels/" ).                                // additional '_rels/' path
        append( std::u16string_view(rFragmentPath).substr(nPathLen) ).               // file name after path
        append( ".rels" ).                                 // '.rels' suffix
        makeStringAndClear();
}

} // namespace

RelationsFragment::RelationsFragment( XmlFilterBase& rFilter, const RelationsRef& xRelations ) :
    FragmentHandler( rFilter, lclGetRelationsPath( xRelations->getFragmentPath() ), xRelations ),
    mxRelations( xRelations )
{
}

Reference< XFastContextHandler > RelationsFragment::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs )
{
    Reference< XFastContextHandler > xRet;
    AttributeList aAttribs( rxAttribs );
    switch( nElement )
    {
        case PR_TOKEN( Relationship ):
        {
            Relation aRelation;
            aRelation.maId     = aAttribs.getString( XML_Id, OUString() );
            aRelation.maType   = aAttribs.getString( XML_Type, OUString() );
            aRelation.maTarget = aAttribs.getString( XML_Target, OUString() );
            if( !aRelation.maId.isEmpty() && !aRelation.maType.isEmpty() && !aRelation.maTarget.isEmpty() )
            {
                sal_Int32 nTargetMode = aAttribs.getToken( XML_TargetMode, XML_Internal );
                SAL_WARN_IF( (nTargetMode != XML_Internal) && (nTargetMode != XML_External), "oox",
                    "RelationsFragment::createFastChildContext - unexpected target mode, assuming external" );
                aRelation.mbExternal = nTargetMode != XML_Internal;

                SAL_WARN_IF( mxRelations->count( aRelation.maId ) != 0, "oox",
                    "RelationsFragment::createFastChildContext - relation identifier exists already" );
                mxRelations->emplace( aRelation.maId, aRelation );
            }
        }
        break;
        case PR_TOKEN( Relationships ):
            xRet = getFastContextHandler();
        break;
    }
    return xRet;
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
