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

#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_CONTENTPROPERTIES_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_CONTENTPROPERTIES_HXX

#include <config_lgpl.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
} } } }

namespace webdav_ucp
{

struct DAVResource;

// PropertyValueMap.
class PropertyValue
{
private:
    css::uno::Any m_aValue;
    bool          m_bIsCaseSensitive;

public:
    PropertyValue()
    : m_bIsCaseSensitive( true ) {}

    PropertyValue( const css::uno::Any & rValue,
                   bool bIsCaseSensitive )
    : m_aValue( rValue),
      m_bIsCaseSensitive( bIsCaseSensitive ) {}

    bool isCaseSensitive() const { return m_bIsCaseSensitive; }
    const css::uno::Any & value() const { return m_aValue; }

};

typedef std::unordered_map
<
    OUString,
    PropertyValue,
    OUStringHash
>
PropertyValueMap;

struct DAVResource;

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

    const css::uno::Any &
    getValue( const OUString & rName ) const;

    // Maps the UCB property names contained in rProps with their DAV property
    // counterparts, if possible. All unmappable properties will be included
    // unchanged in resulting vector unless bIncludeUnmatched is set to false.
    // The vector filles by this method can directly be handed over to
    // DAVResourceAccess::PROPFIND. The result from PROPFIND
    // (vector< DAVResource >) can be used to create a ContentProperties
    // instance which can map DAV properties back to UCB properties.
    static void UCBNamesToDAVNames( const css::uno::Sequence< css::beans::Property > &  rProps,
                                    std::vector< OUString > & resources,
                                    bool bIncludeUnmatched = true );

    // Maps the UCB property names contained in rProps with their HTTP header
    // counterparts, if possible. All unmappable properties will be included
    // unchanged in resulting vector unless bIncludeUnmatched is set to false.
    // The vector filles by this method can directly be handed over to
    // DAVResourceAccess::HEAD. The result from HEAD (vector< DAVResource >)
    // can be used to create a ContentProperties instance which can map header
    // names back to UCB properties.
    static void UCBNamesToHTTPNames( const css::uno::Sequence< css::beans::Property > & rProps,
                                    std::vector< OUString > & resources,
                                    bool bIncludeUnmatched = true );

    // return true, if all properties contained in rProps are contained in
    // this ContentProperties instance. Otherwiese, false will be returned.
    // rNamesNotContained contain the missing names.
    bool containsAllNames(
                    const css::uno::Sequence< css::beans::Property >& rProps,
                    std::vector< OUString > & rNamesNotContained ) const;

    // adds all properties described by rProps that are actually contained in
    // rContentProps to this instance. In case of duplicates the value
    // already contained in this will left unchanged.
    void addProperties( const std::vector< OUString > & rProps,
                        const ContentProperties & rContentProps );

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

    ContentProperties & operator=( const ContentProperties & ) = delete;

    const PropertyValue * get( const OUString & rName ) const;
};

class CachableContentProperties
{
private:
    ContentProperties m_aProps;

    CachableContentProperties & operator=( const CachableContentProperties & ) = delete;
    CachableContentProperties( const CachableContentProperties & ) = delete;

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

} // namespace webdav_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_CONTENTPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
