/*************************************************************************
 *
 *  $RCSfile: ScriptMetadataImporter.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-02-25 16:16:40 $
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
#ifndef _SCRIPTING_STORAGE_SCRIPTMETADATAIMPORTER_HXX_
#define _SCRIPTING_STORAGE_SCRIPTMETADATAIMPORTER_HXX_

#include <vector>

#include <rtl/ustring.h>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase1.hxx> // helper for component factory

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include "ScriptData.hxx"

namespace scripting_impl
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

typedef ::std::vector< ScriptData > InfoImpls_vec;
typedef ::std::pair< ::rtl::OUString, ::std::pair< ::rtl::OUString,
    ::rtl::OUString > > strpair_pair;

/**
 * Script Meta Data Importer
 */
class ScriptMetadataImporter : public
    ::cppu::WeakImplHelper1< css::xml::sax::XExtendedDocumentHandler >
{
public:

    /**
     * This function will begin the parser and parse the meta data
     *
     * @param xInput The XInputStream for the parser which contains the XML
     * @param parcelURI The parcel's URI in the document or the application
     *
     * @see css::io::XInputStream
     */
    void parseMetaData( css::uno::Reference< css::io::XInputStream >
        const & xInput, const ::rtl::OUString & parcelURI,
        InfoImpls_vec & io_ScriptDatas )
        throw ( css::xml::sax::SAXException, css::io::IOException,
            css::uno::RuntimeException );

    /**
     * Constructor for the meta-data parser
     *
     * @param XComponentContext
     */
    explicit ScriptMetadataImporter(
        const css::uno::Reference< css::uno::XComponentContext >&  );

    /**
     * Destructor for the parser
     */
    virtual ~ScriptMetadataImporter() SAL_THROW( () );

    // XExtendedDocumentHandler impl
    /**
     * Function to handle the start of CDATA in XML
     *
     * @see com::sun::star::xml::sax::XExtendedDocumentHandler
     */
    virtual void SAL_CALL startCDATA()
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    /**
     * Function to handle the end of CDATA in XML
     *
     * @see com::sun::star::xml::sax::XExtendedDocumentHandler
     */
    virtual void SAL_CALL endCDATA() throw ( css::uno::RuntimeException );

    /**
     * Function to handle comments in XML
     *
     * @see com::sun::star::xml::sax::XExtendedDocumentHandler
     */
    virtual void SAL_CALL comment( const ::rtl::OUString & sComment )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    /**
     * Function to handle line breaks in XML
     *
     * @see com::sun::star::xml::sax::XExtendedDocumentHandler
     */
    virtual void SAL_CALL allowLineBreak()
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    /**
     * Function to handle unknowns in XML
     *
     * @see com::sun::star::xml::sax::XExtendedDocumentHandler
     */
    virtual void SAL_CALL unknown( const ::rtl::OUString & sString )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    /**
     * Function to handle the start of XML document
     *
     * @see com::sun::star::xml::sax::XExtendedDocumentHandler
     */
    // XDocumentHandler impl
    virtual void SAL_CALL startDocument()
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    /**
     * Function to handle the end of the XML document
     *
     * @see com::sun::star::xml::sax::XDocumentHandler
     */
    virtual void SAL_CALL endDocument()
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    /**
     * Function to handle the start of an element
     *
     * @see com::sun::star::xml::sax::XDocumentHandler
     */
    virtual void SAL_CALL startElement( const ::rtl::OUString& aName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttribs )
        throw ( css::xml::sax::SAXException,
            css::uno::RuntimeException );

    /**
     * Function to handle the end of an element
     *
     * @see com::sun::star::xml::sax::XDocumentHandler
     */
    virtual void SAL_CALL endElement( const ::rtl::OUString & aName )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    /**
     * Function to handle characters in elements
     *
     * @see com::sun::star::xml::sax::XDocumentHandler
     */
    virtual void SAL_CALL characters( const ::rtl::OUString & aChars )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    /**
     * Function to handle whitespace
     *
     * @see com::sun::star::xml::sax::XDocumentHandler
     */
    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString & aWhitespaces )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    /**
     * Function to handle XML processing instructions
     *
     * @see com::sun::star::xml::sax::XDocumentHandler
     */
    virtual void SAL_CALL processingInstruction(
        const ::rtl::OUString & aTarget, const ::rtl::OUString & aData )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    /**
     * Function to set the document locator
     *
     * @see com::sun::star::xml::sax::XDocumentHandler
     */
    virtual void SAL_CALL setDocumentLocator(
        const css::uno::Reference< css::xml::sax::XLocator >& xLocator )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );



private:

    /** Vector contains the ScriptData structs */
    InfoImpls_vec* mpv_ScriptDatas;

    /** @internal */
    osl::Mutex     m_mutex;

    /** @internal */
    css::uno::Reference< css::uno::XComponentContext >  m_xContext;

    /** Placeholder for the parcel URI */
    ::rtl::OUString ms_parcelURI;

    /** States for state machine during parsing */
    enum { PARCEL, SCRIPT, LOCALE, DISPLAYNAME, DESCRIPTION, FUNCTIONNAME,
        LOGICALNAME, LANGUAGEDEPPROPS, LANGDEPPROPS, FILESET, FILESETPROPS,
        FILES, FILEPROPS } m_state;

    /** Build up the struct during parsing the meta data */
    ScriptData m_ScriptData;

    /** @internal */
    ::rtl::OUString ms_localeLang;
    ::rtl::OUString ms_localeDisName;
    ::rtl::OUStringBuffer *ms_localeDesc;

    props_vec mv_filesetprops;

    ::rtl::OUString ms_filename;
    ::rtl::OUString ms_filesetname;

    props_vec mv_fileprops;

    strpairvec_map mm_files;

    InfoImpls_vec mv_ScriptDatas;

    /**
     *   Helper function to set the state
     *
     *   @param tagName
     *           The current tag being processed
     */
    void setState(const ::rtl::OUString & tagName);
}
; // class ScriptMetadataImporter

}

#endif
