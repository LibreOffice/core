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

#include "oox/core/contexthandler.hxx"

#include "oox/core/fragmenthandler.hxx"

namespace oox {
namespace core {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

ContextHandler::ContextHandler( const ContextHandler& rParent ) :
    ContextHandler_BASE(),
    mxBaseData( rParent.mxBaseData )
{
}

ContextHandler::ContextHandler( const FragmentBaseDataRef& rxBaseData ) :
    mxBaseData( rxBaseData )
{
}

ContextHandler::~ContextHandler()
{
}

XmlFilterBase& ContextHandler::getFilter() const
{
    return mxBaseData->mrFilter;
}

const Relations& ContextHandler::getRelations() const
{
    return *mxBaseData->mxRelations;
}

const OUString& ContextHandler::getFragmentPath() const
{
    return mxBaseData->maFragmentPath;
}

OUString ContextHandler::getFragmentPathFromRelation( const Relation& rRelation ) const
{
    return mxBaseData->mxRelations->getFragmentPathFromRelation( rRelation );
}

OUString ContextHandler::getFragmentPathFromRelId( const OUString& rRelId ) const
{
    return mxBaseData->mxRelations->getFragmentPathFromRelId( rRelId );
}

OUString ContextHandler::getFragmentPathFromFirstType( const OUString& rType ) const
{
    return mxBaseData->mxRelations->getFragmentPathFromFirstType( rType );
}

OUString ContextHandler::getFragmentPathFromFirstTypeFromOfficeDoc( const OUString& rType ) const
{
    return mxBaseData->mxRelations->getFragmentPathFromFirstTypeFromOfficeDoc( rType );
}

void ContextHandler::implSetLocator( const Reference< XLocator >& rxLocator )
{
    mxBaseData->mxLocator = rxLocator;
}

// com.sun.star.xml.sax.XFastContextHandler interface -------------------------

void ContextHandler::startFastElement( sal_Int32, const Reference< XFastAttributeList >& ) throw( SAXException, RuntimeException, std::exception )
{
}

void ContextHandler::startUnknownElement( const OUString&, const OUString&, const Reference< XFastAttributeList >& ) throw( SAXException, RuntimeException, std::exception )
{
}

void ContextHandler::endFastElement( sal_Int32 ) throw( SAXException, RuntimeException, std::exception )
{
}

void ContextHandler::endUnknownElement( const OUString&, const OUString& ) throw( SAXException, RuntimeException, std::exception )
{
}

Reference< XFastContextHandler > ContextHandler::createFastChildContext( sal_Int32, const Reference< XFastAttributeList >& ) throw( SAXException, RuntimeException, std::exception )
{
    return nullptr;
}

Reference< XFastContextHandler > ContextHandler::createUnknownChildContext( const OUString&, const OUString&, const Reference< XFastAttributeList >& ) throw( SAXException, RuntimeException, std::exception )
{
    return nullptr;
}

void ContextHandler::characters( const OUString& ) throw( SAXException, RuntimeException, std::exception )
{
}

// record context interface ---------------------------------------------------

ContextHandlerRef ContextHandler::createRecordContext( sal_Int32, SequenceInputStream& )
{
    return nullptr;
}

void ContextHandler::startRecord( sal_Int32, SequenceInputStream& )
{
}

void ContextHandler::endRecord( sal_Int32 )
{
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
