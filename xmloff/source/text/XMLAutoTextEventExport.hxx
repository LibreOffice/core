/*************************************************************************
 *
 *  $RCSfile: XMLAutoTextEventExport.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-09 14:53:43 $
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

#ifndef _XMLOFF_XMLAUTOTEXTEVENTEXPORT_HXX
#define _XMLOFF_XMLAUTOTEXTEVENTEXPORT_HXX

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

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

    XMLAutoTextEventExport();

    XMLAutoTextEventExport(
        const ::rtl::OUString& rFileName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & rModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XNameAccess > & rEvents);

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
        const sal_Char *pClass = NULL );

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
    XMLAutoTextEventExport_createInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & )
    throw( ::com::sun::star::uno::Exception );

#endif

