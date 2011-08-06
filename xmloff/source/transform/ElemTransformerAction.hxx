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

#ifndef _XMLOFF_ELEMTRANSFORMERACTION_HXX
#define _XMLOFF_ELEMTRANSFORMERACTION_HXX

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
    XML_ETACTION_USER_DEFINED=0x40000000,// user defined actions start here
    XML_ETACTION_END=XML_TACTION_END
};

#endif  //  _XMLOFF_ELEMTRANSFORMERACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
