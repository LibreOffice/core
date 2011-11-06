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


#ifndef _XMLOFF_XMLTEXTCOLUMNSEXPORT_HXX
#define _XMLOFF_XMLTEXTCOLUMNSEXPORT_HXX

namespace com { namespace sun { namespace star { namespace uno {
    class Any; } } } }
class SvXMLExport;

class XMLTextColumnsExport
{
    SvXMLExport&        rExport;

    const ::rtl::OUString sSeparatorLineIsOn;
    const ::rtl::OUString sSeparatorLineWidth;
    const ::rtl::OUString sSeparatorLineColor;
    const ::rtl::OUString sSeparatorLineRelativeHeight;
    const ::rtl::OUString sSeparatorLineVerticalAlignment;
    const ::rtl::OUString sIsAutomatic;
    const ::rtl::OUString sAutomaticDistance;

protected:

    SvXMLExport& GetExport() { return rExport; }

public:

    XMLTextColumnsExport( SvXMLExport& rExport );

    void exportXML( const ::com::sun::star::uno::Any& rAny );
};


#endif
