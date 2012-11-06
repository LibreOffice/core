/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
//.........................................................................
} // namespace svt
//.........................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
