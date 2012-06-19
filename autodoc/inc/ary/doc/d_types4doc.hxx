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

#ifndef ARY_DOC_D_TYPES4DOC_HXX
#define ARY_DOC_D_TYPES4DOC_HXX

// USED SERVICES



namespace ary
{
namespace doc
{


/** Type of a documentation: multiple lines or single line.
*/
enum E_BlockType
{
    dbt_none = 0,
    dbt_multiline,
    dbt_singleline
};

/** Type of documentation text: with html or without.
*/
enum E_TextType
{
    dtt_none = 0,
    dtt_plain,
    dtt_html
};

namespace nodetype
{

typedef int id;

}   //  namespace nodetype



}   //  namespace doc
}   //  namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
