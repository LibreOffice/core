/*************************************************************************
 *
 *  $RCSfile: alienwarn.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2005-04-04 15:30:41 $
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

#include "alienwarn.hxx"
#include "sfxresid.hxx"
#include "sfxuno.hxx"

#include "alienwarn.hrc"
#include "dialog.hrc"

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif

// class SfxAlienWarningDialog -------------------------------------------

SfxAlienWarningDialog::SfxAlienWarningDialog( Window* pParent, const String& _rFormatName ) :

    SfxModalDialog( pParent, SfxResId( RID_DLG_ALIEN_WARNING ) ),

    m_aQueryImage   ( this, ResId( FI_QUERY ) ),
    m_aInfoText     ( this, ResId( FT_INFOTEXT ) ),
    m_aYesBtn       ( this, ResId( PB_YES ) ),
    m_aNoBtn        ( this, ResId( PB_NO ) ),
    m_aMoreInfoBtn  ( this, ResId( PB_MOREINFO ) ),
    m_aOptionLine   ( this, ResId( FL_OPTION ) ),
    m_aWarningOffBox( this, ResId( CB_WARNING_OFF ) )

{
    FreeResource();

    // set questionmark image
    m_aQueryImage.SetImage( QueryBox::GetStandardImage() );

    // replace formatname
    String sInfoText = m_aInfoText.GetText();
    sInfoText.SearchAndReplaceAll( DEFINE_CONST_UNICODE("%FORMATNAME"), _rFormatName );
    m_aInfoText.SetText( sInfoText );

    // if the button text is too wide, then broaden the button
    long nTxtW = m_aMoreInfoBtn.GetTextWidth( m_aMoreInfoBtn.GetText() );
    long nCtrlW = m_aMoreInfoBtn.GetSizePixel().Width();
    if ( nTxtW >= nCtrlW )
    {
        long nDelta = nTxtW - nCtrlW;
        nDelta += 6; // a little more space looks better
        Point aNextPoint = m_aNoBtn.GetPosPixel();
        aNextPoint.X() += m_aNoBtn.GetSizePixel().Width();
        Point aNewPoint = m_aMoreInfoBtn.GetPosPixel();
        aNewPoint.X() -= nDelta;
        if ( aNextPoint.X() >= aNewPoint.X() )
        {
            long nSpace = aNextPoint.X() - aNewPoint.X();
            nSpace += 2;
            nDelta -= nSpace;
            aNewPoint.X() += nSpace;
        }
        Size aNewSize = m_aMoreInfoBtn.GetSizePixel();
        aNewSize.Width() += nDelta;
        m_aMoreInfoBtn.SetPosSizePixel( aNewPoint, aNewSize );
    }

    // text of checkbox to wide -> add new line
    nTxtW = m_aWarningOffBox.GetTextWidth( m_aWarningOffBox.GetText() );
    nCtrlW = m_aWarningOffBox.GetSizePixel().Width();
    if ( nTxtW >= nCtrlW )
    {
        long nTextHeight = m_aWarningOffBox.GetTextHeight();
        Size aNewSize = m_aWarningOffBox.GetSizePixel();
        aNewSize.Height() += nTextHeight;
        m_aWarningOffBox.SetSizePixel( aNewSize );
        aNewSize = GetSizePixel();
        aNewSize.Height() += nTextHeight;
        SetSizePixel( aNewSize );
    }

    // load value of "warning off" checkbox from save options
    m_aWarningOffBox.Check( SvtSaveOptions().IsWarnAlienFormat() == sal_False );

    // set focus to "No" button
    m_aNoBtn.GrabFocus();
}

// -----------------------------------------------------------------------

SfxAlienWarningDialog::~SfxAlienWarningDialog()
{
    // save value of "warning off" checkbox, if necessary
    SvtSaveOptions aSaveOpt;
    sal_Bool bChecked = !m_aWarningOffBox.IsChecked();
    if ( aSaveOpt.IsWarnAlienFormat() != bChecked )
        aSaveOpt.SetWarnAlienFormat( bChecked );
}

