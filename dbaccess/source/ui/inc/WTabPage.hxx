/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WTabPage.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:49:00 $
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

#ifndef DBAUI_WIZ_TABBPAGE_HXX
#define DBAUI_WIZ_TABBPAGE_HXX

#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif

namespace dbaui
{
    // ========================================================
    // Wizard Page
    // ========================================================
    class OCopyTableWizard;
    class OWizardPage       : public TabPage
    {
    protected:
        OCopyTableWizard*           m_pParent;
        sal_Bool                    m_bFirstTime;   // Page wird das erste mal gerufen ; should be set in the reset methode

        OWizardPage( Window* pParent, const ResId& rResId );// : TabPage(pParent,rResId),m_pParent((OCopyTableWizard*)pParent),m_bFirstTime(sal_True) {};

    public:
        virtual void        Reset ( )           = 0;
        virtual sal_Bool    LeavePage()         = 0;
        virtual String      GetTitle() const    = 0;

        sal_Bool            IsFirstTime() const { return m_bFirstTime; }
    };
}
#endif // DBAUI_WIZ_TABBPAGE_HXX



