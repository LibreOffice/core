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



#ifndef OOX_DRAWINGML_CLRSCHEME_HXX
#define OOX_DRAWINGML_CLRSCHEME_HXX

#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
#include "oox/drawingml/color.hxx"

namespace oox { namespace drawingml {

class ClrMap
{
    std::map < sal_Int32, sal_Int32 > maClrMap;

public:

    sal_Bool getColorMap( sal_Int32& nClrToken );
    void     setColorMap( sal_Int32 nClrToken, sal_Int32 nMappedClrToken );
};

typedef boost::shared_ptr< ClrMap > ClrMapPtr;

class ClrScheme
{
    std::map < sal_Int32, sal_Int32 > maClrScheme;

public:

    ClrScheme();
    ~ClrScheme();

    sal_Bool getColor( sal_Int32 nSchemeClrToken, sal_Int32& rColor ) const;
    void     setColor( sal_Int32 nSchemeClrToken, sal_Int32 nColor );
};

typedef boost::shared_ptr< ClrScheme > ClrSchemePtr;

} }

#endif  //  OOX_DRAWINGML_CLRSCHEME_HXX
