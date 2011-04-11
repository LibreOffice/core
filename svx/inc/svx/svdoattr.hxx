/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
