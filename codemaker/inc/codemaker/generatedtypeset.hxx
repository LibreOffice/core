/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: generatedtypeset.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:06:16 $
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

#ifndef INCLUDED_codemaker_generatedtypeset_hxx
#define INCLUDED_codemaker_generatedtypeset_hxx

#include "rtl/string.hxx"

#include <hash_set>

/// @HTML

namespace codemaker {

/**
   A simple class to track which types have already been processed by a code
   maker.

   <p>This class is not multi-thread&ndash;safe.</p>
 */
class GeneratedTypeSet {
public:
    GeneratedTypeSet() {}

    ~GeneratedTypeSet() {}

    /**
       Add a type to the set of generated types.

       <p>If the type was already present, nothing happens.</p>

       @param type a UNO type registry name
     */
    void add(rtl::OString const & type) { m_set.insert(type); }

    /**
       Checks whether a given type has already been generated.

       @param type a UNO type registry name

       @return true iff the given type has already been generated
     */
    bool contains(rtl::OString const & type) const
    { return m_set.find(type) != m_set.end(); }

private:
    GeneratedTypeSet(GeneratedTypeSet &); // not implemented
    void operator =(GeneratedTypeSet); // not implemented

    std::hash_set< rtl::OString, rtl::OStringHash > m_set;
};

}

#endif
