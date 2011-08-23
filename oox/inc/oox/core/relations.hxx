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

#ifndef OOX_CORE_RELATIONS
#define OOX_CORE_RELATIONS

#include <map>
#include <boost/shared_ptr.hpp>
#include "oox/helper/helper.hxx"

namespace oox {
namespace core {

/** Expands to an OUString containing an officeDocument relation type created
    from the passed literal(!) ASCII(!) character array. */
#define CREATE_OFFICEDOC_RELATIONSTYPE( ascii ) \
    CREATE_OUSTRING( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/" ascii )

/** Expands to an OUString containing an MS Office specific relation type
    created from the passed literal(!) ASCII(!) character array. */
#define CREATE_MSOFFICE_RELATIONSTYPE( ascii ) \
    CREATE_OUSTRING( "http://schemas.microsoft.com/office/2006/relationships/" ascii )

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

class Relations : public ::std::map< ::rtl::OUString, Relation >
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
    /** Returns the external target of the first relation with the passed type. */
    ::rtl::OUString     getExternalTargetFromFirstType( const ::rtl::OUString& rType ) const;

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

#endif // OOX_CORE_RELATIONS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
