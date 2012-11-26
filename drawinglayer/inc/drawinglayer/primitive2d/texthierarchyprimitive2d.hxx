/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
            TextHierarchyLinePrimitive2D(const Primitive2DSequence& rChildren);

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
            TextHierarchyBulletPrimitive2D(const Primitive2DSequence& rChildren);

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
            TextHierarchyParagraphPrimitive2D(const Primitive2DSequence& rChildren);

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
            TextHierarchyBlockPrimitive2D(const Primitive2DSequence& rChildren);

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
        /** TextHierarchyFieldPrimitive2D class

            This primitive encapsulates text fields.
            Also: This type uses a type enum to transport the encapsulated field
            type. Also added is a String which is type-dependent. E.g. for URL
            fields, it contains the URL.
         */
        class DRAWINGLAYER_DLLPUBLIC TextHierarchyFieldPrimitive2D : public GroupPrimitive2D
        {
        public:
        /** FieldType definition */
        enum FieldType
        {
            /** unspecified. If more info is needed for a FieldType,
                create a new type and it's handling

                    maStringA and maStringB unused
             */
            FIELD_TYPE_COMMON,

                /** uses "FIELD_SEQ_BEGIN;PageField" -> special handling

                    maStringA and maStringB unused
                */
            FIELD_TYPE_PAGE,

                /** uses URL as string -> special handling

                    In maStringA: URL
                    In maStringB: TargetFrame
                */
            FIELD_TYPE_URL
        };

        private:
            FieldType                               meType;
            rtl::OUString                           maStringA;
            rtl::OUString                           maStringB;

        public:
            /// constructor
            TextHierarchyFieldPrimitive2D(
                const Primitive2DSequence& rChildren,
                const FieldType& rFieldType,
                const rtl::OUString& rStringA = rtl::OUString(),
                const rtl::OUString& rStringB = rtl::OUString());

            /// data read access
            FieldType getType() const { return meType; }
            const rtl::OUString& getStringA() const { return maStringA; }
            const rtl::OUString& getStringB() const { return maStringB; }

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
            TextHierarchyEditPrimitive2D(const Primitive2DSequence& rChildren);

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTHIERARCHYPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
