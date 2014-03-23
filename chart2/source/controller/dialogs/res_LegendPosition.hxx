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


#ifndef _CHART2_RES_LEGENPOSITION_HXX
#define _CHART2_RES_LEGENPOSITION_HXX

// header for class CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
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

//.............................................................................
} //namespace chart
//.............................................................................

#endif
