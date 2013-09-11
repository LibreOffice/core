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

#ifndef OOX_DRAWINGML_CHART_MODELBASE_HXX
#define OOX_DRAWINGML_CHART_MODELBASE_HXX

#include "oox/helper/helper.hxx"
#include "oox/helper/refmap.hxx"
#include "oox/helper/refvector.hxx"

namespace oox { class AttributeList; }

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

template< typename ModelType >
class ModelRef : public ::boost::shared_ptr< ModelType >
{
public:
                 ModelRef() {}
                 ModelRef( const ::boost::shared_ptr< ModelType >& rxModel ) : ::boost::shared_ptr< ModelType >( rxModel ) {}
                 ~ModelRef() {}

    bool         is() const { return this->get() != 0; }

    ModelType&   create() { this->reset( new ModelType ); return **this; }
    template< typename Param1Type >
    ModelType&   create( const Param1Type& rParam1 ) { this->reset( new ModelType( rParam1 ) ); return **this; }

    ModelType&   getOrCreate() { if( !*this ) this->reset( new ModelType ); return **this; }
    template< typename Param1Type >
    ModelType&   getOrCreate( const Param1Type& rParam1 ) { if( !*this ) this->reset( new ModelType( rParam1 ) ); return **this; }
};

// ============================================================================

template< typename ModelType >
class ModelVector : public RefVector< ModelType >
{
public:
    typedef typename RefVector< ModelType >::value_type value_type;
    typedef typename RefVector< ModelType >::size_type  size_type;

                 ModelVector() {}
                 ~ModelVector() {}

    ModelType&   create() { return append( new ModelType ); }
    template< typename Param1Type >
    ModelType&   create( const Param1Type& rParam1 ) { return append( new ModelType( rParam1 ) ); }

private:
    ModelType&   append( ModelType* pModel ) { this->push_back( value_type( pModel ) ); return *pModel; }
};

// ============================================================================

template< typename KeyType, typename ModelType >
class ModelMap : public RefMap< KeyType, ModelType >
{
public:
    typedef typename RefMap< KeyType, ModelType >::key_type     key_type;
    typedef typename RefMap< KeyType, ModelType >::mapped_type  mapped_type;
    typedef typename RefMap< KeyType, ModelType >::value_type   value_type;

                 ModelMap() {}
                 ~ModelMap() {}

    ModelType&   create( KeyType eKey ) { return insert( eKey, new ModelType ); }
    template< typename Param1Type >
    ModelType&   create( KeyType eKey, const Param1Type& rParam1 ) { return insert( eKey, new ModelType( rParam1 ) ); }

private:
    ModelType&   insert( KeyType eKey, ModelType* pModel ) { (*this)[ eKey ].reset( pModel ); return *pModel; }
};

// ============================================================================

struct NumberFormat
{
    OUString     maFormatCode;       /// Number format code.
    bool                mbSourceLinked;     /// True = number format linked to source data.

                        NumberFormat();

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

                        LayoutModel();
                        ~LayoutModel();
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
