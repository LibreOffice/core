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

#ifndef _SDR_MASTERPAGEDESCRIPTOR_HXX
#define _SDR_MASTERPAGEDESCRIPTOR_HXX

#include <svx/sdrpageuser.hxx>
#include <svx/svdsob.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrObject;
class SfxItemSet;
class SdrPageProperties;

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
    private:
        SdrPage&                                        maOwnerPage;
        SdrPage&                                        maUsedPage;
        SetOfByte                                       maVisibleLayers;

        // ViewContact part
        sdr::contact::ViewContact*                      mpViewContact;

        // assignment operator defined, but not implemented
        void operator=(const MasterPageDescriptor& rCandidate);

    protected:
        virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    public:
        MasterPageDescriptor(SdrPage& aOwnerPage, SdrPage& aUsedPage);
        virtual ~MasterPageDescriptor();

        // ViewContact part
        sdr::contact::ViewContact& GetViewContact() const;

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

        const SdrPageProperties* getCorrectSdrPageProperties() const;
    };
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_MASTERPAGEDESCRIPTOR_HXX
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
