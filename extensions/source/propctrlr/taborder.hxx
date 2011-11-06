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



#ifndef EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX

#include <svtools/svtreebx.hxx>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/lstbox.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _TOOLS_LIST_HXX
#include <tools/list.hxx>
#endif
#include <comphelper/uno3.hxx>

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= TabOrderListBox
    //========================================================================
    class TabOrderListBox : public SvTreeListBox
    {
    public:
        TabOrderListBox( Window* pParent, const ResId& rResId  );
        virtual ~TabOrderListBox();

        void            MoveSelection( long nRelPos );

    protected:
        virtual void    ModelHasMoved(SvListEntry* pSource );

    private:
        using SvTreeListBox::MoveSelection;
    };


    //========================================================================
    //= TabOrderDialog
    //========================================================================
    class TabOrderDialog : public ModalDialog
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >
                                    m_xTempModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >
                                    m_xModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
                                    m_xControlContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                    m_xORB;

        FixedText                   aFT_Controls;
        TabOrderListBox         aLB_Controls;

        OKButton                    aPB_OK;
        CancelButton                aPB_CANCEL;
        HelpButton                  aPB_HELP;

        PushButton                  aPB_MoveUp;
        PushButton                  aPB_MoveDown;
        PushButton                  aPB_AutoOrder;

        ImageList*                  pImageList;

        DECL_LINK( MoveUpClickHdl, Button* );
        DECL_LINK( MoveDownClickHdl, Button* );
        DECL_LINK( AutoOrderClickHdl, Button* );
        DECL_LINK( OKClickHdl, Button* );

        void FillList();
        Image GetImage(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _rxSet
        ) const;

    public:
        TabOrderDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >& _rxTabModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& _rxControlCont,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        virtual ~TabOrderDialog();

        void SetModified();
    };

//............................................................................
}  // namespace pcr
//............................................................................

#endif  // EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX
