/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_MTVELEMENTS_HXX
#define SC_MTVELEMENTS_HXX

#include "svl/broadcast.hxx"

#define DEBUG_COLUMN_STORAGE 0

#if DEBUG_COLUMN_STORAGE
#ifdef NDEBUG
#undef NDEBUG
#endif
#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#endif

#include <mdds/multi_type_vector_macro.hpp>
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_custom_func1.hpp>

namespace sc {

struct CellTextAttr
{
    sal_uInt16 mnTextWidth;
    sal_uInt8 mnScriptType;

    CellTextAttr();
    CellTextAttr(const CellTextAttr& r);
    CellTextAttr(sal_uInt16 nTextWidth, sal_uInt8 nScriptType);
};

// Custom element type IDs for multi_type_vector.

const mdds::mtv::element_t element_type_broadcaster = mdds::mtv::element_type_user_start;
const mdds::mtv::element_t element_type_celltextattr = mdds::mtv::element_type_user_start + 1;

// Custom element blocks.

typedef mdds::mtv::noncopyable_managed_element_block<element_type_broadcaster, SvtBroadcaster> custom_broadcaster_block;
typedef mdds::mtv::default_element_block<element_type_celltextattr, CellTextAttr> custom_celltextattr_block;

// This needs to be in the same namespace as CellTextAttr.
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(CellTextAttr, element_type_celltextattr, CellTextAttr(), custom_celltextattr_block)

}

// This needs to be in global namespace just like SvtBroacaster is.
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(SvtBroadcaster, sc::element_type_broadcaster, NULL, sc::custom_broadcaster_block)

namespace sc {

// Broadcaster storage container
typedef mdds::mtv::custom_block_func1<sc::element_type_broadcaster, sc::custom_broadcaster_block> BCBlkFunc;
typedef mdds::multi_type_vector<BCBlkFunc> BroadcasterStoreType;

// Cell text attribute container.
typedef mdds::mtv::custom_block_func1<sc::element_type_celltextattr, sc::custom_celltextattr_block> CTAttrFunc;
typedef mdds::multi_type_vector<CTAttrFunc> CellTextAttrStoreType;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
