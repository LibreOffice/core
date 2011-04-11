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

#ifndef DBAUI_QUERYFILTER_HXX
#define DBAUI_QUERYFILTER_HXX

#include <vcl/dialog.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/edit.hxx>

#include <vcl/fixed.hxx>

#include <vcl/button.hxx>
#include <connectivity/sqliterator.hxx>

#include <connectivity/predicateinput.hxx>
#include "svx/ParseContext.hxx"

namespace rtl
{
    class OUString;
}
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
//==================================================================
// DlgFilterCrit
//==================================================================
namespace dbaui
{
    class DlgFilterCrit :public ModalDialog
                        ,public ::svxform::OParseContextClient
    {
    private:
        ListBox         aLB_WHEREFIELD1;
        ListBox         aLB_WHERECOMP1;
        Edit            aET_WHEREVALUE1;
        ListBox         aLB_WHERECOND2;
        ListBox         aLB_WHEREFIELD2;
        ListBox         aLB_WHERECOMP2;
        Edit            aET_WHEREVALUE2;
        ListBox         aLB_WHERECOND3;
        ListBox         aLB_WHEREFIELD3;
        ListBox         aLB_WHERECOMP3;
        Edit            aET_WHEREVALUE3;
        FixedText       aFT_WHEREFIELD;
        FixedText       aFT_WHERECOMP;
        FixedText       aFT_WHEREVALUE;
        FixedText       aFT_WHEREOPER;
        FixedLine       aFL_FIELDS;
        OKButton        aBT_OK;
        CancelButton    aBT_CANCEL;
        HelpButton      aBT_HELP;
        String          aSTR_NOENTRY;
        String          aSTR_COMPARE_OPERATORS;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer>    m_xQueryComposer;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>     m_xColumns;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>          m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>    m_xMetaData;

        ::dbtools::OPredicateInputController    m_aPredicateInput;

        void            SelectField( ListBox& rBox, const String& rField );
        DECL_LINK( ListSelectHdl, ListBox * );
        DECL_LINK( ListSelectCompHdl, ListBox * );

        void            SetLine( sal_uInt16 nIdx,const ::com::sun::star::beans::PropertyValue& _rItem,sal_Bool _bOr );
        void            EnableLines();
        sal_Int32       GetOSQLPredicateType( const String& _rSelectedPredicate ) const;
        sal_uInt16          GetSelectionPos(sal_Int32 eType,const ListBox& rListBox) const;
        sal_Bool        getCondition(const ListBox& _rField,const ListBox& _rComp,const Edit& _rValue,::com::sun::star::beans::PropertyValue& _rFilter) const;
        void            fillLines(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& _aValues);

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getMatchingColumn( const Edit& _rValueInput ) const;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getColumn( const ::rtl::OUString& _rFieldName ) const;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getQueryColumn( const ::rtl::OUString& _rFieldName ) const;

    public:
        DlgFilterCrit(  Window * pParent,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConnection,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer>& _rxComposer,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxCols
                    );
        ~DlgFilterCrit();

        void            BuildWherePart();

    protected:
        DECL_LINK( PredicateLoseFocus, Edit* );
    };

}

#endif // DBAUI_QUERYFILTER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
