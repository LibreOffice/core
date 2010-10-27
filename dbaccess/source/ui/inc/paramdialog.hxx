/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DBAUI_PARAMDIALOG_HXX_
#define _DBAUI_PARAMDIALOG_HXX_

#include "commontypes.hxx"

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>

#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <connectivity/predicateinput.hxx>
#include "svx/ParseContext.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
