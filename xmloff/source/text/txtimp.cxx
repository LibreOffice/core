/*************************************************************************
 *
 *  $RCSfile: txtimp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dvo $ $Date: 2000-09-27 15:58:45 $
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

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVSTDARR_STRINGSDTOR_DECL
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XCHAPTERNUMBERINGSUPPLIER_HPP_
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAMESSUPPLIER_HPP_
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTGRAPHICOBJECTSSUPPLIER_HPP_
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTEMBEDDEDOBJECTSSUPPLIER_HPP_
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAME_HPP_
#include <com/sun/star/text/XTextFrame.hpp>
#endif


#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include "prstylei.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _XMLOFF_XMLNUMFI_HXX
#include "xmlnumfi.hxx"
#endif
#ifndef _XMLOFF_XMLNUMI_HXX
#include "xmlnumi.hxx"
#endif

#ifndef _XMLOFF_TXTPARAI_HXX
#include "txtparai.hxx"
#endif
#ifndef _XMLOFF_TXTIMP_HXX
#include "txtimp.hxx"
#endif
#ifndef _XMLOFF_TXTPRMAP_HXX
#include "txtprmap.hxx"
#endif
#ifndef _XMLOFF_TXTIMPPR_HXX
#include "txtimppr.hxx"
#endif
#ifndef _XMLOFF_TXTVFLDI_HXX
#include "txtvfldi.hxx"
#endif
#ifndef _XMLOFF_I18NMAP_HXX
#include "i18nmap.hxx"
#endif
#ifndef _XMLTEXTLISTITEMCONTEXT_HXX
#include "XMLTextListItemContext.hxx"
#endif
#ifndef _XMLTEXTLISTBLOCKCONTEXT_HXX
#include "XMLTextListBlockContext.hxx"
#endif
#ifndef _XMLTEXTFRAMECONTEXT_HXX
#include "XMLTextFrameContext.hxx"
#endif
#ifndef _XMLOFF_XMLPROPERTYBACKPATCHTER_HXX
#include "XMLPropertyBackpatcher.hxx"
#endif
#ifndef _XMLTEXTFRAMEHYPERLINKCONTEXT_HXX
#include "XMLTextFrameHyperlinkContext.hxx"
#endif

using namespace ::rtl;
using namespace ::std;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::xml::sax;


static __FAR_DATA SvXMLTokenMapEntry aTextElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_p,               XML_TOK_TEXT_P              },
    { XML_NAMESPACE_TEXT, sXML_h,               XML_TOK_TEXT_H              },
    { XML_NAMESPACE_TEXT, sXML_ordered_list,    XML_TOK_TEXT_ORDERED_LIST   },
    { XML_NAMESPACE_TEXT, sXML_unordered_list,  XML_TOK_TEXT_UNORDERED_LIST },
    { XML_NAMESPACE_TEXT, sXML_text_box,        XML_TOK_TEXT_TEXTBOX_PAGE },
    { XML_NAMESPACE_OFFICE, sXML_image,         XML_TOK_TEXT_IMAGE_PAGE },
    { XML_NAMESPACE_DRAW, sXML_a,               XML_TOK_DRAW_A_PAGE },
    { XML_NAMESPACE_TABLE,sXML_table,           XML_TOK_TABLE_TABLE         },
//  { XML_NAMESPACE_TABLE,sXML_sub_table,       XML_TOK_TABLE_SUBTABLE      },
    { XML_NAMESPACE_TEXT, sXML_variable_decls,  XML_TOK_TEXT_VARFIELD_DECLS },
    { XML_NAMESPACE_TEXT, sXML_user_field_decls,XML_TOK_TEXT_USERFIELD_DECLS },
    { XML_NAMESPACE_TEXT, sXML_sequence_decls,  XML_TOK_TEXT_SEQUENCE_DECLS },
    { XML_NAMESPACE_TEXT, sXML_dde_connection_decls, XML_TOK_TEXT_DDE_DECLS },

    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextPElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_span, XML_TOK_TEXT_SPAN },
    { XML_NAMESPACE_TEXT, sXML_tab_stop, XML_TOK_TEXT_TAB_STOP },
    { XML_NAMESPACE_TEXT, sXML_line_break, XML_TOK_TEXT_LINE_BREAK },
    { XML_NAMESPACE_TEXT, sXML_s, XML_TOK_TEXT_S },
    { XML_NAMESPACE_TEXT, sXML_a, XML_TOK_TEXT_HYPERLINK },

    { XML_NAMESPACE_TEXT, sXML_footnote, XML_TOK_TEXT_FOOTNOTE },
    { XML_NAMESPACE_TEXT, sXML_endnote, XML_TOK_TEXT_ENDNOTE },
    { XML_NAMESPACE_TEXT, sXML_bookmark, XML_TOK_TEXT_BOOKMARK },
    { XML_NAMESPACE_TEXT, sXML_bookmark_start, XML_TOK_TEXT_BOOKMARK_START },
    { XML_NAMESPACE_TEXT, sXML_bookmark_end, XML_TOK_TEXT_BOOKMARK_END },
    { XML_NAMESPACE_TEXT, sXML_reference_mark, XML_TOK_TEXT_REFERENCE },
    { XML_NAMESPACE_TEXT, sXML_reference_mark_start,
      XML_TOK_TEXT_REFERENCE_START },
    { XML_NAMESPACE_TEXT, sXML_reference_mark_end,
      XML_TOK_TEXT_REFERENCE_END },

    { XML_NAMESPACE_TEXT, sXML_text_box, XML_TOK_TEXT_TEXTBOX },
    { XML_NAMESPACE_OFFICE, sXML_image, XML_TOK_TEXT_IMAGE },
    { XML_NAMESPACE_DRAW, sXML_a,               XML_TOK_DRAW_A },

    // sender fields
    { XML_NAMESPACE_TEXT, sXML_sender_firstname,XML_TOK_TEXT_SENDER_FIRSTNAME},
    { XML_NAMESPACE_TEXT, sXML_sender_lastname, XML_TOK_TEXT_SENDER_LASTNAME },
    { XML_NAMESPACE_TEXT, sXML_sender_initials, XML_TOK_TEXT_SENDER_INITIALS },
    { XML_NAMESPACE_TEXT, sXML_sender_title, XML_TOK_TEXT_SENDER_TITLE },
    { XML_NAMESPACE_TEXT, sXML_sender_position, XML_TOK_TEXT_SENDER_POSITION },
    { XML_NAMESPACE_TEXT, sXML_sender_email, XML_TOK_TEXT_SENDER_EMAIL },
    { XML_NAMESPACE_TEXT, sXML_sender_phone_private,
      XML_TOK_TEXT_SENDER_PHONE_PRIVATE },
    { XML_NAMESPACE_TEXT, sXML_sender_fax, XML_TOK_TEXT_SENDER_FAX },
    { XML_NAMESPACE_TEXT, sXML_sender_company, XML_TOK_TEXT_SENDER_COMPANY },
    { XML_NAMESPACE_TEXT, sXML_sender_phone_work,
      XML_TOK_TEXT_SENDER_PHONE_WORK },
    { XML_NAMESPACE_TEXT, sXML_sender_street, XML_TOK_TEXT_SENDER_STREET },
    { XML_NAMESPACE_TEXT, sXML_sender_city, XML_TOK_TEXT_SENDER_CITY },
    { XML_NAMESPACE_TEXT, sXML_sender_postal_code,
      XML_TOK_TEXT_SENDER_POSTAL_CODE },
    { XML_NAMESPACE_TEXT, sXML_sender_country, XML_TOK_TEXT_SENDER_COUNTRY },
    { XML_NAMESPACE_TEXT, sXML_sender_state_or_province,
      XML_TOK_TEXT_SENDER_STATE_OR_PROVINCE },

    // misc. document fields
    { XML_NAMESPACE_TEXT, sXML_author_name, XML_TOK_TEXT_AUTHOR_NAME },
    { XML_NAMESPACE_TEXT, sXML_author_initials, XML_TOK_TEXT_AUTHOR_INITIALS },
    { XML_NAMESPACE_TEXT, sXML_date, XML_TOK_TEXT_DATE },
    { XML_NAMESPACE_TEXT, sXML_time, XML_TOK_TEXT_TIME },
    { XML_NAMESPACE_TEXT, sXML_page_number, XML_TOK_TEXT_PAGE_NUMBER },
    { XML_NAMESPACE_TEXT, sXML_page_continuation_string,
      XML_TOK_TEXT_PAGE_CONTINUATION_STRING },

    // variable fields
    { XML_NAMESPACE_TEXT, sXML_variable_set, XML_TOK_TEXT_VARIABLE_SET },
    { XML_NAMESPACE_TEXT, sXML_variable_get, XML_TOK_TEXT_VARIABLE_GET },
    { XML_NAMESPACE_TEXT, sXML_variable_input, XML_TOK_TEXT_VARIABLE_INPUT },
    { XML_NAMESPACE_TEXT, sXML_user_field_get, XML_TOK_TEXT_USER_FIELD_GET },
    { XML_NAMESPACE_TEXT, sXML_user_field_input,XML_TOK_TEXT_USER_FIELD_INPUT},
    { XML_NAMESPACE_TEXT, sXML_sequence, XML_TOK_TEXT_SEQUENCE },
    { XML_NAMESPACE_TEXT, sXML_expression, XML_TOK_TEXT_EXPRESSION },
    { XML_NAMESPACE_TEXT, sXML_text_input, XML_TOK_TEXT_TEXT_INPUT },

    // database fields
    { XML_NAMESPACE_TEXT, sXML_database_display,
      XML_TOK_TEXT_DATABASE_DISPLAY },
    { XML_NAMESPACE_TEXT, sXML_database_next,
      XML_TOK_TEXT_DATABASE_NEXT },
    { XML_NAMESPACE_TEXT, sXML_database_select,
      XML_TOK_TEXT_DATABASE_SELECT },
    { XML_NAMESPACE_TEXT, sXML_database_row_number,
      XML_TOK_TEXT_DATABASE_ROW_NUMBER },
    { XML_NAMESPACE_TEXT, sXML_database_name, XML_TOK_TEXT_DATABASE_NAME },

    // docinfo fields
    { XML_NAMESPACE_TEXT, sXML_initial_creator,
      XML_TOK_TEXT_DOCUMENT_CREATION_AUTHOR },
    { XML_NAMESPACE_TEXT, sXML_description, XML_TOK_TEXT_DOCUMENT_DESCRIPTION},
    { XML_NAMESPACE_TEXT, sXML_user_info_0,
      XML_TOK_TEXT_DOCUMENT_INFORMATION_0 },
    { XML_NAMESPACE_TEXT, sXML_user_info_1,
      XML_TOK_TEXT_DOCUMENT_INFORMATION_1 },
    { XML_NAMESPACE_TEXT, sXML_user_info_2,
      XML_TOK_TEXT_DOCUMENT_INFORMATION_2 },
    { XML_NAMESPACE_TEXT, sXML_user_info_3,
      XML_TOK_TEXT_DOCUMENT_INFORMATION_3 },
    { XML_NAMESPACE_TEXT, sXML_printed_by, XML_TOK_TEXT_DOCUMENT_PRINT_AUTHOR},
    { XML_NAMESPACE_TEXT, sXML_title, XML_TOK_TEXT_DOCUMENT_TITLE },
    { XML_NAMESPACE_TEXT, sXML_subject, XML_TOK_TEXT_DOCUMENT_SUBJECT },
    { XML_NAMESPACE_TEXT, sXML_keywords, XML_TOK_TEXT_DOCUMENT_KEYWORDS },
    { XML_NAMESPACE_TEXT, sXML_creator, XML_TOK_TEXT_DOCUMENT_SAVE_AUTHOR },
    { XML_NAMESPACE_TEXT, sXML_editing_cycles,
      XML_TOK_TEXT_DOCUMENT_REVISION },
    { XML_NAMESPACE_TEXT, sXML_creation_date,
      XML_TOK_TEXT_DOCUMENT_CREATION_DATE },
    { XML_NAMESPACE_TEXT, sXML_creation_time,
      XML_TOK_TEXT_DOCUMENT_CREATION_TIME },
    { XML_NAMESPACE_TEXT, sXML_print_date, XML_TOK_TEXT_DOCUMENT_PRINT_DATE },
    { XML_NAMESPACE_TEXT, sXML_print_time, XML_TOK_TEXT_DOCUMENT_PRINT_TIME },
    { XML_NAMESPACE_TEXT, sXML_modification_date,
      XML_TOK_TEXT_DOCUMENT_SAVE_DATE },
    { XML_NAMESPACE_TEXT, sXML_modification_time,
      XML_TOK_TEXT_DOCUMENT_SAVE_TIME },
    { XML_NAMESPACE_TEXT, sXML_editing_duration,
      XML_TOK_TEXT_DOCUMENT_EDIT_DURATION },
    { XML_NAMESPACE_TEXT, sXML_user_defined,
      XML_TOK_TEXT_DOCUMENT_USER_DEFINED },

    // misc fields
    { XML_NAMESPACE_TEXT, sXML_placeholder, XML_TOK_TEXT_PLACEHOLDER },
    { XML_NAMESPACE_TEXT, sXML_hidden_text, XML_TOK_TEXT_HIDDEN_TEXT },
    { XML_NAMESPACE_TEXT, sXML_hidden_paragraph,
      XML_TOK_TEXT_HIDDEN_PARAGRAPH },
    { XML_NAMESPACE_TEXT, sXML_conditional_text,
      XML_TOK_TEXT_CONDITIONAL_TEXT },
    { XML_NAMESPACE_TEXT, sXML_file_name, XML_TOK_TEXT_FILENAME },
    { XML_NAMESPACE_TEXT, sXML_chapter, XML_TOK_TEXT_CHAPTER },
    { XML_NAMESPACE_TEXT, sXML_template_name, XML_TOK_TEXT_TEMPLATENAME },
    { XML_NAMESPACE_TEXT, sXML_paragraph_count, XML_TOK_TEXT_PARAGRAPH_COUNT },
    { XML_NAMESPACE_TEXT, sXML_word_count, XML_TOK_TEXT_WORD_COUNT },
    { XML_NAMESPACE_TEXT, sXML_table_count, XML_TOK_TEXT_TABLE_COUNT },
    { XML_NAMESPACE_TEXT, sXML_character_count, XML_TOK_TEXT_CHARACTER_COUNT },
    { XML_NAMESPACE_TEXT, sXML_image_count, XML_TOK_TEXT_IMAGE_COUNT },
    { XML_NAMESPACE_TEXT, sXML_object_count, XML_TOK_TEXT_OBJECT_COUNT },
    { XML_NAMESPACE_TEXT, sXML_page_count, XML_TOK_TEXT_PAGE_COUNT },
    { XML_NAMESPACE_TEXT, sXML_page_variable_get, XML_TOK_TEXT_GET_PAGE_VAR },
    { XML_NAMESPACE_TEXT, sXML_page_variable_set, XML_TOK_TEXT_SET_PAGE_VAR },
    { XML_NAMESPACE_TEXT, sXML_execute_macro, XML_TOK_TEXT_MACRO },
    { XML_NAMESPACE_TEXT, sXML_dde_connection, XML_TOK_TEXT_DDE },
    { XML_NAMESPACE_TEXT, sXML_reference_ref, XML_TOK_TEXT_REFERENCE_REF },
    { XML_NAMESPACE_TEXT, sXML_bookmark_ref, XML_TOK_TEXT_BOOKMARK_REF },
    { XML_NAMESPACE_TEXT, sXML_sequence_ref, XML_TOK_TEXT_SEQUENCE_REF },
    { XML_NAMESPACE_TEXT, sXML_footnote_ref, XML_TOK_TEXT_FOOTNOTE_REF },
    { XML_NAMESPACE_TEXT, sXML_endnote_ref, XML_TOK_TEXT_ENDNOTE_REF },

    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextPAttrTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_style_name,  XML_TOK_TEXT_P_STYLE_NAME },
    { XML_NAMESPACE_TEXT, sXML_cond_style_name,
                                            XML_TOK_TEXT_P_COND_STYLE_NAME },
    { XML_NAMESPACE_TEXT, sXML_level,       XML_TOK_TEXT_P_LEVEL },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextListBlockAttrTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_style_name,
            XML_TOK_TEXT_LIST_BLOCK_STYLE_NAME },
    { XML_NAMESPACE_TEXT, sXML_continue_numbering,
            XML_TOK_TEXT_LIST_BLOCK_CONTINUE_NUMBERING },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextListBlockElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_list_header, XML_TOK_TEXT_LIST_HEADER },
    { XML_NAMESPACE_TEXT, sXML_list_item,   XML_TOK_TEXT_LIST_ITEM   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextFrameAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW, sXML_style_name, XML_TOK_TEXT_FRAME_STYLE_NAME },
    { XML_NAMESPACE_TEXT, sXML_name, XML_TOK_TEXT_FRAME_NAME },
    { XML_NAMESPACE_TEXT, sXML_anchor_type, XML_TOK_TEXT_FRAME_ANCHOR_TYPE },
    { XML_NAMESPACE_TEXT, sXML_anchor_page_number, XML_TOK_TEXT_FRAME_ANCHOR_PAGE_NUMBER },
    { XML_NAMESPACE_SVG, sXML_x, XML_TOK_TEXT_FRAME_X },
    { XML_NAMESPACE_SVG, sXML_y, XML_TOK_TEXT_FRAME_Y },
    { XML_NAMESPACE_SVG, sXML_width, XML_TOK_TEXT_FRAME_WIDTH },
//  { XML_NAMESPACE_FO, sXML_min_width, XML_TOK_TEXT_FRAME_MIN_WIDTH },
    { XML_NAMESPACE_SVG, sXML_height, XML_TOK_TEXT_FRAME_HEIGHT },
    { XML_NAMESPACE_FO, sXML_min_height, XML_TOK_TEXT_FRAME_MIN_HEIGHT },
    { XML_NAMESPACE_TEXT, sXML_chain_next_name, XML_TOK_TEXT_FRAME_NEXT_CHAIN_NAME },
    { XML_NAMESPACE_XLINK, sXML_href, XML_TOK_TEXT_FRAME_HREF },
    { XML_NAMESPACE_TEXT, sXML_name, XML_TOK_TEXT_FRAME_FILTER_NAME },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTextHyperlinkAttrTokenMap[] =
{
    { XML_NAMESPACE_XLINK, sXML_href, XML_TOK_TEXT_HYPERLINK_HREF },
    { XML_NAMESPACE_OFFICE, sXML_name, XML_TOK_TEXT_HYPERLINK_NAME },
    { XML_NAMESPACE_XLINK, sXML_show, XML_TOK_TEXT_HYPERLINK_SHOW },
    { XML_NAMESPACE_OFFICE, sXML_target_frame_name, XML_TOK_TEXT_HYPERLINK_TARGET_FRAME },
    { XML_NAMESPACE_TEXT, sXML_style_name, XML_TOK_TEXT_HYPERLINK_STYLE_NAME },
    { XML_NAMESPACE_TEXT, sXML_visited_style_name, XML_TOK_TEXT_HYPERLINK_VIS_STYLE_NAME },
    { XML_NAMESPACE_OFFICE, sXML_server_map, XML_TOK_TEXT_HYPERLINK_SERVER_MAP },
    XML_TOKEN_MAP_END
};
XMLTextImportHelper::XMLTextImportHelper(
        const Reference < XModel >& rModel,
        sal_Bool bInsertM, sal_Bool bStylesOnlyM ) :
    pTextElemTokenMap( 0 ),
    pTextPElemTokenMap( 0 ),
    pTextPAttrTokenMap( 0 ),
    pTextListBlockAttrTokenMap( 0 ),
    pTextListBlockElemTokenMap( 0 ),
    pTextFieldAttrTokenMap( 0 ),
    pTextFrameAttrTokenMap( 0 ),
    pTextHyperlinkAttrTokenMap( 0 ),
    pPrevFrmNames( 0 ),
    pNextFrmNames( 0 ),
    pRenameMap( 0 ),
    pOutlineStyles( 0 ),
    bInsertMode( bInsertM ),
    bStylesOnlyMode( bStylesOnlyM ),
    pFootnoteBackpatcher(NULL),
    pSequenceIdBackpatcher(NULL),
    pSequenceNameBackpatcher(NULL),
    sParaStyleName(RTL_CONSTASCII_USTRINGPARAM("ParaStyleName")),
    sCharStyleName(RTL_CONSTASCII_USTRINGPARAM("CharStyleName")),
    sHeadingStyleName(RTL_CONSTASCII_USTRINGPARAM("HeadingStyleName")),
    sNumberingLevel(RTL_CONSTASCII_USTRINGPARAM("NumberingLevel")),
    sNumberingStartValue(RTL_CONSTASCII_USTRINGPARAM("NumberingStartValue")),
    sNumberingStyleName(RTL_CONSTASCII_USTRINGPARAM("NumberingStyleName")),
    sNumberingRules(RTL_CONSTASCII_USTRINGPARAM("NumberingRules")),
    sSequenceNumber(RTL_CONSTASCII_USTRINGPARAM("SequenceNumber")),
    sSourceName(RTL_CONSTASCII_USTRINGPARAM("SourceName")),
    sCurrentPresentation(RTL_CONSTASCII_USTRINGPARAM("CurrentPresentation")),
    sNumberingIsNumber(RTL_CONSTASCII_USTRINGPARAM("NumberingIsNumber")),
    sChainNextName(RTL_CONSTASCII_USTRINGPARAM("ChainNextName")),
    sChainPrevName(RTL_CONSTASCII_USTRINGPARAM("ChainPrevName")),
    sHyperLinkURL(RTL_CONSTASCII_USTRINGPARAM("HyperLinkURL")),
    sHyperLinkName(RTL_CONSTASCII_USTRINGPARAM("HyperLinkName")),
    sHyperLinkTarget(RTL_CONSTASCII_USTRINGPARAM("HyperLinkTarget")),
    sUnvisitedCharStyleName(RTL_CONSTASCII_USTRINGPARAM("UnvisitedCharStyleName")),
    sVisitedCharStyleName(RTL_CONSTASCII_USTRINGPARAM("VisitedCharStyleName")),
    sTextFrame(RTL_CONSTASCII_USTRINGPARAM("TextFrame"))
{
    Reference< XChapterNumberingSupplier > xCNSupplier( rModel, UNO_QUERY );

    if( xCNSupplier.is() )
        xChapterNumbering = xCNSupplier->getChapterNumberingRules();

    Reference< XStyleFamiliesSupplier > xFamiliesSupp( rModel, UNO_QUERY );
    DBG_ASSERT( xFamiliesSupp.is(), "no chapter numbering supplier" );

    if( xFamiliesSupp.is() )
    {
        Reference< XNameAccess > xFamilies = xFamiliesSupp->getStyleFamilies();

        const OUString aParaStyles(RTL_CONSTASCII_USTRINGPARAM("ParagraphStyles"));
        if( xFamilies->hasByName( aParaStyles ) )
        {
            Any aAny( xFamilies->getByName( aParaStyles ) );
            aAny >>= xParaStyles;
        }

        const OUString aCharStyles(RTL_CONSTASCII_USTRINGPARAM("CharacterStyles"));
        if( xFamilies->hasByName( aCharStyles ) )
        {
            Any aAny( xFamilies->getByName( aCharStyles ) );
            aAny >>= xTextStyles;
        }

        const OUString aNumStyles(RTL_CONSTASCII_USTRINGPARAM("NumberingStyles"));
        if( xFamilies->hasByName( aNumStyles ) )
        {
            Any aAny( xFamilies->getByName( aNumStyles ) );
            aAny >>= xNumStyles;
        }

        const OUString aFrameStyles(RTL_CONSTASCII_USTRINGPARAM("FrameStyles"));
        if( xFamilies->hasByName( aFrameStyles ) )
        {
            Any aAny( xFamilies->getByName( aFrameStyles ) );
            aAny >>= xFrameStyles;
        }
    }

    Reference < XTextFramesSupplier > xTFS( rModel, UNO_QUERY );
    if( xTFS.is() )
        xTextFrames = xTFS->getTextFrames();

    Reference < XTextGraphicObjectsSupplier > xTGOS( rModel, UNO_QUERY );
    if( xTGOS.is() )
        xGraphics = xTGOS->getGraphicObjects();

    Reference < XTextEmbeddedObjectsSupplier > xTEOS( rModel, UNO_QUERY );
    if( xTEOS.is() )
        xObjects = xTEOS->getEmbeddedObjects();

    XMLPropertySetMapper *pPropMapper =
            new XMLTextPropertySetMapper( TEXT_PROP_MAP_PARA );
    xParaImpPrMap = new XMLTextImportPropertyMapper( pPropMapper );

    pPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_TEXT );
    xTextImpPrMap = new XMLTextImportPropertyMapper( pPropMapper );

    pPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_FRAME );
    xFrameImpPrMap = new XMLTextImportPropertyMapper( pPropMapper );
}

XMLTextImportHelper::~XMLTextImportHelper()
{
    delete pTextElemTokenMap;
    delete pTextPElemTokenMap;
    delete pTextPAttrTokenMap;
    delete pTextListBlockAttrTokenMap;
    delete pTextListBlockElemTokenMap;
    delete pTextFieldAttrTokenMap;
    delete pTextFrameAttrTokenMap;
    delete pTextHyperlinkAttrTokenMap;

    delete pRenameMap;

    delete pPrevFrmNames;
    delete pNextFrmNames;
}

void XMLTextImportHelper::SetCursor( const Reference < XTextCursor > & rCursor )
{
    xCursor = rCursor;
    xText = rCursor->getText();
    xCursorAsRange = Reference < XTextRange >( rCursor, UNO_QUERY );
}

void XMLTextImportHelper::ResetCursor()
{
    xCursor = 0;
    xText = 0;
    xCursorAsRange = 0;
}

SvXMLTokenMap *XMLTextImportHelper::_GetTextElemTokenMap()
{
    return new SvXMLTokenMap( aTextElemTokenMap );
}

SvXMLTokenMap *XMLTextImportHelper::_GetTextPElemTokenMap()
{
    return new SvXMLTokenMap( aTextPElemTokenMap );
}

SvXMLTokenMap *XMLTextImportHelper::_GetTextPAttrTokenMap()
{
    return new SvXMLTokenMap( aTextPAttrTokenMap );
}

SvXMLTokenMap *XMLTextImportHelper::_GetTextFrameAttrTokenMap()
{
    return new SvXMLTokenMap( aTextFrameAttrTokenMap );
}

SvXMLTokenMap *XMLTextImportHelper::_GetTextHyperlinkAttrTokenMap()
{
    return new SvXMLTokenMap( aTextHyperlinkAttrTokenMap );
}

sal_Bool XMLTextImportHelper::HasFrameByName( const OUString& rName ) const
{
    return ( xTextFrames.is() && xTextFrames->hasByName( rName ) ) ||
           ( xGraphics.is() && xGraphics->hasByName( rName ) ) ||
           ( xObjects.is() && xObjects->hasByName( rName ) );
}

void XMLTextImportHelper::InsertString( const OUString& rChars )
{
    DBG_ASSERT( xText.is(), "no text" );
    DBG_ASSERT( xCursorAsRange.is(), "no range" );
    if( xText.is() )
        xText->insertString( xCursorAsRange, rChars, sal_False );
}

void XMLTextImportHelper::InsertString( const OUString& rChars,
                                         sal_Bool& rIgnoreLeadingSpace )
{
    DBG_ASSERT( xText.is(), "no text" );
    DBG_ASSERT( xCursorAsRange.is(), "no range" );
    if( xText.is() )
    {
        sal_Int32 nLen = rChars.getLength();
        OUStringBuffer sChars( nLen );

        for( sal_Int32 i=0; i < nLen; i++ )
        {
            sal_Unicode c = rChars[i];
            switch( c )
            {
                case 0x20:
                case 0x09:
                case 0x0a:
                case 0x0d:
                    if( !rIgnoreLeadingSpace )
                        sChars.append( (sal_Unicode)0x20 );
                    rIgnoreLeadingSpace = sal_True;
                    break;
                default:
                    rIgnoreLeadingSpace = sal_False;
                    sChars.append( c );
                    break;
            }
        }
        xText->insertString( xCursorAsRange, sChars.makeStringAndClear(),
                             sal_False );
    }
}
void XMLTextImportHelper::InsertControlCharacter( sal_Int16 nControl )
{
    DBG_ASSERT( xText.is(), "no text" );
    DBG_ASSERT( xCursorAsRange.is(), "no range" );
    if( xText.is() )
        xText->insertControlCharacter( xCursorAsRange, nControl, sal_False );
}

void XMLTextImportHelper::InsertTextContent(
    Reference < XTextContent > & xContent )
{
    DBG_ASSERT( xText.is(), "no text" );
    DBG_ASSERT( xCursorAsRange.is(), "no range" );
    if( xText.is() )
        xText->insertTextContent( xCursorAsRange, xContent, sal_False );
}

OUString XMLTextImportHelper::SetStyleAndAttrs(
        const Reference < XTextCursor >& rCursor,
        const OUString& rStyleName,
        sal_Bool bPara )
{
    XMLPropStyleContext *pStyle = 0;
    OUString sStyleName( rStyleName );
    if( sStyleName.getLength() && xAutoStyles.Is() )
    {
        sal_uInt32 nFamily = bPara ? XML_STYLE_FAMILY_TEXT_PARAGRAPH
                                   : XML_STYLE_FAMILY_TEXT_TEXT;
        pStyle = PTR_CAST( XMLPropStyleContext,
              ((SvXMLStylesContext *)&xAutoStyles)->
                    FindStyleChildContext( nFamily, sStyleName, sal_True ) );
    }
    if( pStyle )
        sStyleName = pStyle->GetParent();

    Reference < XPropertySet > xPropSet( rCursor, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
        xPropSet->getPropertySetInfo();

    // style
    if( sStyleName.getLength() )
    {
        const String& rPropName = bPara ? sParaStyleName : sCharStyleName;
        const Reference < XNameContainer > & rStyles = bPara ? xParaStyles
                                                          : xTextStyles;
        if( rStyles.is() &&
            xPropSetInfo->hasPropertyByName( rPropName ) &&
            rStyles->hasByName( sStyleName ) )
        {
            Any aAny;
            aAny <<= sStyleName;
            xPropSet->setPropertyValue( rPropName, aAny );
        }
        else
            sStyleName = OUString();
    }

    // hard paragraph properties
    if( pStyle )
        pStyle->FillPropertySet( xPropSet );

    if( bPara && xPropSetInfo->hasPropertyByName( sNumberingRules )  )
    {
        // Set numbering rules
        Reference < XIndexReplace > xNumRule;
        Any aAny = xPropSet->getPropertyValue( sNumberingRules );
        aAny >>= xNumRule;

        if( IsInList() )
        {
            OUString sNumRuleName;
            if( xNumRule.is() &&
                xPropSetInfo->hasPropertyByName( sNumberingStyleName ) )
            {
                aAny = xPropSet->getPropertyValue( sNumberingStyleName );
                aAny >>= sNumRuleName;
            }

            DBG_ASSERT( !xNumRule.is() || sNumRuleName.getLength(),
                        "num rule name is missing!" );
            XMLTextListBlockContext *pListBlock = GetListBlock();
            const OUString& rStyleName = pListBlock->GetRealName();
            if( !xNumRule.is() ||
                ( !pListBlock->HasGeneratedStyle() &&
                  sNumRuleName != rStyleName ) )
            {
                aAny <<= rStyleName;
                xPropSet->setPropertyValue( sNumberingStyleName, aAny );
            }

            XMLTextListItemContext *pListItem = GetListItem();

            sal_Int8 nLevel = pListBlock->GetLevel();
            if( !pListItem &&
                xPropSetInfo->hasPropertyByName( sNumberingIsNumber ) )
            {
                sal_Bool bTmp = sal_False;
                aAny.setValue( &bTmp, ::getBooleanCppuType() );
                xPropSet->setPropertyValue( sNumberingIsNumber, aAny );
            }

            aAny <<= nLevel;
            xPropSet->setPropertyValue( sNumberingLevel, aAny );

            if( pListBlock->IsRestartNumbering() )
            {
                // TODO: property missing
                // aNodeNum.SetStart();
                pListBlock->ResetRestartNumbering();
            }
            if( pListItem && pListItem->HasStartValue() &&
                xPropSetInfo->hasPropertyByName( sNumberingStartValue ) )
            {
                aAny <<= pListItem->GetStartValue();
                xPropSet->setPropertyValue( sNumberingStartValue, aAny );
            }
            SetListItem( 0 );
        }
        else
        {
            // If the paragraph is not in a list but its style, remove it from
            // the list.
            if( xNumRule.is() && xPropSetInfo->hasPropertyByName( sNumberingStyleName ) )
            {
                OUString sTmp;
                aAny <<= sTmp;
                xPropSet->setPropertyValue( sNumberingStyleName, aAny );
            }
        }
    }

    return sStyleName;
}

void XMLTextImportHelper::SetOutlineStyle(
        sal_Int8 nLevel,
        const OUString& rStyleName )
{
    if( rStyleName.getLength() &&
        xChapterNumbering.is() &&
        nLevel > 0 && nLevel <= xChapterNumbering->getCount() )
    {
        if( !pOutlineStyles )
            pOutlineStyles = new OUString[xChapterNumbering->getCount()];
        pOutlineStyles[nLevel-1] = rStyleName;
    }
}

void XMLTextImportHelper::SetOutlineStyles()
{
    if( pOutlineStyles &&
        xChapterNumbering.is() )
    {
        sal_Int32 nCount = xChapterNumbering->getCount();
        for( sal_Int32 i=0; i < nCount; i++ )
        {
            Sequence < PropertyValue > aProps( 1 );
            PropertyValue *pProps = aProps.getArray();
            pProps->Name = sHeadingStyleName;
            pProps->Value <<= pOutlineStyles[i];

            Any aAny;
            aAny <<= aProps;
            xChapterNumbering->replaceByIndex( i, aAny );
        }
    }
}

void XMLTextImportHelper::SetHyperlink(
    const Reference < XTextCursor >& rCursor,
    const OUString& rHRef,
    const OUString& rName,
    const OUString& rTargetFrameName,
    const OUString& rStyleName,
    const OUString& rVisitedStyleName )
{
    Reference < XPropertySet > xPropSet( rCursor, UNO_QUERY );
    Reference < XPropertySetInfo > xPropSetInfo =
        xPropSet->getPropertySetInfo();
    if( !xPropSetInfo.is() || !xPropSetInfo->hasPropertyByName(sHyperLinkURL) )
        return;

    Any aAny;
    aAny <<= rHRef;
    xPropSet->setPropertyValue( sHyperLinkURL, aAny );

    if( xPropSetInfo->hasPropertyByName( sHyperLinkName ) )
    {
        aAny <<= rName;
        xPropSet->setPropertyValue( sHyperLinkName, aAny );
    }

    if( xPropSetInfo->hasPropertyByName( sHyperLinkTarget ) )
    {
        aAny <<= rTargetFrameName;
        xPropSet->setPropertyValue( sHyperLinkTarget, aAny );
    }

    if( xTextStyles.is() )
    {
        if( rStyleName.getLength() &&
            xPropSetInfo->hasPropertyByName( sUnvisitedCharStyleName ) &&
            xTextStyles->hasByName( rStyleName ) )
        {
            aAny <<= rStyleName;
            xPropSet->setPropertyValue( sUnvisitedCharStyleName, aAny );
        }

        if( rVisitedStyleName.getLength() &&
            xPropSetInfo->hasPropertyByName( sVisitedCharStyleName ) &&
            xTextStyles->hasByName( rVisitedStyleName ) )
        {
            aAny <<= rVisitedStyleName;
            xPropSet->setPropertyValue( sVisitedCharStyleName, aAny );
        }
    }
}

void XMLTextImportHelper::SetAutoStyles( SvXMLStylesContext *pStyles )
{
    xAutoStyles = pStyles;
}

SvXMLImportContext *XMLTextImportHelper::CreateTextChildContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList,
        XMLTextType eType )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetTextElemTokenMap();
    sal_Bool bOrdered = sal_False;
    sal_Bool bHeading = sal_False;
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_TEXT_H:
        bHeading = sal_True;
    case XML_TOK_TEXT_P:
        pContext = new XMLParaContext( rImport,
                                       nPrefix, rLocalName,
                                       xAttrList, bHeading );
        break;
    case XML_TOK_TEXT_ORDERED_LIST:
        bOrdered = sal_True;
    case XML_TOK_TEXT_UNORDERED_LIST:
        pContext = new XMLTextListBlockContext( rImport, *this,
                                                nPrefix, rLocalName,
                                                xAttrList, bOrdered );
        break;
    case XML_TOK_TABLE_TABLE:
        if( XML_TEXT_TYPE_BODY == eType || XML_TEXT_TYPE_TEXTBOX == eType )
            pContext = CreateTableChildContext( rImport, nPrefix, rLocalName,
                                                xAttrList );
        break;
    case XML_TOK_TEXT_SEQUENCE_DECLS:
        if( XML_TEXT_TYPE_BODY == eType )
            pContext = new XMLVariableDeclsImportContext(
                rImport, *this, nPrefix, rLocalName, VarTypeSequence);
        break;

    case XML_TOK_TEXT_VARFIELD_DECLS:
        if( XML_TEXT_TYPE_BODY == eType )
            pContext = new XMLVariableDeclsImportContext(
                rImport, *this, nPrefix, rLocalName, VarTypeSimple);
        break;

    case XML_TOK_TEXT_USERFIELD_DECLS:
        if( XML_TEXT_TYPE_BODY == eType )
            pContext = new XMLVariableDeclsImportContext(
                rImport, *this, nPrefix, rLocalName, VarTypeUserField);
        break;

    case XML_TOK_TEXT_DDE_DECLS:
        if( XML_TEXT_TYPE_BODY == eType )
            pContext = new XMLDdeFieldDeclsImportContext(
                rImport, nPrefix, rLocalName);
        break;

    case XML_TOK_TEXT_TEXTBOX_PAGE:
        if( XML_TEXT_TYPE_BODY == eType || XML_TEXT_TYPE_TEXTBOX == eType )
        {
            TextContentAnchorType eAnchorType =
                XML_TEXT_TYPE_TEXTBOX == eType ? TextContentAnchorType_AT_FRAME
                                               : TextContentAnchorType_AT_PAGE;
            pContext = new XMLTextFrameContext( rImport, nPrefix,
                                                rLocalName, xAttrList,
                                                eAnchorType,
                                                XML_TEXT_FRAME_TEXTBOX );
        }
        break;

    case XML_TOK_TEXT_IMAGE_PAGE:
        if( XML_TEXT_TYPE_BODY == eType || XML_TEXT_TYPE_TEXTBOX == eType )
        {
            TextContentAnchorType eAnchorType =
                XML_TEXT_TYPE_TEXTBOX == eType ? TextContentAnchorType_AT_FRAME
                                               : TextContentAnchorType_AT_PAGE;
            pContext = new XMLTextFrameContext( rImport, nPrefix,
                                                rLocalName, xAttrList,
                                                eAnchorType,
                                                XML_TEXT_FRAME_GRAPHIC );
        }
        break;

    case XML_TOK_DRAW_A_PAGE:
        if( XML_TEXT_TYPE_BODY == eType || XML_TEXT_TYPE_TEXTBOX == eType )
        {
            TextContentAnchorType eAnchorType =
                XML_TEXT_TYPE_TEXTBOX == eType ? TextContentAnchorType_AT_FRAME
                                               : TextContentAnchorType_AT_PAGE;
            pContext = new XMLTextFrameHyperlinkContext( rImport, nPrefix,
                                                rLocalName, xAttrList,
                                                eAnchorType );
        }
        break;



    }

//  if( !pContext )
//      pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

SvXMLImportContext *XMLTextImportHelper::CreateTableChildContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    return 0;
}

/// get data style key for use with NumberFormat property
sal_Int32 XMLTextImportHelper::GetDataStyleKey(const OUString& sStyleName)
{
    // get appropriate context
    SvXMLNumFormatContext* pStyle =
        PTR_CAST( SvXMLNumFormatContext,
                  ((SvXMLStylesContext *)&xAutoStyles)->
                  FindStyleChildContext( XML_STYLE_FAMILY_DATA_STYLE,
                                              sStyleName, sal_True ) );

    // return key or default (-1)
    return (0 != pStyle) ? pStyle->GetKey() : -1;
}

const SvxXMLListStyleContext *XMLTextImportHelper::FindAutoListStyle( const OUString& rName ) const
{
    const SvxXMLListStyleContext *pStyle = 0;
    if( xAutoStyles.Is() )
    {
        pStyle = PTR_CAST( SvxXMLListStyleContext ,
              ((SvXMLStylesContext *)&xAutoStyles)->
                    FindStyleChildContext( XML_STYLE_FAMILY_TEXT_LIST, rName,
                                           sal_True ) );
    }

    return pStyle;
}

XMLPropStyleContext *XMLTextImportHelper::FindAutoFrameStyle( const OUString& rName ) const
{
    XMLPropStyleContext *pStyle = 0;
    if( xAutoStyles.Is() )
    {
        pStyle = PTR_CAST( XMLPropStyleContext ,
              ((SvXMLStylesContext *)&xAutoStyles)->
                    FindStyleChildContext( XML_STYLE_FAMILY_SD_GRAPHICS_ID, rName,
                                           sal_True ) );
    }

    return pStyle;
}
XMLTextListItemContext *XMLTextImportHelper::GetListItem()
{
    return (XMLTextListItemContext *)&xListItem;
}

void XMLTextImportHelper::SetListItem( XMLTextListItemContext *pListItem )
{
    xListItem = pListItem;
}

XMLTextListBlockContext *XMLTextImportHelper::GetListBlock()
{
    return (XMLTextListBlockContext *)&xListBlock;
}

void XMLTextImportHelper::SetListBlock( XMLTextListBlockContext *pListBlock )
{
    xListBlock = pListBlock;
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextListBlockAttrTokenMap()
{
    if( !pTextListBlockAttrTokenMap )
        pTextListBlockAttrTokenMap =
            new SvXMLTokenMap( aTextListBlockAttrTokenMap );

    return *pTextListBlockAttrTokenMap;
}

const SvXMLTokenMap& XMLTextImportHelper::GetTextListBlockElemTokenMap()
{
    if( !pTextListBlockElemTokenMap )
        pTextListBlockElemTokenMap =
            new SvXMLTokenMap( aTextListBlockElemTokenMap );

    return *pTextListBlockElemTokenMap;
}

SvI18NMap& XMLTextImportHelper::GetRenameMap()
{
    if( 0 == pRenameMap )
        pRenameMap = new SvI18NMap();
    return *pRenameMap;
}

void XMLTextImportHelper::InsertBookmarkStartRange(
    const OUString sName,
    const Reference<XTextRange> & rRange)
{
    aBookmarkStartRanges[sName] = rRange;
}

sal_Bool XMLTextImportHelper::FindAndRemoveBookmarkStartRange(
    Reference<XTextRange> & rRange,
    const OUString sName)
{
    if (aBookmarkStartRanges.count(sName))
    {
        rRange = aBookmarkStartRanges[sName];
        aBookmarkStartRanges.erase(sName);
        return sal_True;
    }
    else
    {
        return sal_False;
    }
}

XMLPropertyBackpatcher<sal_Int16>& XMLTextImportHelper::GetFootnoteBP()
{
    if (NULL == pFootnoteBackpatcher)
    {
        pFootnoteBackpatcher =
            new XMLPropertyBackpatcher<sal_Int16>(sSequenceNumber
//                                                ,sCurrentPresentation,
//                                                sal_False, 0
                );
    }
    return *pFootnoteBackpatcher;
}

XMLPropertyBackpatcher<sal_Int16>& XMLTextImportHelper::GetSequenceIdBP()
{
    if (NULL == pSequenceIdBackpatcher)
    {
        pSequenceIdBackpatcher =
            new XMLPropertyBackpatcher<sal_Int16>(sSequenceNumber
//                                                ,sCurrentPresentation,
//                                                sal_False, 0
                );
    }
    return *pSequenceIdBackpatcher;
}

XMLPropertyBackpatcher<OUString>& XMLTextImportHelper::GetSequenceNameBP()
{
    if (NULL == pSequenceNameBackpatcher)
    {
        pSequenceNameBackpatcher =
            new XMLPropertyBackpatcher<OUString>(sSourceName
//                                               ,sCurrentPresentation,
//                                               sal_False, sSequenceNumber
                );
                                            // hack: last parameter not used
    }
    return *pSequenceNameBackpatcher;
}

void XMLTextImportHelper::InsertFootnoteID(
    const OUString& sXMLId,
    sal_Int16 nAPIId)
{
    GetFootnoteBP().ResolveId(sXMLId, nAPIId);
}

void XMLTextImportHelper::ProcessFootnoteReference(
    const OUString& sXMLId,
    const Reference<XPropertySet> & xPropSet)
{
    GetFootnoteBP().SetProperty(xPropSet, sXMLId);
}

void XMLTextImportHelper::InsertSequenceID(
    const OUString& sXMLId,
    const OUString& sName,
    sal_Int16 nAPIId)
{
    GetSequenceIdBP().ResolveId(sXMLId, nAPIId);
    GetSequenceNameBP().ResolveId(sXMLId, sName);
}

void XMLTextImportHelper::ProcessSequenceReference(
    const OUString& sXMLId,
    const Reference<XPropertySet> & xPropSet)
{
    GetSequenceIdBP().SetProperty(xPropSet, sXMLId);
    GetSequenceNameBP().SetProperty(xPropSet, sXMLId);
}


void XMLTextImportHelper::ConnectFrameChains(
        const OUString& rFrmName,
        const OUString& rNextFrmName,
        const Reference < XPropertySet >& rFrmPropSet )
{
    if( !rFrmName.getLength() )
        return;

    if( rNextFrmName.getLength() )
    {
        OUString sNextFrmName = GetRenameMap().Get( XML_TEXT_RENAME_TYPE_FRAME,
                                                    rNextFrmName );
        if( xTextFrames.is() && xTextFrames->hasByName( sNextFrmName ) )
        {
            Any aAny;
            aAny <<= sNextFrmName;
            rFrmPropSet->setPropertyValue( sChainNextName, aAny );
        }
        else
        {
            if( !pPrevFrmNames )
            {
                pPrevFrmNames = new SvStringsDtor;
                pNextFrmNames = new SvStringsDtor;
            }
            pPrevFrmNames->Insert( new String( rFrmName ),
                                   pPrevFrmNames->Count() );
            pNextFrmNames->Insert( new String( sNextFrmName ),
                                   pNextFrmNames->Count() );
        }
    }
    if( pPrevFrmNames && pPrevFrmNames->Count() )
    {
        sal_uInt16 nCount = pPrevFrmNames->Count();
        for( sal_uInt16 i=0; i<nCount; i++ )
        {
            String *pNext = (*pNextFrmNames)[i];
            if( OUString(*pNext) == rFrmName )
            {
                // The previuous frame must exist, because it existing than
                // inserting the entry
                String *pPrev = (*pPrevFrmNames)[i];

                Any aAny;
                aAny <<= OUString( *pPrev );
                rFrmPropSet->setPropertyValue( sChainPrevName, aAny );

                pPrevFrmNames->Remove( i, 1 );
                pNextFrmNames->Remove( i, 1 );
                delete pPrev;
                delete pNext;

                // There cannot be more than one previous frames
                break;
            }
        }
    }
}

sal_Bool XMLTextImportHelper::IsInFrame()
{
    sal_Bool bIsInFrame = sal_False;

    // are we currently in a text frame? yes, if the cursor has a
    // TextFrame property and it's non-NULL
    Reference<XPropertySet> xPropSet(GetCursor(), UNO_QUERY);
    if (xPropSet.is())
    {
        if (xPropSet->getPropertySetInfo()->hasPropertyByName(sTextFrame))
        {
            Any aAny = xPropSet->getPropertyValue(sTextFrame);
            Reference<XTextFrame> xFrame;
            aAny >>= xFrame;

            if (xFrame.is())
            {
                bIsInFrame = sal_True;
            }
        }
    }

    return bIsInFrame;
}
