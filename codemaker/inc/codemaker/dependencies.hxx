/*************************************************************************
 *
 *  $RCSfile: dependencies.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 03:08:18 $
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

#ifndef INCLUDED_codemaker_dependencies_hxx
#define INCLUDED_codemaker_dependencies_hxx

#include "rtl/string.hxx"

#include <hash_map>

namespace rtl { class OUString; }
class TypeManager;

/// @HTML

namespace codemaker {

/**
   A simple class to track which other types a given type depends on.

   <p>This class is not multi-thread&ndash;safe.</p>
 */
class Dependencies {
public:
    /**
       Flags to distinguish whether or not one type depends on another type
       because the second is a direct base of the first.
     */
    enum Kind { KIND_NO_BASE, KIND_BASE };

    typedef std::hash_map< rtl::OString, Kind, rtl::OStringHash > Map;

    /**
       Constructs the dependencies for a given type.

       <p>If the given type is not successfully available at the given type
       manager, <code>isValid()</code> will return <code>false</code>.</p>

       @param manager a type manager, to obtain information about the given type

       @param type the UNO type registry name of an enum type, plain struct
       type, polymorphic struct type template, exception type, interface type,
       typedef, module, constant group, service, or singleton
     */
    Dependencies(TypeManager const & manager, rtl::OString const & type);

    ~Dependencies();

    /**
       Add a special dependency (which is not obvious from the type's data
       available at the type manager).

       @param type a UNO type registry name
     */
    void add(rtl::OString const & type) { insert(type, false); }

    bool isValid() const { return m_valid; }

    Map const & getMap() const { return m_map; }

    bool hasVoidDependency() const { return m_voidDependency; }

    bool hasBooleanDependency() const { return m_booleanDependency; }

    bool hasByteDependency() const { return m_byteDependency; }

    bool hasShortDependency() const { return m_shortDependency; }

    bool hasUnsignedShortDependency() const
    { return m_unsignedShortDependency; }

    bool hasLongDependency() const { return m_longDependency; }

    bool hasUnsignedLongDependency() const { return m_unsignedLongDependency; }

    bool hasHyperDependency() const { return m_hyperDependency; }

    bool hasUnsignedHyperDependency() const
    { return m_unsignedHyperDependency; }

    bool hasFloatDependency() const { return m_floatDependency; }

    bool hasDoubleDependency() const { return m_doubleDependency; }

    bool hasCharDependency() const { return m_charDependency; }

    bool hasStringDependency() const { return m_stringDependency; }

    bool hasTypeDependency() const { return m_typeDependency; }

    bool hasAnyDependency() const { return m_anyDependency; }

    bool hasSequenceDependency() const { return m_sequenceDependency; }

private:
    Dependencies(Dependencies &); // not implemented
    void operator =(Dependencies); // not implemented

    void insert(rtl::OUString const & type, bool base);

    void insert(rtl::OString const & type, bool base);

    Map m_map;
    bool m_valid;
    bool m_voidDependency;
    bool m_booleanDependency;
    bool m_byteDependency;
    bool m_shortDependency;
    bool m_unsignedShortDependency;
    bool m_longDependency;
    bool m_unsignedLongDependency;
    bool m_hyperDependency;
    bool m_unsignedHyperDependency;
    bool m_floatDependency;
    bool m_doubleDependency;
    bool m_charDependency;
    bool m_stringDependency;
    bool m_typeDependency;
    bool m_anyDependency;
    bool m_sequenceDependency;
};

}

#endif
