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

#include <drawinglayer/primitive2d/structuretagprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <vcl/pdfwriter.hxx>

using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        StructureTagPrimitive2D::StructureTagPrimitive2D(
            const vcl::pdf::StructElement& rStructureElement,
            bool bBackground,
            bool bIsImage,
            bool bIsDecorative,
            Primitive2DContainer&& aChildren,
            void const*const pAnchorStructureElementKey,
            ::std::vector<sal_Int32> const*const pAnnotIds)
        :   GroupPrimitive2D(std::move(aChildren)),
            maStructureElement(rStructureElement),
            mbBackground(bBackground),
            mbIsImage(bIsImage),
            mbIsDecorative(bIsDecorative)
        ,   m_pAnchorStructureElementKey(pAnchorStructureElementKey)
        {
            if (pAnnotIds)
            {
                m_AnnotIds = *pAnnotIds;
            }
        }

        bool StructureTagPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const StructureTagPrimitive2D& rCompare = static_cast<const StructureTagPrimitive2D&>(rPrimitive);

                return (getStructureElement() == rCompare.getStructureElement()
                        && isBackground() == rCompare.isBackground()
                        && isImage() == rCompare.isImage()
                        && isDecorative() == rCompare.isDecorative());
            }

            return false;
        }

        bool StructureTagPrimitive2D::opensStructureElement() const
        {
            return !isDecorative() && !isBackground()
                   && vcl::pdf::StructElement::NonStructElement != getStructureElement();
        }

        // provide unique ID
        sal_uInt32 StructureTagPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_STRUCTURETAGPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
