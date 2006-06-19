/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmltabe.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 17:59:49 $
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

#ifndef _XMLOFF_XMLTABE_HXX
#define _XMLOFF_XMLTABE_HXX

// prevent funny things like "#define sun 1" from the compiler
#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

class SvXMLExport;
namespace com { namespace sun { namespace star {
    namespace style { struct TabStop; }
    namespace uno { class Any; }
} } }


class SvxXMLTabStopExport
{
    SvXMLExport& rExport;   // for access to document handler

protected:

    void exportTabStop( const ::com::sun::star::style::TabStop* pTabStop );

public:

    SvxXMLTabStopExport(  SvXMLExport& rExport );
    virtual ~SvxXMLTabStopExport();

    // core API
    void Export( const ::com::sun::star::uno::Any& rAny );
};


#endif  //  _XMLOFF_XMLTABE_HXX

