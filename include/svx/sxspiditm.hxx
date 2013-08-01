/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SXSPIDITM_HXX
#define _SXSPIDITM_HXX

#include <svx/svddef.hxx>
#include <svl/stritem.hxx>

/**
 * class SdrObjectShapeOriginalIdItem
 */
class SdrObjectShapeOriginalIdItem: public SfxStringItem {
public:
    SdrObjectShapeOriginalIdItem()                   : SfxStringItem() { SetWhich(SDRATTR_OBJECTSHAPEORIGINALID); }
    SdrObjectShapeOriginalIdItem(const String& rStr) : SfxStringItem(SDRATTR_OBJECTSHAPEORIGINALID,rStr) {}
    SdrObjectShapeOriginalIdItem(SvStream& rIn)      : SfxStringItem(SDRATTR_OBJECTSHAPEORIGINALID,rIn)  {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
