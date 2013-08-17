/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

namespace dbaui
{

    // OParameterDialog
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
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext);
        ~OParameterDialog();

        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                    getValues() const { return m_aFinalValues; }

    protected:
        void Construct();

        ::connectivity::OSQLParseNode* implPredicateTree(OUString& _rErrorMessage, const OUString& _rStatement, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _rxField);

    private:
        DECL_LINK(OnVisitedTimeout, Timer*);
        DECL_LINK(OnValueModified, Control*);
        DECL_LINK(OnEntrySelected, ListBox*);
        DECL_LINK(OnButtonClicked, PushButton*);
        DECL_LINK(OnValueLoseFocus, Control*);
    };

}   // namespace dbaui

#endif // _DBAUI_PARAMDIALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
