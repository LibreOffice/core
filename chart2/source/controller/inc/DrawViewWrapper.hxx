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

#include <memory>
#include <svx/view3d.hxx>

namespace com::sun::star::drawing { class XShape; }
namespace com::sun::star::frame { class XModel; }

class SdrModel;

namespace chart
{

/** The DrawViewWrapper should help us to reduce effort if the underlying DrawingLayer changes.
Another task is to hide functionality we do not need, for example more than one page.
*/

class MarkHandleProvider
{
public:
    virtual bool getMarkHandles( SdrHdlList& rHdlList ) =0;
    virtual bool getFrameDragSingles() =0;

protected:
    ~MarkHandleProvider() {}
};

class DrawViewWrapper : public E3dView
{
public:
    DrawViewWrapper(
        SdrModel& rSdrModel,
        OutputDevice* pOut);

    virtual ~DrawViewWrapper() override;

    //triggers the use of an updated first page
    void    ReInit();

    /// tries to get an OutputDevice from the XParent of the model to use as reference device
    void attachParentReferenceDevice(
        const css::uno::Reference< css::frame::XModel > & xChartModel );

    //fill list of selection handles 'aHdl'
    virtual void SetMarkHandles(SfxViewShell* pOtherShell) override;

    SdrPageView*    GetPageView() const;

    SdrObject* getHitObject( const Point& rPnt ) const;

    void MarkObject( SdrObject* pObj );

    //pMarkHandleProvider can be NULL; ownership is not taken
    void setMarkHandleProvider( MarkHandleProvider* pMarkHandleProvider );
    void CompleteRedraw(OutputDevice* pOut, const vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = nullptr) override;

    SdrObject*   getSelectedObject() const;
    SdrObject*   getTextEditObject() const;
    SdrOutliner* getOutliner() const;

    SfxItemSet   getPositionAndSizeItemSetFromMarkedObject() const;

    SdrObject* getNamedSdrObject( const OUString& rName ) const;
    static bool IsObjectHit( SdrObject const * pObj, const Point& rPnt );

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    static SdrObject* getSdrObject( const css::uno::Reference< css::drawing::XShape >& xShape );

private:
    mutable MarkHandleProvider*     m_pMarkHandleProvider;

    std::unique_ptr< SdrOutliner >  m_apOutliner;

    // #i79965# scroll back view when ending text edit
    bool m_bRestoreMapMode;
    MapMode m_aMapModeToRestore;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
