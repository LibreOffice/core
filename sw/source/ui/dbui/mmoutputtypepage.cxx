/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mmoutputtypepage.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 14:06:48 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#ifndef _MAILMERGEOUTPUTTYPEPAGE_HXX
#include <mmoutputtypepage.hxx>
#endif
#ifndef _MAILMERGEWIZARD_HXX
#include <mailmergewizard.hxx>
#endif
#ifndef _MMCONFIGITEM_HXX
#include <mmconfigitem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBUI_HRC
#include <dbui.hrc>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#include <mmoutputtypepage.hrc>
#include <dbui.hrc>

/*-- 02.04.2004 11:06:55---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeOutputTypePage::SwMailMergeOutputTypePage( SwMailMergeWizard* _pParent) :
    svt::OWizardPage( _pParent, SW_RES(DLG_MM_OUTPUTTYPE_PAGE)),
#ifdef _MSC_VER
#pragma warning (disable : 4355)
#endif
    m_aHeaderFI(    this, SW_RES( FI_HEADER    )),
    m_aTypeFT(      this, SW_RES( FT_TYPE    )),
    m_aLetterRB(    this, SW_RES( RB_LETTER  )),
    m_aMailRB(      this, SW_RES( RB_MAIL    )),
    m_aHintHeaderFI(this, SW_RES( FI_HINTHEADER)),
    m_aHintFI(      this, SW_RES( FI_HINT)),
    m_aNoMailHintFI(this, SW_RES( FT_NOMAILHINT)),
#ifdef _MSC_VER
#pragma warning (default : 4355)
#endif
    m_pWizard(_pParent),
    m_sLetterHintHeader(  SW_RES( ST_LETTERHINTHEADER)),
    m_sMailHintHeader(      SW_RES( ST_MAILHINTHEADER)),
    m_sLetterHint(          SW_RES( ST_LETTERHINT)),
    m_sMailHint(            SW_RES( ST_MAILHINT))
{
    FreeResource();
    Link aLink = LINK(this, SwMailMergeOutputTypePage, TypeHdl_Impl);
    m_aLetterRB.SetClickHdl(aLink);
    m_aMailRB.SetClickHdl(aLink);

    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    m_pWizard->EnterWait();
    bool bMailAvailable = rConfigItem.IsMailAvailable();
    m_pWizard->LeaveWait();
    if(rConfigItem.IsOutputToLetter())
        m_aLetterRB.Check();
    else
        m_aMailRB.Check();
    if(!bMailAvailable)
    {
        m_aNoMailHintFI.Show();
        m_aMailRB.Enable(sal_False);
        m_aLetterRB.Check();
    }
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


