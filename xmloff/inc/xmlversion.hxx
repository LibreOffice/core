/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlversion.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:21:56 $
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
#ifndef _XMLOFF_XMLVERSION_HXX
#define _XMLOFF_XMLVERSION_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTREVISIONLISTPERSISTENCE_HPP_
#include <com/sun/star/document/XDocumentRevisionListPersistence.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_REVISIONTAG_HPP_
#include <com/sun/star/util/RevisionTag.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif

#include <cppuhelper/implbase1.hxx>

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmlictxt.hxx>
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include <xmlexp.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmlimp.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <nmspmap.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmlnmspe.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmltoken.hxx>
#endif

// ------------------------------------------------------------------------
class XMLVersionListExport : public SvXMLExport
{
private:
    const com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >& maVersions;
public:
    XMLVersionListExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        const com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >& rVersions,
        const rtl::OUString &rFileName,
        com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > &rHandler );
    virtual     ~XMLVersionListExport() {}

    sal_uInt32  exportDoc( enum ::xmloff::token::XMLTokenEnum eClass );
    void        _ExportAutoStyles() {}
    void        _ExportMasterStyles () {}
    void        _ExportContent() {}
};

// ------------------------------------------------------------------------
class XMLVersionListImport : public SvXMLImport
{
private:
    com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >& maVersions;

protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

public:

    // #110897#
    XMLVersionListImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >& rVersions );
    ~XMLVersionListImport() throw();

    com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >&
        GetList() { return maVersions; }
};

// ------------------------------------------------------------------------
class XMLVersionListContext : public SvXMLImportContext
{
private:
    XMLVersionListImport & rLocalRef;

public:

    XMLVersionListContext( XMLVersionListImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    ~XMLVersionListContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

};

// ------------------------------------------------------------------------
class XMLVersionContext: public SvXMLImportContext
{
private:
    XMLVersionListImport&  rLocalRef;

    static sal_Bool         ParseISODateTimeString(
                                const rtl::OUString& rString,
                                com::sun::star::util::DateTime& rDateTime );

public:

    XMLVersionContext( XMLVersionListImport& rImport,
                          sal_uInt16 nPrefix,
                          const rtl::OUString& rLocalName,
                          const ::com::sun::star::uno::Reference<
                          ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    ~XMLVersionContext();
};


// ------------------------------------------------------------------------
class XMLVersionListPersistence : public ::cppu::WeakImplHelper1< ::com::sun::star::document::XDocumentRevisionListPersistence >
{
public:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::util::RevisionTag > SAL_CALL load( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL store( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage, const ::com::sun::star::uno::Sequence< ::com::sun::star::util::RevisionTag >& List ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
};

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    XMLVersionListPersistence_getSupportedServiceNames()
    throw();

::rtl::OUString SAL_CALL XMLVersionPersistence_getImplementationName()
    throw();

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    XMLVersionListPersistence_createInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & )
    throw( ::com::sun::star::uno::Exception );

#endif
