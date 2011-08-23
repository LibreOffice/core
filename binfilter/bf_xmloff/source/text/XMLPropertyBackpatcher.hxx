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


#ifndef _XMLOFF_XMLPROPERTYBACKPATCHER_HXX
#define _XMLOFF_XMLPROPERTYBACKPATCHER_HXX

#ifndef __SGI_STL_MAP
#include <map>
#endif

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

namespace rtl { class OUString; }
namespace com { namespace sun { namespace star { 
    namespace beans { class XPropertySet; }
    namespace uno { template<class A> class Reference; }
} } }
namespace binfilter {


/** This class maintains an OUString->sal_Int16 mapping for cases in
 * which an XPropertySet needs to be filled with values that are not
 * yet known.
 *
 * A good example for appropriate use are footnotes and references to
 * footnoes. Internally, the StarOffice API numbers footnotes, and
 * references to footnotes refer to that internal numbering. In the
 * XML file format, these numbers are replaced with name strings. Now
 * if during import of a document a reference to a footnote is
 * encountered, two things can happen: 1) The footnote already
 * appeared in the document. In this case the name is already known
 * and the proper ID can be requested from the footnote. 2) The
 * footnote will appear later in the document. In this case the ID is
 * not yet known, and the reference-ID property of the reference
 * cannot be determined. Hence, the reference has to be stored and the
 * ID needs to bet set later, when the footnote is eventually found in
 * the document.
 *
 * This class simplifies this process: If the footnote is found,
 * ResolveId with the XML name and the ID is called. When a reference
 * is encountered, SetProperty gets called with the reference's
 * XPropertySet and the XML name. All remaining tasks are handled by
 * the class.
 */
template <class A>
class XMLPropertyBackpatcher
{

    /// name of property that gets set or backpatched
     ::rtl::OUString sPropertyName;	

    /// should a default value be set for unresolved properties
    sal_Bool bDefaultHandling;

    /// should the sPreservePropertyName be preserved
    sal_Bool bPreserveProperty;

    /// name of the property to preserve
    ::rtl::OUString sPreservePropertyName;

    /// default value for unresolved properties (if bDefaultHandling)
    A aDefault;

    /// backpatch list type
    typedef ::std::vector< 
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet> > BackpatchListType;

    /* use void* instead of BackpatchListType to avoid linker problems
       with long typenames. The real typename (commented out) contains
       >1200 chars. */

    /// backpatch list for unresolved IDs
    //::std::map<const ::rtl::OUString, BackpatchListType*> aBackpatchListMap;
    ::std::map<const ::rtl::OUString, void*, ::comphelper::UStringLess> aBackpatchListMap;

    /// mapping of names -> IDs
    ::std::map<const ::rtl::OUString, A, ::comphelper::UStringLess> aIDMap;

public:
    
    XMLPropertyBackpatcher(
        const ::rtl::OUString& sPropertyName);

    ~XMLPropertyBackpatcher();

    /// resolve a known ID.
    /// Call this as soon as the value for a particular name is known.
    void ResolveId(
        const ::rtl::OUString& sName,
        A aValue);

    /// Set property with the proper value for this name. If the value
    /// is not yet known, store the XPropertySet in the backpatch list.
    /// Use this whenever the value should be set, even if it is not yet known.
    /// const version
    void SetProperty(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet> & xPropSet,
        const ::rtl::OUString& sName);

    /// non-const version of SetProperty
    void SetProperty(
        ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet> & xPropSet,
        const ::rtl::OUString& sName);

    /// set default (if bDefaultHandling) for unresolved names
    /// called by destructor
    void SetDefault();

};

}//end of namespace binfilter
#endif
