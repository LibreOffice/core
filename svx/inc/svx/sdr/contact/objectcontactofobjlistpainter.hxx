/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: objectcontactofobjlistpainter.hxx,v $
 * $Revision: 1.6 $
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

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class XOutputDevice;
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
            // Bitfield
            // Flag to remember if DrawHierarchy was built or not.
            // Inited with sal_False.
            unsigned                                        mbIsInitialized : 1;

            // Flag to allow/forbid buffering. Is set from constructor.
            unsigned                                        mbBufferingAllowed : 1;

            // Hierarchy access methods
            virtual sal_uInt32 GetPaintObjectCount() const = 0;
            virtual ViewContact& GetPaintObjectViewContact(sal_uInt32 nIndex) const = 0;

            // Update Draw Hierarchy data
            virtual void EnsureValidDrawHierarchy(DisplayInfo& rDisplayInfo);

        public:
            // basic constructor
            ObjectContactPainter(sal_Bool bBufferingAllowed);

            // The destructor.
            virtual ~ObjectContactPainter();

            // Process the whole displaying
            virtual void ProcessDisplay(DisplayInfo& rDisplayInfo);
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // typedef for transferring SdrObject
        typedef ::std::vector< const SdrObject* > SdrObjectVector;

        class SVX_DLLPUBLIC ObjectContactOfObjListPainter : public ObjectContactPainter
        {
        protected:
            // Set StartPoint for next run, also given in constructor
            SdrObjectVector                                 maStartObjects;

            // Hierarchy access methods
            virtual sal_uInt32 GetPaintObjectCount() const;
            virtual ViewContact& GetPaintObjectViewContact(sal_uInt32 nIndex) const;

        public:
            // basic constructor
            ObjectContactOfObjListPainter(const SdrObjectVector& rObjects,
                sal_Bool bBufferingAllowed = sal_False);

            // The destructor. When PrepareDelete() was not called before (see there)
            // warnings will be generated in debug version if there are still contacts
            // existing.
            virtual ~ObjectContactOfObjListPainter();
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
            // Set StartPoint for next run, also given in constructor
            const SdrPage*                                  mpStartPage;

            // Hierarchy access methods
            virtual sal_uInt32 GetPaintObjectCount() const;
            virtual ViewContact& GetPaintObjectViewContact(sal_uInt32 nIndex) const;

        public:
            // basic constructor
            ObjectContactOfPagePainter(const SdrPage* pPage,
                sal_Bool bBufferingAllowed = sal_False);

            // The destructor. When PrepareDelete() was not called before (see there)
            // warnings will be generated in debug version if there are still contacts
            // existing.
            virtual ~ObjectContactOfPagePainter();

            // set another page
            void SetStartPage(const SdrPage* pPage);
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_OBJECTCONTACTOFOBJLISTPAINTER_HXX

// eof
