/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <sal/types.h>

using namespace ::com::sun::star;
namespace Justify
{
// Distribute space between glyphs ( half the space before and after the CH_BLANK ).
SW_DLLPUBLIC void SpaceDistribution(std::vector<sal_Int32>& rKernArray, const OUString& rText,
                                    sal_Int32 nStt, sal_Int32 nLen, tools::Long nSpaceAdd,
                                    tools::Long nKern, bool bNoHalfSpace);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
