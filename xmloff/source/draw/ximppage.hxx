/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ximppage.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-28 13:34:59 $
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

#ifndef _XIMPPAGE_HXX
#define _XIMPPAGE_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

#ifndef _SDXMLIMP_IMPL_HXX
#include "sdxmlimp_impl.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _XIMPSHAPE_HXX
#include "ximpshap.hxx"
#endif

//////////////////////////////////////////////////////////////////////////////
// draw:g context (RECURSIVE)

class SdXMLGenericPageContext : public SvXMLImportContext
{
    // the shape group this group is working on
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > mxShapes;

protected:
    rtl::OUString               maPageLayoutName;
    rtl::OUString               maUseHeaderDeclName;
    rtl::OUString               maUseFooterDeclName;
    rtl::OUString               maUseDateTimeDeclName;
    rtl::OUString               msNavOrder;

    void SetLocalShapesContext(com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rNew)
        { mxShapes = rNew; }

    /** sets the page style on this page */
    void SetStyle( rtl::OUString& rStyleName );

    /** sets the presentation layout at this page. It is used for drawing pages and for the handout master */
    void SetLayout();

    /** deletes all shapes on this drawing page */
    void DeleteAllShapes();

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

    /** sets the properties from a page master style with the given name on this contexts page */
    void SetPageMaster( rtl::OUString& rsPageMasterName );

    void SetNavigationOrder();

public:
    TYPEINFO();

    SdXMLGenericPageContext( SvXMLImport& rImport, USHORT nPrfx, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLGenericPageContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext *CreateChildContext(
        USHORT nPrefix, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();

    const com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& GetLocalShapesContext() const
        { return mxShapes; }
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& GetLocalShapesContext()
        { return mxShapes; }
};


#endif  //  _XIMPGROUP_HXX
