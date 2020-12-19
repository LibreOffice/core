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

#include <tools/link.hxx>
#include <tools/long.hxx>

#include <memory>

#define VECT_POLY_MAX 8192

#define VECT_FREE_INDEX 0
#define VECT_CONT_INDEX 1
#define VECT_DONE_INDEX 2

#define VECT_POLY_INLINE_INNER 1UL
#define VECT_POLY_INLINE_OUTER 2UL
#define VECT_POLY_OUTLINE_INNER 4UL
#define VECT_POLY_OUTLINE_OUTER 8UL

namespace vcl::Vectorizer
{
constexpr tools::Long BACK_MAP(tools::Long _def_nVal) { return ((_def_nVal + 2) >> 2) - 1; }

void VECT_MAP(const std::unique_ptr<tools::Long[]>& pMapIn,
              const std::unique_ptr<tools::Long[]>& pMapOut, tools::Long nVal);

void VECT_PROGRESS(const Link<tools::Long, void>* pProgress, tools::Long _def_nVal);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
