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

//========================================================================
// class SvxColorWindow_Impl --------------------------------------------------
//========================================================================
#ifndef __SVX_COLORWINDOW_HXX_
#define __SVX_COLORWINDOW_HXX_

#include <sfx2/tbxctrl.hxx>
#include <svl/lstner.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <svx/SvxColorValueSet.hxx>

//========================================================================
// class SvxColorWindow_Impl --------------------------------------------------
//========================================================================

class SvxColorWindow_Impl : public SfxPopupWindow
{
    using FloatingWindow::StateChanged;

private:
    const sal_uInt16        theSlotId;
    SvxColorValueSet        aColorSet;
    rtl::OUString           maCommand;

#if _SOLAR__PRIVATE
    DECL_LINK( SelectHdl, void * );
#endif

protected:
    virtual void    Resize();
    virtual sal_Bool    Close();

public:
    SvxColorWindow_Impl( const rtl::OUString& rCommand,
                         sal_uInt16 nSlotId,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                         const String& rWndTitle,
                         Window* pParentWindow );
    ~SvxColorWindow_Impl();
    void                StartSelection();

    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );

    virtual SfxPopupWindow* Clone() const;
};

#endif
