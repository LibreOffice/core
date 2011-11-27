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

#ifndef _SVTOOLS_LNGMISC_HXX_
#define _SVTOOLS_LNGMISC_HXX_

#include "svl/svldllapi.h"

#include <rtl/ustring.hxx>

#define SVT_SOFT_HYPHEN (static_cast<sal_Unicode>(0x00AD))
#define SVT_HARD_HYPHEN (static_cast<sal_Unicode>(0x2011))

// the non-breaking space
#define SVT_HARD_SPACE  (static_cast<sal_Unicode>(0x00A0))

namespace linguistic
{
    inline bool IsHyphen(sal_Unicode cChar)
    {
        return cChar == SVT_SOFT_HYPHEN  ||  cChar == SVT_HARD_HYPHEN;
    }

    inline bool IsControlChar(sal_Unicode cChar)
    {
        // TODO: why doesn't this include 0x0F DEL?
        return cChar < static_cast<sal_Unicode>(' ');
    }

    SVL_DLLPRIVATE sal_Int32 GetNumControlChars( const rtl::OUString &rTxt );

    SVL_DLLPUBLIC bool RemoveHyphens(rtl::OUString &rTxt);
    SVL_DLLPUBLIC bool RemoveControlChars(rtl::OUString &rTxt);
    SVL_DLLPUBLIC bool ReplaceControlChars(rtl::OUString &rTxt);
    SVL_DLLPUBLIC ::rtl::OUString GetThesaurusReplaceText(const ::rtl::OUString &rText);
} // namespace linguistic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
