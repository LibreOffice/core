/*************************************************************************
 *
 *  $RCSfile: fmtextcontroldialogs.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 15:47:48 $
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

#ifndef SVX_SOURCE_FORM_FMTEXTCONTROLDIALOGS_HXX
#include "fmtextcontroldialogs.hxx"
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif
#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif

#ifndef _SVX_CHARDLG_HXX
#include "chardlg.hxx"
#endif
#ifndef _SVX_PARAGRPH_HXX
#include "paragrph.hxx"
#endif
#ifndef _EEITEM_HXX
#include "eeitem.hxx"
#endif
#define ITEMID_TABSTOP      EE_PARA_TABS
#ifndef _SVX_TABSTPGE_HXX
#include "tabstpge.hxx"
#endif

#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= TextControlCharAttribDialog
    //====================================================================
    //--------------------------------------------------------------------
    TextControlCharAttribDialog::TextControlCharAttribDialog( Window* pParent, const SfxItemSet& _rCoreSet, const SvxFontListItem& _rFontList )
        :SfxTabDialog( pParent, SVX_RES( RID_SVXDLG_TEXTCONTROL_CHARATTR ), &_rCoreSet )
        ,m_aFontList( _rFontList )
    {
        FreeResource();

        AddTabPage( 1, SvxCharNamePage::Create, NULL );
        AddTabPage( 2, SvxCharEffectsPage::Create, NULL );
        AddTabPage( 3, SvxCharPositionPage::Create, NULL );
    }

    //--------------------------------------------------------------------
    TextControlCharAttribDialog::~TextControlCharAttribDialog()
    {
    }

    //--------------------------------------------------------------------
    void TextControlCharAttribDialog::PageCreated( USHORT _nId, SfxTabPage& _rPage )
    {
        switch( _nId )
        {
            case 1:
                static_cast< SvxCharNamePage& >( _rPage ).SetFontList( m_aFontList );
                break;

            case 2:
                static_cast< SvxCharEffectsPage& > ( _rPage ).DisableControls( DISABLE_CASEMAP );
                break;

            case 3:
                static_cast< SvxCharPositionPage& >( _rPage ).SetPreviewBackgroundToCharacter();
                break;
        }
    }

    //====================================================================
    //= TextControlParaAttribDialog
    //====================================================================
    //--------------------------------------------------------------------
    TextControlParaAttribDialog::TextControlParaAttribDialog( Window* _pParent, const SfxItemSet& _rCoreSet )
        :SfxTabDialog( _pParent, SVX_RES( RID_SVXDLG_TEXTCONTROL_PARAATTR ), &_rCoreSet )
    {
        FreeResource();

        AddTabPage( 1, SvxStdParagraphTabPage::Create, SvxStdParagraphTabPage::GetRanges );
        AddTabPage( 2, SvxParaAlignTabPage::Create, SvxParaAlignTabPage::GetRanges );

        SvtCJKOptions aCJKOptions;
        if( aCJKOptions.IsAsianTypographyEnabled() )
            AddTabPage( 3,  SvxAsianTabPage::Create, SvxAsianTabPage::GetRanges );
        else
            RemoveTabPage( 3 );

        AddTabPage( 4, SvxTabulatorTabPage::Create,   SvxTabulatorTabPage::GetRanges );
    }

    //--------------------------------------------------------------------
    TextControlParaAttribDialog::~TextControlParaAttribDialog()
    {
    }

    //--------------------------------------------------------------------
    void TextControlParaAttribDialog::PageCreated( USHORT _nId, SfxTabPage& _rPage )
    {
    }

//........................................................................
}   // namespace svx
//........................................................................

