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
#ifndef _CHART2_DLG_OBJECTPROPERTIES_HXX
#define _CHART2_DLG_OBJECTPROPERTIES_HXX

#include "ObjectIdentifier.hxx"
#include <sfx2/tabdlg.hxx>
#include <svx/dlgctrl.hxx>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

namespace chart
{

class ObjectPropertiesDialogParameter
{
public:
    ObjectPropertiesDialogParameter( const OUString& rObjectCID );
    virtual ~ObjectPropertiesDialogParameter();

    void            init( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
    ObjectType      getObjectType() const;
    OUString   getLocalizedName() const;

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
    const ::com::sun::star::uno::Sequence< OUString >& GetCategories() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >
        getDocument() const;

    bool IsComplexCategoriesAxis() const;

private:
    OUString   m_aObjectCID;
    ObjectType      m_eObjectType;
    bool m_bAffectsMultipleObjects;//is true if more than one object of the given type will be changed (e.g. all axes or all titles)

    OUString   m_aLocalizedName;

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
    ::com::sun::star::uno::Sequence< OUString > m_aCategories;

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

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
