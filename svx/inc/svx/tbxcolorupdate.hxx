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



#ifndef SVX_TBXCOLORUPDATE_HXX
#define SVX_TBXCOLORUPDATE_HXX

#include <svx/svxdllapi.h>

#include <tools/gen.hxx>
#include <tools/color.hxx>

class ToolBox;
class VirtualDevice;

//........................................................................
namespace svx
{
//........................................................................

#define TBX_UPDATER_MODE_NONE               0x00
#define TBX_UPDATER_MODE_CHAR_COLOR         0x01
#define TBX_UPDATER_MODE_CHAR_BACKGROUND    0x02
#define TBX_UPDATER_MODE_CHAR_COLOR_NEW     0x03

    //====================================================================
    //= ToolboxButtonColorUpdater
    //====================================================================
    /** helper class to update a color in a toolbox button image

        formerly known as SvxTbxButtonColorUpdater_Impl, residing in svx/source/tbxctrls/colorwindow.hxx.
    */
    class SVX_DLLPUBLIC ToolboxButtonColorUpdater
    {
    public:
                    ToolboxButtonColorUpdater( sal_uInt16   nSlotId,
                                                sal_uInt16   nTbxBtnId,
                                                ToolBox* ptrTbx,
                                                sal_uInt16   nMode = 0 );
                    ~ToolboxButtonColorUpdater();

        void        Update( const Color& rColor );

    protected:
        void        DrawChar(VirtualDevice&, const Color&);

    private:
        sal_uInt16      mnDrawMode;
        sal_uInt16      mnBtnId;
        sal_uInt16      mnSlotId;
        ToolBox*    mpTbx;
        Color       maCurColor;
        Rectangle   maUpdRect;
        Size        maBmpSize;
        sal_Bool        mbWasHiContrastMode;
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_TBXCOLORUPDATE_HXX
