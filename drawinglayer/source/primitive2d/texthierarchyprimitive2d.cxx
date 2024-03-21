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

#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        TextHierarchyLinePrimitive2D::TextHierarchyLinePrimitive2D(Primitive2DContainer&& aChildren)
        :   GroupPrimitive2D(std::move(aChildren))
        {
        }

        // provide unique ID
        sal_uInt32 TextHierarchyLinePrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_TEXTHIERARCHYLINEPRIMITIVE2D;
        }


        TextHierarchyParagraphPrimitive2D::TextHierarchyParagraphPrimitive2D(
            Primitive2DContainer&& aChildren,
            sal_Int16 nOutlineLevel)
        :   GroupPrimitive2D(std::move(aChildren)),
            mnOutlineLevel(nOutlineLevel)
        {
        }

        bool TextHierarchyParagraphPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const TextHierarchyParagraphPrimitive2D& rCompare = static_cast<const TextHierarchyParagraphPrimitive2D&>(rPrimitive);

                return (getOutlineLevel() == rCompare.getOutlineLevel());
            }

            return false;
        }

        // provide unique ID
        sal_uInt32 TextHierarchyParagraphPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D;
        }



        TextHierarchyBulletPrimitive2D::TextHierarchyBulletPrimitive2D(Primitive2DContainer&& aChildren)
        :   GroupPrimitive2D(std::move(aChildren))
        {
        }

        // provide unique ID
        sal_uInt32 TextHierarchyBulletPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_TEXTHIERARCHYBULLETPRIMITIVE2D;
        }


        TextHierarchyBlockPrimitive2D::TextHierarchyBlockPrimitive2D(Primitive2DContainer&& aChildren)
        :   GroupPrimitive2D(std::move(aChildren))
        {
        }

        // provide unique ID
        sal_uInt32 TextHierarchyBlockPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D;
        }


        TextHierarchyFieldPrimitive2D::TextHierarchyFieldPrimitive2D(
            Primitive2DContainer&& aChildren,
            const FieldType& rFieldType,
            const std::vector< std::pair< OUString, OUString>>* pNameValue)
        :   GroupPrimitive2D(std::move(aChildren)),
            meType(rFieldType)
        {
            if (nullptr != pNameValue)
            {
                meNameValue = *pNameValue;
            }
        }

        OUString TextHierarchyFieldPrimitive2D::getValue(const OUString& rName) const
        {
            for (const std::pair< OUString, OUString >& candidate : meNameValue)
            {
                if (candidate.first.equals(rName))
                {
                    return candidate.second;
                }
            }

            return OUString();
        }

        bool TextHierarchyFieldPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const TextHierarchyFieldPrimitive2D& rCompare = static_cast<const TextHierarchyFieldPrimitive2D&>(rPrimitive);

                return (getType() == rCompare.getType()
                    && meNameValue == rCompare.meNameValue);
            }

            return false;
        }

        // provide unique ID
        sal_uInt32 TextHierarchyFieldPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_TEXTHIERARCHYFIELDPRIMITIVE2D;
        }


        TextHierarchyEditPrimitive2D::TextHierarchyEditPrimitive2D(Primitive2DContainer&& aContent)
        :   GroupPrimitive2D(std::move(aContent))
        {
        }

        void TextHierarchyEditPrimitive2D::get2DDecomposition(
            Primitive2DDecompositionVisitor& rVisitor,
            const geometry::ViewInformation2D& rViewInformation) const
        {
            // check if TextEdit is active. If not, process. If yes, suppress the content
            if (!rViewInformation.getTextEditActive())
                GroupPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
        }

        // provide unique ID
        sal_uInt32 TextHierarchyEditPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_TEXTHIERARCHYEDITPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
