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
#ifndef CHART2_DATASOURCETABPAGE_HXX
#define CHART2_DATASOURCETABPAGE_HXX

#include <svtools/wizardmachine.hxx>

#include "DialogModel.hxx"
#include "TabPageNotifiable.hxx"

// header for class Edit
#include <vcl/edit.hxx>
// header for class ListBox
#include <vcl/lstbox.hxx>
// header for class PushButton, OKButton, CancelButton, HelpButton
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class FixedText, FixedLine
#include <vcl/fixed.hxx>
// header for class SvTabListBox
#include <svtools/svtabbx.hxx>
// header for class SvTreeListBox
#include <svtools/svtreebx.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/sheet/XRangeSelection.hpp>

#include <utility>
#include <vector>
#include <memory>

#include "RangeSelectionListener.hxx"
#include "RangeSelectionButton.hxx"
#include "RangeEdit.hxx"

namespace com { namespace sun { namespace star {
    namespace chart2 {
        class XChartType;
}}}}

namespace chart
{

class ChartTypeTemplateProvider;
class DialogModel;

class DataSourceTabPage :
        public ::svt::OWizardPage,
        public RangeSelectionListenerParent
{
public:
    explicit DataSourceTabPage(
        Window * pParent,
        DialogModel & rDialogModel,
        ChartTypeTemplateProvider* pTemplateProvider,
        Dialog * pParentDialog,
        bool bHideDescription = false );
    virtual ~DataSourceTabPage();

    void commitPage();

protected:
    // OWizardPage
    virtual void ActivatePage();
    virtual sal_Bool commitPage( ::svt::WizardTypes::CommitPageReason eReason );

    //TabPage
    virtual void DeactivatePage();

    virtual void        initializePage();

    DECL_LINK( SeriesSelectionChangedHdl, void* );
    DECL_LINK( RoleSelectionChangedHdl, void* );
    DECL_LINK( MainRangeButtonClickedHdl, void* );
    DECL_LINK( CategoriesRangeButtonClickedHdl, void* );
    DECL_LINK( AddButtonClickedHdl, void* );
    DECL_LINK( RemoveButtonClickedHdl, void* );
    DECL_LINK( RangeModifiedHdl, Edit* );
    DECL_LINK( RangeUpdateDataHdl, Edit* );
    DECL_LINK( UpButtonClickedHdl, void* );
    DECL_LINK( DownButtonClickedHdl, void* );

    // ____ RangeSelectionListenerParent ____
    virtual void listeningFinished( const ::rtl::OUString & rNewRange );
    virtual void disposingRangeSelection();

    void updateControlState();

protected:
    /** updates the internal data according to the content of the given edit
        field.  If pField is 0, all relevant fields are used

        @return
            <TRUE/> if the text from the field is a valid format to the internal
            data was valid
     */
    bool updateModelFromControl( Edit * pField = 0 );

    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XRangeSelectionListener >
        getSelectionRangeListener();

    /** @return </sal_True>, if the edit field contains a valid range entry. if no
        XCellRangesAccess can be obtained, </sal_True> is returned.
     */
    bool isRangeFieldContentValid( Edit & rEdit );

    /** @return </sal_True>, if the tab-page is in a consistent (commitable) state
     */
    bool isValid();
    void setDirty();

    void updateControlsFromDialogModel();

    void fillSeriesListBox();
    void fillRoleListBox();

private:
    FixedText               m_aFT_CAPTION;
    FixedText               m_aFT_SERIES;
    ::std::auto_ptr< SvTreeListBox > m_apLB_SERIES;
    PushButton              m_aBTN_ADD;
    PushButton              m_aBTN_REMOVE;
    PushButton              m_aBTN_UP;
    PushButton              m_aBTN_DOWN;

    FixedText               m_aFT_ROLE;
    SvTabListBox            m_aLB_ROLE;
    FixedText               m_aFT_RANGE;
    RangeEdit               m_aEDT_RANGE;
    RangeSelectionButton    m_aIMB_RANGE_MAIN;

    FixedText               m_aFT_CATEGORIES;
    FixedText               m_aFT_DATALABELS;//used for xy charts
    RangeEdit               m_aEDT_CATEGORIES;
    RangeSelectionButton    m_aIMB_RANGE_CAT;

    ::rtl::OUString         m_aFixedTextRange;

    ChartTypeTemplateProvider * m_pTemplateProvider;
    DialogModel &               m_rDialogModel;
    Edit *                      m_pCurrentRangeChoosingField;
    bool                        m_bIsDirty;
    sal_Int32                   m_nLastChartTypeGroupIndex;

    Dialog *                    m_pParentDialog;
    TabPageNotifiable *         m_pTabPageNotifiable;
};

} //  namespace chart

// CHART2_DATASOURCETABPAGE_HXX
#endif
