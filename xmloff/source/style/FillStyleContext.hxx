/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FillStyleContext.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:23:30 $
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

#ifndef _XMLOFF_FILLSTYLECONTEXTS_HXX_
#define _XMLOFF_FILLSTYLECONTEXTS_HXX_

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _XMLOFF_XMLSTYLE_HXX //autogen
#include <xmlstyle.hxx>
#endif

#ifndef _RTL_USTRING_
#include<rtl/ustring>
#endif

//////////////////////////////////////////////////////////////////////////////
// draw:gardient context

class XMLGradientStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    TYPEINFO();

    XMLGradientStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLGradientStyleContext();

    virtual void EndElement();

    virtual BOOL IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:hatch context

class XMLHatchStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    TYPEINFO();

    XMLHatchStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLHatchStyleContext();

    virtual void EndElement();

    virtual BOOL IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:fill-image context

class XMLBitmapStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > mxBase64Stream;

public:
    TYPEINFO();

    XMLBitmapStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLBitmapStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();

    virtual BOOL IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:transparency context

class XMLTransGradientStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    TYPEINFO();

    XMLTransGradientStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLTransGradientStyleContext();

    virtual void EndElement();

    virtual BOOL IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:marker context

class XMLMarkerStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    TYPEINFO();

    XMLMarkerStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLMarkerStyleContext();

    virtual void EndElement();

    virtual BOOL IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:marker context

class XMLDashStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    TYPEINFO();

    XMLDashStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLDashStyleContext();

    virtual void EndElement();

    virtual BOOL IsTransient() const;
};

#endif  // _XMLOFF_FILLSTYLECONTEXTS_HXX_
