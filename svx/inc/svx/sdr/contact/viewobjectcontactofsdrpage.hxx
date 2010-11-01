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

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTOFSDRPAGE_HXX
#define _SDR_CONTACT_VIEWOBJECTCONTACTOFSDRPAGE_HXX

#include <svx/sdr/contact/viewobjectcontact.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrPage;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContactOfPageSubObject : public ViewObjectContact
        {
        protected:
            const SdrPage& getPage() const;

        public:
            ViewObjectContactOfPageSubObject(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageSubObject();

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;
            virtual bool isPrimitiveGhosted(const DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContactOfPageBackground : public ViewObjectContactOfPageSubObject
        {
        protected:
            virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

        public:
            ViewObjectContactOfPageBackground(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageBackground();

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContactOfMasterPage : public ViewObjectContactOfPageSubObject
        {
        protected:
        public:
            ViewObjectContactOfMasterPage(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfMasterPage();

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContactOfPageFill : public ViewObjectContactOfPageSubObject
        {
        protected:
            virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

        public:
            ViewObjectContactOfPageFill(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageFill();

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContactOfPageShadow : public ViewObjectContactOfPageSubObject
        {
        public:
            ViewObjectContactOfPageShadow(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageShadow();

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContactOfOuterPageBorder : public ViewObjectContactOfPageSubObject
        {
        public:
            ViewObjectContactOfOuterPageBorder(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfOuterPageBorder();

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContactOfInnerPageBorder : public ViewObjectContactOfPageSubObject
        {
        public:
            ViewObjectContactOfInnerPageBorder(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfInnerPageBorder();

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContactOfPageHierarchy : public ViewObjectContactOfPageSubObject
        {
        public:
            ViewObjectContactOfPageHierarchy(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageHierarchy();

            virtual drawinglayer::primitive2d::Primitive2DSequence getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContactOfPageGrid : public ViewObjectContactOfPageSubObject
        {
        protected:
            virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

        public:
            ViewObjectContactOfPageGrid(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageGrid();

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContactOfPageHelplines : public ViewObjectContactOfPageSubObject
        {
        protected:
            virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

        public:
            ViewObjectContactOfPageHelplines(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageHelplines();

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContactOfSdrPage : public ViewObjectContact
        {
        public:
            ViewObjectContactOfSdrPage(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfSdrPage();

            virtual drawinglayer::primitive2d::Primitive2DSequence getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWOBJECTCONTACTOFSDRPAGE_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
