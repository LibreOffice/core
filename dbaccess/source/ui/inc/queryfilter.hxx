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

#pragma once

#include <vcl/weld.hxx>
#include <connectivity/sqliterator.hxx>

#include <connectivity/predicateinput.hxx>
#include <svx/ParseContext.hxx>

namespace com::sun::star {
    namespace sdb
    {
        class XSingleSelectQueryComposer;
    }
    namespace sdbc
    {
        class XConnection;
        class XDatabaseMetaData;
    }
    namespace container
    {
        class XNameAccess;
    }
    namespace beans
    {
        struct PropertyValue;
    }
}

// DlgFilterCrit
namespace dbaui
{
    class DlgFilterCrit final : public weld::GenericDialogController
                              , public ::svxform::OParseContextClient
    {
    private:
        std::vector<OUString>  m_aSTR_COMPARE_OPERATORS;

        css::uno::Reference< css::sdb::XSingleSelectQueryComposer>    m_xQueryComposer;
        css::uno::Reference< css::container::XNameAccess>             m_xColumns;
        css::uno::Reference< css::sdbc::XConnection>                  m_xConnection;
        css::uno::Reference< css::sdbc::XDatabaseMetaData>            m_xMetaData;

        ::dbtools::OPredicateInputController    m_aPredicateInput;

        std::unique_ptr<weld::ComboBox> m_xLB_WHEREFIELD1;
        std::unique_ptr<weld::ComboBox> m_xLB_WHERECOMP1;
        std::unique_ptr<weld::Entry> m_xET_WHEREVALUE1;

        std::unique_ptr<weld::ComboBox> m_xLB_WHERECOND2;
        std::unique_ptr<weld::ComboBox> m_xLB_WHEREFIELD2;
        std::unique_ptr<weld::ComboBox> m_xLB_WHERECOMP2;
        std::unique_ptr<weld::Entry> m_xET_WHEREVALUE2;

        std::unique_ptr<weld::ComboBox> m_xLB_WHERECOND3;
        std::unique_ptr<weld::ComboBox> m_xLB_WHEREFIELD3;
        std::unique_ptr<weld::ComboBox> m_xLB_WHERECOMP3;
        std::unique_ptr<weld::Entry> m_xET_WHEREVALUE3;

        static void SelectField(weld::ComboBox& rBox, const OUString& rField);
        DECL_LINK(ListSelectHdl, weld::ComboBox&, void);
        DECL_LINK(ListSelectCompHdl, weld::ComboBox&, void);

        void            SetLine( int nIdx, const css::beans::PropertyValue& _rItem, bool _bOr );
        void            EnableLines();
        sal_Int32       GetOSQLPredicateType( const OUString& _rSelectedPredicate ) const;
        static sal_Int32  GetSelectionPos(sal_Int32 eType, const weld::ComboBox& rListBox);
        bool            getCondition(const weld::ComboBox& _rField, const weld::ComboBox& _rComp, const weld::Entry& _rValue, css::beans::PropertyValue& _rFilter) const;
        void            fillLines(int &i, const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& _aValues);

        css::uno::Reference< css::beans::XPropertySet > getMatchingColumn( const weld::Entry& _rValueInput ) const;
        css::uno::Reference< css::beans::XPropertySet > getColumn( const OUString& _rFieldName ) const;
        css::uno::Reference< css::beans::XPropertySet > getQueryColumn( const OUString& _rFieldName ) const;

    public:
        DlgFilterCrit(weld::Window * pParent,
                      const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                      const css::uno::Reference< css::sdbc::XConnection>& _rxConnection,
                      const css::uno::Reference< css::sdb::XSingleSelectQueryComposer>& _rxComposer,
                      const css::uno::Reference< css::container::XNameAccess>& _rxCols);
        virtual ~DlgFilterCrit() override;

        void            BuildWherePart();

    private:
        DECL_LINK(PredicateLoseFocus, weld::Widget&, void);
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
