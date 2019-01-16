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


#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLPROPERTYBACKPATCHER_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLPROPERTYBACKPATCHER_HXX

#include <map>
#include <memory>
#include <vector>

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace uno { template<class A> class Reference; }
} } }


/** This class maintains an OUString->sal_Int16 mapping for cases in
 * which an XPropertySet needs to be filled with values that are not
 * yet known.
 *
 * A good example for appropriate use are footnotes and references to
 * footnotes. Internally, the LibreOffice API numbers footnotes, and
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
     OUString const sPropertyName;

    /// backpatch list type
    typedef ::std::vector<
                css::uno::Reference<css::beans::XPropertySet> > BackpatchListType;

    /// backpatch list for unresolved IDs
    ::std::map<const OUString, std::unique_ptr<BackpatchListType>> aBackpatchListMap;

    /// mapping of names -> IDs
    ::std::map<const OUString, A> aIDMap;

public:

    explicit XMLPropertyBackpatcher(
        const OUString& sPropertyName);

    ~XMLPropertyBackpatcher();

    /// resolve a known ID.
    /// Call this as soon as the value for a particular name is known.
    void ResolveId(
        const OUString& sName,
        A aValue);

    /// Set property with the proper value for this name. If the value
    /// is not yet known, store the XPropertySet in the backpatch list.
    /// Use this whenever the value should be set, even if it is not yet known.
    void SetProperty(
        const css::uno::Reference<css::beans::XPropertySet> & xPropSet,
        const OUString& sName);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
