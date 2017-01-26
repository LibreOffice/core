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

#ifndef INCLUDED_SD_SOURCE_UI_INC_SDUNODRAWVIEW_HXX
#define INCLUDED_SD_SOURCE_UI_INC_SDUNODRAWVIEW_HXX

#include "DrawSubController.hxx"
#include "DrawViewShell.hxx"
#include <cppuhelper/basemutex.hxx>

class SdXImpressDocument;

namespace com { namespace sun { namespace star { namespace drawing {
class XLayer;
} } } }

namespace sd {

class DrawViewShell;

/** This class implements the DrawViewShell specific part of the controller.
*/
class SdUnoDrawView
    : private cppu::BaseMutex,
      public DrawSubControllerInterfaceBase
{
public:
    SdUnoDrawView (
        DrawViewShell& rViewShell,
        View& rView) throw();
    virtual ~SdUnoDrawView() throw() override;

    // XSelectionSupplier

    virtual sal_Bool SAL_CALL select (
        const css::uno::Any& aSelection) override;

    virtual css::uno::Any SAL_CALL getSelection() override;

    virtual void SAL_CALL addSelectionChangeListener (
        const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener) override;

    virtual void SAL_CALL removeSelectionChangeListener (
        const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener) override;

    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage >& xPage) override;

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage() override;

    // XFastPropertySet

    virtual void SAL_CALL setFastPropertyValue (
        sal_Int32 nHandle,
        const css::uno::Any& rValue) override;

    virtual css::uno::Any SAL_CALL getFastPropertyValue (
        sal_Int32 nHandle) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

protected:
    bool getMasterPageMode() const throw();
    void setMasterPageMode(bool MasterPageMode_) throw();
    bool getLayerMode() const throw();
    void setLayerMode(bool LayerMode_) throw();
public:
    /** Return a reference to the active layer object.
        @return
            The returned value may be empty when the internal state of this
            view is not valid (like during destruction.)
    */
    css::uno::Reference< css::drawing::XLayer> getActiveLayer() throw ();
protected:
    /** Make the specified object the active layer.
        @param rxLayer
            The new layer object.
        @throws css::uno::RuntimeException
    */
    void setActiveLayer (const css::uno::Reference< css::drawing::XLayer>& rxLayer);

    void SetZoom( sal_Int16 nZoom );
    sal_Int16 GetZoom() const;

    void SetViewOffset(const css::awt::Point& rWinPos );
    css::awt::Point GetViewOffset() const;

    void SetZoomType( sal_Int16 nType );

    css::uno::Any getDrawViewMode() const;

private:
    DrawViewShell& mrDrawViewShell;
    sd::View& mrView;

    SdXImpressDocument* GetModel() const throw();
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
