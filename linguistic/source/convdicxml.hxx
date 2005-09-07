/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: convdicxml.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:49:46 $
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

#ifndef _LINGUISTIC_CONVDICXML_HXX_
#define _LINGUISTIC_CONVDICXML_HXX_

#ifndef _COM_SUN_STAR_LINGUISTIC2_XCONVERSIONDICTIONARY_HPP_
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include <xmloff/xmlexp.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _LINGUISTIC_MISC_HXX_
#include "misc.hxx"
#endif
#ifndef _LINGUISTIC_DEFS_HXX_
#include "defs.hxx"
#endif


class ConvDic;

///////////////////////////////////////////////////////////////////////////

class ConvDicXMLExport : public SvXMLExport
{
    ConvDic     &rDic;
    sal_Bool    bSuccess;

protected:
    //void ExportNodes(const SmNode *pIn, int nLevel);

public:
    ConvDicXMLExport( ConvDic &rConvDic,
        const rtl::OUString &rFileName,
        com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > &rHandler) :
        SvXMLExport ( utl::getProcessServiceFactory(), rFileName, rHandler ),
        rDic        ( rConvDic ),
        bSuccess    ( sal_False )
    {
    }
    virtual ~ConvDicXMLExport()
    {
    }

    // XServiceInfo (override parent method)
    ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

    // SvXMLExport
    void _ExportAutoStyles()    {}
    void _ExportMasterStyles()  {}
    void _ExportContent();
    sal_uInt32 exportDoc( enum ::xmloff::token::XMLTokenEnum eClass );

    sal_Bool    Export( SfxMedium &rMedium );
};


class ConvDicXMLImport : public SvXMLImport
{
    ConvDic        *pDic;       // conversion dictionary to be used
                                // if != NULL: whole file will be read and
                                //   all entries will be added to the dictionary
                                // if == NULL: no entries will be added
                                //   but the language and conversion type will
                                //   still be determined!

    INT16           nLanguage;          // language of the dictionary
    sal_Int16       nConversionType;    // conversion type the dictionary is used for
    sal_Bool        bSuccess;

public:

    //!!  see comment for pDic member
    ConvDicXMLImport( ConvDic *pConvDic, const rtl::OUString &rFileName ) :
        SvXMLImport ( utl::getProcessServiceFactory(), IMPORT_ALL ),
        pDic        ( pConvDic )
    {
        nLanguage       = LANGUAGE_NONE;
        nConversionType = -1;
        bSuccess        = sal_False;
    }

    virtual ~ConvDicXMLImport() throw ()
    {
    }

    // XServiceInfo (override parent method)
    ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL startDocument(void) throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL endDocument(void) throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    virtual SvXMLImportContext * CreateContext(
        sal_uInt16 nPrefix, const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference < com::sun::star::xml::sax::XAttributeList > &rxAttrList );

    ConvDic *   GetDic()                    { return pDic; }
    INT16       GetLanguage() const         { return nLanguage; }
    sal_Int16   GetConversionType() const   { return nConversionType; }
    sal_Bool    GetSuccess() const          { return bSuccess; }

    void        SetLanguage( INT16 nLang )              { nLanguage = nLang; }
    void        SetConversionType( sal_Int16 nType )    { nConversionType = nType; }
};

///////////////////////////////////////////////////////////////////////////

#endif

