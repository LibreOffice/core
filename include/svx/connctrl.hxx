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
#ifndef INCLUDED_SVX_CONNCTRL_HXX
#define INCLUDED_SVX_CONNCTRL_HXX

#include <vcl/ctrl.hxx>
#include <svx/svxdllapi.h>

class SfxItemSet;
class SdrEdgeObj;
class SdrView;
class SdrObjList;

/*************************************************************************
|*
|* SvxXConnectionPreview
|*
\************************************************************************/
class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxXConnectionPreview : public Control
{
 friend class SvxConnectionPage;

private:
    SdrEdgeObj*         pEdgeObj;
    SdrObjList*         pObjList;
    const SdrView*      pView;

    SVX_DLLPRIVATE void SetStyles();
    SVX_DLLPRIVATE void AdaptSize();
public:
    SvxXConnectionPreview( vcl::Window* pParent, WinBits nStyle);
    virtual ~SvxXConnectionPreview();
    virtual void dispose() override;

    virtual void Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void Resize() override;
    virtual Size GetOptimalSize() const override;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

    void         SetAttributes( const SfxItemSet& rInAttrs );
    sal_uInt16   GetLineDeltaAnz();

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
};

#endif // INCLUDED_SVX_CONNCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
