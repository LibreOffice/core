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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svgio.hxx"

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

        void SvgDocument::removeSvgStyleAttributesFromMapper(const rtl::OUString& rStr)
        {
            if(rStr.getLength())
            {
                maIdStyleTokenMapperList.erase(rStr);
            }
        }

        const SvgStyleAttributes* SvgDocument::findGlobalCssStyleAttributes(const rtl::OUString& rStr) const
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
