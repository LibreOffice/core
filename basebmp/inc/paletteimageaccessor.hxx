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

#ifndef INCLUDED_BASEBMP_INC_PALETTEIMAGEACCESSOR_HXX
#define INCLUDED_BASEBMP_INC_PALETTEIMAGEACCESSOR_HXX

#include <colortraits.hxx>
#include <accessortraits.hxx>

#include <vigra/numerictraits.hxx>
#include <vigra/metaprogramming.hxx>

#include <algorithm>
#include <functional>

namespace basebmp
{

/** Access pixel data via palette indirection

    @tpl Accessor
    Raw accessor, to be used to actually access the pixel values

    @tpl ColorType
    The color value type to use - e.g. the palette is an array of that
    type
 */
template< class Accessor, typename ColorType > class PaletteImageAccessor
{
public:
    typedef typename Accessor::value_type  data_type;
    typedef ColorType                      value_type;

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
// making all members public, if no member template friends
private:
    template<class A, typename C> friend class PaletteImageAccessor;
#endif

    Accessor          maAccessor;
    const value_type* mpPalette;
    std::size_t       mnNumEntries;

public:
    PaletteImageAccessor() :
        maAccessor(),
        mpPalette(0),
        mnNumEntries(0)
    {}

    template< class A > explicit
    PaletteImageAccessor( PaletteImageAccessor<A,ColorType> const& rSrc ) :
        maAccessor( rSrc.maAccessor ),
        mpPalette( rSrc.mpPalette ),
        mnNumEntries( rSrc.mnNumEntries )
    {}

    PaletteImageAccessor( const value_type* pPalette,
                          std::size_t       numEntries ) :
        maAccessor(),
        mpPalette(pPalette),
        mnNumEntries(numEntries)
    {}

    template< class T > PaletteImageAccessor( T                 accessor,
                                              const value_type* pPalette,
                                              std::size_t       numEntries ) :
        maAccessor(accessor),
        mpPalette(pPalette),
        mnNumEntries(numEntries)
    {}



    Accessor const& getWrappedAccessor() const { return maAccessor; }
    Accessor&       getWrappedAccessor() { return maAccessor; }



    data_type lookup(value_type const& v) const
    {
        // TODO(P3): use table-based/octree approach here!
        const value_type* best_entry;
        const value_type* palette_end( mpPalette+mnNumEntries );
        if( (best_entry=std::find( mpPalette, palette_end, v)) != palette_end )
            return best_entry-mpPalette;

        const value_type* curr_entry( mpPalette );
        best_entry = curr_entry;
        while( curr_entry != palette_end )
        {
            if( ColorTraits<value_type>::distance(*curr_entry,
                                                  *best_entry)
                > ColorTraits<value_type>::distance(*curr_entry,
                                                    v) )
            {
                best_entry = curr_entry;
            }

            ++curr_entry;
        }

        return best_entry-mpPalette;
    }



    template< class Iterator >
    value_type operator()(Iterator const& i) const
    {
        return mpPalette[ maAccessor(i) ];
    }

    template< class Iterator, class Difference >
    value_type operator()(Iterator const& i, Difference const& diff) const
    {
        return mpPalette[ maAccessor(i,diff) ];
    }



    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        maAccessor.set(
            lookup(
                vigra::detail::RequiresExplicitCast<value_type>::cast(value) ),
            i );
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        maAccessor.set(
            lookup(
                vigra::detail::RequiresExplicitCast<value_type>::cast(value) ),
            i,
            diff );
    }
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_INC_PALETTEIMAGEACCESSOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
