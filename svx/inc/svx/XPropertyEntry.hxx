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

#ifndef _XPROPERTYENTRY_HXX
#define _XPROPERTYENTRY_HXX

#include <svx/svxdllapi.h>
#include <tools/string.hxx>
#include <vcl/bitmap.hxx>

// ---------------------
// class XPropertyEntry

class SVX_DLLPUBLIC XPropertyEntry
{
private:
    String  maName;
    Bitmap  maUiBitmap;

protected:
    XPropertyEntry(const String& rName);
    XPropertyEntry(const XPropertyEntry& rOther);

public:
    virtual ~XPropertyEntry();

    void SetName(const String& rName) { maName = rName; }
    const String& GetName() const { return maName; }
    void SetUiBitmap(const Bitmap& rUiBitmap) { maUiBitmap = rUiBitmap; }
    const Bitmap& GetUiBitmap() const { return maUiBitmap; }
};

#endif // _XPROPERTYENTRY_HXX
