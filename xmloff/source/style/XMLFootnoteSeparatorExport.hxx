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



#ifndef _XMLOFF_XMLFOOTNOTESEPARATOREXPORT_HXX
#define _XMLOFF_XMLFOOTNOTESEPARATOREXPORT_HXX

#include <sal/types.h>

class SvXMLExport;
class XMLPropertySetMapper;
struct XMLPropertyState;
template<class X> class UniReference;


#include <vector>


/**
 * export footnote separator element in page styles
 */
class XMLFootnoteSeparatorExport
{
    SvXMLExport& rExport;

public:

    XMLFootnoteSeparatorExport(SvXMLExport& rExp);

    ~XMLFootnoteSeparatorExport();

    void exportXML(
        const ::std::vector<XMLPropertyState> * pProperties,
        sal_uInt32 nIdx,
        /// used only for debugging
        const UniReference<XMLPropertySetMapper> & rMapper);
};

#endif
