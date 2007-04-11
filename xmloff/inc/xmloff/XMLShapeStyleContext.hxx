/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLShapeStyleContext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:24:13 $
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

#ifndef _XMLOFF_XMLSHAPESTYLECONTEXT_HXX
#define _XMLOFF_XMLSHAPESTYLECONTEXT_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _XMLOFF_PRSTYLEI_HXX_
#include <xmloff/prstylei.hxx>
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif

class SvXMLImport;

//////////////////////////////////////////////////////////////////////////////
// style:style context

class XMLOFF_DLLPUBLIC XMLShapeStyleContext: public XMLPropStyleContext
{
protected:
    ::rtl::OUString     m_sControlDataStyleName;
    ::rtl::OUString     m_sListStyleName;
    sal_Bool            m_bIsNumRuleAlreadyConverted;

protected:
    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const ::rtl::OUString& rLocalName,
                               const ::rtl::OUString& rValue );
public:
    TYPEINFO();

    XMLShapeStyleContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList,
        SvXMLStylesContext& rStyles,
        sal_uInt16 nFamily = XML_STYLE_FAMILY_SD_GRAPHICS_ID);
    virtual ~XMLShapeStyleContext();

    SvXMLImportContext * CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void Finish( sal_Bool bOverwrite );

    virtual void FillPropertySet(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & rPropSet );
};

#endif  //  _XMLOFF_XMLSHAPESTYLECONTEXT_HXX
