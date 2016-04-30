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

#ifndef INCLUDED_OOX_DRAWINGML_CLRSCHEME_HXX
#define INCLUDED_OOX_DRAWINGML_CLRSCHEME_HXX

#include <cstddef>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include <oox/dllapi.h>
#include <sal/types.h>

namespace oox { namespace drawingml {

class ClrMap
{
    std::map < sal_Int32, sal_Int32 > maClrMap;

public:

    bool     getColorMap( sal_Int32& nClrToken );
    void     setColorMap( sal_Int32 nClrToken, sal_Int32 nMappedClrToken );
};

typedef std::shared_ptr< ClrMap > ClrMapPtr;

class OOX_DLLPUBLIC ClrScheme
{
    std::vector< std::pair<sal_Int32, sal_Int32> > maClrScheme;

public:

    bool     getColor( sal_Int32 nSchemeClrToken, sal_Int32& rColor ) const;
    void     setColor( sal_Int32 nSchemeClrToken, sal_Int32 nColor );

    bool     getColorByIndex(size_t nIndex,
            sal_Int32& rColor) const;
};

typedef std::shared_ptr< ClrScheme > ClrSchemePtr;

} }

#endif // INCLUDED_OOX_DRAWINGML_CLRSCHEME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
