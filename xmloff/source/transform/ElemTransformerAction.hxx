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

#ifndef INCLUDED_XMLOFF_SOURCE_TRANSFORM_ELEMTRANSFORMERACTION_HXX
#define INCLUDED_XMLOFF_SOURCE_TRANSFORM_ELEMTRANSFORMERACTION_HXX

#include "TransformerAction.hxx"

enum XMLElemTransformerAction
{
    XML_ETACTION_EOT=XML_TACTION_EOT,   // uses for initialization only
    XML_ETACTION_COPY,                  // copy all (default)
                                        // for persistent elements: keep
                                        // elem content
    XML_ETACTION_COPY_TEXT,             // copy all
                                        // for persistent elements: keep
                                        // text content
    XML_ETACTION_COPY_CONTENT,          // ignore elem, process content
    XML_ETACTION_RENAME_ELEM,           // rename element:
                                        // - param1: namespace +
                                        //           token of local name
    XML_ETACTION_RENAME_ELEM_PROC_ATTRS,// rename element and proc attr:
                                        // - param1: elem namespace +
                                        //           token of local name
                                        // - param2: attr acation map
    XML_ETACTION_RENAME_ELEM_ADD_ATTR,  // rename element and add attr:
                                        // - param1: elem namespace +
                                        //           token of local name
                                        // - param2: attr namespace +
                                        //           token of local name
                                        // - param3: attr value
    XML_ETACTION_RENAME_ELEM_ADD_PROC_ATTR, // rename element and add attr:
                                        // - param1: elem namespace +
                                        //           token of local name
                                        // - param2: attr namespace +
                                        //           token of local name
                                        // - param3: attr value (low)
                                        //           attr action map (high)
    XML_ETACTION_RENAME_ELEM_COND,      // rename element if child of another:
                                        // - param1: namespace +
                                        //           token of new local name
                                        // - param2: namespace +
                                        //           token of parent element
    XML_ETACTION_RENAME_ELEM_PROC_ATTRS_COND,// rename element and proc attr
                                        // if child of another:
                                        // - param1: elem namespace +
                                        //           token of new local name
                                        // - param2: attr acation map
                                        // - param3: namespace +
                                        //           token of parent element
    XML_ETACTION_PROC_ATTRS,            // proc attr:
                                        // - param1: attr acation map
    XML_ETACTION_MOVE_ATTRS_TO_ELEMS,   // turn some attributes into elems
                                        // - param1: action map specifying
                                        //   the attrs that shall be moved
    XML_ETACTION_MOVE_ELEMS_TO_ATTRS,   // turn some elems into attrs
                                        // - param1: action map specifying
                                        //   the elems that shall be moved
    XML_ETACTION_PROC_ATTRS_COND,       // proc attr if child of another:
                                        // - param1: namespace +
                                        //           token of parent element
                                        // - param2: attr acation map
    XML_ETACTION_EXTRACT_CHARACTERS,    // takes the characters of the current
                                        // element and all child elements
                                        // and put them together into the
                                        // current element
                                        // the child elements are removed
    XML_ETACTION_MOVE_TO_ATTR,          // turn elem into an attr:
                                        // - param1: namespace +
                                        //           token of local name
    XML_ETACTION_MOVE_TO_ATTR_RNG2ISO_DATETIME, // turn elem into an attr and convert . to , in datetimes
                                        // - param1: namespace +
                                        //           token of local name
    XML_ETACTION_USER_DEFINED=0x40000000// user defined actions start here
};

#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_ELEMTRANSFORMERACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
