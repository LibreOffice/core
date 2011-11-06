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


#ifndef _CHART2_DLG_OBJECTPROPERTIES_HXX
#define _CHART2_DLG_OBJECTPROPERTIES_HXX

#include "ObjectIdentifier.hxx"
#include <sfx2/tabdlg.hxx>
// header for typedef ChangeType
#include <svx/tabarea.hxx>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

class ObjectPropertiesDialogParameter
{
public:
    ObjectPropertiesDialogParameter( const rtl::OUString& rObjectCID );
    virtual ~ObjectPropertiesDialogParameter();

    void            init( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
    ObjectType      getObjectType() const;
    rtl::OUString   getLocalizedName() const;

    bool HasGeometryProperties() const;
    bool HasStatisticProperties() const;
    bool ProvidesSecondaryYAxis() const;
    bool ProvidesOverlapAndGapWidth() const;
    bool ProvidesBarConnectors() const;
    bool HasAreaProperties() const;
    bool HasSymbolProperties() const;
    bool HasNumberProperties() const;
    bool ProvidesStartingAngle() const;
    bool ProvidesMissingValueTreatments() const;

    bool HasScaleProperties() const;
    bool CanAxisLabelsBeStaggered() const;
    bool IsSupportingAxisPositioning() const;
    bool ShowAxisOrigin() const;
    bool IsCrossingAxisIsCategoryAxis() const;
    const ::com::sun::star::uno::Sequence< rtl::OUString >& GetCategories() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >
        getDocument() const;

    bool IsComplexCategoriesAxis() const;

private:
    rtl::OUString   m_aObjectCID;
    ObjectType      m_eObjectType;
    bool m_bAffectsMultipleObjects;//is true if more than one object of the given type will be changed (e.g. all axes or all titles)

    rtl::OUString   m_aLocalizedName;

    bool m_bHasGeometryProperties;
    bool m_bHasStatisticProperties;
    bool m_bProvidesSecondaryYAxis;
    bool m_bProvidesOverlapAndGapWidth;
    bool m_bProvidesBarConnectors;
    bool m_bHasAreaProperties;
    bool m_bHasSymbolProperties;
    bool m_bHasNumberProperties;
    bool m_bProvidesStartingAngle;
    bool m_bProvidesMissingValueTreatments;

    bool m_bHasScaleProperties;
    bool m_bCanAxisLabelsBeStaggered;

    bool m_bSupportingAxisPositioning;
    bool m_bShowAxisOrigin;
    bool m_bIsCrossingAxisIsCategoryAxis;
    ::com::sun::star::uno::Sequence< rtl::OUString > m_aCategories;

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument > m_xChartDocument;

    bool m_bComplexCategoriesAxis;
};

/*************************************************************************
|*
|* dialog for properties of different chart object
|*
\************************************************************************/

class ViewElementListProvider;

class SchAttribTabDlg : public SfxTabDialog
{
private:
    ObjectType               eObjectType;
    bool                     bAffectsMultipleObjects;//is true if more than one object of the given type will be changed (e.g. all axes or all titles)
    sal_uInt16                   nDlgType;
    sal_uInt16                   nPageType;

    const ObjectPropertiesDialogParameter * const        m_pParameter;
    const ViewElementListProvider* const                 m_pViewElementListProvider;
    SvNumberFormatter* m_pNumberFormatter;

    SfxItemSet*     m_pSymbolShapeProperties;
    Graphic*        m_pAutoSymbolGraphic;

    double          m_fAxisMinorStepWidthForErrorBarDecimals;
    bool            m_bOKPressed;

    virtual void PageCreated(sal_uInt16 nId, SfxTabPage& rPage);

    Link m_aOriginalOKClickHdl;
    DECL_LINK( OKPressed, void * );

public:
    SchAttribTabDlg(Window* pParent, const SfxItemSet* pAttr,
                    const ObjectPropertiesDialogParameter* pDialogParameter,
                    const ViewElementListProvider* pViewElementListProvider,
                    const ::com::sun::star::uno::Reference<
                            ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );
    virtual ~SchAttribTabDlg();

    //pSymbolShapeProperties: Properties to be set on the symbollist shapes
    //pAutoSymbolGraphic: Graphic to be shown if AutoSymbol gets selected
    //this class takes ownership over both parameter
    void setSymbolInformation( SfxItemSet* pSymbolShapeProperties, Graphic* pAutoSymbolGraphic );

    void SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth );

    bool DialogWasClosedWithOK() const;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

