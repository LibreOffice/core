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
#ifndef _SVX_TAB_AREA_HXX
#define _SVX_TAB_AREA_HXX

// enum ------------------------------------------------------------------

enum ColorModel
{
    CM_RGB,
    CM_CMYK // kann um weitere Modelle, z.B. HSV erweitert werden
};

enum PageType
{
    PT_AREA,
    PT_GRADIENT,
    PT_HATCH,
    PT_BITMAP,
    PT_COLOR,
    PT_SHADOW,
    PT_TRANSPARENCE
};

// define ----------------------------------------------------------------

typedef sal_uInt16 ChangeType; // auch in tab_line.hxx (mitpflegen !!!)

#define CT_NONE                 ( (ChangeType) 0x0000 )
#define CT_MODIFIED             ( (ChangeType) 0x0001 )
#define CT_CHANGED              ( (ChangeType) 0x0002 )
#define CT_SAVED                ( (ChangeType) 0x0004 )

#endif // _SVX_TAB_AREA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
