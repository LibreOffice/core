/*************************************************************************
 *
 *  $RCSfile: TableFieldDescWin.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 14:25:40 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

#ifndef DBAUI_TABLEFIELDDESCRIPTION_HXX
#include "TableFieldDescWin.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif
#ifndef DBAUI_TABLEDESIGNHELPBAR_HXX
#include "TableDesignHelpBar.hxx"
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif

#define STANDARD_MARGIN                  6
#define DETAILS_HEADER_HEIGHT           25
#define CONTROL_SPACING_X   18  // 6
#define CONTROL_SPACING_Y   5
#define CONTROL_HEIGHT      20
#define CONTROL_WIDTH_1     140 // 100
#define CONTROL_WIDTH_2     100 // 60
#define CONTROL_WIDTH_3     250
#define CONTROL_WIDTH_4     (CONTROL_WIDTH_3 - CONTROL_HEIGHT - 5)
#define DETAILS_OPT_PAGE_WIDTH          (CONTROL_WIDTH_1 + CONTROL_SPACING_X + CONTROL_WIDTH_4 + 50)
#define DETAILS_OPT_PAGE_HEIGHT         ((CONTROL_HEIGHT + CONTROL_SPACING_Y) * 5)
#define DETAILS_MIN_HELP_WIDTH          100
#define DETAILS_OPT_HELP_WIDTH          200
#define DETAILS_MIN_HELP_HEIGHT         50
#define DETAILS_OPT_HELP_HEIGHT         100


using namespace dbaui;
//==================================================================
// class OTableFieldDescWin
//==================================================================
DBG_NAME(OTableFieldDescWin);
//------------------------------------------------------------------------------
OTableFieldDescWin::OTableFieldDescWin( Window* pParent)
    :TabPage(pParent, WB_3DLOOK)
{
    DBG_CTOR(OTableFieldDescWin,NULL);
    //////////////////////////////////////////////////////////////////////
    // Header
    m_pHeader = new FixedText( this, WB_CENTER | WB_INFO ); //  | WB_3DLOOK
    m_pHeader->SetText( String(ModuleRes(STR_TAB_PROPERTIES)) );
    m_pHeader->Show();

    //////////////////////////////////////////////////////////////////////
    // HelpBar
    m_pHelpBar = new OTableDesignHelpBar( this );
    m_pHelpBar->SetHelpId(HID_TAB_DESIGN_HELP_TEXT_FRAME);
    m_pHelpBar->Show();

    m_pGenPage = new OFieldDescGenWin( this, m_pHelpBar );
    m_pGenPage->SetHelpId( HID_TABLE_DESIGN_TABPAGE_GENERAL );
    m_pGenPage->Show();
}

//------------------------------------------------------------------------------
OTableFieldDescWin::~OTableFieldDescWin()
{
    DBG_DTOR(OTableFieldDescWin,NULL);
    //////////////////////////////////////////////////////////////////////
    // Childs zerstoeren
    m_pHelpBar->Hide();
    m_pGenPage->Hide();
    m_pHeader->Hide();

    delete m_pHelpBar;
    delete m_pGenPage;
    delete m_pHeader;
}

//------------------------------------------------------------------------------
void OTableFieldDescWin::Init()
{
    DBG_ASSERT(m_pGenPage != NULL, "OTableFieldDescWin::Init : ups ... no GenericPage ... this will crash ...");
    m_pGenPage->Init();
}

//------------------------------------------------------------------------------
void OTableFieldDescWin::SetReadOnly( sal_Bool bRead )
{
    DBG_CHKTHIS(OTableFieldDescWin,NULL);
    m_pGenPage->SetReadOnly( bRead );
}

//------------------------------------------------------------------------------
void OTableFieldDescWin::DisplayData( OFieldDescription* pFieldDescr )
{
    DBG_CHKTHIS(OTableFieldDescWin,NULL);
    m_pGenPage->DisplayData( pFieldDescr );
}

//------------------------------------------------------------------------------
void OTableFieldDescWin::SaveData( OFieldDescription* pFieldDescr )
{
    DBG_CHKTHIS(OTableFieldDescWin,NULL);
    m_pGenPage->SaveData( pFieldDescr );
}

//------------------------------------------------------------------------------
void OTableFieldDescWin::Paint( const Rectangle& rRect )
{
    DBG_CHKTHIS(OTableFieldDescWin,NULL);
    //////////////////////////////////////////////////////////////////////
    // 3D-Linie am oberen Fensterrand
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    SetLineColor( rStyleSettings.GetLightColor() );
    DrawLine( Point(0,0), Point(GetSizePixel().Width(),0) );

    //////////////////////////////////////////////////////////////////////
    // 3D-Linie zum Abtrennen des Headers
    DrawLine( Point(3, DETAILS_HEADER_HEIGHT), Point(GetSizePixel().Width()-6, DETAILS_HEADER_HEIGHT) );
    SetLineColor( rStyleSettings.GetShadowColor() );
    DrawLine( Point(3, DETAILS_HEADER_HEIGHT-1), Point(GetSizePixel().Width()-6, DETAILS_HEADER_HEIGHT-1) );
}

//------------------------------------------------------------------------------
void OTableFieldDescWin::Resize()
{
    DBG_CHKTHIS(OTableFieldDescWin,NULL);
    //////////////////////////////////////////////////////////////////////
    // Abmessungen parent window
    Size aOutputSize( GetOutputSizePixel() );
    long nOutputWidth = aOutputSize.Width();
    long nOutputHeight = aOutputSize.Height();

    // da die GenPage scrollen kann, ich selber aber nicht, positioniere ich das HelpFenster, wenn ich zu schmal werde,
    // _unter_ der Genpage, nicht rechts daneben. Zuvor versuche ich aber noch, es etwas schmaler zu machen

    long nHelpX, nHelpY;
    long nHelpWidth, nHelpHeight;
    long nPageWidth, nPageHeight;

    // passen beide nebeneinander (Rand + Page + Rand + Help) ?
    if (STANDARD_MARGIN + DETAILS_OPT_PAGE_WIDTH + STANDARD_MARGIN + DETAILS_MIN_HELP_WIDTH <= nOutputWidth)
    {   // ja -> dann ist die Frage, ob man der Hilfe ihre Optimal-Breite geben kann
        nHelpWidth = DETAILS_OPT_HELP_WIDTH;
        nPageWidth = nOutputWidth - nHelpWidth - STANDARD_MARGIN - STANDARD_MARGIN;
        if (nPageWidth < DETAILS_OPT_PAGE_WIDTH)
        {   // dann doch lieber die Hilfe von ihrer optimalen in Richtung auf die minimale Groesse
            long nTransfer = DETAILS_OPT_PAGE_WIDTH - nPageWidth;
            nPageWidth += nTransfer;
            nHelpWidth -= nTransfer;
        }
        nHelpX = nOutputWidth - nHelpWidth;
        // die Hoehen sind dann einfach ...
        nHelpY = DETAILS_HEADER_HEIGHT + 1;
        nHelpHeight = nOutputHeight - nHelpY;
        nPageHeight = nOutputHeight - STANDARD_MARGIN - DETAILS_HEADER_HEIGHT - STANDARD_MARGIN;
    }
    else
    {   // nebeneinander geht nicht, also untereinander (Rand + Header + Page + Help)
        if (STANDARD_MARGIN + DETAILS_HEADER_HEIGHT + DETAILS_OPT_PAGE_HEIGHT + DETAILS_MIN_HELP_HEIGHT <= nOutputHeight)
        {   // es reicht zumindest, um beide untereinander (Page optimal, Help minimal) unterzubringen
            nHelpHeight = DETAILS_OPT_HELP_HEIGHT;
            nPageHeight = nOutputHeight - nHelpHeight - DETAILS_HEADER_HEIGHT - STANDARD_MARGIN;
            if (nPageHeight < DETAILS_OPT_PAGE_HEIGHT)
            {   // wie oben : Page optimal, Hilfe soviel wie eben bleibt (das ist groesser/gleich ihrem Minimum)
                long nTransfer = DETAILS_OPT_PAGE_HEIGHT - nPageHeight;
                nPageHeight += nTransfer;
                nHelpHeight -= nTransfer;
            }
            nHelpY = nOutputHeight - nHelpHeight;
            // und ueber die ganze Breite
            nHelpX = 0;                 // ohne Margin, da das HelpCtrl einen eigenen hat
            nHelpWidth = nOutputWidth;  // dito
            nPageWidth = nOutputWidth - STANDARD_MARGIN - STANDARD_MARGIN;
        }
        else
        {   // dummerweise reicht es nicht mal, um Page optimal und Help minimal zu zeigen
            nHelpX = nHelpY = nHelpWidth = nHelpHeight = 0; // -> kein Help-Fenster
            nPageWidth = nOutputWidth - STANDARD_MARGIN - STANDARD_MARGIN;
            nPageHeight = nOutputHeight - STANDARD_MARGIN - DETAILS_HEADER_HEIGHT - STANDARD_MARGIN;
        }
    }

    m_pHeader->SetPosSizePixel( Point(0, STANDARD_MARGIN), Size(nOutputWidth, 15) );

    m_pGenPage->SetPosSizePixel(Point   (   STANDARD_MARGIN,
                                        STANDARD_MARGIN + DETAILS_HEADER_HEIGHT
                                    ),
                              Size  (   nPageWidth,
                                        nPageHeight
                                    )
                             );
    if (nHelpHeight)
    {
        m_pHelpBar->Show();
        m_pHelpBar->SetPosSizePixel(Point   (   nHelpX,
                                            nHelpY
                                        ),
                                  Size  (   nHelpWidth,
                                            nHelpHeight
                                        )
                                 );
    }
    else
    {
        m_pHelpBar->Hide();
    }
    Invalidate();
}
