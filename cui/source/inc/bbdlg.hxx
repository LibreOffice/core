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


#ifndef _SVX_BBDLG_HXX
#define _SVX_BBDLG_HXX

// include ---------------------------------------------------------------

#include <sfx2/tabdlg.hxx>

/*--------------------------------------------------------------------
    Beschreibung:   Border Background Pages buendeln
 --------------------------------------------------------------------*/

class SvxBorderBackgroundDlg: public SfxTabDialog
{
public:
    SvxBorderBackgroundDlg(Window *pParent,
        const SfxItemSet& rCoreSet,
        bool bEnableSelector = false,
        bool bEnableDrawingLayerFillStyles = false);

    ~SvxBorderBackgroundDlg();

protected:
    virtual void    PageCreated(sal_uInt16 nPageId,SfxTabPage& rTabPage);

private:
    /// bitfield
    bool        mbEnableBackgroundSelector : 1;         // fuer Border/Background-Dlg
    bool        mbEnableDrawingLayerFillStyles : 1;     // for full DrawingLayer FillStyles
};


#endif

