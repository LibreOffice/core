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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCFRAMEBASE_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCFRAMEBASE_HXX

#include <acccontext.hxx>
#include <calbck.hxx>
#include <pam.hxx>

class SwFlyFrm;

class SwAccessibleFrameBase : public SwAccessibleContext,
                              public SwClient
{
    sal_Bool    bIsSelected;    // protected by base class mutex
    sal_Bool    IsSelected();

protected:
    // Set states for getAccessibleStateSet.
    // This drived class additionally sets SELECTABLE(1), SELECTED(+),
    // FOCUSABLE(1) and FOCUSED(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet ) SAL_OVERRIDE;
    SwFlyFrm* getFlyFrm() const;
    sal_Bool GetSelectedState( );
    SwPaM* GetCrsr();

    virtual void _InvalidateCursorPos() SAL_OVERRIDE;
    virtual void _InvalidateFocus() SAL_OVERRIDE;

    virtual ~SwAccessibleFrameBase();
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) SAL_OVERRIDE;

public:
    SwAccessibleFrameBase( SwAccessibleMap* pInitMap,
                           sal_Int16 nInitRole,
                           const SwFlyFrm *pFlyFrm );

    virtual sal_Bool HasCursor() SAL_OVERRIDE;   // required by map to remember that object

    static sal_uInt8 GetNodeType( const SwFlyFrm *pFlyFrm );

    // The object is not visible an longer and should be destroyed
    virtual void Dispose( sal_Bool bRecursive = sal_False ) SAL_OVERRIDE;
    virtual sal_Bool SetSelectedState( sal_Bool bSeleted ) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
