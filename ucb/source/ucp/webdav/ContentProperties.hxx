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

#include <memory>
#include <unordered_map>
#include <vector>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "DAVResource.hxx"

namespace com::sun::star::beans {
    struct Property;
}

namespace http_dav_ucp
{

struct DAVResource;

// PropertyValueMap.
class PropertyValue
{
private:
    css::uno::Any   m_aValue;
    bool            m_bIsCaseSensitive;

public:
    PropertyValue()
    : m_bIsCaseSensitive( true ) {}

    explicit PropertyValue( const css::uno::Any & rValue,
                   bool bIsCaseSensitive )
    : m_aValue( rValue),
      m_bIsCaseSensitive( bIsCaseSensitive ) {}

    bool isCaseSensitive() const { return m_bIsCaseSensitive; }
    const css::uno::Any & value() const { return m_aValue; }

};

typedef std::unordered_map< OUString, PropertyValue > PropertyValueMap;

class ContentProperties
{
public:
    ContentProperties();

    explicit ContentProperties( const DAVResource& rResource );

    // Mini props for transient contents.
    ContentProperties( const OUString & rTitle, bool bFolder );

    // Micro props for non-existing contents.
    explicit ContentProperties( const OUString & rTitle );

    ContentProperties( const ContentProperties & rOther );

    bool contains( const OUString & rName ) const;

    const css::uno::Any& getValue( const OUString & rName ) const;

    // Maps the UCB property names contained in rProps with their DAV property
    // counterparts, if possible. All unmappable properties will be included
    // unchanged in resulting vector unless bIncludeUnmatched is set to false.
    // The vector filled by this method can directly be handed over to
    // DAVResourceAccess::PROPFIND. The result from PROPFIND
    // (vector< DAVResource >) can be used to create a ContentProperties
    // instance which can map DAV properties back to UCB properties.
    static void UCBNamesToDAVNames( const css::uno::Sequence< css::beans::Property > & rProps,
                                    std::vector< OUString > & resources,
                                    bool bIncludeUnmatched = true );

    // Maps the UCB property names contained in rProps with their HTTP header
    // counterparts, if possible. All unmappable properties will be included
    // unchanged in resulting vector unless bIncludeUnmatched is set to false.
    // The vector filled by this method can directly be handed over to
    // DAVResourceAccess::HEAD. The result from HEAD (vector< DAVResource >)
    // can be used to create a ContentProperties instance which can map header
    // names back to UCB properties.
    static void UCBNamesToHTTPNames( const css::uno::Sequence< css::beans::Property > & rProps,
                                    std::vector< OUString > & resources,
                                    bool bIncludeUnmatched = true );

    // return true, if all properties contained in rProps are contained in
    // this ContentProperties instance. Otherwise, false will be returned.
    // rNamesNotContained contain the missing names.
    bool containsAllNames(
                    const css::uno::Sequence< css::beans::Property >& rProps,
                    std::vector< OUString > & rNamesNotContained ) const;

    // adds all properties described by rProps that are actually contained in
    // rContentProps to this instance. In case of duplicates the value
    // already contained in this will left unchanged.
    void addProperties( const std::vector< OUString > & rProps,
                        const ContentProperties & rContentProps );

    // overwrites probably existing entries.
    void addProperties( const ContentProperties & rProps );

    // overwrites probably existing entries.
    void addProperties( const std::vector< DAVPropertyValue > & rProps );

    // overwrites probably existing entry.
    void addProperty( const OUString & rName,
                     const css::uno::Any & rValue,
                     bool bIsCaseSensitive );

    // overwrites probably existing entry.
    void addProperty( const DAVPropertyValue & rProp );

    bool isTrailingSlash() const { return m_bTrailingSlash; }

    const OUString & getEscapedTitle() const { return m_aEscapedTitle; }

    // Not good to expose implementation details, but this is actually an
    // internal class.
    const std::unique_ptr< PropertyValueMap > & getProperties() const
    { return m_xProps; }

private:
    OUString m_aEscapedTitle;
    std::unique_ptr< PropertyValueMap > m_xProps;
    bool m_bTrailingSlash;

    static css::uno::Any m_aEmptyAny;

    ContentProperties & operator=( const ContentProperties & ); // n.i.

    const PropertyValue * get( const OUString & rName ) const;
};

class CachableContentProperties
{
private:
    ContentProperties m_aProps;

    CachableContentProperties & operator=( const CachableContentProperties & ); // n.i.
    CachableContentProperties( const CachableContentProperties & ); // n.i.

public:
    explicit CachableContentProperties( const ContentProperties & rProps );

    void addProperties( const ContentProperties & rProps );

    void addProperties( const std::vector< DAVPropertyValue > & rProps );

    bool containsAllNames(
                    const css::uno::Sequence< css::beans::Property >& rProps,
                    std::vector< OUString > & rNamesNotContained ) const
    { return m_aProps.containsAllNames( rProps, rNamesNotContained ); }

    const css::uno::Any &
    getValue( const OUString & rName ) const
    { return m_aProps.getValue( rName ); }

    operator const ContentProperties & () const { return m_aProps; }
};

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
