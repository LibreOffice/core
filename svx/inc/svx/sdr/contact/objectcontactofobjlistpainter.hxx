/*************************************************************************
 *
 *  $RCSfile: objectcontactofobjlistpainter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:20:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        typedef ::std::vector< SdrObject* > SdrObjectVector;

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
