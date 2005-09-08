/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_parceldesc.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:34:37 $
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

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>

#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
namespace css = ::com::sun::star;
namespace dp_registry
{
namespace backend
{
namespace sfwk
{

typedef ::cppu::WeakImplHelper1< css::xml::sax::XDocumentHandler > t_DocHandlerImpl;

class ParcelDescDocHandler : public t_DocHandlerImpl
{
private:
    bool m_bIsParsed;
    ::rtl::OUString m_sLang;
    sal_Int32 skipIndex;
public:
    ParcelDescDocHandler():m_bIsParsed( false ), skipIndex( 0 ){}
    ::rtl::OUString getParcelLanguage() { return m_sLang; }
    bool isParsed() { return m_bIsParsed; }
    // XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    virtual void SAL_CALL endDocument()
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    virtual void SAL_CALL startElement( const ::rtl::OUString& aName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttribs )
        throw ( css::xml::sax::SAXException,
            css::uno::RuntimeException );

    virtual void SAL_CALL endElement( const ::rtl::OUString & aName )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    virtual void SAL_CALL characters( const ::rtl::OUString & aChars )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString & aWhitespaces )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    virtual void SAL_CALL processingInstruction(
        const ::rtl::OUString & aTarget, const ::rtl::OUString & aData )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );

    virtual void SAL_CALL setDocumentLocator(
        const css::uno::Reference< css::xml::sax::XLocator >& xLocator )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException );
};
}
}
}
