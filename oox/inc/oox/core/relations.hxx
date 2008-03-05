/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: relations.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:36:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OOX_CORE_RELATIONS
#define OOX_CORE_RELATIONS

#include <map>
#include <boost/shared_ptr.hpp>
#include "oox/helper/helper.hxx"

namespace oox {
namespace core {

/** Expands to an OUString containing a relation type created from the passed
    literal(!) ASCII(!) character array.
 */
#define CREATE_OFFICEDOC_RELATIONSTYPE( ascii ) \
    CREATE_OUSTRING( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/" ascii )

// ============================================================================

struct Relation
{
    ::rtl::OUString     maId;
    ::rtl::OUString     maType;
    ::rtl::OUString     maTarget;
};

// ============================================================================

class Relations;
typedef ::boost::shared_ptr< Relations > RelationsRef;

class Relations : public ::std::map< ::rtl::OUString, Relation >
{
public:
    explicit            Relations( const ::rtl::OUString& rBasePath );

    /** Returns the base path of this relations collection. */
    inline const ::rtl::OUString& getBasePath() const { return maBasePath; }

    /** Returns the relation with the passed relation identifier. */
    const Relation*     getRelationFromRelId( const ::rtl::OUString& rId ) const;
    /** Returns the first relation with the passed type. */
    const Relation*     getRelationFromType( const ::rtl::OUString& rType ) const;
    /** Finds all relations associated with the passed type. */
    RelationsRef        getRelationsFromType( const ::rtl::OUString& rType ) const;

    /** Returns the plain target of the relation with the passed relation identifier. */
    ::rtl::OUString     getTargetFromRelId( const ::rtl::OUString& rRelId ) const;
    /** Returns the plain target of the relation with the passed type. */
    ::rtl::OUString     getTargetFromType( const ::rtl::OUString& rType ) const;

    /** Returns the full fragment path for the passed relative target. */
    ::rtl::OUString     getFragmentPathFromTarget( const ::rtl::OUString& rTarget ) const;
    /** Returns the full fragment path for the passed relation identifier. */
    ::rtl::OUString     getFragmentPathFromRelId( const ::rtl::OUString& rRelId ) const;
    /** Returns the full fragment path for the passed type. */
    ::rtl::OUString     getFragmentPathFromType( const ::rtl::OUString& rType ) const;

private:
    ::rtl::OUString     maBasePath;
};

// ============================================================================

} // namespace core
} // namespace oox

#endif // OOX_CORE_RELATIONS

