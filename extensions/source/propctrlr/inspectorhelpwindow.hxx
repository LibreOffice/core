/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inspectorhelpwindow.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 12:00:17 $
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

