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

#include <svgio/svgreader/svgdocument.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        SvgDocument::SvgDocument(const rtl::OUString& rAbsolutePath)
        :   maNodes(),
            maAbsolutePath(rAbsolutePath),
            maIdTokenMapperList(),
            maIdStyleTokenMapperList()
        {
        }

        SvgDocument::~SvgDocument()
        {
            while(!maNodes.empty())
            {
                SvgNode* pCandidate = maNodes[maNodes.size() - 1];
                delete pCandidate;
                maNodes.pop_back();
            }
        }

        void SvgDocument::appendNode(SvgNode* pNode)
        {
            OSL_ENSURE(pNode, "OOps, empty node added (!)");
            maNodes.push_back(pNode);
        }

        void SvgDocument::addSvgNodeToMapper(const rtl::OUString& rStr, const SvgNode& rNode)
        {
            if(rStr.getLength())
            {
                maIdTokenMapperList.insert(IdTokenValueType(rStr, &rNode));
            }
        }

        void SvgDocument::removeSvgNodeFromMapper(const rtl::OUString& rStr)
        {
            if(rStr.getLength())
            {
                maIdTokenMapperList.erase(rStr);
            }
        }

        const SvgNode* SvgDocument::findSvgNodeById(const rtl::OUString& rStr) const
        {
            const IdTokenMapper::const_iterator aResult(maIdTokenMapperList.find(rStr));

            if(aResult == maIdTokenMapperList.end())
            {
                return 0;
            }
            else
            {
                return aResult->second;
            }
        }

        void SvgDocument::addSvgStyleAttributesToMapper(const rtl::OUString& rStr, const SvgStyleAttributes& rSvgStyleAttributes)
        {
            if(rStr.getLength())
            {
                maIdStyleTokenMapperList.insert(IdStyleTokenValueType(rStr, &rSvgStyleAttributes));
            }
        }

        const SvgStyleAttributes* SvgDocument::findSvgStyleAttributesById(const rtl::OUString& rStr) const
        {
            const IdStyleTokenMapper::const_iterator aResult(maIdStyleTokenMapperList.find(rStr));

            if(aResult == maIdStyleTokenMapperList.end())
            {
                return 0;
            }
            else
            {
                return aResult->second;
            }
        }

    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
