/*************************************************************************
 *
 *  $RCSfile: cuisrchdlg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:51:03 $
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

// include ---------------------------------------------------------------

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_MOREBTN_HXX //autogen wg. MoreButton
#include <vcl/morebtn.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen wg. RET_OK
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSLSTITM_HXX
#include <svtools/slstitm.hxx>
#endif
#ifndef _SFXITEMITER_HXX
#include <svtools/itemiter.hxx>
#endif
#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif
#ifndef _SVT_SEARCHOPT_HXX_
#include <svtools/searchopt.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HPP_
#include <com/sun/star/i18n/TransliterationModules.hpp>
#endif

#pragma hdrstop

#define _CUI_SRCHDLG_CXX
#include "cuisrchdlg.hxx"

#include "dialogs.hrc"
#include "svxitems.hrc"
#include "srchdlg.hrc"

#define ITEMID_SEARCH       SID_ATTR_SEARCH
#define ITEMID_SETITEM      0

#include "srchitem.hxx"
#include "pageitem.hxx"
#include "srchctrl.hxx"
//CHINA001 #include "srchxtra.hxx"
#include "dialmgr.hxx"
#include "dlgutil.hxx"

#ifndef _SVX_OPTJSEARCH_HXX_
#include <optjsearch.hxx>
#endif
#define ITEMID_BRUSH SID_ATTR_BRUSH
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <brshitem.hxx>
#endif
#ifndef _SVX_BACKGRND_HXX //autogen
#include "backgrnd.hxx"
#endif


// class SvxJSearchOptionsDialog -----------------------------------------

SvxJSearchOptionsDialog::SvxJSearchOptionsDialog(
            Window *pParent,
            const SfxItemSet& rOptionsSet, USHORT nUniqueId, INT32 nInitialFlags ) :
    SfxSingleTabDialog  ( pParent, rOptionsSet, RID_SVXPAGE_JSEARCH_OPTIONS ),
    nInitialTlFlags( nInitialFlags )
{
    pPage = (SvxJSearchOptionsPage *)
                    SvxJSearchOptionsPage::Create( this, rOptionsSet );
    SetTabPage( pPage );    //! implicitly calls pPage->Reset(...)!
    pPage->EnableSaveOptions( FALSE );
}


SvxJSearchOptionsDialog::~SvxJSearchOptionsDialog()
{
    // pPage will be implicitly destroyed by the
    // SfxSingleTabDialog destructor
}


void SvxJSearchOptionsDialog::Activate()
{
    pPage->SetTransliterationFlags( nInitialTlFlags );
}


INT32 SvxJSearchOptionsDialog::GetTransliterationFlags() const
{
    return pPage->GetTransliterationFlags();
}


void SvxJSearchOptionsDialog::SetTransliterationFlags( INT32 nSettings )
{
    pPage->SetTransliterationFlags( nSettings );
}
