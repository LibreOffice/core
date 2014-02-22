/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svgio/svgreader/svgtspannode.hxx>



namespace svgio
{
    namespace svgreader
    {
        SvgTspanNode::SvgTspanNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenTspan, rDocument, pParent),
            maSvgStyleAttributes(*this),
            maSvgTextPositions()
        {
        }

        SvgTspanNode::~SvgTspanNode()
        {
        }

        const SvgStyleAttributes* SvgTspanNode::getSvgStyleAttributes() const
        {
            return &maSvgStyleAttributes;
        }

        void SvgTspanNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
        {
            
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            
            maSvgStyleAttributes.parseStyleAttribute(rTokenName, aSVGToken, aContent);

            
            maSvgTextPositions.parseTextPositionAttributes(rTokenName, aSVGToken, aContent);

            
            switch(aSVGToken)
            {
                case SVGTokenStyle:
                {
                    maSvgStyleAttributes.readStyle(aContent);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

    } 
} 




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
