/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
