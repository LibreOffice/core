/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlwrap.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:04:52 $
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

#ifndef SC_XMLWRAP_HXX
#define SC_XMLWRAP_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

class ScDocument;
class SfxMedium;
class ScMySharedData;

#include <tools/errcode.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
    namespace frame { class XModel; }
    namespace task { class XStatusIndicator; }
    namespace lang { class XMultiServiceFactory; }
    namespace uno { class XInterface; }
    namespace embed { class XStorage; }
    namespace xml {
        namespace sax { struct InputSource; } }
} } }

class ScXMLImportWrapper
{
    ScDocument&     rDoc;
    SfxMedium*      pMedium;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > xStorage;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator> GetStatusIndicator(
        com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rModel);
    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator> GetStatusIndicator();

    sal_uInt32 ImportFromComponent(com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& xServiceFactory,
        com::sun::star::uno::Reference<com::sun::star::frame::XModel>& xModel,
        com::sun::star::uno::Reference<com::sun::star::uno::XInterface>& xXMLParser,
        com::sun::star::xml::sax::InputSource& aParserInput,
        const rtl::OUString& sComponentName, const rtl::OUString& sDocName, const rtl::OUString& sOldDocName,
        com::sun::star::uno::Sequence<com::sun::star::uno::Any>& aArgs,
        sal_Bool bMustBeSuccessfull);

    sal_Bool ExportToComponent(com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& xServiceFactory,
        com::sun::star::uno::Reference<com::sun::star::frame::XModel>& xModel,
        com::sun::star::uno::Reference<com::sun::star::uno::XInterface>& xWriter,
        com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aDescriptor,
        const rtl::OUString& sName, const rtl::OUString& sMediaType, const rtl::OUString& sComponentName,
        const sal_Bool bPlainText, com::sun::star::uno::Sequence<com::sun::star::uno::Any>& aArgs,
        ScMySharedData*& pSharedData);

public:
    ScXMLImportWrapper(ScDocument& rD, SfxMedium* pM, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >&);
    BOOL Import(sal_Bool bStylesOnly, ErrCode& );
    BOOL Export(sal_Bool bStylesOnly);
};

#endif

