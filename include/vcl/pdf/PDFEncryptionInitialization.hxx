/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>

namespace com::sun::star::beans
{
class XMaterialHolder;
}
namespace com::sun::star::uno
{
template <typename> class Reference;
}

namespace vcl::pdf
{
VCL_DLLPUBLIC css::uno::Reference<css::beans::XMaterialHolder>
initEncryption(const OUString& i_rOwnerPassword, const OUString& i_rUserPassword);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
