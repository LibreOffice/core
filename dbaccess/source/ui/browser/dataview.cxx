/*************************************************************************
 *
 *  $RCSfile: dataview.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: fs $ $Date: 2001-08-23 14:41:10 $
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

#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif
#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen wg. SFX_APP
#include <sfx2/app.hxx>
#endif
#ifndef _SFXIMGMGR_HXX
#include <sfx2/imgmgr.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

//.........................................................................
namespace dbaui
{
//.........................................................................
    //=====================================================================
    //= ColorChanger
    //=====================================================================
    class ColorChanger
    {
    protected:
        OutputDevice*   m_pDev;

    public:
        ColorChanger( OutputDevice* _pDev, const Color& _rNewLineColor, const Color& _rNewFillColor )
            :m_pDev( _pDev )
        {
            m_pDev->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            m_pDev->SetLineColor( _rNewLineColor );
            m_pDev->SetFillColor( _rNewFillColor );
        }

        ~ColorChanger()
        {
            m_pDev->Pop();
        }
    };

    DBG_NAME(ODataView)
    // -------------------------------------------------------------------------
    ODataView::ODataView(Window* pParent, const Reference< XMultiServiceFactory >& _rFactory, WinBits nStyle)
        :Window(pParent,nStyle)
        ,m_pToolBox(NULL)
        ,m_xServiceFactory(_rFactory)
        ,m_pSeparator( NULL )
    {
        DBG_CTOR(ODataView,NULL);
    }

    // -------------------------------------------------------------------------
    void ODataView::Construct()
    {
    }

    // -------------------------------------------------------------------------
    ODataView::~ODataView()
    {
        setToolBox(NULL);

        enableSeparator( sal_False );

        DBG_DTOR(ODataView,NULL);
    }

    // -------------------------------------------------------------------------
    void ODataView::enableSeparator( const sal_Bool _bEnable )
    {
        if ( _bEnable == isSeparatorEnabled() )
            // nothing to do
            return;

        if ( _bEnable )
        {
            m_pSeparator = new FixedLine( this );
            m_pSeparator->Show( );
        }
        else
        {
            delete m_pSeparator;
            m_pSeparator = NULL;
        }
        Resize();
    }

    // -------------------------------------------------------------------------
    void ODataView::setToolBox(ToolBox* pTB)
    {
        if (pTB == m_pToolBox)
            return;

        if (m_pToolBox)
            delete m_pToolBox;

        m_pToolBox = pTB;
        if (m_pToolBox)
        {
            m_pToolBox->SetParent(this);
            m_pToolBox->SetOutStyle(TOOLBOX_STYLE_FLAT);
            m_pToolBox->Show();
        }

        // rearrange the grid and the TB
        Resize();
    }
    // -------------------------------------------------------------------------
    void ODataView::resizeDocumentView( Rectangle& _rPlayground )
    {
    }

    // -------------------------------------------------------------------------
    void ODataView::Paint( const Rectangle& _rRect )
    {
        //.................................................................
        // draw the background
        {
            ColorChanger aColors( this, COL_TRANSPARENT, GetSettings().GetStyleSettings().GetFaceColor() );
            DrawRect( _rRect );
        }

        // let the base class do anything it needs
        Window::Paint( _rRect );
    }

    // -------------------------------------------------------------------------
    void ODataView::resizeAll( const Rectangle& _rPlayground )
    {
        Rectangle aPlayground( _rPlayground );

        // position thew separator
        if ( m_pSeparator )
        {
            Size aSeparatorSize = Size( aPlayground.GetWidth(), 2 );

            m_pSeparator->SetPosSizePixel( aPlayground.TopLeft(), aSeparatorSize );

            aPlayground.Top() += aSeparatorSize.Height() + 1;
        }

        // position the tool box
        if ( m_pToolBox )
        {
            m_pToolBox->SetPosPixel( aPlayground.TopLeft() );

            Size aToolboxSize( aPlayground.GetSize().Width(), m_pToolBox->GetSizePixel().Height() );
            m_pToolBox->SetSizePixel( aToolboxSize );

            aPlayground.Top() += aToolboxSize.Height();
        }

        // position the controls of the document's view
        resizeDocumentView( aPlayground );
    }

    // -------------------------------------------------------------------------
    void ODataView::Resize()
    {
        Window::Resize();
        resizeAll( Rectangle( Point( 0, 0), GetSizePixel() ) );
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................
