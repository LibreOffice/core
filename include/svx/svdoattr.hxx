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

#ifndef _SVDOATTR_HXX
#define _SVDOATTR_HXX

#include <svx/xfillit0.hxx>
#include <svx/xflasit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnasit.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdattr.hxx>
#include "svx/svxdllapi.h"

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SfxPoolItem;
class SfxSetItem;
class SdrOutliner;
class SfxItemSet;
class SfxItemPool;

//************************************************************
//   SdrAttrObj
//************************************************************

class SVX_DLLPUBLIC SdrAttrObj : public SdrObject
{
private:
    friend class                SdrOutliner;

protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    Rectangle                   maSnapRect;

protected:
    // Strichstaerke ermitteln. Keine Linie -> 0.
    sal_Int32 ImpGetLineWdt() const;

    // Zuhoeren, ob sich ein StyleSheet aendert
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    SdrAttrObj();
    virtual ~SdrAttrObj();

public:
    TYPEINFO();

    // Feststellen, ob bFilledObj && Fuellung!=FillNone
    sal_Bool HasFill() const;

    // Feststellen, ob Linie!=LineNone
    sal_Bool HasLine() const;

    virtual const Rectangle& GetSnapRect() const;

    virtual void SetModel(SdrModel* pNewModel);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOATTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
