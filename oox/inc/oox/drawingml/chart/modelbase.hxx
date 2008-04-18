/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: modelbase.hxx,v $
 * $Revision: 1.4 $
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

#include "oox/helper/containerhelper.hxx"
#include "tokens.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

template< typename Type >
class OptValue
{
public:
    inline explicit     OptValue() : mbHasValue( false ) {}
    inline explicit     OptValue( const Type& rValue ) : maValue( rValue ), mbHasValue( true ) {}

    inline bool         has() const { return mbHasValue; }
    inline bool         operator!() const { return !mbHasValue; }

    inline const Type&  get() const { return maValue; }
    inline const Type&  get( const Type& rDefValue ) const { return mbHasValue ? maValue : rDefValue; }

    inline void         reset() { mbHasValue = false; }
    inline void         set( const Type& rValue ) { maValue = rValue; mbHasValue = true; }
    inline OptValue&    operator=( const Type& rValue ) { set( rValue ); return *this; }

private:
    Type                maValue;
    bool                mbHasValue;
};

typedef OptValue< bool >            OptBool;
typedef OptValue< sal_Int32 >       OptInt32;
typedef OptValue< double >          OptDouble;
typedef OptValue< ::rtl::OUString > OptString;

// ============================================================================

template< typename ModelType >
class ModelRef : public ::boost::shared_ptr< ModelType >
{
public:
    inline explicit     ModelRef() {}
    inline              ~ModelRef() {}

    inline bool         is() const { return this->get() != 0; }

    inline ModelType&   create() { reset( new ModelType ); return **this; }
    template< typename Param1Type >
    inline ModelType&   create( const Param1Type& rParam1 ) { reset( new ModelType( rParam1 ) ); return **this; }

    inline ModelType&   getOrCreate() { if( !*this ) reset( new ModelType ); return **this; }
    template< typename Param1Type >
    inline ModelType&   getOrCreate( const Param1Type& rParam1 ) { if( !*this ) reset( new ModelType( rParam1 ) ); return **this; }
};

// ============================================================================

template< typename ModelType >
class ModelVector : public RefVector< ModelType >
{
public:
    typedef typename RefVector< ModelType >::value_type value_type;
    typedef typename RefVector< ModelType >::size_type  size_type;

    inline explicit     ModelVector() {}
    inline              ~ModelVector() {}

    inline ModelType&   create() { return append( new ModelType ); }
    template< typename Param1Type >
    inline ModelType&   create( const Param1Type& rParam1 ) { return append( new ModelType( rParam1 ) ); }

private:
    inline ModelType&   append( ModelType* pModel ) { this->push_back( value_type( pModel ) ); return *pModel; }
};

// ============================================================================

template< typename KeyType, typename ModelType >
class ModelMap : public RefMap< KeyType, ModelType >
{
public:
    typedef typename RefMap< KeyType, ModelType >::key_type     key_type;
    typedef typename RefMap< KeyType, ModelType >::mapped_type  mapped_type;
    typedef typename RefMap< KeyType, ModelType >::value_type   value_type;

    inline explicit     ModelMap() {}
    inline              ~ModelMap() {}

    inline ModelType&   create( KeyType eKey ) { return insert( eKey, new ModelType ); }
    template< typename Param1Type >
    inline ModelType&   create( KeyType eKey, const Param1Type& rParam1 ) { return insert( eKey, new ModelType( rParam1 ) ); }

private:
    inline ModelType&   insert( KeyType eKey, ModelType* pModel ) { (*this)[ eKey ].reset( pModel ); return *pModel; }
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

