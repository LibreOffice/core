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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTHIERARCHYPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTHIERARCHYPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <tools/string.hxx>

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
            DeclPrimitrive2DIDBlock()
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
            DeclPrimitrive2DIDBlock()
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
            DeclPrimitrive2DIDBlock()
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
            DeclPrimitrive2DIDBlock()
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
            rtl::OUString                           maString;

        public:
            /// constructor
            TextHierarchyFieldPrimitive2D(
                const Primitive2DSequence& rChildren,
                const FieldType& rFieldType,
                const rtl::OUString& rString);

            /// data read access
            FieldType getType() const { return meType; }
            const rtl::OUString& getString() const { return maString; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
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
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTHIERARCHYPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
