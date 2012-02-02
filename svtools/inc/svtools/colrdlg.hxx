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

#ifndef _SV_COLRDLG_HXX
#define _SV_COLRDLG_HXX

#include "svtools/svtdllapi.h"
#include <tools/color.hxx>

class Window;

// ---------------
// - ColorDialog -
// ---------------

namespace svtools
{
    // SELECT is the default
    enum ColorPickerMode { ColorPickerMode_SELECT = 0, ColorPickerMode_ADD = 1, ColorPickerMode_MODIFY = 2 };
}

class SVT_DLLPUBLIC SvColorDialog
{
public:
    SvColorDialog( ::Window* pParent );

    void            SetColor( const Color& rColor );
    const Color&    GetColor() const;

    void            SetMode( sal_Int16 eMode );
    virtual short   Execute();

private:
    Window*         mpParent;
    Color           maColor;
    sal_Int16       meMode;
};

#endif  // _SV_COLRDLG_HXX
