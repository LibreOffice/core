/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlstyle.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:16:32 $
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

#ifndef _XMLOFF_XMLSTYLE_HXX
#define _XMLOFF_XMLSTYLE_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _RSCSFX_HXX //autogen wg. SfxStyleFamily
#include <rsc/rscsfx.hxx>
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef _UNIVERSALL_REFERENCE_HXX
#include <xmloff/uniref.hxx>
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include <xmloff/xmltkmap.hxx>
#endif
#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

class SvXMLStylesContext_Impl;
class SvXMLUnitConverter;
class SvXMLImportPropertyMapper;

namespace com { namespace sun { namespace star {
namespace container { class XNameContainer; }
namespace style { class XAutoStyleFamily; }
} } }

enum XMLStyleStylesElemTokens
{
    XML_TOK_STYLE_STYLE,
    XML_TOK_STYLE_PAGE_MASTER,
    XML_TOK_TEXT_LIST_STYLE,
    XML_TOK_TEXT_OUTLINE,
    XML_TOK_STYLES_GRADIENTSTYLES,
    XML_TOK_STYLES_HATCHSTYLES,
    XML_TOK_STYLES_BITMAPSTYLES,
    XML_TOK_STYLES_TRANSGRADIENTSTYLES,
    XML_TOK_STYLES_MARKERSTYLES,
    XML_TOK_STYLES_DASHSTYLES,
    XML_TOK_TEXT_NOTE_CONFIG,
    XML_TOK_TEXT_BIBLIOGRAPHY_CONFIG,
    XML_TOK_TEXT_LINENUMBERING_CONFIG,
    XML_TOK_STYLE_DEFAULT_STYLE,
    XML_TOK_STYLE_DEFAULT_PAGE_LAYOUT,  //text grid enhancement
    XML_TOK_STYLE_STYLES_ELEM_END=XML_TOK_UNKNOWN
};

class XMLOFF_DLLPUBLIC SvXMLStyleContext : public SvXMLImportContext
{
    ::rtl::OUString     maName;
    ::rtl::OUString     maDisplayName;
    ::rtl::OUString     maAutoName;
    ::rtl::OUString     maParentName;// Will be moved to XMLPropStyle soon!!!!
    ::rtl::OUString     maFollow;   // Will be moved to XMLPropStyle soon!!!!

    ::rtl::OUString     maHelpFile; // Will be removed very soon!!!!

    sal_uInt32          mnHelpId;   // Will be removed very soon!!!!
    sal_uInt16          mnFamily;

    sal_Bool            mbValid : 1;    // Set this to false in CreateAndInsert
                                    // if the style shouldn't be processed
                                    // by Finish() or si somehow invalid.
    sal_Bool            mbNew : 1;  // Set this to false in CreateAnsInsert
                                    // if the style is already existing.
    sal_Bool            mbDefaultStyle : 1;

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const ::rtl::OUString& rLocalName,
                               const ::rtl::OUString& rValue );

    void SetFamily( sal_uInt16 nSet ) { mnFamily = nSet; }
    void SetAutoName( const ::rtl::OUString& rName ) { maAutoName = rName; }

public:

    TYPEINFO();

    SvXMLStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
    sal_uInt16 nFamily=0, sal_Bool bDefaultStyle = sal_False );

    virtual ~SvXMLStyleContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    const ::rtl::OUString&  GetName() const { return maName; }
    const ::rtl::OUString&  GetDisplayName() const { return maDisplayName.getLength() ? maDisplayName : maName; }
    const ::rtl::OUString&  GetAutoName() const { return maAutoName; }
    const ::rtl::OUString&  GetParentName() const { return maParentName; }
    const ::rtl::OUString&  GetFollow() const { return maFollow; }

    const ::rtl::OUString&  GetHelpFile() const { return maHelpFile; }
    sal_uInt32  GetHelpId() const { return mnHelpId; }

    sal_uInt16 GetFamily() const { return mnFamily; }

    BOOL IsValid() const { return mbValid; }
    void SetValid( sal_Bool b ) { mbValid = b; }

    BOOL IsNew() const { return mbNew; }
    void SetNew( sal_Bool b ) { mbNew = b; }

    // This method is called for every default style
    virtual void SetDefaults();

    // This method is called for every style. It must create it and insert
    // it into the document.
    virtual void CreateAndInsert( sal_Bool bOverwrite );

    // This method is called for every style. It must create it and insert
    // it into the document if this hasn't happened already in CreateAndInsert().
    virtual void CreateAndInsertLate( sal_Bool bOverwrite );

    // This method is called fpr every style after all styles have been
    // inserted into the document.
    virtual void Finish( sal_Bool bOverwrite );

    sal_Bool IsDefaultStyle() const { return mbDefaultStyle; }

    /** if this method returns true, its parent styles context
        should not add it to its container.<br>
        Transient styles can't be accessed from its
        parent SvXMLStylesContext after theyre imported and
        the methods CreateAndInsert(), CreateAndInsertLate()
        and Finish() will not be called.
        The default return value is false
    */
    virtual BOOL IsTransient() const;
};

