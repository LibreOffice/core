/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SvXMLAutoCorrectImport.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:24:58 $
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
#ifndef _SV_XMLAUTOCORRECTIMPORT_HXX
#define _SV_XMLAUTOCORRECTIMPORT_HXX

#ifndef _SVSTOR_HXX
#include <sot/storage.hxx>
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _MySVXACORR_HXX
#include "svxacorr.hxx"
#endif

class SvXMLAutoCorrectImport : public SvXMLImport
{
protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                  const ::rtl::OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
public:
    SvxAutocorrWordList     *pAutocorr_List;
    SvxAutoCorrect          &rAutoCorrect;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xStorage;
    //SvStorageRef            &rStorage;

    // #110680#
    SvXMLAutoCorrectImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        SvxAutocorrWordList *pNewAutocorr_List,
        SvxAutoCorrect &rNewAutoCorrect,
        const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rNewStorage);

    ~SvXMLAutoCorrectImport ( void ) throw ();
};

class SvXMLWordListContext : public SvXMLImportContext
{
private:
    SvXMLAutoCorrectImport & rLocalRef;
public:
    SvXMLWordListContext ( SvXMLAutoCorrectImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLWordListContext ( void );
};

class SvXMLWordContext : public SvXMLImportContext
{
private:
    SvXMLAutoCorrectImport & rLocalRef;
public:
    SvXMLWordContext ( SvXMLAutoCorrectImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLWordContext ( void );
};


class SvXMLExceptionListImport : public SvXMLImport
{
protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                  const ::rtl::OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
public:
    SvStringsISortDtor  &rList;

    // #110680#
    SvXMLExceptionListImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        SvStringsISortDtor & rNewList );

    ~SvXMLExceptionListImport ( void ) throw ();
};

class SvXMLExceptionListContext : public SvXMLImportContext
{
private:
    SvXMLExceptionListImport & rLocalRef;
public:
    SvXMLExceptionListContext ( SvXMLExceptionListImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLExceptionListContext ( void );
};

class SvXMLExceptionContext : public SvXMLImportContext
{
private:
    SvXMLExceptionListImport & rLocalRef;
public:
    SvXMLExceptionContext ( SvXMLExceptionListImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLExceptionContext ( void );
};


#endif
