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

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDRPAGE_HXX
#define _SDR_CONTACT_VIEWCONTACTOFSDRPAGE_HXX

#include <sal/types.h>
#include <svx/sdr/contact/viewcontact.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrPage;

namespace sdr { namespace contact {
    class ViewContactOfSdrPage;
}}

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfPageSubObject : public ViewContact
        {
        protected:
            ViewContactOfSdrPage&                       mrParentViewContactOfSdrPage;

        public:
            ViewContactOfPageSubObject(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
            virtual ~ViewContactOfPageSubObject();

            virtual ViewContact* GetParentContact() const;
            const SdrPage& getPage() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfPageBackground : public ViewContactOfPageSubObject
        {
        protected:
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            ViewContactOfPageBackground(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
            virtual ~ViewContactOfPageBackground();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfPageShadow : public ViewContactOfPageSubObject
        {
        protected:
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            ViewContactOfPageShadow(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
            virtual ~ViewContactOfPageShadow();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfPageFill : public ViewContactOfPageSubObject
        {
        protected:
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            ViewContactOfPageFill(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
            virtual ~ViewContactOfPageFill();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfMasterPage : public ViewContactOfPageSubObject
        {
        protected:
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            ViewContactOfMasterPage(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
            virtual ~ViewContactOfMasterPage();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfOuterPageBorder : public ViewContactOfPageSubObject
        {
        protected:
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            ViewContactOfOuterPageBorder(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
            virtual ~ViewContactOfOuterPageBorder();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfInnerPageBorder : public ViewContactOfPageSubObject
        {
        protected:
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            ViewContactOfInnerPageBorder(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
            virtual ~ViewContactOfInnerPageBorder();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfPageHierarchy : public ViewContactOfPageSubObject
        {
        protected:
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            ViewContactOfPageHierarchy(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
            virtual ~ViewContactOfPageHierarchy();

            virtual sal_uInt32 GetObjectCount() const;
            virtual ViewContact& GetViewContact(sal_uInt32 nIndex) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfGrid : public ViewContactOfPageSubObject
        {
        protected:
            // bitfield
            unsigned                                    mbFront : 1;

            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            ViewContactOfGrid(ViewContactOfSdrPage& rParentViewContactOfSdrPage, bool bFront);
            virtual ~ViewContactOfGrid();

            bool getFront() const { return mbFront; }
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfHelplines : public ViewContactOfPageSubObject
        {
        protected:
            // bitfield
            unsigned                                    mbFront : 1;

            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            ViewContactOfHelplines(ViewContactOfSdrPage& rParentViewContactOfSdrPage, bool bFront);
            virtual ~ViewContactOfHelplines();

            bool getFront() const { return mbFront; }
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfSdrPage : public ViewContact
        {
        protected:
            // the owner of this ViewContact. Set from constructor and not
            // to be changed in any way.
            SdrPage&                                        mrPage;

            // helper viewContacts to build a clear paint hierarchy
            ViewContactOfPageBackground                     maViewContactOfPageBackground;
            ViewContactOfPageShadow                         maViewContactOfPageShadow;
            ViewContactOfPageFill                           maViewContactOfPageFill;
            ViewContactOfMasterPage                         maViewContactOfMasterPage;
            ViewContactOfOuterPageBorder                    maViewContactOfOuterPageBorder;
            ViewContactOfInnerPageBorder                    maViewContactOfInnerPageBorder;
            ViewContactOfGrid                               maViewContactOfGridBack;
            ViewContactOfHelplines                          maViewContactOfHelplinesBack;
            ViewContactOfPageHierarchy                      maViewContactOfPageHierarchy;
            ViewContactOfGrid                               maViewContactOfGridFront;
            ViewContactOfHelplines                          maViewContactOfHelplinesFront;

            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something. Default is to create
            // a standard ViewObjectContact containing the given ObjectContact and *this
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

        public:
            // access to SdrObject
            SdrPage& GetSdrPage() const
            {
                return mrPage;
            }

            // basic constructor, used from SdrPage.
            ViewContactOfSdrPage(SdrPage& rObj);
            virtual ~ViewContactOfSdrPage();

            // Access to possible sub-hierarchy
            virtual sal_uInt32 GetObjectCount() const;
            virtual ViewContact& GetViewContact(sal_uInt32 nIndex) const;

            // React on changes of the object of this ViewContact
            virtual void ActionChanged();

            // overload for acessing the SdrPage
            virtual SdrPage* TryToGetSdrPage() const;

        protected:
            // This method is responsible for creating the graphical visualisation data
            // ONLY based on model data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFSDRPAGE_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
