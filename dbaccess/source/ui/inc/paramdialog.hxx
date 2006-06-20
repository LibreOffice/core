/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: paramdialog.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:18:14 $
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

#ifndef _DBAUI_PARAMDIALOG_HXX_
#define _DBAUI_PARAMDIALOG_HXX_

#ifndef _DBAUI_COMMON_TYPES_HXX_
#include "commontypes.hxx"
#endif

#ifndef _DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef CONNECTIVITY_PREDICATEINPUT_HXX
#include <connectivity/predicateinput.hxx>
#endif
#ifndef SVX_QUERYDESIGNCONTEXT_HXX
#include "svx/ParseContext.hxx"
#endif

namespace connectivity
{
    class OSQLParseNode;
}

//.........................................................................
namespace dbaui
{
//.........................................................................

    //==================================================================
    //= OParameterDialog
    //==================================================================
    class OParameterDialog
            :public ModalDialog
            ,public ::svxform::OParseContextClient
    {
    protected:
        // the controls
        FixedLine       m_aNamesFrame;
        ListBox         m_aAllParams;
        FixedText       m_aValueFrame;
        Edit            m_aParam;
        PushButton      m_aTravelNext;
        OKButton        m_aOKBtn;
        CancelButton    m_aCancelBtn;

        sal_uInt16          m_nCurrentlySelected;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >
                        m_xParams;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                        m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >
                        m_xFormatter;
        ::dbtools::OPredicateInputController
                        m_aPredicateInput;

        ByteVector      m_aVisitedParams;
        Timer       m_aResetVisitFlag;
            // we reset the "visited flag" 1 second after and entry has been selected

        sal_Bool    m_bNeedErrorOnCurrent;

        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                    m_aFinalValues;     /// the final values as entered by the user

    public:
        OParameterDialog(Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > & _rParamContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > & _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
        ~OParameterDialog();

        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                    getValues() const { return m_aFinalValues; }


    protected:
        void Construct();

        ::connectivity::OSQLParseNode* implPredicateTree(::rtl::OUString& _rErrorMessage, const UniString& _rStatement, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _rxField);

    private:
        DECL_LINK(OnVisitedTimeout, Timer*);
        DECL_LINK(OnValueModified, Control*);
        DECL_LINK(OnEntrySelected, ListBox*);
        DECL_LINK(OnButtonClicked, PushButton*);
        DECL_LINK(OnValueLoseFocus, Control*);
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_PARAMDIALOG_HXX_

