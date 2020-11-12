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

#ifndef INCLUDED_VCL_STDTEXT_HXX
#define INCLUDED_VCL_STDTEXT_HXX

#include <sal/config.h>

#include <string_view>

#include <rtl/ustring.hxx>
#include <tools/wintypes.hxx>
#include <vcl/dllapi.h>

class Image;
namespace weld { class Widget; }

void VCL_DLLPUBLIC ShowServiceNotAvailableError(weld::Widget* pParent, std::u16string_view rServiceName, bool bError);

OUString VCL_DLLPUBLIC GetStandardText(StandardButtonType eButton);

VCL_DLLPUBLIC Image const & GetStandardInfoBoxImage();
VCL_DLLPUBLIC OUString GetStandardInfoBoxText();

VCL_DLLPUBLIC Image const & GetStandardWarningBoxImage();
VCL_DLLPUBLIC OUString GetStandardWarningBoxText();

VCL_DLLPUBLIC Image const & GetStandardErrorBoxImage();
VCL_DLLPUBLIC OUString GetStandardErrorBoxText();

VCL_DLLPUBLIC Image const & GetStandardQueryBoxImage();
VCL_DLLPUBLIC OUString GetStandardQueryBoxText();


#endif // INCLUDED_VCL_STDTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
