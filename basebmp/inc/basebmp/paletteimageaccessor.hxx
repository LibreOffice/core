/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: paletteimageaccessor.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2006-05-31 09:49:42 $
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

#ifndef INCLUDED_BASEBMP_PALETTEIMAGEACCESSOR_HXX
#define INCLUDED_BASEBMP_PALETTEIMAGEACCESSOR_HXX

#include "metafunctions.hxx"

#include <vigra/numerictraits.hxx>
#include <vigra/mathutil.hxx>

#include <algorithm>


namespace basebmp
{

/** Access (possibly packed-pixel) data via palette indirection
 */
template< typename Valuetype, typename Datatype > class PaletteImageAccessor
{
public:
    typedef Valuetype                                   value_type;
    typedef Datatype                                    data_type;
    typedef typename remove_const<data_type>::type      count_type;


private:
    const value_type* palette;
    count_type        num_entries;

    double norm( value_type const& rLHS,
                 value_type const& rRHS ) const
    {
        return (rRHS - rLHS).magnitude();
    }

    data_type find_best_match(value_type const& v) const
    {
        // TODO(P3): use table-based/octree approach here!
        const value_type* best_entry;
        const value_type* palette_end( palette+num_entries );
        if( (best_entry=std::find( palette, palette_end, v)) != palette_end )
            return best_entry-palette;

        // TODO(F3): HACK. Need palette traits, and an error function
        // here. We blatantly assume value_type is a normed linear
        // space.
        const value_type* curr_entry( palette );
        best_entry = curr_entry;
        while( curr_entry != palette_end )
        {
            if( norm(*curr_entry,*best_entry) > norm(*curr_entry,v) )
                best_entry = curr_entry;

            ++curr_entry;
        }

        return best_entry-palette;
    }

    value_type toCol( value_type const& rCol ) const
    {
        return rCol;
    }

public:
    PaletteImageAccessor() :
        palette(0),
        num_entries(0)
    {}

    PaletteImageAccessor( const value_type* pPalette,
                          data_type         entries ) :
        palette(pPalette),
        num_entries(entries)
    {}

    template< class Iterator >
    value_type operator()(Iterator const& i) const { return toCol(palette[i.get()]); }
    value_type operator()(data_type const* i) const { return toCol(palette[*i]); }

    template< class Iterator, class Difference >
    value_type operator()(Iterator const& i, Difference const& diff) const
    {
        return toCol(palette[i.get(diff)]);
    }

    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        i.set(
            find_best_match(
                vigra::detail::RequiresExplicitCast<value_type>::cast(value) ));
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        i.set(
            find_best_match(
                vigra::detail::RequiresExplicitCast<value_type>::cast(value)),
            diff );
    }
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_PALETTEIMAGEACCESSOR_HXX */
