/*************************************************************************
 *
 *  $RCSfile: cuitbxform.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:33:25 $
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

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SVX_FMITEMS_HXX //autogen
#include "fmitems.hxx"
#endif

#ifndef _TOOLS_INTN_HXX //autogen
#include <tools/intn.hxx>
#endif

#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif

#pragma hdrstop

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif
#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif
/*
#ifndef _TBXCTL_HXX
#include "tbxctl.hxx"
#endif
#ifndef _TBXDRAW_HXX
#include "tbxdraw.hxx"
#endif
*/
#ifndef _CUI_TBXFORM_HXX
#include "cuitbxform.hxx"
#endif
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#ifndef _SVX_FMITEMS_HXX
#include "fmitems.hxx"
#endif
#ifndef _SVX_FMHELP_HRC
#include "fmhelp.hrc"
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
//========================================================================
// class FmInputRecordNoDialog
//========================================================================

FmInputRecordNoDialog::FmInputRecordNoDialog(Window * pParent)
    :ModalDialog( pParent, SVX_RES(RID_SVX_DLG_INPUTRECORDNO))
    ,m_aLabel(this, ResId(1))
    ,m_aRecordNo(this, ResId(1))
    ,m_aOk(this, ResId(1))
    ,m_aCancel(this, ResId(1))
{
    m_aRecordNo.SetMin(1);
    m_aRecordNo.SetMax(0x7FFFFFFF);
    m_aRecordNo.SetStrictFormat(TRUE);
    m_aRecordNo.SetDecimalDigits(0);

    FreeResource();
}
