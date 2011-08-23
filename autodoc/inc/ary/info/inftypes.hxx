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

#ifndef ARY_INFO_INFTYPES_HXX
#define ARY_INFO_INFTYPES_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace info
{




typedef uintt GlobalTextId;



/** Because this enum is used as index list for displayed
    tag headlines, the items must neither be moved nor deleted.
    Only adding to the end is allowed. atid_MAX always has to exist
    and to be the last used value.
    Also assigning numbers to the values is forbidden.
*/
enum E_AtTagId
{
    atid_ATT = 0,
    atid_author,
    atid_change,
    atid_collab,
    atid_contact,

    atid_copyright,
    atid_deprecated,
    atid_descr,
    atid_docdate,
    atid_derive,

    atid_dyn,
    atid_instance,
    atid_interface,
    atid_invariant,
    atid_life,

    atid_multi,
    atid_onerror,
    atid_persist,
    atid_postcond,
    atid_precond,

    atid_resp,
    atid_return,
    atid_short,
    atid_todo,
    atid_version,

    atid_MAX
};

/** Because this enum is used as index list for displayed
    tag headlines, the items must neither be moved nor deleted.
    Only adding to the end is allowed. C_eAtTag_NrOfClasses always has to exist
    and to be the last used value.
    Also assigning other numbers to the values, than in this
    existing scheme, is forbidden.
*/
enum E_AtTagClass
{
    atc_std = 0,

    atc_base = atid_MAX,
    atc_exception = atid_MAX + 1,
    atc_implements = atid_MAX + 2,
    atc_keyword = atid_MAX + 3,
    atc_parameter = atid_MAX + 4,

    atc_see = atid_MAX + 5,
    atc_template = atid_MAX + 6,
    atc_label = atid_MAX + 7,
    atc_since = atid_MAX + 8,
    C_eAtTag_NrOfClasses
};



}   // namespace info
}   // namespace ary


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
