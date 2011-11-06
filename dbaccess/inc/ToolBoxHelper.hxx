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



#ifndef DBAUI_TOOLBOXHELPER_HXX
#define DBAUI_TOOLBOXHELPER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#include "dbaccessdllapi.h"

class SvtMiscOptions;
class ToolBox;
class VclWindowEvent;

namespace dbaui
{
    class DBACCESS_DLLPUBLIC OToolBoxHelper
    {
        sal_Bool        m_bIsHiContrast;// true when the toolbox is in hi contrast mode
        sal_Int16       m_nSymbolsSize; // shows the toolbox large or small bitmaps
        ToolBox*        m_pToolBox;     // our toolbox (may be NULL)
    public:
        OToolBoxHelper();
        virtual ~OToolBoxHelper();

        /** will be called when the controls need to be resized.
            @param  _rDiff
                Contains the difference of the old and new toolbox size.
        */
        virtual void resizeControls(const Size& _rDiff) = 0;

        /** will be called when the image list is needed.
            @param  _eSymbolsSize
                <svtools/imgdef.hxx>
            @param  _bHiContast
                <TRUE/> when in high contrast mode.
        */
        virtual ImageList getImageList(sal_Int16 _eSymbolsSize,sal_Bool _bHiContast) const = 0;

        /** only the member will be set, derived classes can overload this function and do what need to be done.
            @param  _pTB
                The new ToolBox.
            @attention
                Must be called after a FreeResource() call.
        */
        virtual void    setToolBox(ToolBox* _pTB);

        inline ToolBox* getToolBox() const          { return m_pToolBox; }

        /** checks if the toolbox needs a new imagelist.
        */
        void checkImageList();

        inline sal_Bool isToolBoxHiContrast() const { return m_bIsHiContrast; }
    protected:
        DECL_LINK(ConfigOptionsChanged, SvtMiscOptions*);
        DECL_LINK(SettingsChanged, VclWindowEvent* );
    };
}
#endif // DBAUI_TOOLBOXHELPER_HXX

