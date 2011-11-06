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


#ifndef _SWFLDDINF_HXX
#define _SWFLDDINF_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <svtools/svtreebx.hxx>

#include "numfmtlb.hxx"
#include "fldpage.hxx"

namespace com{namespace sun{ namespace star{ namespace beans{
    class XPropertySet;
}}}}
/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

class SwFldDokInfPage : public SwFldPage
{
    FixedText           aTypeFT;
    SvTreeListBox       aTypeTLB;
    FixedText           aSelectionFT;
    ListBox             aSelectionLB;
    FixedText           aFormatFT;
    NumFormatListBox    aFormatLB;
    CheckBox            aFixedCB;

    SvLBoxEntry*        pSelEntry;
    com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > xCustomPropertySet;

    String              aInfoStr;

    sal_uInt16              nOldSel;
    sal_uLong               nOldFormat;
    ::rtl::OUString     m_sOldCustomFieldName;

    DECL_LINK( TypeHdl, ListBox* pLB = 0 );
    DECL_LINK( SubTypeHdl, ListBox* pLB = 0 );

    sal_uInt16              FillSelectionLB(sal_uInt16 nSubTypeId);

protected:
    virtual sal_uInt16      GetGroup();

public:
                        SwFldDokInfPage(Window* pWindow, const SfxItemSet& rSet);

                        ~SwFldDokInfPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
};


#endif

