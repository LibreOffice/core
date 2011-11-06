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



#ifndef _XMLOFF_VISAREAEXPORT_HXX
#define _XMLOFF_VISAREAEXPORT_HXX

#include <tools/mapunit.hxx>
#include <sal/types.h>

class SvXMLExport;
class Rectangle;

namespace com { namespace sun { namespace star { namespace awt {
    struct Rectangle;
} } } }

class XMLVisAreaExport
{
public:
    // the complete export is done in the constructor
    XMLVisAreaExport(SvXMLExport& rExport, const sal_Char *pName,
            const Rectangle& aRect, const MapUnit aMapUnit);

    XMLVisAreaExport(SvXMLExport& rExport, const sal_Char *pName,
                    const com::sun::star::awt::Rectangle& aRect, const sal_Int16 nMeasureUnit );

    ~XMLVisAreaExport();
};

#endif

