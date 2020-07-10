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

#ifndef INCLUDED_SVX_INC_SDR_CONTACT_VIEWOBJECTCONTACTOFSDRPAGE_HXX
#define INCLUDED_SVX_INC_SDR_CONTACT_VIEWOBJECTCONTACTOFSDRPAGE_HXX

#include <svx/sdr/contact/viewobjectcontact.hxx>

class SdrPage;

namespace sdr::contact
    {
        class ViewObjectContactOfPageSubObject : public ViewObjectContact
        {
        protected:
            const SdrPage& getPage() const;

        public:
            ViewObjectContactOfPageSubObject(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageSubObject() override;

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const override;
            virtual bool isPrimitiveGhosted(const DisplayInfo& rDisplayInfo) const override;
        };

} // end of namespace sdr::contact

namespace sdr::contact
    {
        class ViewObjectContactOfPageBackground final : public ViewObjectContactOfPageSubObject
        {
            virtual drawinglayer::primitive2d::Primitive2DContainer createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const override;

        public:
            ViewObjectContactOfPageBackground(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageBackground() override;

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const override;
        };

} // end of namespace sdr::contact

namespace sdr::contact
    {
        class ViewObjectContactOfMasterPage final : public ViewObjectContactOfPageSubObject
        {
        protected:
        public:
            ViewObjectContactOfMasterPage(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfMasterPage() override;

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const override;
        };
} // end of namespace sdr::contact

namespace sdr::contact
    {
        class ViewObjectContactOfPageFill final : public ViewObjectContactOfPageSubObject
        {
            virtual drawinglayer::primitive2d::Primitive2DContainer createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const override;

        public:
            ViewObjectContactOfPageFill(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageFill() override;

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const override;
        };
} // end of namespace sdr::contact

namespace sdr::contact
    {
        class ViewObjectContactOfPageShadow final : public ViewObjectContactOfPageSubObject
        {
        public:
            ViewObjectContactOfPageShadow(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageShadow() override;

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const override;
        };
} // end of namespace sdr::contact

namespace sdr::contact
    {
        class ViewObjectContactOfOuterPageBorder final : public ViewObjectContactOfPageSubObject
        {
        public:
            ViewObjectContactOfOuterPageBorder(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfOuterPageBorder() override;

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const override;
        };
} // end of namespace sdr::contact

namespace sdr::contact
    {
        class ViewObjectContactOfInnerPageBorder final : public ViewObjectContactOfPageSubObject
        {
        public:
            ViewObjectContactOfInnerPageBorder(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfInnerPageBorder() override;

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const override;
        };
} // end of namespace sdr::contact

namespace sdr::contact
    {
        class ViewObjectContactOfPageHierarchy final : public ViewObjectContactOfPageSubObject
        {
        public:
            ViewObjectContactOfPageHierarchy(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageHierarchy() override;

            virtual drawinglayer::primitive2d::Primitive2DContainer getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const override;
        };
} // end of namespace sdr::contact

namespace sdr::contact
    {
        class ViewObjectContactOfPageGrid final : public ViewObjectContactOfPageSubObject
        {
            virtual drawinglayer::primitive2d::Primitive2DContainer createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const override;

        public:
            ViewObjectContactOfPageGrid(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageGrid() override;

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const override;
        };
} // end of namespace sdr::contact

namespace sdr::contact
    {
        class ViewObjectContactOfPageHelplines final : public ViewObjectContactOfPageSubObject
        {
            virtual drawinglayer::primitive2d::Primitive2DContainer createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const override;

        public:
            ViewObjectContactOfPageHelplines(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfPageHelplines() override;

            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const override;
        };
} // end of namespace sdr::contact

namespace sdr::contact
    {
        class ViewObjectContactOfSdrPage final : public ViewObjectContact
        {
        public:
            ViewObjectContactOfSdrPage(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContactOfSdrPage() override;

            virtual drawinglayer::primitive2d::Primitive2DContainer getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const override;
        };
} // end of namespace sdr::contact

#endif // INCLUDED_SVX_INC_SDR_CONTACT_VIEWOBJECTCONTACTOFSDRPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
