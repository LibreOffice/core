/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fixedhyper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:24:43 $
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
#ifndef SVTOOLS_FIXEDHYPER_HXX
#define SVTOOLS_FIXEDHYPER_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#include <toolkit/helper/fixedhyperbase.hxx>

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= FixedHyperlink
    //=====================================================================
    class SVT_DLLPUBLIC FixedHyperlink : public ::toolkit::FixedHyperlinkBase
    {
    private:
        long                m_nTextLen;
        Pointer             m_aOldPointer;
        Link                m_aClickHdl;
        String              m_sURL;

        /** initializes the font (link color and underline).

            Called by the Ctors.
        */
        void                Initialize();

    protected:
        /** overwrites Window::MouseMove().

            Changes the pointer only over the text.
        */
        virtual void        MouseMove( const MouseEvent& rMEvt );

        /** overwrites Window::MouseButtonUp().

            Calls the set link if the mouse is over the text.
        */
        virtual void        MouseButtonUp( const MouseEvent& rMEvt );

        /** overwrites Window::RequestHelp().

            Shows tooltip only if the mouse is over the text.
        */
        virtual void        RequestHelp( const HelpEvent& rHEvt );

    public:
        /** ctors

            With ResId or WinBits.
        */
        FixedHyperlink( Window* pParent, const ResId& rId );
        FixedHyperlink( Window* pParent, WinBits nWinStyle = 0 );

        /** dtor

        */
        virtual ~FixedHyperlink();

        /** overwrites Window::GetFocus().

            Changes the color of the text and shows a focus rectangle.
        */
        virtual void        GetFocus();

        /** overwrites Window::LoseFocus().

            Changes the color of the text and hides the focus rectangle.
        */
        virtual void        LoseFocus();

        /** overwrites Window::KeyInput().

            KEY_RETURN and KEY_SPACE calls the link handler.
        */
        virtual void        KeyInput( const KeyEvent& rKEvt );

        /** sets <member>m_aClickHdl</member> with <arg>rLink</arg>.

            <member>m_aClickHdl</member> is called if the text is clicked.
        */
        inline void         SetClickHdl( const Link& rLink ) { m_aClickHdl = rLink; }

        /** returns <member>m_aClickHdl</member>.

            @return
                <member>m_aClickHdl</member>
        */
        inline const Link&  GetClickHdl() const { return m_aClickHdl; }

        // ::toolkit::FixedHyperbaseLink

        /** sets the URL of the hyperlink and uses it as tooltip. */
        virtual void        SetURL( const String& rNewURL );

        /** returns the URL of the hyperlink.

            @return
                <member>m_sURL</member>
        */
        virtual String      GetURL() const;

        /** sets new text and recalculates the text length. */
        virtual void        SetDescription( const String& rNewDescription );
    };

//.........................................................................
} // namespace svt
//.........................................................................

#endif

