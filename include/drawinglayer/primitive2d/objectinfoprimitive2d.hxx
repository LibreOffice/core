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
#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_INFOHIERARCHYPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_INFOHIERARCHYPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <rtl/ustring.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** ObjectInfoPrimitive2D class

            Info hierarchy helper class to hold contents like Name, Title and
            Description which are valid for the child content, e.g. created for
            primitives based on DrawingLayer objects or SVG parts. It decomposes
            to it's content, so all direct renderers may ignore it. May e.g.
            be used when re-creating graphical content from a sequence of primitives
         */
        class DRAWINGLAYER_DLLPUBLIC ObjectInfoPrimitive2D : public GroupPrimitive2D
        {
        private:
            OUString                           maName;
            OUString                           maTitle;
            OUString                           maDesc;

        public:
            /// constructor
            ObjectInfoPrimitive2D(
                const Primitive2DSequence& rChildren,
                const OUString& rName,
                const OUString& rTitle,
                const OUString& rDesc);

            /// data read access
            const OUString& getName() const { return maName; }
            const OUString& getTitle() const { return maTitle; }
            const OUString& getDesc() const { return maDesc; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_INFOHIERARCHYPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
