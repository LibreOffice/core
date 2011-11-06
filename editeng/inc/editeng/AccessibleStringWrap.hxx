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



#ifndef _ACCESSIBLESTRINGWRAP_HXX
#define _ACCESSIBLESTRINGWRAP_HXX

#include <sal/types.h>
#include <tools/string.hxx>

#include <editeng/editengdllapi.h>

class OutputDevice;
class SvxFont;
class Rectangle;
class Point;

//------------------------------------------------------------------------
//
// AccessibleStringWrap declaration
//
//------------------------------------------------------------------------

class AccessibleStringWrap
{
public:

    EDITENG_DLLPUBLIC AccessibleStringWrap( OutputDevice& rDev, SvxFont& rFont, const String& rText );

    EDITENG_DLLPUBLIC sal_Bool GetCharacterBounds( sal_Int32 nIndex, Rectangle& rRect );
    EDITENG_DLLPUBLIC sal_Int32 GetIndexAtPoint( const Point& rPoint );

private:

    OutputDevice&       mrDev;
    SvxFont&            mrFont;
    String              maText;
};

#endif /* _ACCESSIBLESTRINGWRAP_HXX */
