/*************************************************************************
 *
 *  $RCSfile: generatedtypeset.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 03:08:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
