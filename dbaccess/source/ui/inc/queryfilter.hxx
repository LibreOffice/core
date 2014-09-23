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
        ListBox*        m_pLB_WHEREFIELD1;
        ListBox*        m_pLB_WHERECOMP1;
        Edit*           m_pET_WHEREVALUE1;

        ListBox*        m_pLB_WHERECOND2;
        ListBox*        m_pLB_WHEREFIELD2;
        ListBox*        m_pLB_WHERECOMP2;
        Edit*           m_pET_WHEREVALUE2;

        ListBox*        m_pLB_WHERECOND3;
        ListBox*        m_pLB_WHEREFIELD3;
        ListBox*        m_pLB_WHERECOMP3;
        Edit*           m_pET_WHEREVALUE3;

        OUString        m_aSTR_COMPARE_OPERATORS;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer>    m_xQueryComposer;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>     m_xColumns;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>          m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>    m_xMetaData;

        ::dbtools::OPredicateInputController    m_aPredicateInput;

        void            SelectField( ListBox& rBox, const OUString& rField );
        DECL_LINK( ListSelectHdl, ListBox * );
        DECL_LINK( ListSelectCompHdl, ListBox * );

        void            SetLine( sal_uInt16 nIdx,const ::com::sun::star::beans::PropertyValue& _rItem,bool _bOr );
        void            EnableLines();
        sal_Int32       GetOSQLPredicateType( const OUString& _rSelectedPredicate ) const;
        sal_uInt16          GetSelectionPos(sal_Int32 eType,const ListBox& rListBox) const;
        bool        getCondition(const ListBox& _rField,const ListBox& _rComp,const Edit& _rValue,::com::sun::star::beans::PropertyValue& _rFilter) const;
        void            fillLines(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& _aValues);

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getMatchingColumn( const Edit& _rValueInput ) const;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getColumn( const OUString& _rFieldName ) const;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getQueryColumn( const OUString& _rFieldName ) const;

    public:
        DlgFilterCrit(  vcl::Window * pParent,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConnection,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer>& _rxComposer,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxCols
                    );
        virtual ~DlgFilterCrit();

        void            BuildWherePart();

    protected:
        DECL_LINK( PredicateLoseFocus, Edit* );
    };

}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
