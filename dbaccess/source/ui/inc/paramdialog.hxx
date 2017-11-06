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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_PARAMDIALOG_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_PARAMDIALOG_HXX

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
#include <svx/ParseContext.hxx>
#include <o3tl/typed_flags_set.hxx>

namespace connectivity
{
    class OSQLParseNode;
}

enum class VisitFlags {
    NONE        = 0x00,
    Visited     = 0x01,
    Dirty       = 0x02,
};
namespace o3tl {
    template<> struct typed_flags<VisitFlags> : is_typed_flags<VisitFlags, 0x03> {};
}


namespace dbaui
{

    // OParameterDialog
    class OParameterDialog final
            :public ModalDialog
            ,public ::svxform::OParseContextClient
    {
        // the controls
        VclPtr<ListBox>        m_pAllParams;
        VclPtr<Edit>           m_pParam;
        VclPtr<PushButton>     m_pTravelNext;
        VclPtr<OKButton>       m_pOKBtn;
        VclPtr<CancelButton>   m_pCancelBtn;

        sal_Int32              m_nCurrentlySelected;

        css::uno::Reference< css::container::XIndexAccess >
                               m_xParams;
        css::uno::Reference< css::sdbc::XConnection >
                               m_xConnection;
        css::uno::Reference< css::util::XNumberFormatter >
                               m_xFormatter;
        ::dbtools::OPredicateInputController
                               m_aPredicateInput;

        std::vector<VisitFlags>  m_aVisitedParams;
        Timer                  m_aResetVisitFlag;
            // we reset the "visited flag" 1 second after and entry has been selected

        bool                   m_bNeedErrorOnCurrent;

        css::uno::Sequence< css::beans::PropertyValue >
                               m_aFinalValues;     /// the final values as entered by the user

    public:
        OParameterDialog(vcl::Window* _pParent,
            const css::uno::Reference< css::container::XIndexAccess > & _rParamContainer,
            const css::uno::Reference< css::sdbc::XConnection > & _rxConnection,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext);
        virtual ~OParameterDialog() override;
        virtual void dispose() override;

        const css::uno::Sequence< css::beans::PropertyValue >&
                    getValues() const { return m_aFinalValues; }

    private:
        void Construct();

        DECL_LINK(OnVisitedTimeout, Timer*, void);
        DECL_LINK(OnValueModified, Edit&, void);
        DECL_LINK(OnEntryListBoxSelected, ListBox&, void);
        DECL_LINK(OnButtonClicked, Button*, void);
        DECL_LINK(OnValueLoseFocusHdl, Control&, void);
        bool OnValueLoseFocus();
        bool OnEntrySelected();
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_PARAMDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
