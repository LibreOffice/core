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

#ifndef _SVDTEXT_HXX
#define _SVDTEXT_HXX

#include <sal/types.h>
#include "svx/svxdllapi.h"
#include <tools/weakbase.hxx>

// --------------------------------------------------------------------

class OutlinerParaObject;
class SdrOutliner;
class SdrTextObj;
class SdrModel;
class SfxItemSet;

namespace sdr { namespace properties {
    class TextProperties;
}}

/** This class stores information about one text inside a shape.
*/

class SVX_DLLPUBLIC SdrText : public tools::WeakBase< SdrText >
{
public:
    SdrText( SdrTextObj& rObject, OutlinerParaObject* pOutlinerParaObject = 0 );
    virtual ~SdrText();

    virtual void SetModel(SdrModel* pNewModel);
    virtual void ForceOutlinerParaObject( sal_uInt16 nOutlMode );

    virtual void SetOutlinerParaObject( OutlinerParaObject* pTextObject );
    virtual OutlinerParaObject* GetOutlinerParaObject() const;

    virtual void CheckPortionInfo( SdrOutliner& rOutliner );
    virtual void ReformatText();

    // default uses GetObjectItemSet, but may be overloaded to
    // return a text-specific ItemSet
    virtual const SfxItemSet& GetItemSet() const;

    SdrModel* GetModel() const { return mpModel; }
    SdrTextObj& GetObject() const { return mrObject; }

    /** returns the current OutlinerParaObject and removes it from this instance */
    OutlinerParaObject* RemoveOutlinerParaObject();

protected:
    virtual const SfxItemSet& GetObjectItemSet();
    virtual void SetObjectItem(const SfxPoolItem& rItem);
    virtual SfxStyleSheet* GetStyleSheet() const;

private:
    OutlinerParaObject* mpOutlinerParaObject;
    SdrTextObj& mrObject;
    SdrModel* mpModel;
    bool mbPortionInfoChecked;
};

#endif //_SVDTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
