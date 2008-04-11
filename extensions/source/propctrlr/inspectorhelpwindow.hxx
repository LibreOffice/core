/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: inspectorhelpwindow.hxx,v $
 * $Revision: 1.3 $
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
#ifndef INSPECTORHELPWINDOW_HXX
#define INSPECTORHELPWINDOW_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <vcl/fixed.hxx>
#include <svtools/svmedit.hxx>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= InspectorHelpWindow
    //====================================================================
    class InspectorHelpWindow : public Window
    {
    private:
        FixedLine       m_aSeparator;
        MultiLineEdit   m_aHelpText;

        sal_Int32       m_nMinLines;
        sal_Int32       m_nMaxLines;

    public:
        InspectorHelpWindow( Window* _pParent );

        virtual void    SetText( const XubString& rStr );

        void            SetLimits( sal_Int32 _nMinLines, sal_Int32 _nMaxLines );
        long            GetMinimalHeightPixel();
        long            GetOptimalHeightPixel();
        long            GetMaximalHeightPixel();

    protected:
        // Window overridables
        virtual void    Resize();

    private:
        long            impl_getMinimalTextWindowHeight();
        long            impl_getMaximalTextWindowHeight();
        long            impl_getHelpTextBorderHeight();
        long            impl_getSpaceAboveTextWindow();
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // HELPWINDOW_HXX

