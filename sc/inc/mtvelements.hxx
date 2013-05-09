/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_MTVBLOCKFUNC_HXX
#define SC_MTVBLOCKFUNC_HXX

#include "svl/broadcast.hxx"

#include <mdds/multi_type_vector_macro.hpp>
#include <mdds/multi_type_vector_types.hpp>

namespace sc {

// Custom element type IDs for multi_type_vector.

const mdds::mtv::element_t element_type_broadcaster = mdds::mtv::element_type_user_start;

// Custom element blocks.

typedef mdds::mtv::noncopyable_managed_element_block<element_type_broadcaster, SvtBroadcaster> custom_broadcaster_block;

}

// This needs to be in global namespace just like SvtBroacaster is.
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(SvtBroadcaster, sc::element_type_broadcaster, NULL, sc::custom_broadcaster_block)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
