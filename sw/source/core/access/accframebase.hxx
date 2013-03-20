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

#ifndef _ACCFRAMEBASE_HXX
#define _ACCFRAMEBASE_HXX

#include <acccontext.hxx>
#include <calbck.hxx>

class SwFlyFrm;

class SwAccessibleFrameBase : public SwAccessibleContext,
                              public SwClient
{
    sal_Bool    bIsSelected;    // protected by base class mutex
    sal_Bool    IsSelected();

protected:
    // Set states for getAccessibleStateSet.
    // This drived class additionaly sets SELECTABLE(1), SELECTED(+),
    // FOCUSABLE(1) and FOCUSED(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet );

    virtual void _InvalidateCursorPos();
    virtual void _InvalidateFocus();

    virtual ~SwAccessibleFrameBase();
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:
    SwAccessibleFrameBase( SwAccessibleMap* pInitMap,
                           sal_Int16 nInitRole,
                           const SwFlyFrm *pFlyFrm );

    virtual sal_Bool HasCursor();   // required by map to remember that object

    static sal_uInt8 GetNodeType( const SwFlyFrm *pFlyFrm );

    // The object is not visible an longer and should be destroyed
    virtual void Dispose( sal_Bool bRecursive = sal_False );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
