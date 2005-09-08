/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: multipat.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:35:21 $
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
#ifndef _SVX_MULTIPAT_HXX
#define _SVX_MULTIPAT_HXX

// include ---------------------------------------------------------------

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

// define ----------------------------------------------------------------

// different delimiter for Unix (:) and Windows (;)

#ifdef UNX
#define CLASSPATH_DELIMITER ':'
#else
#define CLASSPATH_DELIMITER ';'
#endif

// forward ---------------------------------------------------------------

struct MultiPath_Impl;

// class SvxMultiPathDialog ----------------------------------------------

class SvxMultiPathDialog : public ModalDialog
{
protected:
    ListBox         aPathLB;
    PushButton      aAddBtn;
    PushButton      aDelBtn;
    FixedLine       aPathGB;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpButton;

    MultiPath_Impl* pImpl;

    DECL_LINK( AddHdl_Impl, PushButton * );
    DECL_LINK( DelHdl_Impl, PushButton * );
    DECL_LINK( SelectHdl_Impl, ListBox * );

public:
    SvxMultiPathDialog( Window* pParent, BOOL bEmptyAllowed = FALSE );
    ~SvxMultiPathDialog();

    String          GetPath() const;
    void            SetPath( const String& rPath );
    void            SetClassPathMode();
    sal_Bool        IsClassPathMode() const;
};

#endif // #ifndef _SVX_MULTIPAT_HXX

