/*************************************************************************
 *
 *  $RCSfile: dlg_ChartType.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-04 12:37:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHART2_DLG_CHARTTYPE_HXX
#define _CHART2_DLG_CHARTTYPE_HXX

/*
#ifndef _SCH_DIAGRTYP_HXX_
#define _SCH_DIAGRTYP_HXX_

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif
#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
*/

// header for class ModalDialog
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
// header for class RadioButton
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class FixedLine
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for class MetricField
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
// header for class ValueSet
#ifndef _VALUESET_HXX
#include <svtools/valueset.hxx>
#endif
// header for class SfxItemSet
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
// header for SvxChartStyle
#ifndef _SVX_CHRTITEM_HXX
#include <svx/chrtitem.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <drafts/com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTTYPETEMPLATE_HPP_
#include <drafts/com/sun/star/chart2/XChartTypeTemplate.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

enum ChartDimension
{
    CHDIM_2D,
    CHDIM_3D
};

#define CHDIM_COUNT (CHDIM_3D + 1)

/*************************************************************************
|*
|* Dialog zur Auswahl eines Diagrammtyps
|*
\************************************************************************/
class SchDiagramTypeDlg : public ModalDialog
{
private:
    long            n3DGeometry;
    RadioButton     aRbt2D;
    RadioButton     aRbt3D;
    FixedLine       aFlDimension;
    FixedText       aFtDeep;
    MetricField     aMtrFldDeep;
    FixedText       aFtGran;
    MetricField     aMtrFldGran;
    FixedText       aFtNumLines;
    MetricField     aMtrFldNumLines;

    FixedText       aFtType;
    ValueSet        aCtlType;
    FixedText       aFtVariant;
    ValueSet        aCtlVariant;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    ChartDimension  eDimension;

//  const SfxItemSet&       rOutAttrs;

    ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XDiagram > m_xDiagram;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > m_xTemplateManager;

    void Reset();

    void FillTypeSet(ChartDimension eDim, bool bForce=false);
    void FillVariantSet(USHORT nType);

    DECL_LINK(SelectDimensionHdl, void*);
    DECL_LINK(SelectTypeHdl, void*);
    DECL_LINK(DoubleClickHdl, void*);
    DECL_LINK(ClickHdl, void*);

    void SwitchDepth( SvxChartStyle eID );

    /** Hides/Shows the controls for line/bar combination chart according to the
        chart type id given as nID.
     */
    void SwitchNumLines( SvxChartStyle eID );

    void FillValueSets();

public:
    SchDiagramTypeDlg(
        Window* pWindow,
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XDiagram > & xDiagram,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xTemplateManager );
    virtual ~SchDiagramTypeDlg();

//BM    void    GetAttr(SfxItemSet& rOutAttrs);

     sal_Int32 GetDepth();
    void      SetDepth( sal_Int32 nDeep );

     sal_Int32 GetGranularity();
     void      SetGranularity( sal_Int32 nGranularity );

    /** The value set here determines the maximum number of lines in a line/bar
        combination chart.  This should usually be one less than the number of
        series, such that at least one series remains a bar.
     */
    void    SetMaximumNumberOfLines( sal_Int32 nMaxLines );
    /** set the current number of lines that are used in a line/bar combination
        chart.
     */
//     void    SetNumberOfLines( long nLines );
    /** get the number of lines that should be used for a line/bar combination
        chart.  This has to be set before to be meaningful
     */
    sal_Int32  GetNumberOfLines() const;

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XChartTypeTemplate >
        getTemplate() const;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif


