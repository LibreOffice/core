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
#ifndef _UTL_MISCCFG_HXX
#define _UTL_MISCCFG_HXX

#include "unotools/unotoolsdllapi.h"
#include "tools/solar.h"
#include "unotools/options.hxx"

namespace utl
{
    class SfxMiscCfg;

class UNOTOOLS_DLLPUBLIC MiscCfg : public detail::Options
{
    SfxMiscCfg*     pImpl;

public:
    MiscCfg( );
    virtual ~MiscCfg( );

    sal_Bool        IsNotFoundWarning()     const;
    void        SetNotFoundWarning( sal_Bool bSet);

    sal_Bool        IsPaperSizeWarning()    const;
    void        SetPaperSizeWarning(sal_Bool bSet);

    sal_Bool        IsPaperOrientationWarning()     const;
    void        SetPaperOrientationWarning( sal_Bool bSet);

                // 0 ... 99
    sal_Int32   GetYear2000()           const;
    void        SetYear2000( sal_Int32 nSet );
};

}

#endif // _MISCCFG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
