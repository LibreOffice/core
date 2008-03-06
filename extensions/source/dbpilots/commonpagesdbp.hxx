/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commonpagesdbp.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:40:30 $
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

#ifndef _EXTENSIONS_DBP_COMMONPAGESDBP_HXX_
#define _EXTENSIONS_DBP_COMMONPAGESDBP_HXX_

#ifndef _EXTENSIONS_DBP_CONTROLWIZARD_HXX
#include "controlwizard.hxx"
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

//.........................................................................
namespace dbp
{
//.........................................................................

    //=====================================================================
    //= OTableSelectionPage
    //=====================================================================
    class OTableSelectionPage : public OControlWizardPage
    {
    protected:
        FixedLine       m_aData;
        FixedText       m_aExplanation;
        FixedText       m_aDatasourceLabel;
        ListBox         m_aDatasource;
        PushButton      m_aSearchDatabase;
        FixedText       m_aTableLabel;
        ListBox         m_aTable;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                        m_xDSContext;

    public:
        OTableSelectionPage(OControlWizard* _pParent);

    protected:
        // TabPage overridables
        void ActivatePage();

        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( CommitPageReason _eReason );

    protected:
        DECL_LINK( OnListboxSelection, ListBox* );
        DECL_LINK( OnListboxDoubleClicked, ListBox* );
        DECL_LINK( OnSearchClicked, PushButton* );

        void implCollectDatasource();
        void implFillTables(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >&
                        _rxConn = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >());

        // OControlWizardPage overridables
        virtual bool    canAdvance() const;
    };

    //=====================================================================
    //= OMaybeListSelectionPage
    //=====================================================================
    class OMaybeListSelectionPage : public OControlWizardPage
    {
    protected:
        RadioButton*    m_pYes;
        RadioButton*    m_pNo;
        ListBox*        m_pList;

    public:
        OMaybeListSelectionPage( OControlWizard* _pParent, const ResId& _rId );

    protected:
        DECL_LINK( OnRadioSelected, RadioButton* );

        // TabPage overridables
        void ActivatePage();

        // own helper
        void    announceControls(
            RadioButton& _rYesButton,
            RadioButton& _rNoButton,
            ListBox& _rSelection);

        void implEnableWindows();

        void implInitialize(const String& _rSelection);
        void implCommit(String& _rSelection);
    };

    //=====================================================================
    //= ODBFieldPage
    //=====================================================================
    class ODBFieldPage : public OMaybeListSelectionPage
    {
    protected:
        FixedLine       m_aFrame;
        FixedText       m_aDescription;
        FixedText       m_aQuestion;
        RadioButton     m_aStoreYes;
        RadioButton     m_aStoreNo;
        ListBox         m_aStoreWhere;

    public:
        ODBFieldPage( OControlWizard* _pParent );

    protected:
        void setDescriptionText(const String& _rDesc) { m_aDescription.SetText(_rDesc); }

        // OWizardPage overridables
        virtual void initializePage();
        virtual sal_Bool commitPage( CommitPageReason _eReason );

        // own overridables
        virtual String& getDBFieldSetting() = 0;
    };

//.........................................................................
}   // namespace dbp
//.........................................................................


#endif // _EXTENSIONS_DBP_COMMONPAGESDBP_HXX_

