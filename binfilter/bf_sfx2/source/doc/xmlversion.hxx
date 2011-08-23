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
#ifndef _SFX_XMLVERSION_HXX
#define _SFX_XMLVERSION_HXX

#ifndef _SVSTOR_HXX 
#include <bf_so3/svstor.hxx>
#endif


//#ifndef _XMLOFF_XMLITMAP_HXX
//#include <bf_xmloff/xmlitmap.hxx>
//#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include <bf_xmloff/xmlexp.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include <bf_xmloff/xmlimp.hxx>
#endif



class DateTime;
namespace binfilter {

class SfxVersionTableDtor;

// ------------------------------------------------------------------------
class SfxXMLVersListImport_Impl : public SvXMLImport
{
private:
    SfxVersionTableDtor *mpVersions;

protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

public:

    // #110680#
    SfxXMLVersListImport_Impl( 
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        SfxVersionTableDtor *pVersions );
    ~SfxXMLVersListImport_Impl() throw();

    SfxVersionTableDtor*	 GetList() { return mpVersions; }
};

// ------------------------------------------------------------------------
class SfxXMLVersListContext_Impl : public SvXMLImportContext
{
private:
    SfxXMLVersListImport_Impl & rLocalRef;

public:

    SfxXMLVersListContext_Impl( SfxXMLVersListImport_Impl& rImport,
                           sal_uInt16 nPrefix, 
                           const ::rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference< 
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    ~SfxXMLVersListContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const ::rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference< 
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

};

// ------------------------------------------------------------------------
class SfxXMLVersionContext_Impl : public SvXMLImportContext
{
private:
    SfxXMLVersListImport_Impl&	rLocalRef;

    static sal_Bool			ParseISODateTimeString(
                                const ::rtl::OUString& rString,
                                DateTime& rDateTime );

public:
    
    SfxXMLVersionContext_Impl( SfxXMLVersListImport_Impl& rImport,
                          sal_uInt16 nPrefix, 
                          const ::rtl::OUString& rLocalName,
                          const ::com::sun::star::uno::Reference< 
                          ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    ~SfxXMLVersionContext_Impl();
};


// ------------------------------------------------------------------------
class SfxXMLVersList_Impl
{
public:
    static sal_Bool	ReadInfo( SvStorageRef xRoot, SfxVersionTableDtor *pList );
};



}//end of namespace binfilter
#endif
