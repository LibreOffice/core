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

#ifndef SC_XMLWRAP_HXX
#define SC_XMLWRAP_HXX

#include <tools/solar.h>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

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

//UNUSED2008-05  com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator> GetStatusIndicator(
//UNUSED2008-05      com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rModel);
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
    sal_Bool Import(sal_Bool bStylesOnly, ErrCode& );
    sal_Bool Export(sal_Bool bStylesOnly);
};

#endif

