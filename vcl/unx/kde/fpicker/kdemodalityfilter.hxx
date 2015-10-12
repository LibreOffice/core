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

#ifndef INCLUDED_VCL_UNX_KDE_FPICKER_KDEMODALITYFILTER_HXX
#define INCLUDED_VCL_UNX_KDE_FPICKER_KDEMODALITYFILTER_HXX

#include <sal/config.h>

#include <config_vclplug.h>
#include <sal/types.h>

#if ENABLE_TDE
#include <shell/tde_defines.h>
#endif // ENABLE_TDE

#if ENABLE_TDE
#include <tqobject.h>
#else // ENABLE_TDE
#include <qobject.h>
#endif // ENABLE_TDE

class KDEModalityFilter : public QObject
{
private:
    WId m_nWinId;

public:
    KDEModalityFilter( WId nWinId );
    virtual ~KDEModalityFilter();

    virtual bool eventFilter( QObject *pObject, QEvent *pEvent ) override;
};

#endif // INCLUDED_VCL_UNX_KDE_FPICKER_KDEMODALITYFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
