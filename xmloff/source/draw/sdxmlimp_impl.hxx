/*************************************************************************
 *
 *  $RCSfile: sdxmlimp_impl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2000-12-05 23:22:55 $
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

#ifndef _SDXMLIMP_IMPL_HXX
#define _SDXMLIMP_IMPL_HXX

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef __SGI_STL_VECTOR
#include <stl/vector>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

//////////////////////////////////////////////////////////////////////////////

enum SdXMLDocElemTokenMap
{
    XML_TOK_DOC_STYLES,
    XML_TOK_DOC_AUTOSTYLES,
    XML_TOK_DOC_MASTERSTYLES,
    XML_TOK_DOC_META,
    XML_TOK_DOC_BODY,
    XML_TOK_OFFICE_END = XML_TOK_UNKNOWN
};

enum SdXMLBodyElemTokenMap
{
    XML_TOK_BODY_PAGE
};

enum SdXMLStylesElemTokenMap
{
    XML_TOK_STYLES_MASTER_PAGE,
    XML_TOK_STYLES_STYLE,
    XML_TOK_STYLES_PAGE_MASTER,
    XML_TOK_STYLES_PRESENTATION_PAGE_LAYOUT
};

enum SdXMLAutoStylesElemTokenMap
{
    XML_TOK_AUTOSTYLES_STYLE
};

enum SdXMLMasterPageElemTokenMap
{
    XML_TOK_MASTERPAGE_STYLE,
    XML_TOK_MASTERPAGE_NOTES
};

enum SdXMLMasterPageAttrTokenMap
{
    XML_TOK_MASTERPAGE_NAME,
    XML_TOK_MASTERPAGE_PAGE_MASTER_NAME,
    XML_TOK_MASTERPAGE_STYLE_NAME
};

enum SdXMLPageMasterAttrTokenMap
{
    XML_TOK_PAGEMASTER_NAME
};

enum SdXMLPageMasterStyleAttrTokenMap
{
    XML_TOK_PAGEMASTERSTYLE_MARGIN_TOP,
    XML_TOK_PAGEMASTERSTYLE_MARGIN_BOTTOM,
    XML_TOK_PAGEMASTERSTYLE_MARGIN_LEFT,
    XML_TOK_PAGEMASTERSTYLE_MARGIN_RIGHT,
    XML_TOK_PAGEMASTERSTYLE_PAGE_WIDTH,
    XML_TOK_PAGEMASTERSTYLE_PAGE_HEIGHT,
    XML_TOK_PAGEMASTERSTYLE_PAGE_ORIENTATION
};

enum SdXMLDocStyleAttrTokenMap
{
    XML_TOK_DOCSTYLE_NAME,
    XML_TOK_DOCSTYLE_FAMILY,
    XML_TOK_DOCSTYLE_PARENT_STYLE_NAME,
    XML_TOK_DOCSTYLE_AUTOMATIC
};

enum SdXMLDocStyleElemTokenMap
{
    XML_TOK_DOCSTYLE_PROPERTIES,
    XML_TOK_DOCSTYLE_PRESENTATION_PLACEHOLDER
};

enum SdXMLDrawPageAttrTokenMap
{
    XML_TOK_DRAWPAGE_NAME,
    XML_TOK_DRAWPAGE_STYLE_NAME,
    XML_TOK_DRAWPAGE_MASTER_PAGE_NAME,
    XML_TOK_DRAWPAGE_PAGE_LAYOUT_NAME
};

enum SdXMLDrawPageElemTokenMap
{
    XML_TOK_DRAWPAGE_NOTES
};

enum SdXMLPresentationPlaceholderAttrTokenMap
{
    XML_TOK_PRESENTATIONPLACEHOLDER_OBJECTNAME,
    XML_TOK_PRESENTATIONPLACEHOLDER_X,
    XML_TOK_PRESENTATIONPLACEHOLDER_Y,
    XML_TOK_PRESENTATIONPLACEHOLDER_WIDTH,
    XML_TOK_PRESENTATIONPLACEHOLDER_HEIGHT
};

//////////////////////////////////////////////////////////////////////////////

class SvXMLUnitConverter;
class SvXMLTokenMap;
class XMLSdPropHdlFactory;
class XMLPropertySetMapper;
class XMLPropStyleContext;
class SdXMLStylesContext;
class SdXMLMasterStylesContext;

//////////////////////////////////////////////////////////////////////////////

class SdXMLImport: public SvXMLImport
{
    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > mxStatusIndicator;
    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > mxDocStyleFamilies;
    com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > mxDocMasterPages;
    com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > mxDocDrawPages;

    // contexts for Style and AutoStyle import
    SdXMLMasterStylesContext*   mpMasterStylesContext;

    // token map lists
    SvXMLTokenMap*              mpDocElemTokenMap;
    SvXMLTokenMap*              mpBodyElemTokenMap;
    SvXMLTokenMap*              mpStylesElemTokenMap;
    SvXMLTokenMap*              mpAutoStylesElemTokenMap;
    SvXMLTokenMap*              mpMasterPageElemTokenMap;
    SvXMLTokenMap*              mpMasterPageAttrTokenMap;
    SvXMLTokenMap*              mpPageMasterAttrTokenMap;
    SvXMLTokenMap*              mpPageMasterStyleAttrTokenMap;
    SvXMLTokenMap*              mpDocStyleAttrTokenMap;
    SvXMLTokenMap*              mpDocStyleElemTokenMap;
    SvXMLTokenMap*              mpDrawPageAttrTokenMap;
    SvXMLTokenMap*              mpDrawPageElemTokenMap;
    SvXMLTokenMap*              mpPresentationPlaceholderAttrTokenMap;

    sal_uInt16                  mnStyleFamilyMask;

    sal_Int32                   mnNewPageCount;
    sal_Int32                   mnNewMasterPageCount;

    sal_Bool                    mbIsDraw;
    sal_Bool                    mbLoadDoc;

protected:
    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext(sal_uInt16 nPrefix,
      const rtl::OUString& rLocalName,
      const com::sun::star::uno::Reference<
        com::sun::star::xml::sax::XAttributeList>& xAttrList);

public:
    SdXMLImport(
        com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rMod,
        com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer >& rGrfContainer,
        sal_Bool bLoadDoc,
        sal_uInt16 nStyleFamMask,
        sal_Bool bShowProgr,
        sal_Bool bIsDraw);
    ~SdXMLImport();

    void SetProgress(sal_Int32 nProg);

    // namespace office
    SvXMLImportContext* CreateMetaContext(const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    SvXMLImportContext* CreateBodyContext(const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    SvXMLStylesContext* CreateStylesContext(const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    SvXMLStylesContext* CreateAutoStylesContext(const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    SvXMLImportContext* CreateMasterStylesContext(const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);

    // Styles and AutoStyles contexts
    const SdXMLMasterStylesContext* GetMasterStylesContext() const { return mpMasterStylesContext; }

    sal_uInt16 GetStyleFamilyMask() const { return mnStyleFamilyMask; }
    sal_Bool IsStylesOnlyMode() const { return !mbLoadDoc; }

    const SvXMLTokenMap& GetDocElemTokenMap();
    const SvXMLTokenMap& GetBodyElemTokenMap();
    const SvXMLTokenMap& GetStylesElemTokenMap();
    const SvXMLTokenMap& GetMasterPageElemTokenMap();
    const SvXMLTokenMap& GetMasterPageAttrTokenMap();
    const SvXMLTokenMap& GetPageMasterAttrTokenMap();
    const SvXMLTokenMap& GetPageMasterStyleAttrTokenMap();
    const SvXMLTokenMap& GetDrawPageAttrTokenMap();
    const SvXMLTokenMap& GetDrawPageElemTokenMap();
    const SvXMLTokenMap& GetPresentationPlaceholderAttrTokenMap();

    // export local parameters concerning page access and similar
    const com::sun::star::uno::Reference<
        com::sun::star::container::XNameAccess >& GetLocalDocStyleFamilies() const { return mxDocStyleFamilies; }
    const com::sun::star::uno::Reference<
        com::sun::star::container::XIndexAccess >& GetLocalMasterPages() const { return mxDocMasterPages; }
    const com::sun::star::uno::Reference<
        com::sun::star::container::XIndexAccess >& GetLocalDrawPages() const { return mxDocDrawPages; }

    sal_Int32 GetNewPageCount() const { return mnNewPageCount; }
    void IncrementNewPageCount() { mnNewPageCount++; }
    sal_Int32 GetNewMasterPageCount() const { return mnNewMasterPageCount; }
    void IncrementNewMasterPageCount() { mnNewMasterPageCount++; }

    sal_Bool IsDraw() const { return mbIsDraw; }
    sal_Bool IsImpress() const { return !mbIsDraw; }

    // import pool defaults. Parameter contains pool defaults read
    // from input data. These data needs to be set at the model.
    void ImportPoolDefaults(const XMLPropStyleContext* pPool);
};

::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler >
    CreateSdXMLImport(
        com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rMod,
        com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer >& rGrfContainer,
        sal_Bool bLoadDoc,
        sal_uInt16 nStyleFamMask,
        sal_Bool bShowProgr,
        sal_Bool bIsDraw );

#endif  //  _SDXMLIMP_HXX
