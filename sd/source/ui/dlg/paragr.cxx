/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: paragr.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:41:18 $
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
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include <svx/editdata.hxx>


#include <svx/dialogs.hrc>
#include "eetext.hxx"
#include <svx/eeitem.hxx>
//CHINA001 #include <svx/tabstpge.hxx>

//CHINA001 #ifndef _SVX_PARAGRPH_HXX //autogen
//CHINA001 #include <svx/paragrph.hxx>
//CHINA001 #endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#include <svx/flagsdef.hxx> //CHINA001
#include "paragr.hxx"
#include "sdresid.hxx"

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

SdParagraphDlg::SdParagraphDlg( Window* pParent, const SfxItemSet* pAttr ) :
        SfxTabDialog        ( pParent, SdResId( TAB_PARAGRAPH ), pAttr ),
        rOutAttrs           ( *pAttr )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_STD_PARAGRAPH );//CHINA001 AddTabPage( RID_SVXPAGE_STD_PARAGRAPH, SvxStdParagraphTabPage::Create, 0);

    SvtCJKOptions aCJKOptions;
    if( aCJKOptions.IsAsianTypographyEnabled() )
        AddTabPage( RID_SVXPAGE_PARA_ASIAN);//CHINA001 AddTabPage( RID_SVXPAGE_PARA_ASIAN, SvxAsianTabPage::Create,0);
    else
        RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );

    AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH );//CHINA001 ddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH, SvxParaAlignTabPage::Create,0);
    AddTabPage( RID_SVXPAGE_TABULATOR );//CHINA001 AddTabPage( RID_SVXPAGE_TABULATOR, SvxTabulatorTabPage::Create, 0);
}
