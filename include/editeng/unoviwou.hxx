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

#ifndef INCLUDED_EDITENG_UNOVIWOU_HXX
#define INCLUDED_EDITENG_UNOVIWOU_HXX

#include <editeng/unoedsrc.hxx>
#include <editeng/editengdllapi.h>

class OutlinerView;

/// Specialization for Draw/Impress
class EDITENG_DLLPUBLIC SvxDrawOutlinerViewForwarder : public SvxEditViewForwarder
{
private:
    OutlinerView&       mrOutlinerView;
    Point               maTextShapeTopLeft;

    EDITENG_DLLPRIVATE Point                GetTextOffset() const;

public:
    explicit            SvxDrawOutlinerViewForwarder( OutlinerView& rOutl );
                        SvxDrawOutlinerViewForwarder( OutlinerView& rOutl, const Point& rShapePosTopLeft );
    virtual             ~SvxDrawOutlinerViewForwarder();

    virtual bool        IsValid() const override;

    virtual Rectangle   GetVisArea() const override;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const override;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const override;

    virtual bool        GetSelection( ESelection& rSelection ) const override;
    virtual bool        SetSelection( const ESelection& rSelection ) override;
    virtual bool        Copy() override;
    virtual bool        Cut() override;
    virtual bool        Paste() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
