/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
namespace com { namespace sun { namespace star {
    namespace uno { template<class X> class Reference; }
    namespace frame { class XModel; }
    namespace task { class XStatusIndicator; }
    namespace lang { class XMultiServiceFactory; }
    namespace uno { class XInterface; }
    namespace xml {
        namespace sax { struct InputSource; } }
} } }

#ifndef SC_XMLWRAP_HXX
#define SC_XMLWRAP_HXX
namespace binfilter {

class SvStorage;
class ScDocument;
class SfxMedium;
class ScMySharedData;

//STRIP008 namespace com { namespace sun { namespace star {
//STRIP008 	namespace uno { template<class X> class Reference; }
//STRIP008 	namespace frame { class XModel; }
//STRIP008 	namespace task { class XStatusIndicator; }
//STRIP008 	namespace lang { class XMultiServiceFactory; }
//STRIP008 	namespace uno { class XInterface; }
//STRIP008 	namespace xml {
//STRIP008 		namespace sax { struct InputSource; } }
//STRIP008 } } }

class ScXMLImportWrapper
{
    ScDocument&		rDoc;
    SfxMedium*		pMedium;
    SvStorage*		pStorage;

    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator> GetStatusIndicator(
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rModel);
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator> GetStatusIndicator();

    sal_uInt32 ImportFromComponent(::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& xServiceFactory,
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& xModel,
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xXMLParser,
        ::com::sun::star::xml::sax::InputSource& aParserInput,
        const ::rtl::OUString& sComponentName, const ::rtl::OUString& sDocName, const ::rtl::OUString& sOldDocName,
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& aArgs,
        sal_Bool bMustBeSuccessfull);

    sal_Bool ExportToComponent(::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& xServiceFactory,
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& xModel,
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xWriter,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aDescriptor,
        const ::rtl::OUString& sName,	const ::rtl::OUString& sMediaType, const ::rtl::OUString& sComponentName,
        const sal_Bool bPlainText, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& aArgs,
        ScMySharedData*& pSharedData);

public:
    ScXMLImportWrapper(ScDocument& rD, SfxMedium* pM, SvStorage* pS);
    BOOL Import(sal_Bool bStylesOnly);
    BOOL Export(sal_Bool bStylesOnly);
};

} //namespace binfilter
#endif

