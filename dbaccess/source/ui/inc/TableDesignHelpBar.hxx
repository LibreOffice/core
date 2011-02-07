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
#ifndef DBAUI_TABLEDESIGNHELPBAR_HXX
#define DBAUI_TABLEDESIGNHELPBAR_HXX

#include <tools/string.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include "IClipBoardTest.hxx"

class MultiLineEdit;
namespace dbaui
{
    //==================================================================
    // Ableitung von TabPage ist ein Trick von TH,
    // um Aenderungen der Systemfarben zu bemerken
    class OTableDesignHelpBar : public TabPage
                                ,public IClipboardTest
    {
    private:
        String          m_sHelpText;
        MultiLineEdit*  m_pTextWin;
        USHORT          m_nDummy;

    protected:
        virtual void Resize();

    public:
        OTableDesignHelpBar( Window* pParent );
        virtual ~OTableDesignHelpBar();

        void SetHelpText( const String& rText );
        String GetHelpText() const { return m_sHelpText; }

        virtual long PreNotify( NotifyEvent& rNEvt );

        // IClipboardTest
        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }

        virtual void copy();
        virtual void cut();
        virtual void paste();
    };
}
#endif // DBAUI_TABLEDESIGNHELPBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
