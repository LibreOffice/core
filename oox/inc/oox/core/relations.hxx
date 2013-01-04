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

#ifndef OOX_CORE_RELATIONS_HXX
#define OOX_CORE_RELATIONS_HXX

#include <map>
#include <boost/shared_ptr.hpp>
#include "oox/helper/helper.hxx"
#include "oox/dllapi.h"

namespace oox {
namespace core {

// ============================================================================

/** Expands to an OUString containing an 'officeDocument' relation type created
    from the passed literal(!) ASCII(!) character array. */
#define CREATE_OFFICEDOC_RELATION_TYPE( ascii ) \
    ( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/" ascii )

/** Expands to an OUString containing a 'package' relation type created from
    the passed literal(!) ASCII(!) character array. */
#define CREATE_PACKAGE_RELATION_TYPE( ascii ) \
    ( "http://schemas.openxmlformats.org/package/2006/relationships/" ascii )

/** Expands to an OUString containing an MS Office specific relation type
    created from the passed literal(!) ASCII(!) character array. */
#define CREATE_MSOFFICE_RELATION_TYPE( ascii ) \
    ( "http://schemas.microsoft.com/office/2006/relationships/" ascii )

// ============================================================================

struct Relation
{
    ::rtl::OUString     maId;
    ::rtl::OUString     maType;
    ::rtl::OUString     maTarget;
    bool                mbExternal;

    inline explicit     Relation() : mbExternal( false ) {}
};

// ============================================================================

class Relations;
typedef ::boost::shared_ptr< Relations > RelationsRef;

class OOX_DLLPUBLIC Relations : public ::std::map< ::rtl::OUString, Relation >
{
public:
    explicit            Relations( const ::rtl::OUString& rFragmentPath );

    /** Returns the path of the fragment this relations collection is related to. */
    inline const ::rtl::OUString& getFragmentPath() const { return maFragmentPath; }

    /** Returns the relation with the passed relation identifier. */
    const Relation*     getRelationFromRelId( const ::rtl::OUString& rId ) const;
    /** Returns the first relation with the passed type. */
    const Relation*     getRelationFromFirstType( const ::rtl::OUString& rType ) const;
    /** Finds all relations associated with the passed type. */
    RelationsRef        getRelationsFromType( const ::rtl::OUString& rType ) const;

    /** Returns the external target of the relation with the passed relation identifier. */
    ::rtl::OUString     getExternalTargetFromRelId( const ::rtl::OUString& rRelId ) const;
    /** Returns the internal target of the relation with the passed relation identifier. */
    ::rtl::OUString     getInternalTargetFromRelId( const ::rtl::OUString& rRelId ) const;

    /** Returns the full fragment path for the target of the passed relation. */
    ::rtl::OUString     getFragmentPathFromRelation( const Relation& rRelation ) const;
    /** Returns the full fragment path for the passed relation identifier. */
    ::rtl::OUString     getFragmentPathFromRelId( const ::rtl::OUString& rRelId ) const;
    /** Returns the full fragment path for the first relation of the passed type. */
    ::rtl::OUString     getFragmentPathFromFirstType( const ::rtl::OUString& rType ) const;

private:
    ::rtl::OUString     maFragmentPath;
};

// ============================================================================

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
