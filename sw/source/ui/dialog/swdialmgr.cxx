/*************************************************************************
 *
 *  $RCSfile: swdialmgr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:21:45 $
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

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#include <tools/rc.hxx>
#include <svtools/solar.hrc>
#include <vcl/svapp.hxx>
#include "swdlgfact.hxx"
#include <sfx2/app.hxx>
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>
#endif
#ifndef _GLOSLST_HXX
#include <gloslst.hxx>
#endif
ResMgr* pSwResMgr=0;

//copy from core\bastyp\swtypes.cxx
String aEmptyStr;               // Konstante Strings
/*
//copy from ui/utlui/initui.cxx --begin
String* pOldGrfCat = 0;         //for SwCaptionDialog
String* pOldTabCat = 0;         //for SwCaptionDialog
String* pOldFrmCat = 0;         //for SwCaptionDialog
String* pOldDrwCat = 0;         //for SwCaptionDialog
String* pCurrGlosGroup = 0;         // for SwGlossaryDlg
//copy from ui/utlui/initui.cxx --end

//copy from core\bastyp\init.cxx
#include <breakit.hxx>
SwBreakIt* pBreakIt = new SwBreakIt;  //for SwAutoFormatDlg
*/
//copy from core\tox\tox.cxx
#include <tox.hxx>
const sal_Char* SwForm::aFormEntry      = "<E>";        //for SwTOXEntryTabPage
const sal_Char* SwForm::aFormTab        = "<T>";
const sal_Char* SwForm::aFormPageNums   = "<#>";
const sal_Char* SwForm::aFormLinkStt    = "<LS>";
const sal_Char* SwForm::aFormLinkEnd    = "<LE>";
const sal_Char* SwForm::aFormEntryNum   = "<E#>";
const sal_Char* SwForm::aFormEntryTxt   = "<ET>";
const sal_Char* SwForm::aFormChapterMark= "<C>";
const sal_Char* SwForm::aFormText       = "<X>";
const sal_Char* SwForm::aFormAuth       = "<A>";



ResMgr* SwDialogsResMgr::GetResMgr()
{
    if ( !pSwResMgr )
    {
        pSwResMgr = SfxApplication::CreateResManager( "sw" );
    }

    return pSwResMgr;
}
