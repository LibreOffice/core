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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include <mmoutputtypepage.hxx>
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <vcl/msgbox.hxx>
#ifndef _DBUI_HRC
#include <dbui.hrc>
#endif
#include <swtypes.hxx>
#include <mmoutputtypepage.hrc>
#include <dbui.hrc>

/*-- 02.04.2004 11:06:55---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeOutputTypePage::SwMailMergeOutputTypePage( SwMailMergeWizard* _pParent) :
    svt::OWizardPage( _pParent, SW_RES(DLG_MM_OUTPUTTYPE_PAGE)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aHeaderFI(    this, SW_RES( FI_HEADER    )),
    m_aTypeFT(      this, SW_RES( FT_TYPE    )),
    m_aLetterRB(    this, SW_RES( RB_LETTER  )),
    m_aMailRB(      this, SW_RES( RB_MAIL    )),
    m_aHintHeaderFI(this, SW_RES( FI_HINTHEADER)),
    m_aHintFI(      this, SW_RES( FI_HINT)),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_sLetterHintHeader(  SW_RES( ST_LETTERHINTHEADER)),
    m_sMailHintHeader(      SW_RES( ST_MAILHINTHEADER)),
    m_sLetterHint(          SW_RES( ST_LETTERHINT)),
    m_sMailHint(            SW_RES( ST_MAILHINT)),
    m_pWizard(_pParent)
{
    FreeResource();
    Link aLink = LINK(this, SwMailMergeOutputTypePage, TypeHdl_Impl);
    m_aLetterRB.SetClickHdl(aLink);
    m_aMailRB.SetClickHdl(aLink);

    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    if(rConfigItem.IsOutputToLetter())
        m_aLetterRB.Check();
    else
        m_aMailRB.Check();
    TypeHdl_Impl(&m_aLetterRB);

}
/*-- 02.04.2004 11:06:55---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeOutputTypePage::~SwMailMergeOutputTypePage()
{
}
/*-- 14.04.2004 14:44:07---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwMailMergeOutputTypePage, TypeHdl_Impl, RadioButton*, EMPTYARG )
{
    bool bLetter = m_aLetterRB.IsChecked();
        m_aHintHeaderFI.SetText(bLetter ? m_sLetterHintHeader : m_sMailHintHeader);
        m_aHintFI.SetText(bLetter ? m_sLetterHint : m_sMailHint);
    m_pWizard->GetConfigItem().SetOutputToLetter(bLetter);
    m_pWizard->updateRoadmapItemLabel( MM_ADDRESSBLOCKPAGE );
    m_pWizard->UpdateRoadmap();
    return 0;
}


