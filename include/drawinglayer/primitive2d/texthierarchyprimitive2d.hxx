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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTHIERARCHYPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTHIERARCHYPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <rtl/ustring.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** TextHierarchyLinePrimitive2D class

            Text format hierarchy helper class. It decomposes to it's
            content, so all direct renderers may ignore it. If You need
            to know more about line hierarchies You may react on it and
            also need to take care that the source of data uses it.

            This primitive encapsulates text lines.
         */
        class DRAWINGLAYER_DLLPUBLIC TextHierarchyLinePrimitive2D : public GroupPrimitive2D
        {
        private:
        public:
            /// constructor
            explicit TextHierarchyLinePrimitive2D(const Primitive2DSequence& rChildren);

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** TextHierarchyBulletPrimitive2D class

            This primitive encapsulates text bullets.
         */
        class DRAWINGLAYER_DLLPUBLIC TextHierarchyBulletPrimitive2D : public GroupPrimitive2D
        {
        private:
        public:
            /// constructor
            explicit TextHierarchyBulletPrimitive2D(const Primitive2DSequence& rChildren);

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** TextHierarchyParagraphPrimitive2D class

            This primitive encapsulates text paragraphs.
         */
        class DRAWINGLAYER_DLLPUBLIC TextHierarchyParagraphPrimitive2D : public GroupPrimitive2D
        {
        private:
        public:
            /// constructor
            explicit TextHierarchyParagraphPrimitive2D(const Primitive2DSequence& rChildren);

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** TextHierarchyBlockPrimitive2D class

            This primitive encapsulates text blocks.
         */
        class DRAWINGLAYER_DLLPUBLIC TextHierarchyBlockPrimitive2D : public GroupPrimitive2D
        {
        private:
        public:
            /// constructor
            explicit TextHierarchyBlockPrimitive2D(const Primitive2DSequence& rChildren);

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** FieldType definition */
        enum FieldType
        {
            /** unspecified. If more info is needed for a FieldType,
                create a new type and it's handling
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
        class DRAWINGLAYER_DLLPUBLIC TextHierarchyFieldPrimitive2D : public GroupPrimitive2D
        {
        private:
            FieldType                               meType;
            OUString                           maString;

        public:
            /// constructor
            TextHierarchyFieldPrimitive2D(
                const Primitive2DSequence& rChildren,
                const FieldType& rFieldType,
                const OUString& rString);

            /// data read access
            FieldType getType() const { return meType; }
            const OUString& getString() const { return maString; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** TextHierarchyEditPrimitive2D class

            #i97628#
            Primitive to encapsulate text from an active text edit; some
            renderers need to suppress this output due to painting the
            edited text in e.g. an OutlinerEditView. It's derived from
            GroupPrimitive2D, so the implicit decomposition will use the
            content. To suppress, this primitive needs to be parsed by
            the renderer without taking any action.
         */
        class DRAWINGLAYER_DLLPUBLIC TextHierarchyEditPrimitive2D : public GroupPrimitive2D
        {
        private:
        public:
            /// constructor
            explicit TextHierarchyEditPrimitive2D(const Primitive2DSequence& rChildren);

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTHIERARCHYPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
