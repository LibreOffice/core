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

#include <editeng/outlobj.hxx>
#include <svx/sdr/properties/defaultproperties.hxx>
#include <svx/svxdllapi.h>

class SdrOutliner;
class SdrTextObj;
class SfxItemSet;
enum class OutlinerMode;

/** This class stores information about one text inside a shape.
*/

class SfxStyleSheet;
class SVXCORE_DLLPUBLIC SdrText : public ::cppu::OWeakObject
{
public:
    explicit SdrText(SdrTextObj& rObject);
    virtual ~SdrText() override;

    void ForceOutlinerParaObject(OutlinerMode nOutlMode);

    virtual void SetOutlinerParaObject(std::optional<OutlinerParaObject> pTextObject);
    OutlinerParaObject* GetOutlinerParaObject();
    const OutlinerParaObject* GetOutlinerParaObject() const;

    void CheckPortionInfo(const SdrOutliner& rOutliner);
    void ReformatText();

    // default uses GetObjectItemSet, but may be overridden to
    // return a text-specific ItemSet
    const SfxItemSet& GetItemSet() const;

    // This class does not need an own SdrModel reference - always
    // has the SdrTextObj working with so can use SdrModel::getSdrModelFromSdrObject()
    SdrTextObj& GetObject() const { return mrObject; }

    /** returns the current OutlinerParaObject and removes it from this instance */
    std::optional<OutlinerParaObject> RemoveOutlinerParaObject();

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

protected:
    virtual const SfxItemSet& GetObjectItemSet();
    virtual SfxStyleSheet* GetStyleSheet() const;

private:
    std::optional<OutlinerParaObject> mpOutlinerParaObject;
    SdrTextObj& mrObject;
    bool mbPortionInfoChecked;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
