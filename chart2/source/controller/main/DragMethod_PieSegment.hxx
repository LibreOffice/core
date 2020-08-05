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

#include "DragMethod_Base.hxx"
#include <basegfx/vector/b2dvector.hxx>

namespace chart
{

class DragMethod_PieSegment : public DragMethod_Base
{
public:
    DragMethod_PieSegment( DrawViewWrapper& rDrawViewWrapper, const OUString& rObjectCID
        , const css::uno::Reference< css::frame::XModel >& xChartModel );
    virtual ~DragMethod_PieSegment() override;

    virtual OUString GetSdrDragComment() const override;
    virtual bool BeginSdrDrag() override;
    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual bool EndSdrDrag(bool bCopy) override;

    virtual basegfx::B2DHomMatrix getCurrentTransformation() override;

protected:
    virtual void createSdrDragEntries() override;

private:
    ::basegfx::B2DVector    m_aStartVector;
    double                  m_fInitialOffset;
    double                  m_fAdditionalOffset;
    ::basegfx::B2DVector    m_aDragDirection;
    double                  m_fDragRange;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
