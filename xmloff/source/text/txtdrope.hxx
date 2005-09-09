/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtdrope.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:28:06 $
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
#ifndef _XMLOFF_TXTDROPE_HXX
#define _XMLOFF_TXTDROPE_HXX


class SvXMLExport;
namespace com { namespace sun { namespace star { namespace uno {
    class Any; } } } }
namespace rtl { class OUString; }

class XMLTextDropCapExport
{
    SvXMLExport& rExport;

public:

    XMLTextDropCapExport( SvXMLExport& rExport );
    ~XMLTextDropCapExport();

    void exportXML( const ::com::sun::star::uno::Any& rAny,
                    sal_Bool bWholeWord,
                    const ::rtl::OUString& rStyleName );
};


#endif
