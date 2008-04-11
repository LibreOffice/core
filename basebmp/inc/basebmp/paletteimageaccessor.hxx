/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: paletteimageaccessor.hxx,v $
 * $Revision: 1.9 $
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

#ifndef INCLUDED_BASEBMP_PALETTEIMAGEACCESSOR_HXX
#define INCLUDED_BASEBMP_PALETTEIMAGEACCESSOR_HXX

#include <basebmp/colortraits.hxx>
#include <basebmp/accessortraits.hxx>

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

    // -------------------------------------------------------

    Accessor const& getWrappedAccessor() const { return maAccessor; }
    Accessor&       getWrappedAccessor() { return maAccessor; }

    // -------------------------------------------------------

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

    // -------------------------------------------------------

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

    // -------------------------------------------------------

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

#endif /* INCLUDED_BASEBMP_PALETTEIMAGEACCESSOR_HXX */
