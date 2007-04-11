/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prstylei.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:28:38 $
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
#ifndef _XMLOFF_PRSTYLEI_HXX_
#define _XMLOFF_PRSTYLEI_HXX_

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX_
#include <xmloff/xmlstyle.hxx>
#endif

struct XMLPropertyState;
class SvXMLStylesContext;

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
} } }

class XMLOFF_DLLPUBLIC XMLPropStyleContext : public SvXMLStyleContext
{
    const ::rtl::OUString msIsPhysical;
    const ::rtl::OUString msFollowStyle;
    ::std::vector< XMLPropertyState > maProperties;
    ::com::sun::star::uno::Reference < ::com::sun::star::style::XStyle > mxStyle;
    SvXMLImportContextRef               mxStyles;

    SAL_DLLPRIVATE XMLPropStyleContext(XMLPropStyleContext &); // not defined
    SAL_DLLPRIVATE void operator =(XMLPropStyleContext &); // not defined

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const ::rtl::OUString& rLocalName,
                               const ::rtl::OUString& rValue );
    SvXMLStylesContext *GetStyles() { return (SvXMLStylesContext *)&mxStyles; }
    ::std::vector< XMLPropertyState > & GetProperties() { return maProperties; }

    // This methos my be overloaded to create a new style. Its called by
    // CreateInsert to create a style if a style with the requested family and
    // name couldn't be found. The st
    virtual ::com::sun::star::uno::Reference <
        ::com::sun::star::style::XStyle > Create();

public:

    TYPEINFO();

    XMLPropStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            SvXMLStylesContext& rStyles, sal_uInt16 nFamily = 0,
            sal_Bool bDefaultStyle=sal_False );
    virtual ~XMLPropStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void FillPropertySet(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & rPropSet );

    const SvXMLStylesContext *GetStyles() const { return (const SvXMLStylesContext *)&mxStyles; }
    const ::std::vector< XMLPropertyState > & GetProperties() const { return maProperties; }

    const ::com::sun::star::uno::Reference <
                ::com::sun::star::style::XStyle >&
               GetStyle() const { return mxStyle; }
    void SetStyle(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle >& xStl) { mxStyle = xStl; }

    virtual void SetDefaults();

    virtual void CreateAndInsert( sal_Bool bOverwrite );
    virtual void Finish( sal_Bool bOverwrite );
};

#endif
