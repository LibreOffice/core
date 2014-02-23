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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGDOCUMENT_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGDOCUMENT_HXX

#include <boost/utility.hpp>
#include <svgio/svgreader/svgnode.hxx>



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
            const OUString     maAbsolutePath;

            /// hash mapper to find nodes by their id
            typedef boost::unordered_map< const OUString, const SvgNode*,
                      OUStringHash,
                      ::std::equal_to< OUString > > IdTokenMapper;
            typedef std::pair< const OUString, const SvgNode* > IdTokenValueType;
            IdTokenMapper           maIdTokenMapperList;

            /// hash mapper to find css styles by their id
            typedef boost::unordered_map< const OUString, const SvgStyleAttributes*, OUStringHash, ::std::equal_to< OUString > > IdStyleTokenMapper;
            typedef std::pair< const OUString, const SvgStyleAttributes* > IdStyleTokenValueType;
            IdStyleTokenMapper      maIdStyleTokenMapperList;

        public:
            SvgDocument(const OUString& rAbsolutePath);
            ~SvgDocument();

            /// append anopther root node, ownership changes
            void appendNode(SvgNode* pNode);

            /// add/remove nodes with Id to mapper
            void addSvgNodeToMapper(const OUString& rStr, const SvgNode& rNode);
            void removeSvgNodeFromMapper(const OUString& rStr);

            /// find a node by it's Id
            bool hasSvgNodesById() const { return !maIdTokenMapperList.empty(); }
            const SvgNode* findSvgNodeById(const OUString& rStr) const;

            /// add/remove styles to mapper
            void addSvgStyleAttributesToMapper(const OUString& rStr, const SvgStyleAttributes& rSvgStyleAttributes);

            /// find a style by it's Id
            bool hasSvgStyleAttributesById() const { return !maIdStyleTokenMapperList.empty(); }
            const SvgStyleAttributes* findSvgStyleAttributesById(const OUString& rStr) const;

            /// data read access
            const SvgNodeVector& getSvgNodeVector() const { return maNodes; }
            const OUString& getAbsolutePath() const { return maAbsolutePath; }
        };
    } // end of namespace svgreader
} // end of namespace svgio



#endif //INCLUDED_SVGIO_SVGREADER_SVGDOCUMENT_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
