/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tkresmgr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 13:52:08 $
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

#ifndef _TOOLKIT_HELPER_TKRESMGR_HXX_
#define _TOOLKIT_HELPER_TKRESMGR_HXX_

#ifndef  _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef   _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif

class SimpleResMgr;
class ResMgr;

#define TK_RES_STRING(id)   TkResMgr::loadString(id)
#define TK_RES_IMAGE(id)    TkResMgr::loadImage(id)

// -----------------------------------------------------------------------------
// TkResMgr
// -----------------------------------------------------------------------------

class TkResMgr
{
    static SimpleResMgr* m_pSimpleResMgr;
    static ResMgr* m_pResMgr;

private:
    // no instantiation allowed
    TkResMgr() { }
    ~TkResMgr() { }

    // we'll instantiate one static member of the following class,
    // which in it's dtor ensures that m_pSimpleResMgr will be deleted
    class EnsureDelete
    {
    public:
        EnsureDelete() { }
        ~EnsureDelete();
    };
    friend class EnsureDelete;

protected:
    static void ensureImplExists();

public:
    // loads the string with the specified resource id
    static ::rtl::OUString loadString( sal_uInt16 nResId );

    // loads the image with the specified resource id
    static Image loadImage( sal_uInt16 nResId );
};


#endif // _TOOLKIT_HELPER_TKRESMGR_HXX_

