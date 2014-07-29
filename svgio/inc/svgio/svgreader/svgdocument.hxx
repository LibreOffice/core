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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGDOCUMENT_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGDOCUMENT_HXX

#include <svgio/svgiodllapi.h>
#include <boost/utility.hpp>
#include <svgio/svgreader/svgnode.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgDocument : private boost::noncopyable
        {
        private:
            /// the document hierarchy with all root nodes
            SvgNodeVector           maNodes;

            /// the absolute path of the Svg file in progress (if available)
            const rtl::OUString     maAbsolutePath;

            /// hash mapper to find nodes by their id
            typedef std::hash_map< const rtl::OUString, const SvgNode*, rtl::OUStringHash > IdTokenMapper;
            typedef std::pair< const rtl::OUString, const SvgNode* > IdTokenValueType;
            IdTokenMapper           maIdTokenMapperList;

            /// hash mapper to find css styles by their id
            typedef std::hash_map< const rtl::OUString, const SvgStyleAttributes*, rtl::OUStringHash > IdStyleTokenMapper;
            typedef std::pair< const rtl::OUString, const SvgStyleAttributes* > IdStyleTokenValueType;
            IdStyleTokenMapper      maIdStyleTokenMapperList;

        public:
            SvgDocument(const rtl::OUString& rAbsolutePath);
            ~SvgDocument();

            /// append anopther root node, ownership changes
            void appendNode(SvgNode* pNode);

            /// add/remove nodes with Id to mapper
            void addSvgNodeToMapper(const rtl::OUString& rStr, const SvgNode& rNode);
            void removeSvgNodeFromMapper(const rtl::OUString& rStr);

            /// find a node by it's Id
            bool hasSvgNodesById() const { return !maIdTokenMapperList.empty(); }
            const SvgNode* findSvgNodeById(const rtl::OUString& rStr) const;

            /// add/remove styles to mapper
            void addSvgStyleAttributesToMapper(const rtl::OUString& rStr, const SvgStyleAttributes& rSvgStyleAttributes);
            void removeSvgStyleAttributesFromMapper(const rtl::OUString& rStr);

            /// find a style by it's Id
            bool hasGlobalCssStyleAttributes() const { return !maIdStyleTokenMapperList.empty(); }
            const SvgStyleAttributes* findGlobalCssStyleAttributes(const rtl::OUString& rStr) const;

            /// data read access
            const SvgNodeVector& getSvgNodeVector() const { return maNodes; }
            const rtl::OUString& getAbsolutePath() const { return maAbsolutePath; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGDOCUMENT_HXX

// eof
