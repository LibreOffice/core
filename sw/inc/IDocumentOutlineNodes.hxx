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



#ifndef IDOCUMENTOUTLINENODES_HXX_INCLUDED
#define IDOCUMENTOUTLINENODES_HXX_INCLUDED

#include <sal/types.h>
#include <tools/string.hxx>
#include <vector>
class SwTxtNode;

/** Provides outline nodes of a document.
*/
class IDocumentOutlineNodes
{
public:
    typedef ::std::vector< const SwTxtNode* > tSortedOutlineNodeList;

    virtual sal_Int32 getOutlineNodesCount() const = 0;

    virtual int getOutlineLevel( const sal_Int32 nIdx ) const = 0;
    virtual String getOutlineText( const sal_Int32 nIdx,
                                   const bool bWithNumber = true,
                                   const bool bWithSpacesForLevel = false ) const = 0;
    virtual SwTxtNode* getOutlineNode( const sal_Int32 nIdx ) const = 0;

    virtual void getOutlineNodes( IDocumentOutlineNodes::tSortedOutlineNodeList& orOutlineNodeList ) const = 0;

protected:
    virtual ~IDocumentOutlineNodes() {};
};

 #endif // IDOCUMENTOUTLINENODES_HXX_INCLUDED
