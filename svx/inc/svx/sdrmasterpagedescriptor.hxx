/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrmasterpagedescriptor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:10:46 $
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

#ifndef _SDR_MASTERPAGEDESCRIPTOR_HXX
#define _SDR_MASTERPAGEDESCRIPTOR_HXX

#ifndef _SDR_PAGEUSER_HXX
#include <svx/sdrpageuser.hxx>
#endif

#ifndef _SVDSOB_HXX //autogen
#include <svx/svdsob.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrObject;

namespace sdr
{
    namespace contact
    {
        class ViewContact;
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    class MasterPageDescriptor : public sdr::PageUser
    {
        SdrPage&                                        maOwnerPage;
        SdrPage&                                        maUsedPage;
        SetOfByte                                       maVisibleLayers;

        // ViewContact part
        sdr::contact::ViewContact*                      mpViewContact;
        virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

        // assignment operator defined, but not implemented
        void operator=(const MasterPageDescriptor& rCandidate);

    public:
        MasterPageDescriptor(SdrPage& aOwnerPage, SdrPage& aUsedPage);
        virtual ~MasterPageDescriptor();

        // ViewContact part
        virtual sdr::contact::ViewContact& GetViewContact() const;

        // this method is called form the destructor of the referenced page.
        // do all necessary action to forget the page. It is not necessary to call
        // RemovePageUser(), that is done form the destructor.
        virtual void PageInDestruction(const SdrPage& rPage);

        // member access to UsedPage
        SdrPage& GetUsedPage() const { return maUsedPage; }

        // member access to OwnerPage
        SdrPage& GetOwnerPage() const { return maOwnerPage; }

        // member access to VisibleLayers
        const SetOfByte& GetVisibleLayers() const { return maVisibleLayers; }
        void SetVisibleLayers(const SetOfByte& rNew);

        // operators
        sal_Bool operator==(const MasterPageDescriptor& rCandidate) const;
        sal_Bool operator!=(const MasterPageDescriptor& rCandidate) const;

        // #i42075# Get the correct BackgroundObject
        SdrObject* GetBackgroundObject() const;
    };
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_MASTERPAGEDESCRIPTOR_HXX
// eof
