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

#include "acccontext.hxx"
#include <calbck.hxx>
#include <ndtyp.hxx>

class SwFlyFrame;

class SwAccessibleFrameBase : public SwAccessibleContext,
                              public SwClient
{
    bool    m_bIsSelected;    // protected by base class mutex
    bool    IsSelected();

protected:
    // Set states for getAccessibleStateSet.
    // This derived class additionally sets SELECTABLE(1), SELECTED(+),
    // FOCUSABLE(1) and FOCUSED(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet ) override;
    SwFlyFrame* getFlyFrame() const;
    bool GetSelectedState( );
    SwPaM* GetCursor();

    virtual void InvalidateCursorPos_() override;
    virtual void InvalidateFocus_() override;

    virtual ~SwAccessibleFrameBase() override;
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

public:
    SwAccessibleFrameBase(std::shared_ptr<SwAccessibleMap> const& pInitMap,
                           sal_Int16 nInitRole,
                           const SwFlyFrame *pFlyFrame );

    virtual bool HasCursor() override;   // required by map to remember that object

    static SwNodeType GetNodeType( const SwFlyFrame *pFlyFrame );

    // The object is not visible any longer and should be destroyed
    virtual void Dispose(bool bRecursive, bool bCanSkipInvisible = true) override;
    virtual bool SetSelectedState( bool bSeleted ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
