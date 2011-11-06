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



#ifndef _SFX_INETTBC_HXX
#define _SFX_INETTBC_HXX

// includes *****************************************************************
#include <tools/string.hxx>
#include <tools/urlobj.hxx>

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#include <svtools/acceleratorexecute.hxx>

#if _SOLAR__PRIVATE

#include <sfx2/tbxctrl.hxx>
class SvtURLBox;

class SfxURLToolBoxControl_Impl : public SfxToolBoxControl
{
private:
    ::svt::AcceleratorExecute*  pAccExec;

    SvtURLBox*              GetURLBox() const;
    void                    OpenURL( const String& rName, sal_Bool bNew ) const;

    DECL_LINK(              OpenHdl, void* );
    DECL_LINK(              SelectHdl, void* );
    DECL_LINK( WindowEventListener, VclSimpleEvent* );

    struct ExecuteInfo
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
        ::com::sun::star::util::URL                                                aTargetURL;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >  aArgs;
    };

    DECL_STATIC_LINK( SfxURLToolBoxControl_Impl, ExecuteHdl_Impl, ExecuteInfo* );

public:

                            SFX_DECL_TOOLBOX_CONTROL();

                            SfxURLToolBoxControl_Impl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox );
    virtual                 ~SfxURLToolBoxControl_Impl();

    virtual Window*         CreateItemWindow( Window* pParent );
    virtual void            StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
};

#endif

#endif

