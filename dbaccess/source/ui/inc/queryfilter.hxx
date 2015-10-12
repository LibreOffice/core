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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYFILTER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYFILTER_HXX

#include <vcl/dialog.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/edit.hxx>

#include <vcl/fixed.hxx>

#include <vcl/button.hxx>
#include <connectivity/sqliterator.hxx>

#include <connectivity/predicateinput.hxx>
#include "svx/ParseContext.hxx"

namespace com
{
    namespace sun
    {
        namespace star
        {
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
    }
}
// DlgFilterCrit
namespace dbaui
{
    class DlgFilterCrit :public ModalDialog
                        ,public ::svxform::OParseContextClient
    {
    private:
        VclPtr<ListBox>        m_pLB_WHEREFIELD1;
        VclPtr<ListBox>        m_pLB_WHERECOMP1;
        VclPtr<Edit>           m_pET_WHEREVALUE1;

        VclPtr<ListBox>        m_pLB_WHERECOND2;
        VclPtr<ListBox>        m_pLB_WHEREFIELD2;
        VclPtr<ListBox>        m_pLB_WHERECOMP2;
        VclPtr<Edit>           m_pET_WHEREVALUE2;

        VclPtr<ListBox>        m_pLB_WHERECOND3;
        VclPtr<ListBox>        m_pLB_WHEREFIELD3;
        VclPtr<ListBox>        m_pLB_WHERECOMP3;
        VclPtr<Edit>           m_pET_WHEREVALUE3;

        OUString        m_aSTR_COMPARE_OPERATORS;

        css::uno::Reference< css::sdb::XSingleSelectQueryComposer>    m_xQueryComposer;
        css::uno::Reference< css::container::XNameAccess>             m_xColumns;
        css::uno::Reference< css::sdbc::XConnection>                  m_xConnection;
        css::uno::Reference< css::sdbc::XDatabaseMetaData>            m_xMetaData;

        ::dbtools::OPredicateInputController    m_aPredicateInput;

        static void     SelectField( ListBox& rBox, const OUString& rField );
        DECL_LINK_TYPED( ListSelectHdl, ListBox&, void );
        DECL_LINK_TYPED( ListSelectCompHdl, ListBox&, void );

        void            SetLine( sal_uInt16 nIdx,const css::beans::PropertyValue& _rItem,bool _bOr );
        void            EnableLines();
        sal_Int32       GetOSQLPredicateType( const OUString& _rSelectedPredicate ) const;
        static sal_Int32  GetSelectionPos(sal_Int32 eType,const ListBox& rListBox);
        bool            getCondition(const ListBox& _rField,const ListBox& _rComp,const Edit& _rValue,css::beans::PropertyValue& _rFilter) const;
        void            fillLines(const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& _aValues);

        css::uno::Reference< css::beans::XPropertySet > getMatchingColumn( const Edit& _rValueInput ) const;
        css::uno::Reference< css::beans::XPropertySet > getColumn( const OUString& _rFieldName ) const;
        css::uno::Reference< css::beans::XPropertySet > getQueryColumn( const OUString& _rFieldName ) const;

    public:
        DlgFilterCrit(  vcl::Window * pParent,
                        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                        const css::uno::Reference< css::sdbc::XConnection>& _rxConnection,
                        const css::uno::Reference< css::sdb::XSingleSelectQueryComposer>& _rxComposer,
                        const css::uno::Reference< css::container::XNameAccess>& _rxCols
                    );
        virtual ~DlgFilterCrit();
        virtual void dispose() override;

        void            BuildWherePart();

    protected:
        DECL_LINK_TYPED( PredicateLoseFocus, Control&, void );
    };

}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
