/*************************************************************************
 *
 *  $RCSfile: mailconfigpage.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-09-20 13:22:05 $
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
#ifndef _MAILCONFIGPAGE_HXX
#define _MAILCONFIGPAGE_HXX

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif

class SwTestAccountSettingsDialog;
class SwMailMergeConfigItem;
/*-- 05.05.2004 16:45:45---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailConfigPage : public SfxTabPage
{
    friend class SwTestAccountSettingsDialog;

    FixedLine       m_aIdentityFL;

    FixedText       m_aDisplayNameFT;
    Edit            m_aDisplayNameED;
    FixedText       m_aAddressFT;
    Edit            m_aAddressED;

    CheckBox        m_aReplyToCB;
    FixedText       m_aReplyToFT;
    Edit            m_aReplyToED;

    FixedLine       m_aSMTPFL;

    FixedText       m_aServerFT;
    Edit            m_aServerED;
    FixedText       m_aPortFT;
    NumericField    m_aPortNF;

    FixedText       m_aSecureFT;
    ListBox         m_aSecureLB;

    PushButton      m_aServerAuthenticationPB;

    FixedLine       m_aSeparatorFL;
    PushButton      m_aTestPB;

    SwMailMergeConfigItem*  m_pConfigItem;

    DECL_LINK(ReplyToHdl, CheckBox*);
    DECL_LINK(AuthenticationHdl, PushButton*);
    DECL_LINK(TestHdl, PushButton*);


public:
    SwMailConfigPage( Window* pParent, const SfxItemSet& rSet );
    ~SwMailConfigPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

};

/*-- 18.08.2004 12:02:02---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailConfigDlg : public SfxSingleTabDialog
{
public:

    SwMailConfigDlg( Window* pParent, SfxItemSet& rSet );
    ~SwMailConfigDlg();
};

#endif

