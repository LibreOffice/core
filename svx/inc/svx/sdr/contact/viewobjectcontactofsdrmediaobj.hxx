/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewobjectcontactofsdrmediaobj.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:05:53 $
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

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTOFSDRMEDIAOBJ_HXX
#define _SDR_CONTACT_VIEWOBJECTCONTACTOFSDRMEDIAOBJ_HXX

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

namespace avmedia { class MediaItem; }
class Window;

namespace sdr
{
    namespace contact
    {
        class SdrMediaWindow;

        class ViewObjectContactOfSdrMediaObj : public ViewObjectContact
        {
        public:

            ViewObjectContactOfSdrMediaObj( ObjectContact& rObjectContact,
                                            ViewContact& rViewContact,
                                            const ::avmedia::MediaItem& rMediaItem );

            // The destructor. When PrepareDelete() was not called before (see there)
            // warnings will be generated in debug version if there are still contacts
            // existing.
            virtual ~ViewObjectContactOfSdrMediaObj();

        public:

            Window* getWindow() const;

            bool    hasPreferredSize() const;
            Size    getPreferredSize() const;

            void    updateMediaItem( ::avmedia::MediaItem& rItem ) const;
            void    executeMediaItem( const ::avmedia::MediaItem& rItem );

        protected:

            // Prepare deletion of this object. This needs to be called always
            // before really deleting this objects. This is necessary since in a c++
            // destructor no virtual function calls are allowed. To avoid this problem,
            // it is required to first call PrepareDelete().
            virtual void PrepareDelete();

            // Paint this object. This is before evtl. SubObjects get painted. This method
            // needs to set the flag mbIsPainted and mbIsInvalidated and to set the
            // maPaintedRectangle member. This information is later used for invalidates
            // and repaints.
            virtual void PaintObject(DisplayInfo& rDisplayInfo);

        private:

            ::sdr::contact::SdrMediaWindow* mpMediaWindow;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_CONTACT_VIEWOBJECTCONTACTOFSDRMEDIAOBJ_HXX
