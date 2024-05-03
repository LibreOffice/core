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
#pragma once

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <memory>
#include <span>

namespace com::sun::star
{
namespace beans
{
class XMultiPropertySet;
}
namespace beans
{
class XPropertySet;
}
namespace beans
{
class XPropertySetInfo;
}
}

/**
 * The MultiPropertySetHelper performs the following functions:
 *
 * Given a list of property names (as char** or OUString*), it can
 * query an XMultiPropertySet (or XPropertySet) which of these properties
 * it supports (method hasProperties(...)). The properties *MUST* be
 * sorted alphabetically.
 *
 * Then, the X(Multi)PropertySet can be queried for values, and only
 * the supported properties are queried. (method getValues(...)) The
 * values are stored in the helper itself.
 *
 * Finally, each property can be queried for existence
 * (method hasProperty(...)) or its value (method (getValue(...))).
 *
 * After some initial preparation (hasProperties, getValues) the
 * MultiPropertySetHelper can be used similarly to an
 * XPropertySet in that you can query the values in the places where you
 * need them. However, if an XMultiPropertySet is supplied, the queries
 * are more efficient, often significantly so.
 */
class MultiPropertySetHelper
{
    /// names of all properties
    std::span<const OUString> pPropertyNames;

    /// the sequence of property names that the current (multi)
    /// property set implementation supports
    css::uno::Sequence<OUString> aPropertySequence;

    /// an array of indices that maps from pPropertyNames indices to
    /// aPropertySequence indices
    std::unique_ptr<sal_Int16[]> pSequenceIndex;

    /// the last set of values retrieved by getValues
    css::uno::Sequence<css::uno::Any> aValues;

    /// result of aValues.getConstArray()
    const css::uno::Any* pValues;

    /// an empty Any
    css::uno::Any aEmptyAny;

public:
    MultiPropertySetHelper(std::span<const OUString> pNames);

    ~MultiPropertySetHelper();

    /**
     * Call hasPropertiesByName for the provided XPropertySetInfo and build
     * list of allowed properties.
     */
    void hasProperties(const css::uno::Reference<css::beans::XPropertySetInfo>&);

    /**
     * Return whether hasProperties was called
     * (i.e. if we are ready to call getValues)
     */
    bool checkedProperties();

    /**
     * Get values from the XMultiPropertySet.
     *
     * May only be called after hasProperties() was called for the
     * appropriate XPropertySetInfo.
     */
    void getValues(const css::uno::Reference<css::beans::XMultiPropertySet>&);

    /**
     * Get values from the XPropertySet. This can be much slower than
     * getValues( const Reference<XMultiPropertySet& ) and hence
     * should be avoided.
     *
     * May only be called after hasProperties() was called for the
     * appropriate XPropertySetInfo.
     */
    void getValues(const css::uno::Reference<css::beans::XPropertySet>&);

    /**
     * Get a value from the values array.
     *
     * May only be called after getValues() was called.
     */
    inline const css::uno::Any& getValue(sal_Int16 nIndex);

    /**
     * Find out if this property is supported.
     *
     * May only be called after hasProperties() was called.
     */
    inline bool hasProperty(sal_Int16 nIndex);

    /**
     * Get a value from the XPropertySet on demand.
     *
     * If neither getValues nor getValueOnDemand has been called already
     * after the last call to resetValues, the values are retrieved
     * using getValues. Otherwise the value already retrieved is returned.
     * In case XMultiPropertySet is supported by the XPropertySet and
     * bTryMult is set, the XMultiPropertySet is used to get the values.
     *
     */
    const css::uno::Any& getValue(sal_Int16 nIndex,
                                  const css::uno::Reference<css::beans::XPropertySet>&,
                                  bool bTryMulti = false);

    /**
     * Get a value from the XMultiPropertySet on demand.
     *
     * If neither getValues nor getValueOnDemand has been called already
     * after the last call to resetValues, the values are retrieved
     * using getValues. Otherwise the value already retrieved is returned.
     * In case XMultiPropertySet is supported by the XPropertySet,
     * XMultiPropertySet is used to get the values.
     *
     */
    const css::uno::Any& getValue(sal_Int16 nIndex,
                                  const css::uno::Reference<css::beans::XMultiPropertySet>&);

    void resetValues() { pValues = nullptr; }
};

// inline implementations of the often-called methods getValue and hasProperty:

const css::uno::Any& MultiPropertySetHelper::getValue(sal_Int16 nValueNo)
{
    assert(pValues && "called getValue() without calling getValues()");
    assert(pSequenceIndex && "called getValue() without calling hasProperties()");
    assert(o3tl::make_unsigned(nValueNo) < pPropertyNames.size());

    sal_Int16 nIndex = pSequenceIndex[nValueNo];
    return (nIndex != -1) ? pValues[nIndex] : aEmptyAny;
}

bool MultiPropertySetHelper::hasProperty(sal_Int16 nValueNo)
{
    assert(pSequenceIndex && "called hasProperty() without calling hasProperties()");
    assert(o3tl::make_unsigned(nValueNo) < pPropertyNames.size());

    return pSequenceIndex[nValueNo] != -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
