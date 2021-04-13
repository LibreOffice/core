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
#ifndef INCLUDED_SW_INC_ANCHOREDDRAWOBJECT_HXX
#define INCLUDED_SW_INC_ANCHOREDDRAWOBJECT_HXX

#include "anchoredobject.hxx"
#include <boost/optional.hpp>

namespace tools { class Rectangle; }

/** class for the positioning of drawing objects */
class SW_DLLPUBLIC SwAnchoredDrawObject : public SwAnchoredObject
{
    private:
        // boolean, indicating that the object position has been invalidated
        // and that a positioning has to be performed.
        bool mbValidPos;

        // rectangle, keeping the last object rectangle after the positioning
        boost::optional<tools::Rectangle> maLastObjRect;

        // boolean, indicating that anchored drawing object hasn't been attached
        // to a anchor frame yet. Once, it is attached to a anchor frame the
        // boolean changes its state.
        bool mbNotYetAttachedToAnchorFrame;

        // boolean, indicating that anchored
        // drawing object hasn't been positioned yet. Once, it's positioned the
        // boolean changes its state.
        bool mbNotYetPositioned;

        // boolean, indicating that after change of layout direction the
        // anchored drawing object has to be captured on the page, if it exceeds
        // the left or right page margin.
        // Needed for compatibility option <DoNotCaptureDrawObjsOnPage>
        bool mbCaptureAfterLayoutDirChange;

        /** method for the intrinsic positioning of a at-paragraph|at-character
            anchored drawing object

            helper method for method <MakeObjPos> */
        void MakeObjPosAnchoredAtPara();

        /** method for the intrinsic positioning of a at-page|at-frame anchored
            drawing object

            helper method for method <MakeObjPos> */
        void MakeObjPosAnchoredAtLayout();

            /** method to set positioning attributes (not for as-character anchored)

            During load the positioning attributes aren't set.
            Thus, the positioning attributes are set by the current object geometry.
            This method is also used for the conversion for drawing objects
            (not anchored as-character) imported from OpenOffice.org file format
            once and directly before the first positioning. */
        void SetPositioningAttr();

        /** method to set internal anchor position of <SdrObject> instance
            of the drawing object

            For drawing objects the internal anchor position of the <SdrObject>
            instance has to be set.
            Note: This adjustment is not be done for as-character anchored
            drawing object - the positioning code takes care of this.
            #i31698# - API for drawing objects in Writer has
            been adjusted. Thus, this method will only set the internal anchor
            position of the <SdrObject> instance to the anchor position given
            by its anchor frame.  */
        void SetDrawObjAnchor();

        /** method to invalidate the given page frame */
        void InvalidatePage_( SwPageFrame* _pPageFrame );

    protected:
        virtual void ObjectAttachedToAnchorFrame() override;

        /** method to assure that anchored object is registered at the correct
            page frame
        */
        virtual void RegisterAtCorrectPage() override;
        virtual void RegisterAtPage(SwPageFrame &) override;

        virtual bool SetObjTop_( const SwTwips _nTop) override;
        virtual bool SetObjLeft_( const SwTwips _nLeft) override;

        virtual const SwRect GetObjBoundRect() const override;

    public:

        SwAnchoredDrawObject();
        virtual ~SwAnchoredDrawObject() override;

        // declaration of pure virtual methods of base class <SwAnchoredObject>
        virtual void MakeObjPos() override;
        virtual void InvalidateObjPos() override;
        bool IsValidPos() const
        {
            return mbValidPos;
        }

        // accessors to the format
        virtual SwFrameFormat& GetFrameFormat() override;
        virtual const SwFrameFormat& GetFrameFormat() const override;

        // accessors to the object area and its position
        virtual const SwRect GetObjRect() const override;

        boost::optional<tools::Rectangle> const & GetLastObjRect() const { return maLastObjRect;}
        void SetLastObjRect( const tools::Rectangle& _rNewObjRect );

        /** adjust positioning and alignment attributes for new anchor frame

            Set horizontal and vertical position/alignment to manual position
            relative to anchor frame area using the anchor position of the
            new anchor frame and the current absolute drawing object position.
            Note: For correct Undo/Redo method should only be called inside a
            Undo-/Redo-action.

            @param <_pNewAnchorFrame>
            input parameter - new anchor frame for the anchored object.

            @param <_pNewObjRect>
            optional input parameter - proposed new object rectangle. If not
            provided the current object rectangle is taken.
        */
        void AdjustPositioningAttr( const SwFrame* _pNewAnchorFrame,
                                    const SwRect* _pNewObjRect = nullptr );

        /** method to notify background of drawing object */
        virtual void NotifyBackground( SwPageFrame* _pPageFrame,
                                       const SwRect& _rRect,
                                       PrepareHint _eHint ) override;

        bool NotYetPositioned() const
        {
            return mbNotYetPositioned;
        }

        // change of layout direction needs to be tracked
        // for setting <mbCaptureAfterLayoutDirChange>.
        virtual void UpdateLayoutDir() override;
        bool IsOutsidePage() const;

        // new Loop control
        void ValidateThis() { mbValidPos = true; }

        /** The element name to show in the XML dump.  */
        virtual const char* getElementName( ) const override { return "SwAnchoredDrawObject"; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
