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

#ifndef OOX_DRAWINGML_CLRSCHEME_HXX
#define OOX_DRAWINGML_CLRSCHEME_HXX

#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
#include "oox/drawingml/color.hxx"
#include "oox/dllapi.h"

namespace oox { namespace drawingml {

class ClrMap
{
    std::map < sal_Int32, sal_Int32 > maClrMap;

public:

    sal_Bool getColorMap( sal_Int32& nClrToken );
    void     setColorMap( sal_Int32 nClrToken, sal_Int32 nMappedClrToken );
};

typedef boost::shared_ptr< ClrMap > ClrMapPtr;

class OOX_DLLPUBLIC ClrScheme
{
    std::map < sal_Int32, sal_Int32 > maClrScheme;

public:

    ClrScheme();
    ~ClrScheme();

    sal_Bool getColor( sal_Int32 nSchemeClrToken, sal_Int32& rColor ) const;
    void     setColor( sal_Int32 nSchemeClrToken, sal_Int32 nColor );
};

typedef boost::shared_ptr< ClrScheme > ClrSchemePtr;

} }

#endif  //  OOX_DRAWINGML_CLRSCHEME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
