/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: taborder.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 15:01:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX

#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLERMODEL_HPP_
#include <com/sun/star/awt/XTabControllerModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _TOOLS_LIST_HXX
#include <tools/list.hxx>
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

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
        TabOrderListBox( Window* pParent );
        TabOrderListBox( Window* pParent, const ResId& rResId  );
        virtual ~TabOrderListBox();

        void            MoveSelection( long nRelPos );
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
