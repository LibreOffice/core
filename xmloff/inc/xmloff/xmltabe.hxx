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



#ifndef _XMLOFF_XMLTABE_HXX
#define _XMLOFF_XMLTABE_HXX

// prevent funny things like "#define sun 1" from the compiler
#include <sal/config.h>

class SvXMLExport;
namespace com { namespace sun { namespace star {
    namespace style { struct TabStop; }
    namespace uno { class Any; }
} } }


class SvxXMLTabStopExport
{
    SvXMLExport& rExport;   // for access to document handler

protected:

    void exportTabStop( const ::com::sun::star::style::TabStop* pTabStop );

public:

    SvxXMLTabStopExport(  SvXMLExport& rExport );
    virtual ~SvxXMLTabStopExport();

    // core API
    void Export( const ::com::sun::star::uno::Any& rAny );
};


#endif  //  _XMLOFF_XMLTABE_HXX

