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



#ifndef DBAUI_QUERYFILTER_HXX
#define DBAUI_QUERYFILTER_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _CONNECTIVITY_PARSE_SQLITERATOR_HXX_
#include <connectivity/sqliterator.hxx>
#endif

#ifndef CONNECTIVITY_PREDICATEINPUT_HXX
#include <connectivity/predicateinput.hxx>
#endif
#ifndef SVX_QUERYDESIGNCONTEXT_HXX
#include "svx/ParseContext.hxx"
#endif

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


