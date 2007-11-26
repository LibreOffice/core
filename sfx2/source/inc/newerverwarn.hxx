/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: newerverwarn.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 18:24:35 $
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
#ifndef _SFX2_NEWERVERSIONWARNING_HXX
#define _SFX2_NEWERVERSIONWARNING_HXX

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/basedlgs.hxx>

namespace sfx2
{
    class NewerVersionWarningDialog : public SfxModalDialog
    {
    private:
        FixedImage      m_aImage;
        FixedText       m_aInfoText;
        FixedLine       m_aButtonLine;
        PushButton      m_aUpdateBtn;
        CancelButton    m_aLaterBtn;

        DECL_LINK(      UpdateHdl, PushButton* );
        DECL_LINK(      LaterHdl, CancelButton* );

        void            InitButtonWidth();

    public:
        NewerVersionWarningDialog( Window* pParent );
        ~NewerVersionWarningDialog();
    };
} // namespace sfx2

#endif // #ifndef _SFX2_NEWERVERSIONWARNING_HXX

