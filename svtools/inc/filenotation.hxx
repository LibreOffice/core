/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filenotation.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:27:21 $
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

#ifndef SVTOOLS_FILENOTATION_HXX
#define SVTOOLS_FILENOTATION_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= OFileNotation
    //=====================================================================
    class SVL_DLLPUBLIC OFileNotation
    {
    protected:
        ::rtl::OUString     m_sSystem;
        ::rtl::OUString     m_sFileURL;

    public:
        enum NOTATION
        {
            N_SYSTEM,
            N_URL
        };

        OFileNotation( const ::rtl::OUString& _rUrlOrPath );
        OFileNotation( const ::rtl::OUString& _rUrlOrPath, NOTATION _eInputNotation );

        ::rtl::OUString get(NOTATION _eOutputNotation);

    private:
        SVL_DLLPRIVATE void construct( const ::rtl::OUString& _rUrlOrPath );
        SVL_DLLPRIVATE bool    implInitWithSystemNotation( const ::rtl::OUString& _rSystemPath );
        SVL_DLLPRIVATE bool    implInitWithURLNotation( const ::rtl::OUString& _rURL );
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // SVTOOLS_FILENOTATION_HXX

