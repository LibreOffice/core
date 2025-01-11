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

#include <stdlib.h>
#include <string.h>

#include "hwplib.h"
#include "hinfo.h"

namespace hwpfilter { struct StyleData; }

/**
 * @short Using for global style object like "Standard"
 */
class DLLEXPORT HWPStyle
{
    short nstyles;
    hwpfilter::StyleData *style;
    public:
        HWPStyle( void );
        ~HWPStyle( void );

        int Num( void ) const { return nstyles;}
        char *GetName( int n ) const;
        CharShape *GetCharShape( int n ) const;
        ParaShape *GetParaShape( int n ) const;

        void SetName( int n, char const *name );
        void SetCharShape( int n, CharShape const *cshapep );
        void SetParaShape( int n, ParaShape const *pshapep );

        void Read( HWPFile &hwpf );
};
/* _HWPSTYLE+H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
