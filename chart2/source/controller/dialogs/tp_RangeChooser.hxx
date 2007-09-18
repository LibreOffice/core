/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_RangeChooser.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 14:54:58 $
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

#ifndef _CHART2_TP_RANGECHOOSER_HXX
#define _CHART2_TP_RANGECHOOSER_HXX

#include "RangeSelectionListener.hxx"
#include "RangeSelectionButton.hxx"
#include "TabPageNotifiable.hxx"
#include "RangeEdit.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>

#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPETEMPLATE_HPP_
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#endif

#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif

#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XRANGESELECTION_HPP_
#include <com/sun/star/sheet/XRangeSelection.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class ChartTypeTemplateProvider;
class DialogModel;

class RangeChooserTabPage : public svt::OWizardPage, public RangeSelectionListenerParent
{
public:

    RangeChooserTabPage( Window* pParent
                , DialogModel & rDialogModel
                , ChartTypeTemplateProvider* pTemplateProvider
                , Dialog * pParentDialog
                , bool bHideDescription = false );
virtual ~RangeChooserTabPage();

    //RangeSelectionListenerParent
    virtual void listeningFinished( const ::rtl::OUString & rNewRange );
    virtual void disposingRangeSelection();

protected: //methods

    //OWizardPage
    virtual void ActivatePage();

    void initControlsFromModel();
    void changeDialogModelAccordingToControls();
    void changeModelAccordingToControls();
    bool isValid();
    void setDirty();
    bool isDirty() const;

    DECL_LINK( ChooseRangeHdl, void* );
    DECL_LINK( ControlChangedHdl, void* );

protected: //member

    FixedText       m_aFT_Caption;
    FixedText       m_aFT_Range;
    RangeEdit       m_aED_Range;
    RangeSelectionButton     m_aIB_Range;

    RadioButton     m_aRB_Rows;
    RadioButton     m_aRB_Columns;

    CheckBox        m_aCB_FirstRowAsLabel;
    CheckBox        m_aCB_FirstColumnAsLabel;

    sal_Int32       m_nChangingControlCalls;
    bool            m_bIsDirty;

    ::com::sun::star::uno::Reference<
         ::com::sun::star::chart2::XChartDocument >         m_xChartDocument;
    ::com::sun::star::uno::Reference<
         ::com::sun::star::chart2::data::XDataProvider >    m_xDataProvider;

    rtl::OUString                                           m_aLastValidRangeString;
    ::com::sun::star::uno::Reference<
         ::com::sun::star::chart2::XChartTypeTemplate >     m_xCurrentChartTypeTemplate;
    ChartTypeTemplateProvider*                              m_pTemplateProvider;

    DialogModel &                                           m_rDialogModel;
    Dialog *                                                m_pParentDialog;
    TabPageNotifiable *                                     m_pTabPageNotifiable;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
