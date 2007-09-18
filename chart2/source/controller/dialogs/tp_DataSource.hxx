/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_DataSource.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 14:54:32 $
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
#ifndef CHART2_DATASOURCETABPAGE_HXX
#define CHART2_DATASOURCETABPAGE_HXX

#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif

#include "DialogModel.hxx"
#include "TabPageNotifiable.hxx"

// header for class Edit
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
// header for class ListBox
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
// header for class PushButton, OKButton, CancelButton, HelpButton
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class FixedText, FixedLine
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for class SvTabListBox
#ifndef _SVTABBX_HXX
#include <svtools/svtabbx.hxx>
#endif
// header for class SvTreeListBox
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XDATAPROVIDER_HPP_
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif

#ifndef _COM_SUN_STAR_SHEET_XRANGESELECTION_HPP_
#include <com/sun/star/sheet/XRangeSelection.hpp>
#endif

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

    // OWizardPage
    virtual void ActivatePage();

    virtual void        initializePage();

    DECL_LINK( SeriesSelectionChangedHdl, void* );
    DECL_LINK( RoleSelectionChangedHdl, void* );
    DECL_LINK( MainRangeButtonClickedHdl, void* );
    DECL_LINK( CategoriesRangeButtonClickedHdl, void* );
    DECL_LINK( AddButtonClickedHdl, void* );
    DECL_LINK( RemoveButtonClickedHdl, void* );
    DECL_LINK( RangeModifiedHdl, void* );
    DECL_LINK( CategoriesRangeModifiedHdl, void* );
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

    /// disable all controls (while Calc range chooser is active)
    void disableAll();

    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XRangeSelectionListener >
        getSelectionRangeListener();

    /** @return </TRUE>, if the edit field contains a valid range entry. if no
        XCellRangesAccess can be obtained, </TRUE> is returned.
     */
    bool isRangeFieldContentValid( Edit & rEdit );

    /** @return </TRUE>, if the tab-page is in a consistent (commitable) state
     */
    bool isValid();
    void setDirty();
    bool isDirty() const;

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
