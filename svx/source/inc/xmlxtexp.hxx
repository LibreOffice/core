/*************************************************************************
 *
 *  $RCSfile: xmlxtexp.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2001-02-23 21:26:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVX_XMLXTEXP_HXX
#define _SVX_XMLXTEXP_HXX

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmloff/xmlexp.hxx"
#endif

namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
    namespace uno { template<class X> class Reference; }
    namespace uno { class XInterface; }
    namespace container { class XNameContainer; }
    namespace document { class XGraphicObjectResolver; }
    namespace xml { namespace sax { class XDocumentHandler; } }
} } }

class SvxXMLXTableExportComponent : public SvXMLExport
{
public:
    SvxXMLXTableExportComponent(
        const rtl::OUString& rFileName,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XDocumentHandler > & xHandler,
        const com::sun::star::uno::Reference<
            com::sun::star::container::XNameContainer > & xTable,
        com::sun::star::uno::Reference<
            com::sun::star::document::XGraphicObjectResolver >& xGrfResolver);

    ~SvxXMLXTableExportComponent();

    static sal_Bool save( const rtl::OUString& rURL, const com::sun::star::uno::Reference<::com::sun::star::container::XNameContainer >& xTable ) throw();

    sal_Bool export() throw();

    // methods without content:
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void _ExportContent();

private:
    const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > & mxTable;
};

#endif

