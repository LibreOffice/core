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
