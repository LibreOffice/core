/*************************************************************************
 *
 *  $RCSfile: XMLAutoTextEventExport.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dvo $ $Date: 2001-02-13 16:55:00 $
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
    namespace frame { class XModel; }
    namespace lang { class XMultiServiceFactory; }
    namespace text { class XAutoTextContainer; }
    namespace text { class XAutoTextGroup; }
    namespace text { class XAutoTextEntry; }
    namespace uno { template<class X> class Reference; }
    namespace uno { template<class X> class Sequence; }
    namespace uno { class XInterface; }
    namespace uno { class Exception; }
    namespace xml { namespace sax { class XDocumentHandler; } }
} } }


class XMLAutoTextEventExport : public SvXMLExport
{
    ::std::set< ::rtl::OUString > * eventCount;

    /// names of groups to be exported; export all group if empty.
    ::com::sun::star::uno::Sequence< ::rtl::OUString> & rGroupNames;


    // generate a combined name for sake of counting names
    ::rtl::OUString combinedName(
        const ::rtl::OUString& rGroupName,
        const ::rtl::OUString& rEntryName);

public:
    XMLAutoTextEventExport();

    XMLAutoTextEventExport(
        const ::rtl::OUString& rFileName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & rModel);

    ~XMLAutoTextEventExport();

protected:
    // export the events off all autotexts
    virtual sal_uInt32 exportDoc(
        const sal_Char *pClass = NULL );

    void exportAutoTextContainer(
        const sal_Char* pClass,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XAutoTextContainer> & rAutoTextContainer,
        ::com::sun::star::uno::Sequence< ::rtl::OUString> & rGroupNames);

    void exportAutoTextGroup(
        ::rtl::OUString& rName,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XAutoTextGroup> & rGroup);

    void exportAutoTextEntry(
        ::rtl::OUString& rGroupName,
        ::rtl::OUString& rName,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XAutoTextEntry> & rEntry);


    /// count the number of events (to be used before hasXXX() methods)
    void countEvents(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XAutoTextContainer> & rAutoTextContainer,
        ::com::sun::star::uno::Sequence< ::rtl::OUString> & rGroupNames);

    /// does the document have any events ?
    sal_Bool hasDocumentEvents();

    /// does the group have any events ?
    sal_Bool hasGroupEvents(
        const ::rtl::OUString& rGroupName   );

    /// does this entry have any events ?
    sal_Bool hasEntryEvents(
        const ::rtl::OUString& rGroupName,
        const ::rtl::OUString& rEntryName );



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

