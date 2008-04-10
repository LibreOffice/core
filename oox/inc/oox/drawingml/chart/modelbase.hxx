/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: modelbase.hxx,v $
 * $Revision: 1.3 $
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

