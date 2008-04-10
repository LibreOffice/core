/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofgraphic.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFGRAPHIC_HXX
#define _SDR_CONTACT_VIEWCONTACTOFGRAPHIC_HXX

#include <sal/types.h>
#include <svx/sdr/contact/viewcontactoftextobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrGrafObj;

namespace sdr
{
    namespace event
    {
        class AsynchGraphicLoadingEvent;
    } // end of namespace event
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfGraphic : public ViewContactOfTextObj
        {
            // Member which takes care for the asynch loading events which may be necessary
            // for asynch graphics loading.
            sdr::event::AsynchGraphicLoadingEvent*      mpAsynchLoadEvent;

            // Test graphics state and eventually trigger a SwapIn event or an Asynchronous
            // load event. Return value gives info if SwapIn was triggered.
            sal_Bool ImpPrepareForPaint(
                DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);

        protected:
            // internal access to SdrObject
            SdrGrafObj& GetGrafObject() const
            {
                return ((SdrGrafObj&)GetSdrObject());
            }

            // method to create a AnimationInfo. Needs to give a result if
            // SupportsAnimation() is overloaded and returns sal_True.
            virtual sdr::animation::AnimationInfo* CreateAnimationInfo();

            // For draft object display
            sal_Bool PaintGraphicDraft(
                DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC);

            // For empty Graphic PresObj display
            sal_Bool PaintGraphicPresObj(
                DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC);

            // Decide if graphic should be painted as draft
            sal_Bool DoPaintGraphicDraft() const;

            // Methods to react on start getting viewed or stop getting
            // viewed. This info is derived from the count of members of
            // registered ViewObjectContacts. Default does nothing.
            virtual void StopGettingViewed();

        public:
            // basic constructor, used from SdrObject.
            ViewContactOfGraphic(SdrGrafObj& rGrafObj);

            // The destructor. When PrepareDelete() was not called before (see there)
            // warnings will be generated in debug version if there are still contacts
            // existing.
            virtual ~ViewContactOfGraphic();

            // Prepare deletion of this object. Tghis needs to be called always
            // before really deleting this objects. This is necessary since in a c++
            // destructor no virtual function calls are allowed. To avoid this problem,
            // it is required to first call PrepareDelete().
            virtual void PrepareDelete();

            // When ShouldPaintObject() returns sal_True, the object itself is painted and
            // PaintObject() is called.
            virtual sal_Bool ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);

            // Paint this object. This is before evtl. SubObjects get painted. It needs to return
            // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
            virtual sal_Bool PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC);

            // React on removal of the object of this ViewContact,
            // DrawHierarchy needs to be changed
            virtual void ActionRemoved();

            // Does this ViewContact support animation?
            virtual sal_Bool SupportsAnimation() const;

            // This is the call from the asynch graphic loading. This may only be called from
            // AsynchGraphicLoadingEvent::ExecuteEvent(). Do load the graphics. The event will
            // be deleted (consumed) and ForgetAsynchGraphicLoadingEvent will be called.
            void DoAsynchGraphicLoading();

            // This is the call from the destructor of the asynch graphic loading event.
            // No one else has to call this. It is needed to let this object forget about
            // the event. The parameter allows checking for the correct event.
            void ForgetAsynchGraphicLoadingEvent(sdr::event::AsynchGraphicLoadingEvent* pEvent);
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFGRAPHIC_HXX

// eof
