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

#ifndef OOX_DRAWINGML_CHART_MODELBASE_HXX
#define OOX_DRAWINGML_CHART_MODELBASE_HXX

#include "oox/helper/containerhelper.hxx"
#include "oox/helper/helper.hxx"
#include "tokens.hxx"

namespace oox { class AttributeList; }

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

template< typename ModelType >
class ModelRef : public ::boost::shared_ptr< ModelType >
{
public:
    inline explicit     ModelRef() {}
    inline              ModelRef( const ::boost::shared_ptr< ModelType >& rxModel ) : ::boost::shared_ptr< ModelType >( rxModel ) {}
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

struct NumberFormat
{
    ::rtl::OUString     maFormatCode;       /// Number format code.
    bool                mbSourceLinked;     /// True = number format linked to source data.

    explicit            NumberFormat();

    void                setAttributes( const AttributeList& rAttribs );
};

// ============================================================================

struct LayoutModel
{
    double              mfX;                /// Left position of this object.
    double              mfY;                /// Top position of this object.
    double              mfW;                /// Width of this object.
    double              mfH;                /// Height of this object.
    sal_Int32           mnXMode;            /// Mode for left position.
    sal_Int32           mnYMode;            /// Mode for top position.
    sal_Int32           mnWMode;            /// Mode for width.
    sal_Int32           mnHMode;            /// Mode for height.
    sal_Int32           mnTarget;           /// Layout target for plot area.
    bool                mbAutoLayout;       /// True = automatic positioning.

    explicit            LayoutModel();
                        ~LayoutModel();
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
