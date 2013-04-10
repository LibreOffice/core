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



#ifndef _SVX_FORMAT_CELLS_DLG_HXX
#define _SVX_FORMAT_CELLS_DLG_HXX


#include <sfx2/tabdlg.hxx>

class XColorList;
class XGradientList;
class XHatchList;
class XBitmapList;
class SdrModel;

class SvxFormatCellsDialog : public SfxTabDialog
{
private:
    const SfxItemSet&   mrOutAttrs;

    XColorList*        mpColorTab;
    XGradientList*      mpGradientList;
    XHatchList*         mpHatchingList;
    XBitmapList*        mpBitmapList;

protected:
    virtual void Apply();

public:
    SvxFormatCellsDialog( Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel );
    ~SvxFormatCellsDialog();

    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

};

#endif // _SVX_FORMAT_CELLS_DLG_HXX


