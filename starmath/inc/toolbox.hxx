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


#ifndef TOOLBOX_HXX
#define TOOLBOX_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/childwin.hxx>
#include <vcl/toolbox.hxx>

#include "smmod.hxx"
#include "config.hxx"
#include "toolbox.hrc"

class SmToolBoxWindow : public SfxFloatingWindow
{

protected:
    ToolBox     aToolBoxCat;
    FixedLine   aToolBoxCat_Delim;  // to visualy separate the catalog part
    ToolBox    *pToolBoxCmd;
    ToolBox    *vToolBoxCategories[NUM_TBX_CATEGORIES];
    ImageList  *aImageLists [NUM_TBX_CATEGORIES + 1];   /* regular */
    ImageList  *aImageListsH[NUM_TBX_CATEGORIES + 1];   /* high contrast */
    sal_uInt16      nActiveCategoryRID;

    virtual sal_Bool    Close();
    virtual void    GetFocus();

    void            ApplyImageLists( sal_uInt16 nCategoryRID );

    DECL_LINK( CategoryClickHdl, ToolBox* );
    DECL_LINK( CmdSelectHdl, ToolBox* );

    SmViewShell * GetView();
    const ImageList * GetImageList( sal_uInt16 nResId, sal_Bool bHighContrast );

public:
    SmToolBoxWindow(SfxBindings    *pBindings,
                    SfxChildWindow *pChildWindow,
                    Window         *pParent);
    ~SmToolBoxWindow();

    // Window
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent &rEvt );

    void        AdjustPosSize( sal_Bool bSetPos );
    void        SetCategory(sal_uInt16 nCategory);
};

/**************************************************************************/

class SmToolBoxWrapper : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW(SmToolBoxWrapper);

protected:
    SmToolBoxWrapper(Window *pParentWindow,
                     sal_uInt16, SfxBindings*, SfxChildWinInfo*);
};

#endif

