/*************************************************************************
 *
 *  $RCSfile: xmlwrap.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-01 13:19:06 $
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

#ifndef SC_XMLWRAP_HXX
#define SC_XMLWRAP_HXX

class ScDocument;
class SfxMedium;
class SvStorage;

namespace com { namespace sun { namespace star {
    namespace uno { template<class X> class Reference; }
    namespace frame { class XModel; }
    namespace task { class XStatusIndicator; }
    namespace lang { class XMultiServiceFactory; }
    namespace uno { class XInterface; }
    namespace xml {
        namespace sax { class InputSource; } }
} } }

class ScXMLImportWrapper
{
    ScDocument&     rDoc;
    SfxMedium*      pMedium;
    SvStorage*      pStorage;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator> GetStatusIndicator(
        com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rModel);

    sal_Bool ImportFromComponent(com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& xServiceFactory,
        com::sun::star::uno::Reference<com::sun::star::frame::XModel>& xModel,
        com::sun::star::uno::Reference<com::sun::star::uno::XInterface>& xXMLParser,
        com::sun::star::xml::sax::InputSource& aParserInput,
        const rtl::OUString& sComponentName, const rtl::OUString& sDocName,
        com::sun::star::uno::Sequence<com::sun::star::uno::Any>& aArgs);

    sal_Bool ExportToComponent(com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& xServiceFactory,
        com::sun::star::uno::Reference<com::sun::star::frame::XModel>& xModel,
        com::sun::star::uno::Reference<com::sun::star::uno::XInterface>& xWriter,
        com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aDescriptor,
        const rtl::OUString& sName, const rtl::OUString& sMediaType, const rtl::OUString& sComponentName,
        const sal_Bool bCompress, com::sun::star::uno::Sequence<com::sun::star::uno::Any>& aArgs);

public:
    ScXMLImportWrapper(ScDocument& rD, SfxMedium* pM, SvStorage* pS);
    BOOL Import();
    BOOL Export();
};

#endif

