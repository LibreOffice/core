/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objectcontactofobjlistpainter.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:04:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SDR_CONTACT_OBJECTCONTACTOFOBJLISTPAINTER_HXX
#define _SDR_CONTACT_OBJECTCONTACTOFOBJLISTPAINTER_HXX

#ifndef _SDR_CONTACT_OBJECTCONTACT_HXX
#include <svx/sdr/contact/objectcontact.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
