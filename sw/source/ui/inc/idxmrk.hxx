/*************************************************************************
 *
 *  $RCSfile: idxmrk.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:28:42 $
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
#ifndef _IDXMRK_HXX
#define _IDXMRK_HXX

//CHINA001 #ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
//CHINA001 #include <com/sun/star/container/XNameAccess.hpp>
//CHINA001 #endif
//CHINA001 #ifndef _BASEDLGS_HXX
//CHINA001 #include <sfx2/basedlgs.hxx>
//CHINA001 #endif
//CHINA001
//CHINA001 #ifndef _FIXED_HXX //autogen
//CHINA001 #include <vcl/fixed.hxx>
//CHINA001 #endif

//CHINA001 #ifndef _SV_LSTBOX_HXX
//CHINA001 #include <vcl/lstbox.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _COMBOBOX_HXX //autogen
//CHINA001 #include <vcl/combobox.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _SVX_STDDLG_HXX
//CHINA001 #include <svx/stddlg.hxx>
//CHINA001 #endif
//CHINA001
//CHINA001 #ifndef _FIELD_HXX //autogen
//CHINA001 #include <vcl/field.hxx>
//CHINA001 #endif
//CHINA001
//CHINA001 #ifndef _GROUP_HXX //autogen
//CHINA001 #include <vcl/group.hxx>
//CHINA001 #endif

//CHINA001 #ifndef _BUTTON_HXX //autogen
//CHINA001 #include <vcl/button.hxx>
//CHINA001 #endif
//CHINA001
//CHINA001 #ifndef _IMAGEBTN_HXX //autogen
//CHINA001 #include <vcl/imagebtn.hxx>
//CHINA001 #endif
#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif
//CHINA001 #ifndef _TOXE_HXX
//CHINA001 #include "toxe.hxx"
//CHINA001 #endif
//CHINA001 #ifndef _STDCTRL_HXX
//CHINA001 #include <svtools/stdctrl.hxx>
//CHINA001 #endif
//CHINA001 #ifndef  _COM_SUN_STAR_I18N_XEXTENDEDINDEXENTRYSUPPLIER_HPP_
//CHINA001 #include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
//CHINA001 #endif
#include "swabstdlg.hxx" //CHINA001
class SwWrtShell;

/* -----------------07.09.99 08:02-------------------

 --------------------------------------------------*/
class SwInsertIdxMarkWrapper : public SfxChildWindow
{
    AbstractMarkFloatDlg*   pAbstDlg; //CHINA001
protected:
    SwInsertIdxMarkWrapper( Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SwInsertIdxMarkWrapper);

public:
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

/* -----------------07.09.99 08:02-------------------

 --------------------------------------------------*/
class SwInsertAuthMarkWrapper : public SfxChildWindow
{
    AbstractMarkFloatDlg*   pAbstDlg; //CHINA001
protected:
    SwInsertAuthMarkWrapper(    Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SwInsertAuthMarkWrapper);

public:
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

#endif // _IDXMRK_HXX

