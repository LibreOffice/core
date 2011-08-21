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

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTOFGRAPHIC_HXX
#define _SDR_CONTACT_VIEWOBJECTCONTACTOFGRAPHIC_HXX

#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrGrafObj;

namespace sdr { namespace event {
    class AsynchGraphicLoadingEvent;
}}

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewObjectContactOfGraphic : public ViewObjectContactOfSdrObj
        {
        private:
            // allow async loading event helper to call tooling methods
            friend class sdr::event::AsynchGraphicLoadingEvent;

            // Member which takes care for the asynch loading events which may be necessary
            // for asynch graphics loading.
            sdr::event::AsynchGraphicLoadingEvent*      mpAsynchLoadEvent;

            // async graphics loading helpers. Only to be used internally or from the
            // event helper class (in .cxx file)
            bool impPrepareGraphicWithAsynchroniousLoading();
            bool impPrepareGraphicWithSynchroniousLoading();
            void doAsynchGraphicLoading();
            void forgetAsynchGraphicLoadingEvent(sdr::event::AsynchGraphicLoadingEvent* pEvent);

        protected:
            SdrGrafObj& getSdrGrafObj();

            // This method is responsible for creating the graphical visualisation data
            virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

        public:
            ViewObjectContactOfGraphic(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfGraphic();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWOBJECTCONTACTOFGRAPHIC_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
