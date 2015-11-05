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
#ifndef INCLUDED_SW_INC_FMTSRNDENUM_HXX
#define INCLUDED_SW_INC_FMTSRNDENUM_HXX

#include <com/sun/star/text/WrapTextMode.hpp>

enum SwSurround {
    SURROUND_NONE = css::text::WrapTextMode_NONE,
    SURROUND_BEGIN = SURROUND_NONE,
    SURROUND_THROUGHT = css::text::WrapTextMode_THROUGHT,
    SURROUND_PARALLEL = css::text::WrapTextMode_PARALLEL,
    SURROUND_IDEAL = css::text::WrapTextMode_DYNAMIC,
    SURROUND_LEFT = css::text::WrapTextMode_LEFT,
    SURROUND_RIGHT = css::text::WrapTextMode_RIGHT,
    SURROUND_END
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
