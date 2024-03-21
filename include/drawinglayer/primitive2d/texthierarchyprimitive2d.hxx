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

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <rtl/ustring.hxx>


namespace drawinglayer::primitive2d
{
        /** TextHierarchyLinePrimitive2D class

            Text format hierarchy helper class. It decomposes to its
            content, so all direct renderers may ignore it. If You need
            to know more about line hierarchies You may react on it and
            also need to take care that the source of data uses it.

            This primitive encapsulates text lines.
         */
        class DRAWINGLAYER_DLLPUBLIC TextHierarchyLinePrimitive2D final : public GroupPrimitive2D
        {
        private:
        public:
            /// constructor
            explicit TextHierarchyLinePrimitive2D(Primitive2DContainer&& aChildren);

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };

        /** TextHierarchyBulletPrimitive2D class

            This primitive encapsulates text bullets.
         */
        class DRAWINGLAYER_DLLPUBLIC TextHierarchyBulletPrimitive2D final : public GroupPrimitive2D
        {
        private:
        public:
            /// constructor
            explicit TextHierarchyBulletPrimitive2D(Primitive2DContainer&& aChildren);

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };

        /** TextHierarchyParagraphPrimitive2D class

            This primitive encapsulates text paragraphs.
         */
        class DRAWINGLAYER_DLLPUBLIC TextHierarchyParagraphPrimitive2D final : public GroupPrimitive2D
        {
        private:
            // outline level of the encapsulated paragraph data.
            // -1 means no level, >= 0 is the level
            sal_Int16           mnOutlineLevel;

        public:
            /// constructor
            explicit TextHierarchyParagraphPrimitive2D(
                Primitive2DContainer&& aChildren,
                sal_Int16 nOutlineLevel = -1);

            /// data read access
            sal_Int16 getOutlineLevel() const { return mnOutlineLevel; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };

        /** TextHierarchyBlockPrimitive2D class

            This primitive encapsulates text blocks.
         */
        class DRAWINGLAYER_DLLPUBLIC TextHierarchyBlockPrimitive2D final : public GroupPrimitive2D
        {
        private:
        public:
            /// constructor
            explicit TextHierarchyBlockPrimitive2D(Primitive2DContainer&& aChildren);

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };

        /** FieldType definition */
        enum FieldType
        {
            /** unspecified. If more info is needed for a FieldType,
                create a new type and its handling
             */
            FIELD_TYPE_COMMON,

            /** uses "FIELD_SEQ_BEGIN;PageField" -> special handling */
            FIELD_TYPE_PAGE,

            /** uses URL as string -> special handling */
            FIELD_TYPE_URL
        };

        /** TextHierarchyFieldPrimitive2D class

            This primitive encapsulates text fields.
            Also: This type uses a type enum to transport the encapsulated field
            type. Also added is a String which is type-dependent. E.g. for URL
            fields, it contains the URL.
         */
        class DRAWINGLAYER_DLLPUBLIC TextHierarchyFieldPrimitive2D final : public GroupPrimitive2D
        {
        private:
            /// field type definition
            FieldType                               meType;

            /// field data as name/value pairs (dependent of field type definition)
            std::vector< std::pair< OUString, OUString>>    meNameValue;

        public:
            /// constructor
            TextHierarchyFieldPrimitive2D(
                Primitive2DContainer&& aChildren,
                const FieldType& rFieldType,
                const std::vector< std::pair< OUString, OUString>>* pNameValue = nullptr);

            /// data read access
            FieldType getType() const { return meType; }
            OUString getValue(const OUString& rName) const;

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };

        /** TextHierarchyEditPrimitive2D class

            Primitive to encapsulate text from an active text edit; this is
            separate from other text data since some renderers need to suppress
            this output due to painting the edited text in e.g. an
            OutlinerEditView in the active text edit control.
            It now uses get2DDecomposition to decide if to process or not,
            thus it does not need to be processed in any B2DProcessor at all.
            This is also important e.g. for PDF export - if the object is in
            edit mode, we need to include the most current text from EditEngine/
            Outliner to that export which is contained here.
         */
        class DRAWINGLAYER_DLLPUBLIC TextHierarchyEditPrimitive2D final : public GroupPrimitive2D
        {
        public:
            /// constructor
            explicit TextHierarchyEditPrimitive2D(Primitive2DContainer&& aContent);

            /// local decomposition
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
