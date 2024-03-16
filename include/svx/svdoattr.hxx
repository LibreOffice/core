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

#include <svx/svdobj.hxx>
#include <svx/svxdllapi.h>

//   Initial Declarations
class SfxPoolItem;
class SfxSetItem;
class SdrOutliner;
class SfxItemSet;
class SfxItemPool;

//   SdrAttrObj. This is an abstract class, we only instantiate its subclasses.
class SVXCORE_DLLPUBLIC SdrAttrObj : public SdrObject
{
private:
    friend class                SdrOutliner;

protected:
    tools::Rectangle                   maSnapRect;

protected:
    /// Detects the width of the line. No line ->0.
    SAL_DLLPRIVATE sal_Int32 ImpGetLineWdt() const;

    /// Detects when a stylesheet is changed
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    SAL_DLLPRIVATE SdrAttrObj(SdrModel& rSdrModel);
    // Copy constructor
    SAL_DLLPRIVATE SdrAttrObj(SdrModel& rSdrModel, SdrAttrObj const &);
    SAL_DLLPRIVATE virtual ~SdrAttrObj() override;

public:

    // Detects if bFilledObj && Fill != FillNone
    SAL_DLLPRIVATE bool HasFill() const;

    // Detects if Line != LineNone
    SAL_DLLPRIVATE bool HasLine() const;

    virtual const tools::Rectangle& GetSnapRect() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
