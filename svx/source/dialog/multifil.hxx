/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: multifil.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-06-05 14:33:51 $
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
#ifndef _SVX_MULTIFIL_HXX
#define _SVX_MULTIFIL_HXX

// include ---------------------------------------------------------------

#ifndef _SVX_MULTIPAT_HXX
#include "multipat.hxx"
#endif

// #97807# ----------------------------------------------------
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#include <map>

// class SvxMultiFileDialog ----------------------------------------------

class SvxMultiFileDialog : public SvxMultiPathDialog
{
private:
    // #97807# -------------------------------------
    std::map< String, ::ucbhelper::Content >   aFileContentMap;

    DECL_LINK( AddHdl_Impl, PushButton * );
    DECL_LINK( DelHdl_Impl, PushButton * );

public:
    SvxMultiFileDialog( Window* pParent, BOOL bEmptyAllowed = FALSE );
    ~SvxMultiFileDialog();

    String  GetFiles() const { return SvxMultiPathDialog::GetPath(); }
    void    SetFiles( const String& rPath ) { SvxMultiPathDialog::SetPath(rPath); aDelBtn.Enable(); }
};


#endif