class XMLOFF_DLLPUBLIC SvXMLStylesContext : public SvXMLImportContext
{
    const ::rtl::OUString msParaStyleServiceName;
    const ::rtl::OUString msTextStyleServiceName;

    SvXMLStylesContext_Impl *mpImpl;
    SvXMLTokenMap           *mpStyleStylesElemTokenMap;


    ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer > mxParaStyles;

    ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer > mxTextStyles;

    ::com::sun::star::uno::Reference <
                    ::com::sun::star::style::XAutoStyleFamily > mxParaAutoStyles;

    ::com::sun::star::uno::Reference <
                    ::com::sun::star::style::XAutoStyleFamily > mxTextAutoStyles;

    UniReference < SvXMLImportPropertyMapper > mxParaImpPropMapper;
    UniReference < SvXMLImportPropertyMapper > mxTextImpPropMapper;
    UniReference < SvXMLImportPropertyMapper > mxShapeImpPropMapper;
    mutable UniReference < SvXMLImportPropertyMapper > mxChartImpPropMapper;
    mutable UniReference < SvXMLImportPropertyMapper > mxPageImpPropMapper;

    SAL_DLLPRIVATE const SvXMLTokenMap& GetStyleStylesElemTokenMap();

    SAL_DLLPRIVATE SvXMLStylesContext(SvXMLStylesContext &); // not defined
    SAL_DLLPRIVATE void operator =(SvXMLStylesContext &); // not defined

protected:

    sal_uInt32 GetStyleCount() const;
    SvXMLStyleContext *GetStyle( sal_uInt32 i );
    const SvXMLStyleContext *GetStyle( sal_uInt32 i ) const;

    virtual SvXMLStyleContext *CreateStyleChildContext( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual SvXMLStyleContext *CreateStyleStyleChildContext( sal_uInt16 nFamily,
        sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual sal_Bool InsertStyleFamily( sal_uInt16 nFamily ) const;

public:
    TYPEINFO();

    SvXMLStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
        sal_Bool bAutomatic = sal_False );

    virtual ~SvXMLStylesContext();

    // Create child element.
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    // This method must be overloaded to insert styles into the document.
    virtual void EndElement();

    // This allows to add an SvXMLStyleContext to this context from extern
    void AddStyle(SvXMLStyleContext& rNew);

    const SvXMLStyleContext *FindStyleChildContext(
                                      sal_uInt16 nFamily,
                                      const ::rtl::OUString& rName,
                                      BOOL bCreateIndex=sal_False ) const;
    virtual sal_uInt16 GetFamily( const ::rtl::OUString& rFamily ) const;
    virtual UniReference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
                        sal_uInt16 nFamily ) const;

    virtual ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer >
        GetStylesContainer( sal_uInt16 nFamily ) const;
    virtual ::rtl::OUString GetServiceName( sal_uInt16 nFamily ) const;

       virtual ::com::sun::star::uno::Reference < ::com::sun::star::style::XAutoStyleFamily >
        GetAutoStyles( sal_uInt16 nFamily ) const;
    void CopyAutoStylesToDoc();
    void CopyStylesToDoc( sal_Bool bOverwrite, sal_Bool bFinish=sal_True );
    void FinishStyles( sal_Bool bOverwrite );

    // This method must be called to release the references to all styles
    // that are stored in the context.
    void Clear();
    sal_Bool IsAutomaticStyle() const;
};

#endif  //  _XMLOFF_XMLSTYLE_HXX

