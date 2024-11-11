/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string_view>
#include <vector>
#include <vcl/dllapi.h>

namespace vcl::pdf
{
VCL_DLLPUBLIC std::vector<sal_uInt8>
computeHashR6(const sal_uInt8* pPassword, size_t nPasswordLength,
              std::vector<sal_uInt8> const& rValidationSalt,
              std::vector<sal_uInt8> const& rUserKey = std::vector<sal_uInt8>());

} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
