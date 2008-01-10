/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: saxobject.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 12:50:56 $
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

#ifndef _SAX_OBJECT_HXX_
#define _SAX_OBJECT_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef INCLUDED_SAX_DLLAPI_H
#include "sax/dllapi.h"
#endif

namespace sax
{

    /** simple base class to allow refcounting with rtl::Reference or css::uno::Reference */
    class SAX_DLLPUBLIC SaxObject
    {
    public:
        SaxObject();
        virtual ~SaxObject();
        virtual void SAL_CALL acquire() throw ();
        virtual void SAL_CALL release() throw ();
    private:
        sal_uInt32 mnRefCount;
    };
}

#endif // _SAX_OBJECT_HXX_
