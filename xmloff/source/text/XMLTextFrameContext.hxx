/*************************************************************************
 *
 *  $RCSfile: XMLTextFrameContext.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: dvo $ $Date: 2001-07-25 13:29:47 $
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

#ifndef _XMLTEXTFRAMECONTEXT_HXX
#define _XMLTEXTFRAMECONTEXT_HXX

#ifndef __SGI_STL_MAP
#include <map>
#endif

#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#if SUPD > 632 || DVO_TEST
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#else
#ifndef _XMLOFF_FUNCTIONAL_HXX
#include <xmloff/functional.hxx>
#endif
#endif


namespace com { namespace sun { namespace star {
    namespace text { class XTextCursor; class XTextContent; }
    namespace beans { class XPropertySet; }
    namespace io { class XOutputStream; }
} } }

#define XML_TEXT_FRAME_TEXTBOX 1
#define XML_TEXT_FRAME_GRAPHIC 2
#define XML_TEXT_FRAME_OBJECT 3
#define XML_TEXT_FRAME_OBJECT_OLE 4
#define XML_TEXT_FRAME_APPLET 5
#define XML_TEXT_FRAME_PLUGIN 6
#define XML_TEXT_FRAME_FLOATING_FRAME 7

#if SUPD > 632 || DVO_TEST
typedef ::std::map < const ::rtl::OUString, ::rtl::OUString, ::comphelper::UStringLess> ParamMap;
#else
typedef ::std::map < const ::rtl::OUString, ::rtl::OUString, less_functor> ParamMap;
#endif

class XMLTextFrameContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xOldTextCursor;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet > xPropSet;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::io::XOutputStream > xBase64Stream;

    /// old list item and block (#89891#)
    SvXMLImportContextRef xListBlock;
    SvXMLImportContextRef xListItem;

    const ::rtl::OUString sWidth;
    const ::rtl::OUString sRelativeWidth;
    const ::rtl::OUString sHeight;
    const ::rtl::OUString sRelativeHeight;
    const ::rtl::OUString sSizeType;
    const ::rtl::OUString sIsSyncWidthToHeight;
    const ::rtl::OUString sIsSyncHeightToWidth;
    const ::rtl::OUString sHoriOrientPosition;
    const ::rtl::OUString sVertOrientPosition;
    const ::rtl::OUString sChainNextName;
    const ::rtl::OUString sAnchorType;
    const ::rtl::OUString sAnchorPageNo;
    const ::rtl::OUString sGraphicURL;
    const ::rtl::OUString sGraphicFilter;
    const ::rtl::OUString sAlternativeText;
    const ::rtl::OUString sFrameStyleName;
    const ::rtl::OUString sGraphicRotation;
    const ::rtl::OUString sTextBoxServiceName;
    const ::rtl::OUString sGraphicServiceName;

    ::rtl::OUString sName;
    ::rtl::OUString sStyleName;
    ::rtl::OUString sNextName;
    ::rtl::OUString sHRef;
    ::rtl::OUString sFilterName;
    ::rtl::OUString sCode;
    ::rtl::OUString sObject;
    ::rtl::OUString sArchive;
    ::rtl::OUString sMimeType;
    ::rtl::OUString sFrameName;
    ::rtl::OUString sAppletName;
    ::rtl::OUString sDesc;
    ::rtl::OUString sFilterService;
    ::rtl::OUString sBase64CharsLeft;
    ::rtl::OUString sTblName;

    ParamMap aParamMap;

    sal_Int32   nX;
    sal_Int32   nY;
    sal_Int32   nWidth;
    sal_Int32   nHeight;
    sal_Int32   nZIndex;
    sal_Int16   nPage;
    sal_Int16   nRotation;
    sal_Int16   nRelWidth;
    sal_Int16   nRelHeight;

    sal_uInt16 nType;
    ::com::sun::star::text::TextContentAnchorType   eAnchorType;

    sal_Bool    bMayScript : 1;
    sal_Bool    bMinHeight : 1;
    sal_Bool    bSyncWidth : 1;
    sal_Bool    bSyncHeight : 1;
    sal_Bool    bCreateBase64StreamFailed : 1;
    sal_Bool    bOwnBase64Stream : 1;

    void Create( sal_Bool bHRefOrBase64 );

public:

    TYPEINFO();

    XMLTextFrameContext( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            ::com::sun::star::text::TextContentAnchorType eAnchorType,
            sal_uInt16 nType );
    virtual ~XMLTextFrameContext();

    virtual void EndElement();

    virtual void Characters( const ::rtl::OUString& rChars );

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    void SetHyperlink( const ::rtl::OUString& rHRef,
                       const ::rtl::OUString& rName,
                       const ::rtl::OUString& rTargetFrameName,
                       sal_Bool bMap );

    ::com::sun::star::text::TextContentAnchorType GetAnchorType() const { return eAnchorType; }

    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextContent > GetTextContent() const;
};


#endif
