/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helpagentwindow.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:36:03 $
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

#ifndef _SVTOOLS_HELPAGENTWIDNOW_HXX_
#define _SVTOOLS_HELPAGENTWIDNOW_HXX_

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SV_FLOATWIN_HXX
#include <vcl/floatwin.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= IHelpAgentCallback
    //====================================================================
    class IHelpAgentCallback
    {
    public:
        virtual void helpRequested() = 0;
        virtual void closeAgent() = 0;
    };

    //====================================================================
    //= HelpAgentWindow
    //====================================================================
    class SVT_DLLPUBLIC HelpAgentWindow : public FloatingWindow
    {
    protected:
        Window*                 m_pCloser;
        IHelpAgentCallback*     m_pCallback;
        Size                    m_aPreferredSize;
        Image                   m_aPicture;

    public:
        HelpAgentWindow( Window* _pParent );
        ~HelpAgentWindow();

        /// returns the preferred size of the window
        const Size& getPreferredSizePixel() const { return m_aPreferredSize; }

        // callback handler maintainance
        void                setCallback(IHelpAgentCallback* _pCB) { m_pCallback = _pCB; }
        IHelpAgentCallback* getCallback() const { return m_pCallback; }

    protected:
        virtual void Resize();
        virtual void Paint( const Rectangle& rRect );
        virtual void MouseButtonUp( const MouseEvent& rMEvt );

        DECL_LINK( OnButtonClicked, Window* );

    private:
        SVT_DLLPRIVATE Size implOptimalButtonSize( const Image& _rButtonImage );
    };

//........................................................................
}   // namespace svt
//........................................................................

#endif  // _SVTOOLS_HELPAGENTWIDNOW_HXX_

