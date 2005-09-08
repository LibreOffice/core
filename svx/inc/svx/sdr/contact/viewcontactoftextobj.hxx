/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewcontactoftextobj.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:00:42 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFTEXTOBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFTEXTOBJ_HXX

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrTextObj;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfTextObj : public ViewContactOfSdrObj
        {
        protected:
            // internal access to SdrTextObj
            SdrTextObj& GetTextObj() const
            {
                return (SdrTextObj&)GetSdrObject();
            }

        public:
            // basic constructor, used from SdrObject.
            ViewContactOfTextObj(SdrTextObj& rTextObj);

            // The destructor. When PrepareDelete() was not called before (see there)
            // warnings will be generated in debug version if there are still contacts
            // existing.
            virtual ~ViewContactOfTextObj();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFTEXTOBJ_HXX

// eof
