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

#pragma once

#include <sal/config.h>

#include <memory>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <o3tl/deleter.hxx>
#include <osl/diagnose.h>

enum IDLIST {
     ID_MATHML,
     ID_LINES,
     ID_LINE,
     ID_EXPRLIST,
     ID_EXPR,
     ID_BEGIN,
     ID_END,
     ID_LEFT,
     ID_RIGHT,
     ID_SUBEXPR,
     ID_SUPEXPR,
     ID_SUBSUPEXPR,
     ID_FRACTIONEXPR,
     ID_OVER,
     ID_DECORATIONEXPR,
     ID_SQRTEXPR,
     ID_ROOTEXPR,
     ID_ARROWEXPR,
     ID_ACCENTEXPR,
     ID_UNARYEXPR,
     ID_PRIMARYEXPR,
     ID_BRACKET,
     ID_BLOCK,
     ID_PARENTH,
     ID_FENCE,
     ID_ABS,
     ID_IDENTIFIER,
     ID_STRING,
     ID_CHARACTER,
     ID_NUMBER,
     ID_OPERATOR,
     ID_SPACE,
     ID_DELIMITER
};

class Node{
public:
     explicit Node(int _id) : id(_id)
     {
          child = nullptr;
          next = nullptr;
#ifdef NODE_DEBUG
          count++;
          printf("Node count : [%d]\n",count);
#endif
     }
     ~Node()
     {
          next = nullptr;
          child = nullptr;
#ifdef NODE_DEBUG
          count--;
          printf("Node count : [%d]\n",count);
#endif
     }
public:
     static int count; /* For memory debugging */
     int id;
     std::unique_ptr<char, o3tl::free_delete> value;
     Node *child;
     Node *next;
};
extern std::vector<std::unique_ptr<Node>> nodelist;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
