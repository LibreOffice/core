/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: modelbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:50:12 $
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

#ifndef OOX_DRAWINGML_CHART_MODELBASE_HXX
#define OOX_DRAWINGML_CHART_MODELBASE_HXX

#include <boost/shared_ptr.hpp>
#include "oox/helper/helper.hxx"
#include "tokens.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

/** Inserts a data struct into a model class and provides accessor functions to the data.
 */
template< typename DataStruct >
class ModelData
{
public:
    inline explicit     ModelData() {}
    inline explicit     ModelData( const DataStruct& rData ) : maData( rData ) {}
    template< typename Param1 >
    inline explicit     ModelData( const Param1& rParam1 ) : maData( rParam1 ) {}
    template< typename Param1, typename Param2 >
    inline explicit     ModelData( const Param1& rParam1, const Param2& rParam2 ) : maData( rParam1, rParam2 ) {}
    virtual             ~ModelData() {}

    /** Provides read access to the data struct of this model. */
    inline const DataStruct& getData() const { return maData; }
    /** Provides read/write access to the data struct of this model. */
    inline DataStruct&  getData() { return maData; }

    /** Sets the passed data struct for this model. */
    inline void         setData( const DataStruct& rData ) { maData = rData; }

private:
    DataStruct          maData;
};

// ============================================================================

#define IMPL_MODEL_MEMBER( ClassName, MemberName ) \
public: \
    inline bool         has##MemberName() const { return mx##MemberName.get() != 0; } \
    inline const ClassName* get##MemberName() const { return mx##MemberName.get(); } \
    inline ClassName&   create##MemberName() const { mx##MemberName.rset( new ClassName ); return *mx##MemberName; } \
private: \
    ::boost::shared_ptr< ClassName > mx##MemberName


// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

