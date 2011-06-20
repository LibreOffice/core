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
#ifndef _CHART2_RES_LEGENPOSITION_HXX
#define _CHART2_RES_LEGENPOSITION_HXX

// header for class CheckBox
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
// header for class SfxItemSet
#include <svl/itemset.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

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

    void SetAccessibleRelationMemberOf(Window* pMemberOf); //IAccessibility2 Implementation 2009-----

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

//.............................................................................
} //namespace chart
//.............................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
