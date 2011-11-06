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




#ifndef _XMLOFF_XMLSECTIONFOOTNOTECONFIGEXPORT_HXX
#define _XMLOFF_XMLSECTIONFOOTNOTECONFIGEXPORT_HXX

#include <sal/types.h>

#include <vector>

class SvXMLExport;
class XMLPropertySetMapper;
struct XMLPropertyState;
template<class X> class UniReference;

/**
 * Export the footnote-/endnote-configuration element in section styles.
 *
 * Because this class contains only one method, and all information is
 * available during that method call, we simply make it static.
 */
class XMLSectionFootnoteConfigExport
{

public:
    static void exportXML(
        SvXMLExport& rExport,
        sal_Bool bEndnote,
        const ::std::vector<XMLPropertyState> * pProperties,
        sal_uInt32 nIdx,
        const UniReference<XMLPropertySetMapper> & rMapper);    /// used only for debugging
};

#endif
