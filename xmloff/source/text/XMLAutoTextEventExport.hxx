/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _XMLOFF_XMLAUTOTEXTEVENTEXPORT_HXX
#define _XMLOFF_XMLAUTOTEXTEVENTEXPORT_HXX

#include <xmloff/xmlexp.hxx>

#include <set>


namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace container { class XNameAccess; }
    namespace frame { class XModel; }
    namespace lang { class XMultiServiceFactory; }
    namespace uno { template<class X> class Reference; }
    namespace uno { template<class X> class Sequence; }
    namespace uno { class XInterface; }
    namespace uno { class Exception; }
    namespace xml { namespace sax { class XDocumentHandler; } }
} } }


/**
 * Component for the export of events attached to autotext blocks.
 * Via the XInitialization interface it expects up to two strings, the
 * first giving the file name (URL) of the autotext group, and the second
 * identifying the autotext. If one of the strings is not given, it
 * will export the whole group / all groups.
 */
class XMLAutoTextEventExport : public SvXMLExport
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameAccess> xEvents;

    const ::rtl::OUString sEventType;
    const ::rtl::OUString sNone;


public:

    // #110680#
    //XMLAutoTextEventExport();
    XMLAutoTextEventExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory, sal_uInt16 nFlags
        );

    // #110680#
    //XMLAutoTextEventExport(
    //  const ::rtl::OUString& rFileName,
    //  const ::com::sun::star::uno::Reference<
    //      ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
    //  const ::com::sun::star::uno::Reference<
    //      ::com::sun::star::frame::XModel > & rModel,
    //  const ::com::sun::star::uno::Reference<
    //      ::com::sun::star::container::XNameAccess > & rEvents);
    XMLAutoTextEventExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
        const ::rtl::OUString& rFileName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & rModel,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & rEvents, sal_uInt16 nFlags);

    ~XMLAutoTextEventExport();

    // XInitialization
    virtual void SAL_CALL initialize(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any> & rArguments )
        throw(
            ::com::sun::star::uno::Exception,
            ::com::sun::star::uno::RuntimeException);

protected:

    /// export the events off all autotexts
    virtual sal_uInt32 exportDoc(
        enum ::xmloff::token::XMLTokenEnum eClass = xmloff::token::XML_TOKEN_INVALID );

    /// does the document have any events ?
    sal_Bool hasEvents();

    /// export the events element
    void exportEvents();


    /// add the namespaces used by events
    /// (to be called for the document element)
    void addNamespaces();


    // methods without content:
    virtual void _ExportMeta();
    virtual void _ExportScripts();
    virtual void _ExportFontDecls();
    virtual void _ExportStyles( sal_Bool bUsed ) ;
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void _ExportChangeTracking();
    virtual void _ExportContent();
};



// global functions to support the component

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    XMLAutoTextEventExport_getSupportedServiceNames()
    throw();

::rtl::OUString SAL_CALL XMLAutoTextEventExport_getImplementationName()
    throw();

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    XMLAutoTextEventExportOOO_createInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & )
    throw( ::com::sun::star::uno::Exception );

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    XMLAutoTextEventExportOOO_getSupportedServiceNames()
    throw();

::rtl::OUString SAL_CALL XMLAutoTextEventExportOOO_getImplementationName()
    throw();

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    XMLAutoTextEventExportOOO_createInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & )
    throw( ::com::sun::star::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
