/*************************************************************************
 *
 *  $RCSfile: mmoutputtypepage.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-09-29 09:32:55 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif

#pragma hdrstop

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
#pragma warning (disable : 4355)
    m_aHeaderFI(    this, ResId( FI_HEADER    )),
    m_aTypeFT(      this, ResId( FT_TYPE    )),
    m_aLetterRB(    this, ResId( RB_LETTER  )),
    m_aMailRB(      this, ResId( RB_MAIL    )),
    m_aHintHeaderFI(this, ResId( FI_HINTHEADER)),
    m_aHintFI(      this, ResId( FI_HINT)),
    m_aNoMailHintFI(this, ResId( FT_NOMAILHINT)),
#pragma warning (default : 4355)
    m_pWizard(_pParent),
    m_sLetterHintHeader(  ResId( ST_LETTERHINTHEADER)),
    m_sMailHintHeader(      ResId( ST_MAILHINTHEADER)),
    m_sLetterHint(          ResId( ST_LETTERHINT)),
    m_sMailHint(            ResId( ST_MAILHINT))
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
    return 0;
}


