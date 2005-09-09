/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mmmergepage.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 07:02:57 $
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
#ifndef _MAILMERGEMERGEPAGE_HXX
#define _MAILMERGEMERGEPAGE_HXX

#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _MAILMERGEHELPER_HXX
#include <mailmergehelper.hxx>
#endif
#ifndef _ACTCTRL_HXX
#include <actctrl.hxx>
#endif
class SwMailMergeWizard;
/*-- 02.04.2004 09:21:06---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailMergeMergePage : public svt::OWizardPage
{
    SwBoldFixedInfo m_aHeaderFI;

    FixedInfo       m_aEditFI;
    PushButton      m_aEditPB;

    FixedLine       m_aFindFL;
    FixedText       m_aFineFT;
    ReturnActionEdit    m_aFindED;
    PushButton      m_aFindPB;

    CheckBox        m_aWholeWordsCB;
    CheckBox        m_aBackwardsCB;
    CheckBox        m_aMatchCaseCB;

    SwMailMergeWizard*  m_pWizard;

    DECL_LINK(EditDocumentHdl_Impl, PushButton*);
    DECL_LINK(FindHdl_Impl, PushButton*);
    DECL_LINK(EnteredFindStringHdl_Impl, void*);

public:
        SwMailMergeMergePage( SwMailMergeWizard* _pParent);
        ~SwMailMergeMergePage();

};


#endif


