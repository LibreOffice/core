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
#ifndef _CONDEDTI_HXX
#define _CONDEDTI_HXX

#include <vcl/edit.hxx>
#include <svtools/transfer.hxx>
#include "swdllapi.h"

class SW_DLLPUBLIC ConditionEdit : public Edit, public DropTargetHelper
{
    sal_Bool bBrackets, bEnableDrop;

    SW_DLLPRIVATE virtual sal_Int8  AcceptDrop( const AcceptDropEvent& rEvt );
    SW_DLLPRIVATE virtual sal_Int8  ExecuteDrop( const ExecuteDropEvent& rEvt );

public:
    ConditionEdit( Window* pParent, const ResId& rResId );

    inline void ShowBrackets(sal_Bool bShow)        { bBrackets = bShow; }

    inline void SetDropEnable( sal_Bool bFlag )     { bEnableDrop = bFlag; }
    inline sal_Bool IsDropEnable() const            { return bEnableDrop; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
