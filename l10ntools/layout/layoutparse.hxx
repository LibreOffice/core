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



#ifndef LAYOUTPARSE_HXX
#define LAYOUTPARSE_HXX

#include "xmlparse.hxx"

class LayoutXMLFile : public XMLFile
{
    bool mMergeMode;

public:
    LayoutXMLFile( bool mergeMode );

    void SearchL10NElements( XMLParentNode *pCur, int pos = 0 );
    sal_Bool Write( ByteString &aFilename );
    void HandleElement( XMLElement* element );
    void InsertL10NElement( ByteString const& id, XMLElement* element );

    using XMLFile::InsertL10NElement;
    using XMLFile::Write;
};

std::vector<XMLAttribute*> interestingAttributes( XMLAttributeList* lst );

#endif /* LAYOUTPARSE_HXX */
