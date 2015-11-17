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
#ifndef INCLUDED_UNOTOOLS_MISCCFG_HXX
#define INCLUDED_UNOTOOLS_MISCCFG_HXX

#include <unotools/unotoolsdllapi.h>
#include <unotools/options.hxx>

namespace utl
{
class UNOTOOLS_DLLPUBLIC MiscCfg : public detail::Options
{
public:
    MiscCfg( );
    virtual ~MiscCfg( );

    bool        IsNotFoundWarning()     const;
    void        SetNotFoundWarning( bool bSet);

    bool        IsPaperSizeWarning()    const;
    void        SetPaperSizeWarning(bool bSet);

    bool        IsPaperOrientationWarning()     const;
    void        SetPaperOrientationWarning( bool bSet);

                // 0 ... 99
    sal_Int32   GetYear2000()           const;
    void        SetYear2000( sal_Int32 nSet );
};

}

#endif // _MISCCFG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
