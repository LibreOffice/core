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

#ifndef INCLUDED_SVX_SDR_CONTACT_OBJECTCONTACT_HXX
#define INCLUDED_SVX_SDR_CONTACT_OBJECTCONTACT_HXX

#include <svx/sdr/animation/objectanimator.hxx>
#include <svx/sdr/animation/animationstate.hxx>
#include <svx/svxdllapi.h>
#include <drawinglayer/geometry/viewinformation2d.hxx>

class SdrPageView;
class OutputDevice;

namespace vcl {
    class PDFExtOutDevData;
}

namespace basegfx {
    class B2DRange;
}

namespace sdr::contact {

class DisplayInfo;
class ViewContact;
class ViewObjectContactRedirector;

class SVXCORE_DLLPUBLIC ObjectContact
{
private:
    // make ViewObjectContact a friend to exclusively allow it to use
    // AddViewObjectContact/RemoveViewObjectContact
    friend class ViewObjectContact;

    // All VOCs which are created using this OC, thus remembering this OC
    // as a reference. All those VOCs need to be deleted when the OC goes down.
    // Registering and de-registering is done in the VOC constructors/destructors.
    std::vector< ViewObjectContact* >               maViewObjectContactVector;

    // A new ViewObjectContact was created and shall be remembered.
    void AddViewObjectContact(ViewObjectContact& rVOContact);

    // A ViewObjectContact was deleted and shall be forgotten.
    void RemoveViewObjectContact(ViewObjectContact& rVOContact);

    // the primitiveAnimator which is used if this View and/or the contained primitives
    // support animatedSwitchPrimitives
    sdr::animation::primitiveAnimator               maPrimitiveAnimator;

    // The redirector. If set it is used to pipe all supported calls
    // to the redirector
    ViewObjectContactRedirector*                    mpViewObjectContactRedirector;

    // the Primitive2DParameters containing view information
    drawinglayer::geometry::ViewInformation2D       maViewInformation2D;

    // flag for preview renderer
    bool                                            mbIsPreviewRenderer : 1;

protected:
    // Interface to allow derivates to travel over the registered VOC's
    sal_uInt32 getViewObjectContactCount() const { return maViewObjectContactVector.size(); }
    ViewObjectContact* getViewObjectContact(sal_uInt32 a) const { return maViewObjectContactVector[a]; }

    // interface to allow derivates to set PreviewRenderer flag
    void setPreviewRenderer(bool bNew) { mbIsPreviewRenderer = bNew; }

    // interface to allow derivates to set ViewInformation2D
    void updateViewInformation2D(const drawinglayer::geometry::ViewInformation2D& rViewInformation2D) { maViewInformation2D = rViewInformation2D; }

public:
    // basic constructor
    ObjectContact();
    virtual ~ObjectContact() COVERITY_NOEXCEPT_FALSE;

    // LazyInvalidate request. This is used from the VOCs to mark that they
    // got invalidated by an ActionChanged() call. An active view needs to remember
    // this and take action on it. Default implementation directly calls back
    // triggerLazyInvalidate() which promptly handles the request
    virtual void setLazyInvalidate(ViewObjectContact& rVOC);

    // call this to support evtl. preparations for repaint. Default does nothing
    virtual void PrepareProcessDisplay();

    // Process the whole displaying
    virtual void ProcessDisplay(DisplayInfo& rDisplayInfo);

    // test if visualizing of entered groups is switched on at all. Default
    // implementation returns sal_False.
    virtual bool DoVisualizeEnteredGroup() const;

    // get active group's (the entered group) ViewContact
    virtual const ViewContact* getActiveViewContact() const;

    // Invalidate given rectangle at the window/output which is represented by
    // this ObjectContact. Default does nothing.
    virtual void InvalidatePartOfView(const basegfx::B2DRange& rRange) const;

    // Get info about the need to visualize GluePoints. The default
    // is that it is not necessary.
    virtual bool AreGluePointsVisible() const;

    // method to get the primitiveAnimator
    sdr::animation::primitiveAnimator& getPrimitiveAnimator() {  return maPrimitiveAnimator; }

    // check if text animation is allowed. Default is sal_true.
    virtual bool IsTextAnimationAllowed() const;

    // check if graphic animation is allowed. Default is sal_true.
    virtual bool IsGraphicAnimationAllowed() const;

    // access to ViewObjectContactRedirector
    ViewObjectContactRedirector* GetViewObjectContactRedirector() const {  return mpViewObjectContactRedirector; }
    void SetViewObjectContactRedirector(ViewObjectContactRedirector* pNew);

    // print? Default is false
    virtual bool isOutputToPrinter() const;

    // display page decoration? Default is true
    virtual bool isPageDecorationActive() const;

    // display mster page content (ViewContactOfMasterPage)? Default is true
    virtual bool isMasterPageActive() const;

    // recording MetaFile? Default is false
    virtual bool isOutputToRecordingMetaFile() const;

    // pdf export? Default is false
    virtual bool isOutputToPDFFile() const;
    virtual bool isExportTaggedPDF() const;
    virtual ::vcl::PDFExtOutDevData const* GetPDFExtOutDevData() const;

    // gray display mode
    virtual bool isDrawModeGray() const;

    // high contrast display mode
    virtual bool isDrawModeHighContrast() const;

    // check if this is a preview renderer. Default is sal_False.
    bool IsPreviewRenderer() const { return mbIsPreviewRenderer; }

    // get Primitive2DParameters for this view
    const drawinglayer::geometry::ViewInformation2D& getViewInformation2D() const { return maViewInformation2D; }

    /// access to SdrPageView. May return 0L like the default implementations do. Override as needed.
    virtual SdrPageView* TryToGetSdrPageView() const;

    /// access to OutputDevice. May return 0L like the default implementations do. Override as needed.
    virtual OutputDevice* TryToGetOutputDevice() const;

    // interface to support GridOffset for non-linear ViewToDevice transformation (calc)
    virtual bool supportsGridOffsets() const;
    virtual void calculateGridOffsetForViewObjectContact(
        basegfx::B2DVector& rTarget,
        const ViewObjectContact& rClient) const;
    virtual void calculateGridOffsetForB2DRange(
        basegfx::B2DVector& rTarget,
        const basegfx::B2DRange& rB2DRange) const;
    void resetAllGridOffsets();
};

}


#endif // INCLUDED_SVX_SDR_CONTACT_OBJECTCONTACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
