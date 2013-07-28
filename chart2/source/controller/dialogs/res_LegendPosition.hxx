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
#ifndef _CHART2_RES_LEGENPOSITION_HXX
#define _CHART2_RES_LEGENPOSITION_HXX

// header for class CheckBox
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
// header for class SfxItemSet
#include <svl/itemset.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace chart
{

class LegendPositionResources
{

public:
    //constructor without Display checkbox
    LegendPositionResources( Window* pParent );
    //constructor inclusive Display checkbox
    LegendPositionResources( Window* pParent, const ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XComponentContext>& xCC );
    virtual ~LegendPositionResources();

    void writeToResources( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::frame::XModel >& xChartModel );
    void writeToModel( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::frame::XModel >& xChartModel ) const;

    void initFromItemSet( const SfxItemSet& rInAttrs );
    void writeToItemSet( SfxItemSet& rOutAttrs ) const;

    void SetChangeHdl( const Link& rLink );

    DECL_LINK( PositionEnableHdl, void* );
    DECL_LINK( PositionChangeHdl, RadioButton* );

    void SetAccessibleRelationMemberOf(Window* pMemberOf);

private:
    void impl_setRadioButtonToggleHdl();

private:
    ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XComponentContext>    m_xCC;

    CheckBox        m_aCbxShow;

    RadioButton     m_aRbtLeft;
    RadioButton     m_aRbtRight;
    RadioButton     m_aRbtTop;
    RadioButton     m_aRbtBottom;

    Link            m_aChangeLink;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
