/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: descedit.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:25:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_DESCEDIT_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_DESCEDIT_HXX

#ifndef _SVTOOLS_SVMEDIT2_HXX
#include "svtools/svmedit2.hxx"
#endif

/// @HTML

namespace dp_gui
{

    class DescriptionEdit : public ExtMultiLineEdit
    {
    private:
        bool                m_bIsVerticalScrollBarHidden;

        void                Init();
        void                UpdateScrollBar();

    public:
        DescriptionEdit( Window* pParent, const ResId& rResId );
        inline ~DescriptionEdit() {}

        void                Clear();
        void                SetDescription( const String& rDescription );
    };

} // namespace dp_gui

#endif // INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_DESCEDIT_HXX

