/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dataview.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:56:16 $
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

#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
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
#ifndef DBAUI_ICONTROLLER_HXX
#include "IController.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _SFX_HRC
#include <sfx2/sfx.hrc>
#endif
#ifndef _SVTOOLS_IMGDEF_HXX
#include <svtools/imgdef.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;

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
    ODataView::ODataView(   Window* pParent,
                            IController* _pController,
                            const Reference< XMultiServiceFactory >& _rFactory,
                            WinBits nStyle)
        :Window(pParent,nStyle)
        ,m_xServiceFactory(_rFactory)
        ,m_pController( _pController )
        ,m_pSeparator( NULL )
    {
        DBG_CTOR(ODataView,NULL);
        OSL_ENSURE(m_pController,"Controller must be not NULL!");
        m_pController->acquire();
        m_pAccel.reset(::svt::AcceleratorExecute::createAcceleratorHelper());
    }

    // -------------------------------------------------------------------------
    void ODataView::Construct()
    {
    }

    // -------------------------------------------------------------------------
    ODataView::~ODataView()
    {
        DBG_DTOR(ODataView,NULL);

        enableSeparator( sal_False );
        m_pController->release();
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
            ::std::auto_ptr<FixedLine> aTemp(m_pSeparator);
            m_pSeparator = NULL;
        }
        Resize();
    }
    // -------------------------------------------------------------------------
    void ODataView::resizeDocumentView( Rectangle& /*_rPlayground*/ )
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

        // position the controls of the document's view
        resizeDocumentView( aPlayground );
    }

    // -------------------------------------------------------------------------
    void ODataView::Resize()
    {
        Window::Resize();
        resizeAll( Rectangle( Point( 0, 0), GetSizePixel() ) );
    }
    // -----------------------------------------------------------------------------
    long ODataView::PreNotify( NotifyEvent& rNEvt )
    {
        BOOL bHandled = FALSE;
        switch (rNEvt.GetType())
        {
            case EVENT_KEYINPUT:
            {
                const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
                const KeyCode& aKeyCode = pKeyEvent->GetKeyCode();
                if ( m_pAccel.get() )
                {
                    bHandled = m_pAccel->execute(aKeyCode);
                }
            }
            break;
        }
        return bHandled ? 1L : Window::PreNotify(rNEvt);
    }
    // -----------------------------------------------------------------------------
    void ODataView::StateChanged( StateChangedType nType )
    {
        Window::StateChanged( nType );

        if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        {
            // Check if we need to get new images for normal/high contrast mode
            m_pController->notifyHiContrastChanged();
        }
    }
    // -----------------------------------------------------------------------------
    void ODataView::DataChanged( const DataChangedEvent& rDCEvt )
    {
        Window::DataChanged( rDCEvt );

        if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS )   ||
            ( rDCEvt.GetType() == DATACHANGED_DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & SETTINGS_STYLE        ))
        {
            // Check if we need to get new images for normal/high contrast mode
            m_pController->notifyHiContrastChanged();
        }
    }
    // -----------------------------------------------------------------------------
    void ODataView::attachFrame(const Reference< XFrame >& _xFrame)
    {
        m_pAccel->init(m_xServiceFactory,_xFrame);
    }
//.........................................................................
}
// namespace dbaui
//.........................................................................
