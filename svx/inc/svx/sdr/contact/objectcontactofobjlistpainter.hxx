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

#ifndef _SDR_CONTACT_OBJECTCONTACTOFOBJLISTPAINTER_HXX
#define _SDR_CONTACT_OBJECTCONTACTOFOBJLISTPAINTER_HXX

#include <svx/sdr/contact/objectcontact.hxx>
#include "svx/svxdllapi.h"
#include <svx/svdpage.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrPage;
class SdrObject;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ObjectContactPainter : public ObjectContact
        {
        protected:
            // Hierarchy access methods
            virtual sal_uInt32 GetPaintObjectCount() const = 0;
            virtual ViewContact& GetPaintObjectViewContact(sal_uInt32 nIndex) const = 0;

        public:
            // basic constructor/destructor
            ObjectContactPainter();
            virtual ~ObjectContactPainter();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // typedef for transferring SdrObject
        typedef ::std::vector< SdrObject* > SdrObjectVector;

        class SVX_DLLPUBLIC ObjectContactOfObjListPainter : public ObjectContactPainter
        {
        protected:
            // Target OutputDevice
            OutputDevice&                                   mrTargetOutputDevice;

            // Set StartPoint for next run, also given in constructor
            SdrObjectVector                                 maStartObjects;

            // the processed page which is the base e.g. for PageNumberFields
            const SdrPage*                                  mpProcessedPage;

            // Hierarchy access methods
            virtual sal_uInt32 GetPaintObjectCount() const;
            virtual ViewContact& GetPaintObjectViewContact(sal_uInt32 nIndex) const;

        public:
            // basic constructor/destructor
            ObjectContactOfObjListPainter(
                OutputDevice& rTargetDevice,
                const SdrObjectVector& rObjects,
                const SdrPage* pProcessedPage);
            virtual ~ObjectContactOfObjListPainter();

            // Process the whole displaying
            virtual void ProcessDisplay(DisplayInfo& rDisplayInfo);

            // VirtualDevice? Default is false
            virtual bool isOutputToVirtualDevice() const;

            // recording MetaFile? Default is false
            virtual bool isOutputToRecordingMetaFile() const;

            // pdf export? Default is false
            virtual bool isOutputToPDFFile() const;

            // access to OutputDevice. May return 0L like the default implementations do. Needs to be overloaded as needed.
            virtual OutputDevice* TryToGetOutputDevice() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ObjectContactOfPagePainter : public ObjectContactPainter
        {
        protected:
            // the original ObjectContact this painter is working on
            ObjectContact&                                  mrOriginalObjectContact;

            // Set StartPoint for next run, also given in constructor
            SdrPageWeakRef                                  mxStartPage;

            // Hierarchy access methods
            virtual sal_uInt32 GetPaintObjectCount() const;
            virtual ViewContact& GetPaintObjectViewContact(sal_uInt32 nIndex) const;

        public:
            // basic constructor
            ObjectContactOfPagePainter(
                const SdrPage* pPage,
                ObjectContact& rOriginalObjectContact);
            virtual ~ObjectContactOfPagePainter();

            // set another page
            void SetStartPage(const SdrPage* pPage);
            const SdrPage* GetStartPage() const { return mxStartPage.get(); }

            // access to OutputDevice. May return 0L like the default implementations do. Needs to be overloaded as needed.
            virtual OutputDevice* TryToGetOutputDevice() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_OBJECTCONTACTOFOBJLISTPAINTER_HXX

// eof
