/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mmoutputtypepage.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 07:05:32 $
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
#ifndef _MAILMERGEOUTPUTTYPEPAGE_HXX
#define _MAILMERGEOUTPUTTYPEPAGE_HXX

#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _MAILMERGEHELPER_HXX
#include <mailmergehelper.hxx>
#endif
class SwMailMergeWizard;
/*-- 02.04.2004 09:21:06---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailMergeOutputTypePage : public svt::OWizardPage
{
    SwBoldFixedInfo m_aHeaderFI;
    FixedInfo       m_aTypeFT;
    RadioButton     m_aLetterRB;
    RadioButton     m_aMailRB;

    SwBoldFixedInfo m_aHintHeaderFI;
    FixedInfo       m_aHintFI;
    FixedInfo       m_aNoMailHintFI;

    String          m_sLetterHintHeader;
    String          m_sMailHintHeader;
    String          m_sLetterHint;
    String          m_sMailHint;

    SwMailMergeWizard*  m_pWizard;

    DECL_LINK(TypeHdl_Impl, RadioButton*);

public:
        SwMailMergeOutputTypePage( SwMailMergeWizard* _pParent);
        ~SwMailMergeOutputTypePage();

};
#endif


