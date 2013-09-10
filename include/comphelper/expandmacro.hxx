/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMPHELPER_EXPANDMACRO_HXX_INCLUDED
#define COMPHELPER_EXPANDMACRO_HXX_INCLUDED

#include "rtl/ustring.hxx"
#include "comphelper/comphelperdllapi.h"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace comphelper
{
    /**
     A helper function to get expanded version of macro for filepaths.

     If the given path is prefixed by "vnd.sun.star.expand:", this
     function substitutes contained macro references. It then always
     returns a system file path, if necessary converting file
     URIs. Example:
     vnd.sun.star.expand:$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR gets
     converted to e.g. /usr/lib64/libreoffice/share.

     @param path to operate on. Both system file path and file URIs are accepted.

     @return macro-expanded file URI.
    */
    COMPHELPER_DLLPUBLIC rtl::OUString getExpandedFilePath(const rtl::OUString& filepath);
}
#endif
