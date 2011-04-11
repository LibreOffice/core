/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SVTOOLS_FIXEDHYPER_HXX
#define SVTOOLS_FIXEDHYPER_HXX

#include "svtools/svtdllapi.h"

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

    //=====================================================================
    //= FixedHyperlinkImage
    //=====================================================================
    class SVT_DLLPUBLIC FixedHyperlinkImage : public FixedImage
    {
    private:
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
        FixedHyperlinkImage( Window* pParent, const ResId& rId );
        FixedHyperlinkImage( Window* pParent, WinBits nWinStyle = 0 );

        /** dtor

        */
        virtual ~FixedHyperlinkImage();

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
    };
//.........................................................................
} // namespace svt
//.........................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
