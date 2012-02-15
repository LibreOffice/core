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



#ifndef _SD_DIACTRL_HXX
#define _SD_DIACTRL_HXX

#include "dlgctrls.hxx"
#include <svl/intitem.hxx>
#include <sfx2/bindings.hxx>
#include <svx/itemwin.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/toolbox.hxx>
#include <sfx2/tbxctrl.hxx>
#include <com/sun/star/frame/XFrame.hpp>

//========================================================================
// SdPagesField:

class SdPagesField : public SvxMetricField
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
protected:
    virtual void    Modify();

public:
                    SdPagesField( Window* pParent,
                                  const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                  WinBits nBits = WB_BORDER | WB_SPIN | WB_REPEAT );
                    ~SdPagesField();

    void            UpdatePagesField( const SfxUInt16Item* pItem );
};

//========================================================================
// SdTbxCtlDiaPages:
//========================================================================

class SdTbxCtlDiaPages : public SfxToolBoxControl
{
public:
    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );

    SFX_DECL_TOOLBOX_CONTROL();

    SdTbxCtlDiaPages( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SdTbxCtlDiaPages();
};

#endif // _SD_DIACTRL_HXX



