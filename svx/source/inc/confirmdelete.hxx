/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: confirmdelete.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:11:34 $
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

#ifndef SVX_FORM_CONFIRMDELETE_HXX
#define SVX_FORM_CONFIRMDELETE_HXX

#ifndef _SV_BTNDLG_HXX
#include <vcl/btndlg.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= class ConfirmDeleteDialog
    //====================================================================
    class ConfirmDeleteDialog : public ButtonDialog
    {
        FixedImage  m_aInfoImage;
        FixedText   m_aTitle;
        FixedText   m_aMessage;

    public:
        ConfirmDeleteDialog(Window* pParent, const String& _rTitle);
        ~ConfirmDeleteDialog();
    };

//........................................................................
}   // namespace svxform
//........................................................................

#endif // SVX_FORM_CONFIRMDELETE_HXX


