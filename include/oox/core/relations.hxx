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

#ifndef INCLUDED_OOX_CORE_RELATIONS_HXX
#define INCLUDED_OOX_CORE_RELATIONS_HXX

#include <cstddef>
#include <map>
#include <memory>
#include <string_view>

#include <oox/dllapi.h>
#include <rtl/ustring.hxx>

namespace oox::core {


/** Expands to an OUString containing an 'officeDocument' transitional relation type created
    from the passed literal(!) ASCII(!) character array. */
#define CREATE_OFFICEDOC_RELATION_TYPE( ascii ) \
    ( u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/" ascii ""_ustr )

/** Expands to an OUString containing an 'officeDocument' strict relation type created
    from the passed literal(!) ASCII(!) character array. */
#define CREATE_OFFICEDOC_RELATION_TYPE_STRICT( ascii ) \
    ( u"http://purl.oclc.org/ooxml/officeDocument/relationships/" ascii ""_ustr )

/** Expands to an OUString containing an MS Office specific relation type
    created from the passed literal(!) ASCII(!) character array. */
#define CREATE_MSOFFICE_RELATION_TYPE( ascii ) \
    ( u"http://schemas.microsoft.com/office/2006/relationships/" ascii )

#define CREATE_XL_CONTENT_TYPE( ascii ) \
    ( "application/vnd.openxmlformats-officedocument.spreadsheetml." ascii "+xml" )

struct Relation
{
    OUString     maId;
    OUString     maType;
    OUString     maTarget;
    bool                mbExternal;

    Relation() : mbExternal( false ) {}
};


class Relations;
typedef std::shared_ptr< Relations > RelationsRef;

class OOX_DLLPUBLIC Relations
{
public:
    explicit            Relations( OUString aFragmentPath );

    size_t size() const { return maMap.size(); }
    size_t count( const OUString& rId ) const { return maMap.count( rId ); }
    ::std::map< OUString, Relation >::const_iterator begin() const
    {
        return maMap.begin();
    }
    ::std::map< OUString, Relation >::const_iterator end() const
    {
        return maMap.end();
    }
    template<class... Args>
    void emplace(Args&&... args)
    {
        maMap.emplace(std::forward<Args>(args)...);
    }

    /** Returns the path of the fragment this relations collection is related to. */
    const OUString& getFragmentPath() const { return maFragmentPath; }

    /** Returns the relation with the passed relation identifier. */
    const Relation*     getRelationFromRelId( const OUString& rId ) const;
    /** Returns the first relation with the passed type. */
    const Relation*     getRelationFromFirstType( std::u16string_view rType ) const;
    /** Finds all relations associated with the passed type. */
    RelationsRef        getRelationsFromTypeFromOfficeDoc( std::u16string_view rType ) const;

    /** Returns the external target of the relation with the passed relation identifier. */
    OUString     getExternalTargetFromRelId( const OUString& rRelId ) const;
    /** Returns the internal target of the relation with the passed relation identifier. */
    OUString     getInternalTargetFromRelId( const OUString& rRelId ) const;

    /** Returns the full fragment path for the target of the passed relation. */
    OUString     getFragmentPathFromRelation( const Relation& rRelation ) const;
    /** Returns the full fragment path for the passed relation identifier. */
    OUString     getFragmentPathFromRelId( const OUString& rRelId ) const;
    /** Returns the full fragment path for the first relation of the passed type. */
    OUString     getFragmentPathFromFirstType( std::u16string_view rType ) const;
    OUString     getFragmentPathFromFirstTypeFromOfficeDoc( std::u16string_view rType ) const;

private:
    ::std::map< OUString, Relation > maMap;
    OUString     maFragmentPath;
};


} // namespace oox::core

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
