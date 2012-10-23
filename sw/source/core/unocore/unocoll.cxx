/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <swtypes.hxx>
#include <cmdid.h>
#include <hintids.hxx>
#include <svx/svxids.hrc>
#include <doc.hxx>
#include <docary.hxx>
#include <fmtcol.hxx>
#include <poolfmt.hxx>
#include <unocoll.hxx>
#include <unosett.hxx>
#include <fmtanchr.hxx>
#include <ndtxt.hxx>
#include <section.hxx>
#include <IMark.hxx>
#include <ftnidx.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/text/XTextTableCursor.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <svl/PasswordHelper.hxx>
#include <svtools/unoimap.hxx>
#include <svtools/unoevent.hxx>
#include <unotbl.hxx>
#include <unostyle.hxx>
#include <unofield.hxx>
#include <unoidx.hxx>
#include <unoframe.hxx>
#include <unofootnote.hxx>
#include <vcl/svapp.hxx>
#include <fmtcntnt.hxx>
#include <authfld.hxx>
#include <SwXTextDefaults.hxx>
#include <unochart.hxx>
#include <comphelper/makesequence.hxx>
#include <comphelper/sequence.hxx>
#include <list>
#include <iterator>
#include <unosection.hxx>
#include <unoparagraph.hxx>
#include <unobookmark.hxx>
#include <unorefmark.hxx>
#include <unometa.hxx>
#include "docsh.hxx"
#include <switerator.hxx>
#include <com/sun/star/document/XCodeNameQuery.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <vbahelper/vbaaccesshelper.hxx>
#include <basic/basmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceasvector.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

using rtl::OUString;

#ifndef DISABLE_SCRIPTING

class SwVbaCodeNameProvider : public ::cppu::WeakImplHelper1< document::XCodeNameQuery >
{
    SwDocShell* mpDocShell;
    rtl::OUString msThisDocumentCodeName;
public:
    SwVbaCodeNameProvider( SwDocShell* pDocShell ) : mpDocShell( pDocShell ) {}
        // XCodeNameQuery

    rtl::OUString SAL_CALL getCodeNameForContainer( const uno::Reference< uno::XInterface >& /*xIf*/ ) throw( uno::RuntimeException )
    {
        // not implemented...
        return rtl::OUString();
    }

    rtl::OUString SAL_CALL getCodeNameForObject( const uno::Reference< uno::XInterface >& xIf ) throw( uno::RuntimeException )
    {
        // Initialise the code name
        if ( msThisDocumentCodeName.isEmpty() )
        {
            try
            {
                uno::Reference< beans::XPropertySet > xProps( mpDocShell->GetModel(), uno::UNO_QUERY_THROW );
                uno::Reference< container::XNameAccess > xLibContainer( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicLibraries") ) ), uno::UNO_QUERY_THROW );
        rtl::OUString sProjectName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Standard") ) );
                if ( mpDocShell->GetBasicManager()->GetName().Len() )
                    sProjectName =  mpDocShell->GetBasicManager()->GetName();

                uno::Reference< container::XNameAccess > xLib( xLibContainer->getByName( sProjectName ), uno::UNO_QUERY_THROW );
                uno::Sequence< rtl::OUString > sModuleNames = xLib->getElementNames();
                uno::Reference< script::vba::XVBAModuleInfo > xVBAModuleInfo( xLib, uno::UNO_QUERY );

                for ( sal_Int32 i=0; i < sModuleNames.getLength(); ++i )
                {
                    script::ModuleInfo mInfo;

                    if ( xVBAModuleInfo->hasModuleInfo( sModuleNames[ i ] ) &&  xVBAModuleInfo->getModuleInfo( sModuleNames[ i ] ).ModuleType == script::ModuleType::DOCUMENT )
                    {
                        msThisDocumentCodeName = sModuleNames[ i ];
                        break;
                    }
                }
            }
            catch( uno::Exception& )
            {
            }
        }
        rtl::OUString sCodeName;
        if ( mpDocShell )
        {
            OSL_TRACE( "*** In ScVbaCodeNameProvider::getCodeNameForObject");
            // need to find the page ( and index )  for this control
            uno::Reference< drawing::XDrawPageSupplier > xSupplier( mpDocShell->GetModel(), uno::UNO_QUERY_THROW );
            uno::Reference< container::XIndexAccess > xIndex( xSupplier->getDrawPage(), uno::UNO_QUERY_THROW );

            bool bMatched = false;
            uno::Sequence< script::ScriptEventDescriptor > aFakeEvents;
                try
                {
                    uno::Reference< form::XFormsSupplier >  xFormSupplier( xIndex, uno::UNO_QUERY_THROW );
                    uno::Reference< container::XIndexAccess > xFormIndex( xFormSupplier->getForms(), uno::UNO_QUERY_THROW );
                    // get the www-standard container
                    uno::Reference< container::XIndexAccess > xFormControls( xFormIndex->getByIndex(0), uno::UNO_QUERY_THROW );
                    sal_Int32 nCntrls = xFormControls->getCount();
                    for( sal_Int32 cIndex = 0; cIndex < nCntrls; ++cIndex )
                    {
                        uno::Reference< uno::XInterface > xControl( xFormControls->getByIndex( cIndex ), uno::UNO_QUERY_THROW );
                        bMatched = ( xControl == xIf );
                        if ( bMatched )
                        {
                            sCodeName = msThisDocumentCodeName;
                            break;
                        }
                    }
                }
                catch( uno::Exception& ) {}
        }
        // Probably should throw here ( if !bMatched )
        return sCodeName;
    }
};

typedef boost::unordered_map< rtl::OUString, rtl::OUString, rtl::OUStringHash > StringHashMap;
class SwVbaProjectNameProvider : public ::cppu::WeakImplHelper1< container::XNameContainer >
{
    StringHashMap mTemplateToProject;
public:
    SwVbaProjectNameProvider()
    {
    }
    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException )
    {
        return ( mTemplateToProject.find( aName ) != mTemplateToProject.end() );
    }
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        return uno::makeAny( mTemplateToProject.find( aName )->second );
    }
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        uno::Sequence< rtl::OUString > aElements( mTemplateToProject.size() );
        StringHashMap::iterator it_end = mTemplateToProject.end();
        sal_Int32 index = 0;
        for ( StringHashMap::iterator it = mTemplateToProject.begin(); it != it_end; ++it, ++index )
            aElements[ index ] = it->first;
        return aElements;
    }

    virtual void SAL_CALL insertByName( const rtl::OUString& aName, const uno::Any& aElement ) throw ( com::sun::star::lang::IllegalArgumentException, com::sun::star::container::ElementExistException, com::sun::star::lang::WrappedTargetException )
    {

        rtl::OUString sProjectName;
        aElement >>= sProjectName;
        OSL_TRACE("** Template cache inserting template name %s with project %s"
            , rtl::OUStringToOString( aName, RTL_TEXTENCODING_UTF8 ).getStr()
            , rtl::OUStringToOString( sProjectName, RTL_TEXTENCODING_UTF8 ).getStr() );
        mTemplateToProject[ aName ] = sProjectName;
    }

    virtual void SAL_CALL removeByName( const rtl::OUString& Name ) throw ( com::sun::star::container::NoSuchElementException, com::sun::star::lang::WrappedTargetException )
    {
        if ( !hasByName( Name ) )
            throw container::NoSuchElementException();
        mTemplateToProject.erase( Name );
    }
    virtual void SAL_CALL replaceByName( const rtl::OUString& aName, const uno::Any& aElement ) throw ( com::sun::star::lang::IllegalArgumentException, com::sun::star::container::NoSuchElementException, com::sun::star::lang::WrappedTargetException )
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        insertByName( aName, aElement ); // insert will overwrite
    }
    // XElemenAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return ::getCppuType((const rtl::OUString*)0);
    }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException )
    {

        return ( mTemplateToProject.size() > 0 );
    }

};

class SwVbaObjectForCodeNameProvider : public ::cppu::WeakImplHelper1< container::XNameAccess >
{
    SwDocShell* mpDocShell;
public:
    SwVbaObjectForCodeNameProvider( SwDocShell* pDocShell ) : mpDocShell( pDocShell )
    {
        // #FIXME #TODO is the code name for ThisDocument read anywhere?
    }

    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException )
    {
        // #FIXME #TODO we really need to be checking against the codename for
        // ThisDocument
        if ( aName == "ThisDocument" )
            return sal_True;
        return sal_False;
    }

    ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
    {
        if ( !hasByName( aName ) )
             throw container::NoSuchElementException();
    uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[0] = uno::Any( uno::Reference< uno::XInterface >() );
        aArgs[1] = uno::Any( mpDocShell->GetModel() );
        uno::Reference< uno::XInterface > xDocObj = ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "ooo.vba.word.Document" , aArgs );
        OSL_TRACE("Creating Object ( ooo.vba.word.Document ) 0x%x", xDocObj.get() );
        return  uno::makeAny( xDocObj );
    }
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        uno::Sequence< rtl::OUString > aNames;
        return aNames;
    }
    // XElemenAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException){ return uno::Type(); }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException ) { return sal_True; }

};

#endif

struct  ProvNamesId_Type
{
    const char *    pName;
    sal_uInt16      nType;
};

// note: this thing is indexed as an array, so do not insert/remove entries!
const ProvNamesId_Type aProvNamesId[] =
{
    { "com.sun.star.text.TextTable",                          SW_SERVICE_TYPE_TEXTTABLE },
    { "com.sun.star.text.TextFrame",                          SW_SERVICE_TYPE_TEXTFRAME },
    { "com.sun.star.text.GraphicObject",                      SW_SERVICE_TYPE_GRAPHIC },
    { "com.sun.star.text.TextEmbeddedObject",                 SW_SERVICE_TYPE_OLE },
    { "com.sun.star.text.Bookmark",                           SW_SERVICE_TYPE_BOOKMARK },
    { "com.sun.star.text.Footnote",                           SW_SERVICE_TYPE_FOOTNOTE },
    { "com.sun.star.text.Endnote",                            SW_SERVICE_TYPE_ENDNOTE },
    { "com.sun.star.text.DocumentIndexMark",                  SW_SERVICE_TYPE_INDEXMARK },
    { "com.sun.star.text.DocumentIndex",                      SW_SERVICE_TYPE_INDEX },
    { "com.sun.star.text.ReferenceMark",                      SW_SERVICE_REFERENCE_MARK },
    { "com.sun.star.style.CharacterStyle",                    SW_SERVICE_STYLE_CHARACTER_STYLE },
    { "com.sun.star.style.ParagraphStyle",                    SW_SERVICE_STYLE_PARAGRAPH_STYLE },
    { "com.sun.star.style.FrameStyle",                        SW_SERVICE_STYLE_FRAME_STYLE },
    { "com.sun.star.style.PageStyle",                         SW_SERVICE_STYLE_PAGE_STYLE },
    { "com.sun.star.style.NumberingStyle",                    SW_SERVICE_STYLE_NUMBERING_STYLE },
    { "com.sun.star.text.ContentIndexMark",                   SW_SERVICE_CONTENT_INDEX_MARK },
    { "com.sun.star.text.ContentIndex",                       SW_SERVICE_CONTENT_INDEX },
    { "com.sun.star.text.UserIndexMark",                      SW_SERVICE_USER_INDEX_MARK },
    { "com.sun.star.text.UserIndex",                          SW_SERVICE_USER_INDEX },
    { "com.sun.star.text.TextSection",                        SW_SERVICE_TEXT_SECTION },
    { "com.sun.star.text.TextField.DateTime",                 SW_SERVICE_FIELDTYPE_DATETIME },
    { "com.sun.star.text.TextField.User",                     SW_SERVICE_FIELDTYPE_USER },
    { "com.sun.star.text.TextField.SetExpression",            SW_SERVICE_FIELDTYPE_SET_EXP },
    { "com.sun.star.text.TextField.GetExpression",            SW_SERVICE_FIELDTYPE_GET_EXP },
    { "com.sun.star.text.TextField.FileName",                 SW_SERVICE_FIELDTYPE_FILE_NAME },
    { "com.sun.star.text.TextField.PageNumber",               SW_SERVICE_FIELDTYPE_PAGE_NUM },
    { "com.sun.star.text.TextField.Author",                   SW_SERVICE_FIELDTYPE_AUTHOR },
    { "com.sun.star.text.TextField.Chapter",                  SW_SERVICE_FIELDTYPE_CHAPTER },
    { "",                                                     SW_SERVICE_FIELDTYPE_DUMMY_0 },
    { "com.sun.star.text.TextField.GetReference",             SW_SERVICE_FIELDTYPE_GET_REFERENCE },
    { "com.sun.star.text.TextField.ConditionalText",          SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT },
    { "com.sun.star.text.TextField.Annotation",               SW_SERVICE_FIELDTYPE_ANNOTATION },
    { "com.sun.star.text.TextField.Input",                    SW_SERVICE_FIELDTYPE_INPUT },
    { "com.sun.star.text.TextField.Macro",                    SW_SERVICE_FIELDTYPE_MACRO },
    { "com.sun.star.text.TextField.DDE",                      SW_SERVICE_FIELDTYPE_DDE },
    { "com.sun.star.text.TextField.HiddenParagraph",          SW_SERVICE_FIELDTYPE_HIDDEN_PARA },
    { "" /*com.sun.star.text.TextField.DocumentInfo"*/,       SW_SERVICE_FIELDTYPE_DOC_INFO },
    { "com.sun.star.text.TextField.TemplateName",             SW_SERVICE_FIELDTYPE_TEMPLATE_NAME },
    { "com.sun.star.text.TextField.ExtendedUser",             SW_SERVICE_FIELDTYPE_USER_EXT },
    { "com.sun.star.text.TextField.ReferencePageSet",         SW_SERVICE_FIELDTYPE_REF_PAGE_SET },
    { "com.sun.star.text.TextField.ReferencePageGet",         SW_SERVICE_FIELDTYPE_REF_PAGE_GET },
    { "com.sun.star.text.TextField.JumpEdit",                 SW_SERVICE_FIELDTYPE_JUMP_EDIT },
    { "com.sun.star.text.TextField.Script",                   SW_SERVICE_FIELDTYPE_SCRIPT },
    { "com.sun.star.text.TextField.DatabaseNextSet",          SW_SERVICE_FIELDTYPE_DATABASE_NEXT_SET },
    { "com.sun.star.text.TextField.DatabaseNumberOfSet",      SW_SERVICE_FIELDTYPE_DATABASE_NUM_SET },
    { "com.sun.star.text.TextField.DatabaseSetNumber",        SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM },
    { "com.sun.star.text.TextField.Database",                 SW_SERVICE_FIELDTYPE_DATABASE },
    { "com.sun.star.text.TextField.DatabaseName",             SW_SERVICE_FIELDTYPE_DATABASE_NAME },
    { "com.sun.star.text.TextField.TableFormula",             SW_SERVICE_FIELDTYPE_TABLE_FORMULA },
    { "com.sun.star.text.TextField.PageCount",                SW_SERVICE_FIELDTYPE_PAGE_COUNT },
    { "com.sun.star.text.TextField.ParagraphCount",           SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT },
    { "com.sun.star.text.TextField.WordCount",                SW_SERVICE_FIELDTYPE_WORD_COUNT },
    { "com.sun.star.text.TextField.CharacterCount",           SW_SERVICE_FIELDTYPE_CHARACTER_COUNT },
    { "com.sun.star.text.TextField.TableCount",               SW_SERVICE_FIELDTYPE_TABLE_COUNT },
    { "com.sun.star.text.TextField.GraphicObjectCount",       SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT },
    { "com.sun.star.text.TextField.EmbeddedObjectCount",      SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT },
    { "com.sun.star.text.TextField.DocInfo.ChangeAuthor",     SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR },
    { "com.sun.star.text.TextField.DocInfo.ChangeDateTime",   SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME },
    { "com.sun.star.text.TextField.DocInfo.EditTime",         SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME },
    { "com.sun.star.text.TextField.DocInfo.Description",      SW_SERVICE_FIELDTYPE_DOCINFO_DESCRIPTION },
    { "com.sun.star.text.TextField.DocInfo.CreateAuthor",     SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_AUTHOR },
    { "com.sun.star.text.TextField.DocInfo.CreateDateTime",   SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME },
    { "",                                                     SW_SERVICE_FIELDTYPE_DUMMY_0 },
    { "",                                                     SW_SERVICE_FIELDTYPE_DUMMY_1 },
    { "",                                                     SW_SERVICE_FIELDTYPE_DUMMY_2 },
    { "",                                                     SW_SERVICE_FIELDTYPE_DUMMY_3 },
    { "com.sun.star.text.TextField.DocInfo.Custom",           SW_SERVICE_FIELDTYPE_DOCINFO_CUSTOM },
    { "com.sun.star.text.TextField.DocInfo.PrintAuthor",      SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR },
    { "com.sun.star.text.TextField.DocInfo.PrintDateTime",    SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME },
    { "com.sun.star.text.TextField.DocInfo.KeyWords",         SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS },
    { "com.sun.star.text.TextField.DocInfo.Subject",          SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT },
    { "com.sun.star.text.TextField.DocInfo.Title",            SW_SERVICE_FIELDTYPE_DOCINFO_TITLE },
    { "com.sun.star.text.TextField.DocInfo.Revision",         SW_SERVICE_FIELDTYPE_DOCINFO_REVISION },
    { "com.sun.star.text.TextField.Bibliography",             SW_SERVICE_FIELDTYPE_BIBLIOGRAPHY },
    { "com.sun.star.text.TextField.CombinedCharacters",       SW_SERVICE_FIELDTYPE_COMBINED_CHARACTERS },
    { "com.sun.star.text.TextField.DropDown",                 SW_SERVICE_FIELDTYPE_DROPDOWN },
    { "com.sun.star.text.textfield.MetadataField",            SW_SERVICE_FIELDTYPE_METAFIELD },
    { "",                                                     SW_SERVICE_FIELDTYPE_DUMMY_4 },
    { "",                                                     SW_SERVICE_FIELDTYPE_DUMMY_5 },
    { "",                                                     SW_SERVICE_FIELDTYPE_DUMMY_6 },
    { "",                                                     SW_SERVICE_FIELDTYPE_DUMMY_7 },
    { "com.sun.star.text.FieldMaster.User",                   SW_SERVICE_FIELDMASTER_USER },
    { "com.sun.star.text.FieldMaster.DDE",                    SW_SERVICE_FIELDMASTER_DDE },
    { "com.sun.star.text.FieldMaster.SetExpression",          SW_SERVICE_FIELDMASTER_SET_EXP },
    { "com.sun.star.text.FieldMaster.Database",               SW_SERVICE_FIELDMASTER_DATABASE },
    { "com.sun.star.text.FieldMaster.Bibliography",           SW_SERVICE_FIELDMASTER_BIBLIOGRAPHY },
    { "",                                                     SW_SERVICE_FIELDMASTER_DUMMY2 },
    { "",                                                     SW_SERVICE_FIELDMASTER_DUMMY3 },
    { "",                                                     SW_SERVICE_FIELDMASTER_DUMMY4 },
    { "",                                                     SW_SERVICE_FIELDMASTER_DUMMY5 },
    { "com.sun.star.text.IllustrationsIndex",                 SW_SERVICE_INDEX_ILLUSTRATIONS },
    { "com.sun.star.text.ObjectIndex",                        SW_SERVICE_INDEX_OBJECTS },
    { "com.sun.star.text.TableIndex",                         SW_SERVICE_INDEX_TABLES },
    { "com.sun.star.text.Bibliography",                       SW_SERVICE_INDEX_BIBLIOGRAPHY },
    { "com.sun.star.text.Paragraph",                          SW_SERVICE_PARAGRAPH },
    { "com.sun.star.text.TextField.InputUser",                SW_SERVICE_FIELDTYPE_INPUT_USER },
    { "com.sun.star.text.TextField.HiddenText",               SW_SERVICE_FIELDTYPE_HIDDEN_TEXT },
    { "com.sun.star.style.ConditionalParagraphStyle",         SW_SERVICE_STYLE_CONDITIONAL_PARAGRAPH_STYLE },
    { "com.sun.star.text.NumberingRules",                     SW_SERVICE_NUMBERING_RULES },
    { "com.sun.star.text.TextColumns",                        SW_SERVICE_TEXT_COLUMNS },
    { "com.sun.star.text.IndexHeaderSection",                 SW_SERVICE_INDEX_HEADER_SECTION },
    { "com.sun.star.text.Defaults",                           SW_SERVICE_DEFAULTS },
    { "com.sun.star.image.ImageMapRectangleObject",           SW_SERVICE_IMAP_RECTANGLE },
    { "com.sun.star.image.ImageMapCircleObject",              SW_SERVICE_IMAP_CIRCLE },
    { "com.sun.star.image.ImageMapPolygonObject",             SW_SERVICE_IMAP_POLYGON },
    { "com.sun.star.text.TextGraphicObject",                  SW_SERVICE_TYPE_TEXT_GRAPHIC },
    { "com.sun.star.chart2.data.DataProvider",                SW_SERVICE_CHART2_DATA_PROVIDER },
    { "com.sun.star.text.Fieldmark",                          SW_SERVICE_TYPE_FIELDMARK },
    { "com.sun.star.text.FormFieldmark",                      SW_SERVICE_TYPE_FORMFIELDMARK },
    { "com.sun.star.text.InContentMetadata",                  SW_SERVICE_TYPE_META },
    { "ooo.vba.VBAObjectModuleObjectProvider",                SW_SERVICE_VBAOBJECTPROVIDER },
    { "ooo.vba.VBACodeNameProvider",                          SW_SERVICE_VBACODENAMEPROVIDER },
    { "ooo.vba.VBAProjectNameProvider",                       SW_SERVICE_VBAPROJECTNAMEPROVIDER },
    { "ooo.vba.VBAGlobals",                       SW_SERVICE_VBAGLOBALS },

    // case-correct versions of the service names (see #i67811)
    { CSS_TEXT_TEXTFIELD_DATE_TIME,                   SW_SERVICE_FIELDTYPE_DATETIME },
    { CSS_TEXT_TEXTFIELD_USER,                        SW_SERVICE_FIELDTYPE_USER },
    { CSS_TEXT_TEXTFIELD_SET_EXPRESSION,              SW_SERVICE_FIELDTYPE_SET_EXP },
    { CSS_TEXT_TEXTFIELD_GET_EXPRESSION,              SW_SERVICE_FIELDTYPE_GET_EXP },
    { CSS_TEXT_TEXTFIELD_FILE_NAME,                   SW_SERVICE_FIELDTYPE_FILE_NAME },
    { CSS_TEXT_TEXTFIELD_PAGE_NUMBER,                 SW_SERVICE_FIELDTYPE_PAGE_NUM },
    { CSS_TEXT_TEXTFIELD_AUTHOR,                      SW_SERVICE_FIELDTYPE_AUTHOR },
    { CSS_TEXT_TEXTFIELD_CHAPTER,                     SW_SERVICE_FIELDTYPE_CHAPTER },
    { CSS_TEXT_TEXTFIELD_GET_REFERENCE,               SW_SERVICE_FIELDTYPE_GET_REFERENCE },
    { CSS_TEXT_TEXTFIELD_CONDITIONAL_TEXT,            SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT },
    { CSS_TEXT_TEXTFIELD_ANNOTATION,                  SW_SERVICE_FIELDTYPE_ANNOTATION },
    { CSS_TEXT_TEXTFIELD_INPUT,                       SW_SERVICE_FIELDTYPE_INPUT },
    { CSS_TEXT_TEXTFIELD_MACRO,                       SW_SERVICE_FIELDTYPE_MACRO },
    { CSS_TEXT_TEXTFIELD_DDE,                         SW_SERVICE_FIELDTYPE_DDE },
    { CSS_TEXT_TEXTFIELD_HIDDEN_PARAGRAPH,            SW_SERVICE_FIELDTYPE_HIDDEN_PARA },
    { CSS_TEXT_TEXTFIELD_TEMPLATE_NAME,               SW_SERVICE_FIELDTYPE_TEMPLATE_NAME },
    { CSS_TEXT_TEXTFIELD_EXTENDED_USER,               SW_SERVICE_FIELDTYPE_USER_EXT },
    { CSS_TEXT_TEXTFIELD_REFERENCE_PAGE_SET,          SW_SERVICE_FIELDTYPE_REF_PAGE_SET },
    { CSS_TEXT_TEXTFIELD_REFERENCE_PAGE_GET,          SW_SERVICE_FIELDTYPE_REF_PAGE_GET },
    { CSS_TEXT_TEXTFIELD_JUMP_EDIT,                   SW_SERVICE_FIELDTYPE_JUMP_EDIT },
    { CSS_TEXT_TEXTFIELD_SCRIPT,                      SW_SERVICE_FIELDTYPE_SCRIPT },
    { CSS_TEXT_TEXTFIELD_DATABASE_NEXT_SET,           SW_SERVICE_FIELDTYPE_DATABASE_NEXT_SET },
    { CSS_TEXT_TEXTFIELD_DATABASE_NUMBER_OF_SET,      SW_SERVICE_FIELDTYPE_DATABASE_NUM_SET },
    { CSS_TEXT_TEXTFIELD_DATABASE_SET_NUMBER,         SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM },
    { CSS_TEXT_TEXTFIELD_DATABASE,                    SW_SERVICE_FIELDTYPE_DATABASE },
    { CSS_TEXT_TEXTFIELD_DATABASE_NAME,               SW_SERVICE_FIELDTYPE_DATABASE_NAME },
    { CSS_TEXT_TEXTFIELD_TABLE_FORMULA,               SW_SERVICE_FIELDTYPE_TABLE_FORMULA },
    { CSS_TEXT_TEXTFIELD_PAGE_COUNT,                  SW_SERVICE_FIELDTYPE_PAGE_COUNT },
    { CSS_TEXT_TEXTFIELD_PARAGRAPH_COUNT,             SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT },
    { CSS_TEXT_TEXTFIELD_WORD_COUNT,                  SW_SERVICE_FIELDTYPE_WORD_COUNT },
    { CSS_TEXT_TEXTFIELD_CHARACTER_COUNT,             SW_SERVICE_FIELDTYPE_CHARACTER_COUNT },
    { CSS_TEXT_TEXTFIELD_TABLE_COUNT,                 SW_SERVICE_FIELDTYPE_TABLE_COUNT },
    { CSS_TEXT_TEXTFIELD_GRAPHIC_OBJECT_COUNT,        SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT },
    { CSS_TEXT_TEXTFIELD_EMBEDDED_OBJECT_COUNT,       SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT },
    { CSS_TEXT_TEXTFIELD_DOCINFO_CHANGE_AUTHOR,       SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR },
    { CSS_TEXT_TEXTFIELD_DOCINFO_CHANGE_DATE_TIME,    SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME },
    { CSS_TEXT_TEXTFIELD_DOCINFO_EDIT_TIME,           SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME },
    { CSS_TEXT_TEXTFIELD_DOCINFO_DESCRIPTION,         SW_SERVICE_FIELDTYPE_DOCINFO_DESCRIPTION },
    { CSS_TEXT_TEXTFIELD_DOCINFO_CREATE_AUTHOR,       SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_AUTHOR },
    { CSS_TEXT_TEXTFIELD_DOCINFO_CREATE_DATE_TIME,    SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME },
    { CSS_TEXT_TEXTFIELD_DOCINFO_PRINT_AUTHOR,        SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR },
    { CSS_TEXT_TEXTFIELD_DOCINFO_PRINT_DATE_TIME,     SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME },
    { CSS_TEXT_TEXTFIELD_DOCINFO_KEY_WORDS,           SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS },
    { CSS_TEXT_TEXTFIELD_DOCINFO_SUBJECT,             SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT },
    { CSS_TEXT_TEXTFIELD_DOCINFO_TITLE,               SW_SERVICE_FIELDTYPE_DOCINFO_TITLE },
    { CSS_TEXT_TEXTFIELD_DOCINFO_REVISION,            SW_SERVICE_FIELDTYPE_DOCINFO_REVISION },
    { CSS_TEXT_TEXTFIELD_DOCINFO_CUSTOM,              SW_SERVICE_FIELDTYPE_DOCINFO_CUSTOM },
    { CSS_TEXT_TEXTFIELD_BIBLIOGRAPHY,                SW_SERVICE_FIELDTYPE_BIBLIOGRAPHY },
    { CSS_TEXT_TEXTFIELD_COMBINED_CHARACTERS,         SW_SERVICE_FIELDTYPE_COMBINED_CHARACTERS },
    { CSS_TEXT_TEXTFIELD_DROP_DOWN,                   SW_SERVICE_FIELDTYPE_DROPDOWN },
    { CSS_TEXT_TEXTFIELD_INPUT_USER,                  SW_SERVICE_FIELDTYPE_INPUT_USER },
    { CSS_TEXT_TEXTFIELD_HIDDEN_TEXT,                 SW_SERVICE_FIELDTYPE_HIDDEN_TEXT },
    { CSS_TEXT_FIELDMASTER_USER,                      SW_SERVICE_FIELDMASTER_USER },
    { CSS_TEXT_FIELDMASTER_DDE,                       SW_SERVICE_FIELDMASTER_DDE },
    { CSS_TEXT_FIELDMASTER_SET_EXPRESSION,            SW_SERVICE_FIELDMASTER_SET_EXP },
    { CSS_TEXT_FIELDMASTER_DATABASE,                  SW_SERVICE_FIELDMASTER_DATABASE },
    { CSS_TEXT_FIELDMASTER_BIBLIOGRAPHY,              SW_SERVICE_FIELDMASTER_BIBLIOGRAPHY }
};

const SvEventDescription* sw_GetSupportedMacroItems()
{
    static const SvEventDescription aMacroDescriptionsImpl[] =
    {
        { SFX_EVENT_MOUSEOVER_OBJECT, "OnMouseOver" },
        { SFX_EVENT_MOUSEOUT_OBJECT, "OnMouseOut" },
        { 0, NULL }
    };

    return aMacroDescriptionsImpl;
}

/******************************************************************
 * SwXServiceProvider
 ******************************************************************/
OUString    SwXServiceProvider::GetProviderName(sal_uInt16 nObjectType)
{
    SolarMutexGuard aGuard;
    OUString sRet;
    sal_uInt16 nEntries = sizeof(aProvNamesId) / sizeof(aProvNamesId[0]);
    if(nObjectType < nEntries)
        sRet = rtl::OUString::createFromAscii(aProvNamesId[nObjectType].pName);
    return sRet;
}

uno::Sequence<OUString>     SwXServiceProvider::GetAllServiceNames()
{
    sal_uInt16 nEntries = sizeof(aProvNamesId) / sizeof(aProvNamesId[0]);
    uno::Sequence<OUString> aRet(nEntries);
    OUString* pArray = aRet.getArray();
    sal_uInt16 n = 0;
    for(sal_uInt16 i = 0; i < nEntries; i++)
    {
        String sProv(rtl::OUString::createFromAscii(aProvNamesId[i].pName));
        if(sProv.Len())
        {
            pArray[n] = sProv;
            n++;
        }
    }
    aRet.realloc(n);
    return aRet;

}

sal_uInt16  SwXServiceProvider::GetProviderType(const OUString& rServiceName)
{
    sal_uInt16 nEntries = sizeof(aProvNamesId) / sizeof(aProvNamesId[0]);
    for(sal_uInt16 i = 0; i < nEntries; i++ )
    {
        if( COMPARE_EQUAL == rServiceName.compareToAscii(aProvNamesId[i].pName))
            return aProvNamesId[i].nType;
    }
    return SW_SERVICE_INVALID;
}

uno::Reference< uno::XInterface >   SwXServiceProvider::MakeInstance(sal_uInt16 nObjectType, SwDoc* pDoc)
{
    SolarMutexGuard aGuard;
    uno::Reference< uno::XInterface >  xRet;
    switch(nObjectType)
    {
        case  SW_SERVICE_TYPE_TEXTTABLE:
        {
            SwXTextTable* pTextTable = new SwXTextTable();
            xRet =  (cppu::OWeakObject*)pTextTable;
        }
        break;
        case  SW_SERVICE_TYPE_TEXTFRAME:
        {
            SwXTextFrame* pTextFrame = new SwXTextFrame( pDoc );
            xRet =  (cppu::OWeakObject*)(SwXFrame*)pTextFrame;
        }
        break;
        case  SW_SERVICE_TYPE_GRAPHIC  :
        case  SW_SERVICE_TYPE_TEXT_GRAPHIC /* #i47503# */ :
        {
            SwXTextGraphicObject* pGraphic = new SwXTextGraphicObject( pDoc );
            xRet =  (cppu::OWeakObject*)(SwXFrame*)pGraphic;

        }
        break;
        case  SW_SERVICE_TYPE_OLE      :
        {
            SwXTextEmbeddedObject* pOle = new SwXTextEmbeddedObject( pDoc );
            xRet =  (cppu::OWeakObject*)(SwXFrame*)pOle;
        }
        break;
        case  SW_SERVICE_TYPE_BOOKMARK :
        {
            SwXBookmark* pBookmark = new SwXBookmark;
            xRet =  (cppu::OWeakObject*)pBookmark;
        }
        break;
        case  SW_SERVICE_TYPE_FIELDMARK :
        {
            SwXFieldmark* pFieldmark = new SwXFieldmark(false);
            xRet =  (cppu::OWeakObject*)pFieldmark;
        }
        break;
        case  SW_SERVICE_TYPE_FORMFIELDMARK :
        {
            SwXFieldmark* pFieldmark = new SwXFieldmark(true);
            xRet =  (cppu::OWeakObject*)pFieldmark;
        }
        break;
        case  SW_SERVICE_VBAOBJECTPROVIDER :
#ifndef DISABLE_SCRIPTING
        {
            SwVbaObjectForCodeNameProvider* pObjProv = new SwVbaObjectForCodeNameProvider( pDoc->GetDocShell() );
            xRet =  (cppu::OWeakObject*)pObjProv;
        }
#endif
        break;
        case  SW_SERVICE_VBACODENAMEPROVIDER :
#ifndef DISABLE_SCRIPTING
        {
            if ( pDoc->GetDocShell()  && ooo::vba::isAlienWordDoc( *pDoc->GetDocShell() ) )
            {
                SwVbaCodeNameProvider* pObjProv = new SwVbaCodeNameProvider( pDoc->GetDocShell() );
                xRet =  (cppu::OWeakObject*)pObjProv;
            }
        }
#endif
        break;
        case  SW_SERVICE_VBAPROJECTNAMEPROVIDER :
#ifndef DISABLE_SCRIPTING
        {
                        uno::Reference< container::XNameContainer > xProjProv = pDoc->GetVBATemplateToProjectCache();
                        if ( !xProjProv.is() && pDoc->GetDocShell()  && ooo::vba::isAlienWordDoc( *pDoc->GetDocShell() ) )
                        {
                xProjProv = new SwVbaProjectNameProvider;
                            pDoc->SetVBATemplateToProjectCache( xProjProv );
                        }
            //xRet =  (cppu::OWeakObject*)xProjProv;
            xRet = xProjProv;
        }
#endif
        break;
        case  SW_SERVICE_VBAGLOBALS :
#ifndef DISABLE_SCRIPTING
        {
            if ( pDoc )
            {
                uno::Any aGlobs;
                if ( !pDoc->GetDocShell()->GetBasicManager()->GetGlobalUNOConstant( "VBAGlobals", aGlobs ) )
                {
                    uno::Sequence< uno::Any > aArgs(1);
                    aArgs[ 0 ] <<= pDoc->GetDocShell()->GetModel();
                    aGlobs <<= ::comphelper::getProcessServiceFactory()->createInstanceWithArguments( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Globals")), aArgs );
                    pDoc->GetDocShell()->GetBasicManager()->SetGlobalUNOConstant( "VBAGlobals", aGlobs );
                }
                aGlobs >>= xRet;
            }
        }
#endif
        break;

        case  SW_SERVICE_TYPE_FOOTNOTE :
            xRet =  (cppu::OWeakObject*)new SwXFootnote(sal_False);
        break;
        case  SW_SERVICE_TYPE_ENDNOTE  :
            xRet =  (cppu::OWeakObject*)new SwXFootnote(sal_True);
        break;
        case  SW_SERVICE_CONTENT_INDEX_MARK :
        case  SW_SERVICE_USER_INDEX_MARK    :
        case  SW_SERVICE_TYPE_INDEXMARK:
        {
            TOXTypes eType = TOX_INDEX;
            if(SW_SERVICE_CONTENT_INDEX_MARK== nObjectType)
                eType = TOX_CONTENT;
            else if(SW_SERVICE_USER_INDEX_MARK == nObjectType)
                eType = TOX_USER;
            xRet =  (cppu::OWeakObject*)new SwXDocumentIndexMark(eType);
        }
        break;
        case  SW_SERVICE_CONTENT_INDEX      :
        case  SW_SERVICE_USER_INDEX         :
        case  SW_SERVICE_TYPE_INDEX    :
        case SW_SERVICE_INDEX_ILLUSTRATIONS:
        case SW_SERVICE_INDEX_OBJECTS      :
        case SW_SERVICE_INDEX_TABLES:
        case SW_SERVICE_INDEX_BIBLIOGRAPHY :
        {
            TOXTypes eType = TOX_INDEX;
            if(SW_SERVICE_CONTENT_INDEX == nObjectType)
                eType = TOX_CONTENT;
            else if(SW_SERVICE_USER_INDEX == nObjectType)
                eType = TOX_USER;
            else if(SW_SERVICE_INDEX_ILLUSTRATIONS == nObjectType)
            {
                eType = TOX_ILLUSTRATIONS;
            }
            else if(SW_SERVICE_INDEX_OBJECTS       == nObjectType)
            {
                eType = TOX_OBJECTS;
            }
            else if(SW_SERVICE_INDEX_BIBLIOGRAPHY  == nObjectType)
            {
                eType = TOX_AUTHORITIES;
            }
            else if(SW_SERVICE_INDEX_TABLES == nObjectType)
            {
                eType = TOX_TABLES;
            }
            xRet =  (cppu::OWeakObject*)new SwXDocumentIndex(eType, *pDoc);
        }
        break;
        case SW_SERVICE_INDEX_HEADER_SECTION :
        case SW_SERVICE_TEXT_SECTION :
            xRet = SwXTextSection::CreateXTextSection(0,
                    (SW_SERVICE_INDEX_HEADER_SECTION == nObjectType));

        break;
        case SW_SERVICE_REFERENCE_MARK :
            xRet =  (cppu::OWeakObject*)new SwXReferenceMark(0, 0);
        break;
        case SW_SERVICE_STYLE_CHARACTER_STYLE:
        case SW_SERVICE_STYLE_PARAGRAPH_STYLE:
        case SW_SERVICE_STYLE_CONDITIONAL_PARAGRAPH_STYLE:
        case SW_SERVICE_STYLE_FRAME_STYLE:
        case SW_SERVICE_STYLE_PAGE_STYLE:
        case SW_SERVICE_STYLE_NUMBERING_STYLE:
        {
            SfxStyleFamily  eFamily = SFX_STYLE_FAMILY_CHAR;
            switch(nObjectType)
            {
                case SW_SERVICE_STYLE_PARAGRAPH_STYLE:
                case SW_SERVICE_STYLE_CONDITIONAL_PARAGRAPH_STYLE:
                    eFamily = SFX_STYLE_FAMILY_PARA;
                break;
                case SW_SERVICE_STYLE_FRAME_STYLE:
                    eFamily = SFX_STYLE_FAMILY_FRAME;
                break;
                case SW_SERVICE_STYLE_PAGE_STYLE:
                    eFamily = SFX_STYLE_FAMILY_PAGE;
                break;
                case SW_SERVICE_STYLE_NUMBERING_STYLE:
                    eFamily = SFX_STYLE_FAMILY_PSEUDO;
                break;
            }
            SwXStyle* pNewStyle = SFX_STYLE_FAMILY_PAGE == eFamily ?
                new SwXPageStyle(pDoc->GetDocShell()) :
                    eFamily == SFX_STYLE_FAMILY_FRAME ?
                        new SwXFrameStyle ( pDoc ):
                            new SwXStyle( pDoc, eFamily, nObjectType == SW_SERVICE_STYLE_CONDITIONAL_PARAGRAPH_STYLE);
            xRet = (cppu::OWeakObject*)pNewStyle;
        }
        break;
//      SW_SERVICE_DUMMY_5
//      SW_SERVICE_DUMMY_6
//      SW_SERVICE_DUMMY_7
//      SW_SERVICE_DUMMY_8
//      SW_SERVICE_DUMMY_9
        case SW_SERVICE_FIELDTYPE_DATETIME:
        case SW_SERVICE_FIELDTYPE_USER:
        case SW_SERVICE_FIELDTYPE_SET_EXP:
        case SW_SERVICE_FIELDTYPE_GET_EXP:
        case SW_SERVICE_FIELDTYPE_FILE_NAME:
        case SW_SERVICE_FIELDTYPE_PAGE_NUM:
        case SW_SERVICE_FIELDTYPE_AUTHOR:
        case SW_SERVICE_FIELDTYPE_CHAPTER:
        case SW_SERVICE_FIELDTYPE_GET_REFERENCE:
        case SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT:
        case SW_SERVICE_FIELDTYPE_INPUT:
        case SW_SERVICE_FIELDTYPE_MACRO:
        case SW_SERVICE_FIELDTYPE_DDE:
        case SW_SERVICE_FIELDTYPE_HIDDEN_PARA:
        case SW_SERVICE_FIELDTYPE_DOC_INFO:
        case SW_SERVICE_FIELDTYPE_TEMPLATE_NAME:
        case SW_SERVICE_FIELDTYPE_USER_EXT:
        case SW_SERVICE_FIELDTYPE_REF_PAGE_SET:
        case SW_SERVICE_FIELDTYPE_REF_PAGE_GET:
        case SW_SERVICE_FIELDTYPE_JUMP_EDIT:
        case SW_SERVICE_FIELDTYPE_SCRIPT:
        case SW_SERVICE_FIELDTYPE_DATABASE_NEXT_SET:
        case SW_SERVICE_FIELDTYPE_DATABASE_NUM_SET:
        case SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM:
        case SW_SERVICE_FIELDTYPE_DATABASE:
        case SW_SERVICE_FIELDTYPE_DATABASE_NAME:
        case SW_SERVICE_FIELDTYPE_PAGE_COUNT      :
        case SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT :
        case SW_SERVICE_FIELDTYPE_WORD_COUNT      :
        case SW_SERVICE_FIELDTYPE_CHARACTER_COUNT :
        case SW_SERVICE_FIELDTYPE_TABLE_COUNT     :
        case SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT    :
        case SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT   :
        case SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR     :
        case SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME  :
        case SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME         :
        case SW_SERVICE_FIELDTYPE_DOCINFO_DESCRIPTION       :
        case SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_AUTHOR     :
        case SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME  :
        case SW_SERVICE_FIELDTYPE_DOCINFO_CUSTOM            :
        case SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR      :
        case SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME   :
        case SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS         :
        case SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT           :
        case SW_SERVICE_FIELDTYPE_DOCINFO_TITLE             :
        case SW_SERVICE_FIELDTYPE_DOCINFO_REVISION          :
        case SW_SERVICE_FIELDTYPE_BIBLIOGRAPHY:
        case SW_SERVICE_FIELDTYPE_INPUT_USER                :
        case SW_SERVICE_FIELDTYPE_HIDDEN_TEXT               :
        case SW_SERVICE_FIELDTYPE_COMBINED_CHARACTERS       :
        case SW_SERVICE_FIELDTYPE_DROPDOWN                  :
        case SW_SERVICE_FIELDTYPE_TABLE_FORMULA:
            xRet = (cppu::OWeakObject*)new SwXTextField(nObjectType);
            break;
        case SW_SERVICE_FIELDTYPE_ANNOTATION:
            xRet = (cppu::OWeakObject*)new SwXTextField(nObjectType, pDoc);
            break;
        case SW_SERVICE_FIELDMASTER_USER:
        case SW_SERVICE_FIELDMASTER_DDE:
        case SW_SERVICE_FIELDMASTER_SET_EXP :
        case SW_SERVICE_FIELDMASTER_DATABASE:
        {
            sal_uInt16 nResId = USHRT_MAX;
            switch(nObjectType)
            {
                case SW_SERVICE_FIELDMASTER_USER: nResId = RES_USERFLD; break;
                case SW_SERVICE_FIELDMASTER_DDE:  nResId = RES_DDEFLD; break;
                case SW_SERVICE_FIELDMASTER_SET_EXP : nResId = RES_SETEXPFLD; break;
                case SW_SERVICE_FIELDMASTER_DATABASE: nResId = RES_DBFLD; break;
            }
            xRet =  (cppu::OWeakObject*)new SwXFieldMaster(pDoc, nResId);
        }
        break;
        case SW_SERVICE_FIELDMASTER_BIBLIOGRAPHY:
        {
            SwFieldType* pType = pDoc->GetFldType(RES_AUTHORITY, aEmptyStr, sal_True);
            if(!pType)
            {
                SwAuthorityFieldType aType(pDoc);
                pType = pDoc->InsertFldType(aType);
            }
            else
            {
                SwXFieldMaster* pMaster = SwIterator<SwXFieldMaster,SwFieldType>::FirstElement( *pType );
                if(pMaster)
                    xRet = (cppu::OWeakObject*)pMaster;
            }
            if(!xRet.is())
                xRet =  (cppu::OWeakObject*)new SwXFieldMaster(*pType, pDoc);
        }
        break;
        case SW_SERVICE_PARAGRAPH :
            xRet = (cppu::OWeakObject*)new SwXParagraph();
        break;
        case SW_SERVICE_NUMBERING_RULES :
            xRet = (cppu::OWeakObject*)new SwXNumberingRules(*pDoc);
        break;
        case SW_SERVICE_TEXT_COLUMNS :
            xRet = (cppu::OWeakObject*)new SwXTextColumns(0);
        break;
        case SW_SERVICE_DEFAULTS:
            xRet = (cppu::OWeakObject*)new SwXTextDefaults( pDoc );
        break;
        case SW_SERVICE_IMAP_RECTANGLE :
            xRet = SvUnoImageMapRectangleObject_createInstance( sw_GetSupportedMacroItems() );
        break;
        case SW_SERVICE_IMAP_CIRCLE    :
            xRet = SvUnoImageMapCircleObject_createInstance( sw_GetSupportedMacroItems() );
        break;
        case SW_SERVICE_IMAP_POLYGON   :
            xRet = SvUnoImageMapPolygonObject_createInstance( sw_GetSupportedMacroItems() );
        break;
        case SW_SERVICE_CHART2_DATA_PROVIDER :
            // #i64497# If a chart is in a temporary document during clipoard
            // paste, there should be no data provider, so that own data is used
            // This should not happen during copy/paste, as this will unlink
            // charts using table data.
            OSL_ASSERT( pDoc->GetDocShell()->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED );
            if( pDoc->GetDocShell()->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
                xRet = (cppu::OWeakObject*) pDoc->GetChartDataProvider( true /* create - if not yet available */ );
        break;
        case SW_SERVICE_TYPE_META:
            xRet = static_cast< ::cppu::OWeakObject* >( new SwXMeta(pDoc) );
        break;
        case SW_SERVICE_FIELDTYPE_METAFIELD:
            xRet = static_cast< ::cppu::OWeakObject* >(new SwXMetaField(pDoc));
        break;
        default:
            throw uno::RuntimeException();
    }
    return xRet;
}

/******************************************************************
 * SwXTextTables
 ******************************************************************/
//SMART_UNO_IMPLEMENTATION( SwXTextTables, UsrObject );
SwXTextTables::SwXTextTables(SwDoc* pDc) :
        SwUnoCollection(pDc)
{

}

SwXTextTables::~SwXTextTables()
{

}

sal_Int32 SwXTextTables::getCount(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int32 nRet = 0;
    if(IsValid())
        nRet = GetDoc()->GetTblFrmFmtCount(true);
    return nRet;
}

uno::Any SAL_CALL SwXTextTables::getByIndex(sal_Int32 nIndex)
        throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(IsValid())
    {
        if(0 <= nIndex && GetDoc()->GetTblFrmFmtCount(true) > nIndex)
        {
            SwFrmFmt& rFmt = GetDoc()->GetTblFrmFmt( static_cast< xub_StrLen >(nIndex), true);
            uno::Reference< XTextTable >  xTbl = SwXTextTables::GetObject(rFmt);
            aRet.setValue( &xTbl,
                ::getCppuType((uno::Reference< XTextTable>*)0));
        }
        else
            throw IndexOutOfBoundsException();
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Any SwXTextTables::getByName(const OUString& rItemName)
    throw( NoSuchElementException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(IsValid())
    {
        sal_uInt16 nCount = GetDoc()->GetTblFrmFmtCount(true);
        uno::Reference< XTextTable >  xTbl;
        for( sal_uInt16 i = 0; i < nCount; i++)
        {
            String aName(rItemName);
            SwFrmFmt& rFmt = GetDoc()->GetTblFrmFmt(i, true);
            if(aName == rFmt.GetName())
            {
                xTbl = SwXTextTables::GetObject(rFmt);
                aRet.setValue(&xTbl,
                    ::getCppuType(( uno::Reference< XTextTable >*)0));
                break;
            }
        }
        if(!xTbl.is())
            throw NoSuchElementException();
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Sequence< OUString > SwXTextTables::getElementNames(void)
        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    sal_uInt16 nCount = GetDoc()->GetTblFrmFmtCount(true);
    uno::Sequence<OUString> aSeq(nCount);
    if(nCount)
    {
        OUString* pArray = aSeq.getArray();
        for( sal_uInt16 i = 0; i < nCount; i++)
        {
            SwFrmFmt& rFmt = GetDoc()->GetTblFrmFmt(i, true);

            pArray[i] = OUString(rFmt.GetName());
        }
    }
    return aSeq;
}

sal_Bool SwXTextTables::hasByName(const OUString& rName)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet= sal_False;
    if(IsValid())
    {
        sal_uInt16 nCount = GetDoc()->GetTblFrmFmtCount(true);
        for( sal_uInt16 i = 0; i < nCount; i++)
        {
            String aName(rName);
            SwFrmFmt& rFmt = GetDoc()->GetTblFrmFmt(i, true);
            if(aName == rFmt.GetName())
            {
                bRet = sal_True;
                break;
            }
        }
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

uno::Type SAL_CALL
    SwXTextTables::getElementType(  )
        throw(uno::RuntimeException)
{
    return ::getCppuType((uno::Reference<XTextTable>*)0);
}

sal_Bool SwXTextTables::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return 0 != GetDoc()->GetTblFrmFmtCount(true);
}

OUString SwXTextTables::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTextTables");
}

sal_Bool SwXTextTables::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return rServiceName == C2U("com.sun.star.text.TextTables");
}

uno::Sequence< OUString > SwXTextTables::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArr = aRet.getArray();
    pArr[0] = C2U("com.sun.star.text.TextTables");
    return aRet;
}

XTextTable* SwXTextTables::GetObject( SwFrmFmt& rFmt )
{
    SolarMutexGuard aGuard;
    SwXTextTable* pTbl = SwIterator<SwXTextTable,SwFmt>::FirstElement( rFmt );
    if( !pTbl )
        pTbl = new SwXTextTable(rFmt);
    return pTbl ;
}

/******************************************************************
 *  SwXFrameEnumeration
 ******************************************************************/
namespace
{
    template<FlyCntType T> struct UnoFrameWrap_traits {};

    template<>
    struct UnoFrameWrap_traits<FLYCNTTYPE_FRM>
    {
        typedef SwXTextFrame core_frame_t;
        typedef XTextFrame uno_frame_t;
        static inline bool filter(const SwNode* const pNode) { return !pNode->IsNoTxtNode(); };
    };

    template<>
    struct UnoFrameWrap_traits<FLYCNTTYPE_GRF>
    {
        typedef SwXTextGraphicObject core_frame_t;
        typedef XTextContent uno_frame_t;
        static inline bool filter(const SwNode* const pNode) { return pNode->IsGrfNode(); };
    };

    template<>
    struct UnoFrameWrap_traits<FLYCNTTYPE_OLE>
    {
        typedef SwXTextEmbeddedObject core_frame_t;
        typedef XEmbeddedObjectSupplier uno_frame_t;
        static inline bool filter(const SwNode* const pNode) { return pNode->IsOLENode(); };
    };

    template<FlyCntType T>
    static uno::Any lcl_UnoWrapFrame(SwFrmFmt* pFmt)
    {
        SwXFrame* pFrm = SwIterator<SwXFrame,SwFmt>::FirstElement( *pFmt );
        if(!pFrm)
            pFrm = new typename UnoFrameWrap_traits<T>::core_frame_t(*pFmt);
        Reference< typename UnoFrameWrap_traits<T>::uno_frame_t > xFrm =
            static_cast< typename UnoFrameWrap_traits<T>::core_frame_t* >(pFrm);
        return uno::makeAny(xFrm);
    }

    // runtime adapter for lcl_UnoWrapFrame
    static uno::Any lcl_UnoWrapFrame(SwFrmFmt* pFmt, FlyCntType eType) throw(uno::RuntimeException())
    {
        switch(eType)
        {
            case FLYCNTTYPE_FRM:
                return lcl_UnoWrapFrame<FLYCNTTYPE_FRM>(pFmt);
            case FLYCNTTYPE_GRF:
                return lcl_UnoWrapFrame<FLYCNTTYPE_GRF>(pFmt);
            case FLYCNTTYPE_OLE:
                return lcl_UnoWrapFrame<FLYCNTTYPE_OLE>(pFmt);
            default:
                throw uno::RuntimeException();
        }
    }

    template<FlyCntType T>
    class SwXFrameEnumeration
        : public SwSimpleEnumeration_Base
    {
        private:
            typedef ::std::list< Any > frmcontainer_t;
            frmcontainer_t m_aFrames;
        protected:
            virtual ~SwXFrameEnumeration() {};
        public:
            SwXFrameEnumeration(const SwDoc* const pDoc);

            //XEnumeration
            virtual sal_Bool SAL_CALL hasMoreElements(void) throw( RuntimeException );
            virtual Any SAL_CALL nextElement(void) throw( NoSuchElementException, WrappedTargetException, RuntimeException );

            //XServiceInfo
            virtual OUString SAL_CALL getImplementationName(void) throw( RuntimeException );
            virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( RuntimeException );
            virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( RuntimeException );
    };
}

template<FlyCntType T>
SwXFrameEnumeration<T>::SwXFrameEnumeration(const SwDoc* const pDoc)
    : m_aFrames()
{
    SolarMutexGuard aGuard;
    const SwFrmFmts* const pFmts = pDoc->GetSpzFrmFmts();
    if(pFmts->empty())
        return;
    // #i104937#
//    const SwFrmFmt* const pFmtsEnd = (*pFmts)[pFmts->Count()];
    const sal_uInt16 nSize = pFmts->size();
    ::std::insert_iterator<frmcontainer_t> pInserter = ::std::insert_iterator<frmcontainer_t>(m_aFrames, m_aFrames.begin());
    // #i104937#
    SwFrmFmt* pFmt( 0 );
    for( sal_uInt16 i = 0; i < nSize; ++i )
//    for(SwFrmFmt* pFmt = (*pFmts)[0]; pFmt < pFmtsEnd; ++pFmt)
    {
        // #i104937#
        pFmt = (*pFmts)[i];
        if(pFmt->Which() != RES_FLYFRMFMT)
            continue;
        const SwNodeIndex* pIdx =  pFmt->GetCntnt().GetCntntIdx();
        if(!pIdx || !pIdx->GetNodes().IsDocNodes())
            continue;
        const SwNode* pNd = pDoc->GetNodes()[ pIdx->GetIndex() + 1 ];
        if(UnoFrameWrap_traits<T>::filter(pNd))
            *pInserter++ = lcl_UnoWrapFrame<T>(pFmt);
    }
}

template<FlyCntType T>
sal_Bool SwXFrameEnumeration<T>::hasMoreElements(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    return !m_aFrames.empty();
}

template<FlyCntType T>
Any SwXFrameEnumeration<T>::nextElement(void) throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    if(m_aFrames.empty())
        throw NoSuchElementException();
    Any aResult = *m_aFrames.begin();
    m_aFrames.pop_front();
    return aResult;
}

template<FlyCntType T>
OUString SwXFrameEnumeration<T>::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXFrameEnumeration");
}

template<FlyCntType T>
sal_Bool SwXFrameEnumeration<T>::supportsService(const OUString& ServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.container.XEnumeration") == ServiceName;
}

template<FlyCntType T>
Sequence< OUString > SwXFrameEnumeration<T>::getSupportedServiceNames(void) throw( RuntimeException )
{
    return ::comphelper::makeSequence(C2U("com.sun.star.container.XEnumeration"));
}

/******************************************************************
 *  SwXFrames
 ******************************************************************/
OUString SwXFrames::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXFrames");
}

sal_Bool SwXFrames::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextFrames") == rServiceName;
}

Sequence<OUString> SwXFrames::getSupportedServiceNames(void) throw( RuntimeException )
{
    return ::comphelper::makeSequence(C2U("com.sun.star.text.TextFrames"));
}

SwXFrames::SwXFrames(SwDoc* _pDoc, FlyCntType eSet) :
    SwUnoCollection(_pDoc),
    eType(eSet)
{}

SwXFrames::~SwXFrames()
{}

uno::Reference<container::XEnumeration> SwXFrames::createEnumeration(void) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    switch(eType)
    {
        case FLYCNTTYPE_FRM:
            return uno::Reference< container::XEnumeration >(
                new SwXFrameEnumeration<FLYCNTTYPE_FRM>(GetDoc()));
        case FLYCNTTYPE_GRF:
            return uno::Reference< container::XEnumeration >(
                new SwXFrameEnumeration<FLYCNTTYPE_GRF>(GetDoc()));
        case FLYCNTTYPE_OLE:
            return uno::Reference< container::XEnumeration >(
                new SwXFrameEnumeration<FLYCNTTYPE_OLE>(GetDoc()));
        default:
            throw uno::RuntimeException();
    }
}

sal_Int32 SwXFrames::getCount(void) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return GetDoc()->GetFlyCount(eType);
}

uno::Any SwXFrames::getByIndex(sal_Int32 nIndex)
    throw(IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    if(nIndex < 0 || nIndex >= USHRT_MAX)
        throw IndexOutOfBoundsException();
    SwFrmFmt* pFmt = GetDoc()->GetFlyNum(static_cast<sal_uInt16>(nIndex), eType);
    if(!pFmt)
        throw IndexOutOfBoundsException();
    return lcl_UnoWrapFrame(pFmt, eType);
}

uno::Any SwXFrames::getByName(const OUString& rName)
    throw(NoSuchElementException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    const SwFrmFmt* pFmt;
    switch(eType)
    {
        case FLYCNTTYPE_GRF:
            pFmt = GetDoc()->FindFlyByName(rName, ND_GRFNODE);
            break;
        case FLYCNTTYPE_OLE:
            pFmt = GetDoc()->FindFlyByName(rName, ND_OLENODE);
            break;
        default:
            pFmt = GetDoc()->FindFlyByName(rName, ND_TEXTNODE);
            break;
    }
    if(!pFmt)
        throw NoSuchElementException();
    return lcl_UnoWrapFrame(const_cast<SwFrmFmt*>(pFmt), eType);
}

uno::Sequence<OUString> SwXFrames::getElementNames(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    const Reference<XEnumeration> xEnum = createEnumeration();
    ::std::vector<OUString> vNames;
    while(xEnum->hasMoreElements())
    {
        Reference<container::XNamed> xNamed;
        xEnum->nextElement() >>= xNamed;
        if(xNamed.is())
            vNames.push_back(xNamed->getName());
    }
    return ::comphelper::containerToSequence(vNames);
}

sal_Bool SwXFrames::hasByName(const OUString& rName) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    switch(eType)
    {
        case FLYCNTTYPE_GRF:
            return GetDoc()->FindFlyByName(rName, ND_GRFNODE) != NULL;
        case FLYCNTTYPE_OLE:
            return GetDoc()->FindFlyByName(rName, ND_OLENODE) != NULL;
        default:
            return GetDoc()->FindFlyByName(rName, ND_TEXTNODE) != NULL;
    }
}

uno::Type SAL_CALL SwXFrames::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    switch(eType)
    {
        case FLYCNTTYPE_FRM:
            return ::getCppuType((uno::Reference<XTextFrame>*)0);
        case FLYCNTTYPE_GRF:
            return ::getCppuType((uno::Reference<XTextContent>*)0);
        case FLYCNTTYPE_OLE:
            return ::getCppuType((uno::Reference<XEmbeddedObjectSupplier>*)0);
        default:
            return uno::Type();
    }
}

sal_Bool SwXFrames::hasElements(void) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return GetDoc()->GetFlyCount(eType) > 0;
}

SwXFrame* SwXFrames::GetObject(SwFrmFmt& rFmt, FlyCntType eType)
{
    SwXFrame* pFrm = SwIterator<SwXFrame,SwFmt>::FirstElement( rFmt );
    if(pFrm) return pFrm;
    switch(eType)
    {
        case FLYCNTTYPE_FRM:
            return new SwXTextFrame(rFmt);
        case FLYCNTTYPE_GRF:
            return new SwXTextGraphicObject(rFmt);
        case FLYCNTTYPE_OLE:
            return new SwXTextEmbeddedObject(rFmt);
        default:
            return NULL;
    }
}

/******************************************************************
 * SwXTextFrames
 ******************************************************************/
OUString SwXTextFrames::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextFrames");
}

sal_Bool SwXTextFrames::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextFrames") == rServiceName;
}

Sequence< OUString > SwXTextFrames::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextFrames");
    return aRet;
}

SwXTextFrames::SwXTextFrames(SwDoc* _pDoc) :
    SwXFrames(_pDoc, FLYCNTTYPE_FRM)
{
}

SwXTextFrames::~SwXTextFrames()
{
}

/******************************************************************
 *  SwXTextGraphicObjects
 ******************************************************************/
OUString SwXTextGraphicObjects::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextGraphicObjects");
}

sal_Bool SwXTextGraphicObjects::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextGraphicObjects") == rServiceName;
}

Sequence< OUString > SwXTextGraphicObjects::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextGraphicObjects");
    return aRet;
}

SwXTextGraphicObjects::SwXTextGraphicObjects(SwDoc* _pDoc) :
    SwXFrames(_pDoc, FLYCNTTYPE_GRF)
{
}

SwXTextGraphicObjects::~SwXTextGraphicObjects()
{
}

/******************************************************************
 *  SwXTextEmbeddedObjects
 ******************************************************************/
OUString SwXTextEmbeddedObjects::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextEmbeddedObjects");
}

sal_Bool SwXTextEmbeddedObjects::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextEmbeddedObjects") == rServiceName;
}

Sequence< OUString > SwXTextEmbeddedObjects::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextEmbeddedObjects");
    return aRet;
}

SwXTextEmbeddedObjects::SwXTextEmbeddedObjects(SwDoc* _pDoc) :
        SwXFrames(_pDoc, FLYCNTTYPE_OLE)
{
}

SwXTextEmbeddedObjects::~SwXTextEmbeddedObjects()
{
}

OUString SwXTextSections::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextSections");
}

sal_Bool SwXTextSections::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextSections") == rServiceName;
}

Sequence< OUString > SwXTextSections::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextSections");
    return aRet;
}

SwXTextSections::SwXTextSections(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{
}

SwXTextSections::~SwXTextSections()
{
}

sal_Int32 SwXTextSections::getCount(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    const SwSectionFmts& rSectFmts = GetDoc()->GetSections();
    sal_uInt16 nCount = rSectFmts.size();
    for(sal_uInt16 i = nCount; i; i--)
    {
        if( !rSectFmts[i - 1]->IsInNodesArr())
            nCount--;
    }
    return nCount;
}

uno::Any SwXTextSections::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextSection >  xRet;
    if(IsValid())
    {
        SwSectionFmts& rFmts = GetDoc()->GetSections();

        const SwSectionFmts& rSectFmts = GetDoc()->GetSections();
        sal_uInt16 nCount = rSectFmts.size();
        for(sal_uInt16 i = 0; i < nCount; i++)
        {
            if( !rSectFmts[i]->IsInNodesArr())
                nIndex ++;
            else if(nIndex == i)
                break;
            if(nIndex == i)
                break;
        }
        if(nIndex >= 0 && nIndex < (sal_Int32)rFmts.size())
        {
            SwSectionFmt* pFmt = rFmts[(sal_uInt16)nIndex];
            xRet = GetObject(*pFmt);
        }
        else
            throw IndexOutOfBoundsException();
    }
    else
        throw uno::RuntimeException();
    return makeAny(xRet);
}

uno::Any SwXTextSections::getByName(const OUString& Name)
    throw( NoSuchElementException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(IsValid())
    {
        String aName(Name);
        SwSectionFmts& rFmts = GetDoc()->GetSections();
        uno::Reference< XTextSection >  xSect;
        for(sal_uInt16 i = 0; i < rFmts.size(); i++)
        {
            SwSectionFmt* pFmt = rFmts[i];
            if (pFmt->IsInNodesArr()
                && (aName == pFmt->GetSection()->GetSectionName()))
            {
                xSect = GetObject(*pFmt);
                aRet.setValue(&xSect, ::getCppuType((uno::Reference<XTextSection>*)0));
                break;
            }
        }
        if(!xSect.is())
            throw NoSuchElementException();
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Sequence< OUString > SwXTextSections::getElementNames(void)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    sal_uInt16 nCount = GetDoc()->GetSections().size();
    SwSectionFmts& rSectFmts = GetDoc()->GetSections();
    for(sal_uInt16 i = nCount; i; i--)
    {
        if( !rSectFmts[i - 1]->IsInNodesArr())
            nCount--;
    }

    uno::Sequence<OUString> aSeq(nCount);
    if(nCount)
    {
        SwSectionFmts& rFmts = GetDoc()->GetSections();
        OUString* pArray = aSeq.getArray();
        sal_uInt16 nIndex = 0;
        for( sal_uInt16 i = 0; i < nCount; i++, nIndex++)
        {
            const SwSectionFmt* pFmt = rFmts[nIndex];
            while(!pFmt->IsInNodesArr())
            {
                pFmt = rFmts[++nIndex];
            }
            pArray[i] = pFmt->GetSection()->GetSectionName();
        }
    }
    return aSeq;
}

sal_Bool SwXTextSections::hasByName(const OUString& Name)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    String aName(Name);
    if(IsValid())
    {
        SwSectionFmts& rFmts = GetDoc()->GetSections();
        for(sal_uInt16 i = 0; i < rFmts.size(); i++)
        {
            const SwSectionFmt* pFmt = rFmts[i];
            if (aName == pFmt->GetSection()->GetSectionName())
            {
                bRet = sal_True;
                break;
            }
        }
    }
    else
    {
        //Sonderbehandlung der dbg_ - Methoden
        if( COMPARE_EQUAL != aName.CompareToAscii("dbg_", 4))
            throw uno::RuntimeException();
    }
    return bRet;
}

uno::Type SAL_CALL SwXTextSections::getElementType() throw(uno::RuntimeException)
{
    return ::getCppuType((uno::Reference<XTextSection>*)0);
}

sal_Bool SwXTextSections::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_uInt16 nCount = 0;
    if(IsValid())
    {
        SwSectionFmts& rFmts = GetDoc()->GetSections();
        nCount = rFmts.size();
    }
    else
        throw uno::RuntimeException();
    return nCount > 0;
}

uno::Reference< XTextSection >  SwXTextSections::GetObject( SwSectionFmt& rFmt )
{
    return SwXTextSection::CreateXTextSection(&rFmt);
}

OUString SwXBookmarks::getImplementationName(void) throw( RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("SwXBookmarks"));
}

sal_Bool SwXBookmarks::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Bookmarks")) == rServiceName;
}

Sequence< OUString > SwXBookmarks::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Bookmarks"));
    return aRet;
}

SwXBookmarks::SwXBookmarks(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{ }

SwXBookmarks::~SwXBookmarks()
{ }

sal_Int32 SwXBookmarks::getCount(void)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();

    sal_Int32 count(0);
    IDocumentMarkAccess* const pMarkAccess = GetDoc()->getIDocumentMarkAccess();
    for (IDocumentMarkAccess::const_iterator_t ppMark =
            pMarkAccess->getBookmarksBegin();
         ppMark != pMarkAccess->getBookmarksEnd(); ++ppMark)
    {
        if (IDocumentMarkAccess::BOOKMARK ==
                IDocumentMarkAccess::GetType(**ppMark))
        {
            ++count; // only count real bookmarks
        }
    }
    return count;
}

uno::Any SwXBookmarks::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    IDocumentMarkAccess* const pMarkAccess = GetDoc()->getIDocumentMarkAccess();
    if(nIndex < 0 || nIndex >= pMarkAccess->getBookmarksCount())
        throw IndexOutOfBoundsException();

    sal_Int32 count(0);
    for (IDocumentMarkAccess::const_iterator_t ppMark =
            pMarkAccess->getBookmarksBegin();
         ppMark != pMarkAccess->getBookmarksEnd(); ++ppMark)
    {
        if (IDocumentMarkAccess::BOOKMARK ==
                IDocumentMarkAccess::GetType(**ppMark))
        {
            if (count == nIndex)
            {
                uno::Any aRet;
                const uno::Reference< text::XTextContent > xRef =
                    SwXBookmark::CreateXBookmark(*GetDoc(), **ppMark);
                aRet <<= xRef;
                return aRet;
            }
            ++count; // only count real bookmarks
        }
    }
    throw IndexOutOfBoundsException();
}

uno::Any SwXBookmarks::getByName(const rtl::OUString& rName)
    throw( NoSuchElementException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();

    IDocumentMarkAccess* const pMarkAccess = GetDoc()->getIDocumentMarkAccess();
    IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findBookmark(rName);
    if(ppBkmk == pMarkAccess->getBookmarksEnd())
        throw NoSuchElementException();

    uno::Any aRet;
    const uno::Reference< text::XTextContent > xRef =
        SwXBookmark::CreateXBookmark(*GetDoc(), *(ppBkmk->get()));
    aRet <<= xRef;
    return aRet;
}

uno::Sequence< OUString > SwXBookmarks::getElementNames(void)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();

    ::comphelper::SequenceAsVector< ::rtl::OUString > ret;
    IDocumentMarkAccess* const pMarkAccess = GetDoc()->getIDocumentMarkAccess();
    for (IDocumentMarkAccess::const_iterator_t ppMark =
            pMarkAccess->getBookmarksBegin();
         ppMark != pMarkAccess->getBookmarksEnd(); ++ppMark)
    {
        if (IDocumentMarkAccess::BOOKMARK ==
                IDocumentMarkAccess::GetType(**ppMark))
        {
            ret.push_back((*ppMark)->GetName()); // only add real bookmarks
        }
    }
    return ret.getAsConstList();
}

sal_Bool SwXBookmarks::hasByName(const OUString& rName)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();

    IDocumentMarkAccess* const pMarkAccess = GetDoc()->getIDocumentMarkAccess();
    return pMarkAccess->findBookmark(rName) != pMarkAccess->getBookmarksEnd();
}

uno::Type SAL_CALL SwXBookmarks::getElementType()
    throw(uno::RuntimeException)
{
    return ::getCppuType((uno::Reference<XTextContent>*)0);
}

sal_Bool SwXBookmarks::hasElements(void)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();

    IDocumentMarkAccess* const pMarkAccess = GetDoc()->getIDocumentMarkAccess();
    for (IDocumentMarkAccess::const_iterator_t ppMark =
            pMarkAccess->getBookmarksBegin();
         ppMark != pMarkAccess->getBookmarksEnd(); ++ppMark)
    {
        if (IDocumentMarkAccess::BOOKMARK ==
                IDocumentMarkAccess::GetType(**ppMark))
        {
            return true;
        }
    }
    return false;
}

SwXNumberingRulesCollection::SwXNumberingRulesCollection( SwDoc* _pDoc ) :
    SwUnoCollection(_pDoc)
{
}

SwXNumberingRulesCollection::~SwXNumberingRulesCollection()
{
}

sal_Int32 SwXNumberingRulesCollection::getCount(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return GetDoc()->GetNumRuleTbl().size();
}

uno::Any SwXNumberingRulesCollection::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(IsValid())
    {
        uno::Reference< XIndexReplace >  xRef;
        if ( nIndex < (sal_Int32)GetDoc()->GetNumRuleTbl().size() )
        {
            xRef = new SwXNumberingRules( *GetDoc()->GetNumRuleTbl()[ static_cast< sal_uInt16 >(nIndex) ], GetDoc());
            aRet.setValue(&xRef, ::getCppuType((uno::Reference<XIndexReplace>*)0));
        }

        if(!xRef.is())
            throw IndexOutOfBoundsException();
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Type SAL_CALL SwXNumberingRulesCollection::getElementType() throw(uno::RuntimeException)
{
    return ::getCppuType((uno::Reference<XIndexReplace>*)0);
}

sal_Bool SwXNumberingRulesCollection::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return !GetDoc()->GetNumRuleTbl().empty();
}

OUString SwXFootnotes::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXFootnotes");
}

sal_Bool SwXFootnotes::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.Footnotes") == rServiceName;
}

Sequence< OUString > SwXFootnotes::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.Footnotes");
    return aRet;
}

SwXFootnotes::SwXFootnotes(sal_Bool bEnd, SwDoc* _pDoc)
    : SwUnoCollection(_pDoc)
    , m_bEndnote(bEnd)
{
}

SwXFootnotes::~SwXFootnotes()
{
}

sal_Int32 SwXFootnotes::getCount(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    sal_Int32 nCount = 0;
    sal_uInt16 n, nFtnCnt = GetDoc()->GetFtnIdxs().size();
    SwTxtFtn* pTxtFtn;
    for( n = 0; n < nFtnCnt; ++n )
    {
        pTxtFtn = GetDoc()->GetFtnIdxs()[ n ];
        const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
        if ( rFtn.IsEndNote() != m_bEndnote )
            continue;
        nCount++;
    }
    return nCount;
}

uno::Any SwXFootnotes::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    sal_Int32 nCount = 0;
    if(IsValid())
    {
        sal_uInt16 n, nFtnCnt = GetDoc()->GetFtnIdxs().size();
        SwTxtFtn* pTxtFtn;
        uno::Reference< XFootnote >  xRef;
        for( n = 0; n < nFtnCnt; ++n )
        {
            pTxtFtn = GetDoc()->GetFtnIdxs()[ n ];
            const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
            if ( rFtn.IsEndNote() != m_bEndnote )
                continue;

            if(nCount == nIndex)
            {
                xRef = SwXFootnote::CreateXFootnote(*GetDoc(), rFtn);
                aRet <<= xRef;
                break;
            }
            nCount++;
        }
        if(!xRef.is())
            throw IndexOutOfBoundsException();
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Type SAL_CALL SwXFootnotes::getElementType() throw(uno::RuntimeException)
{
    return ::getCppuType((uno::Reference<XFootnote>*)0);
}

sal_Bool SwXFootnotes::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return !GetDoc()->GetFtnIdxs().empty();
}

Reference<XFootnote>    SwXFootnotes::GetObject( SwDoc& rDoc, const SwFmtFtn& rFmt )
{
    return SwXFootnote::CreateXFootnote(rDoc, rFmt);
}

OUString SwXReferenceMarks::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXReferenceMarks");
}

sal_Bool SwXReferenceMarks::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.ReferenceMarks") == rServiceName;
}

Sequence< OUString > SwXReferenceMarks::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.ReferenceMarks");
    return aRet;
}

SwXReferenceMarks::SwXReferenceMarks(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{
}

SwXReferenceMarks::~SwXReferenceMarks()
{
}

sal_Int32 SwXReferenceMarks::getCount(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return GetDoc()->GetRefMarks();
}

uno::Any SwXReferenceMarks::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(!IsValid())
        throw uno::RuntimeException();
    uno::Reference< XTextContent >  xRef;
    if(0 <= nIndex && nIndex < USHRT_MAX)
    {
        const SwFmtRefMark* pMark = GetDoc()->GetRefMark( (sal_uInt16) nIndex );
        if(pMark)
        {
            xRef = SwXReferenceMarks::GetObject( GetDoc(), pMark );
            aRet.setValue(&xRef, ::getCppuType((uno::Reference<XTextContent>*)0));
        }
    }
    if(!xRef.is())
        throw IndexOutOfBoundsException();
    return aRet;
}

uno::Any SwXReferenceMarks::getByName(const OUString& rName)
    throw( NoSuchElementException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(IsValid())
    {
        const SwFmtRefMark* pMark = GetDoc()->GetRefMark(rName);
        if(pMark)
        {
            uno::Reference< XTextContent >  xRef = SwXReferenceMarks::GetObject( GetDoc(), pMark );
            aRet.setValue(&xRef, ::getCppuType((uno::Reference<XTextContent>*)0));
        }
        else
            throw NoSuchElementException();
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Sequence< OUString > SwXReferenceMarks::getElementNames(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aRet;
    if(IsValid())
    {
        std::vector<rtl::OUString> aStrings;
        sal_uInt16 nCount = GetDoc()->GetRefMarks( &aStrings );
        aRet.realloc(nCount);
        OUString* pNames = aRet.getArray();
        for(sal_uInt16 i = 0; i < nCount; i++)
            pNames[i] = aStrings[i];
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

sal_Bool SwXReferenceMarks::hasByName(const OUString& rName) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return 0 != GetDoc()->GetRefMark( rName);
}

uno::Type SAL_CALL SwXReferenceMarks::getElementType() throw(uno::RuntimeException)
{
    return ::getCppuType((uno::Reference<XTextContent>*)0);
}

sal_Bool SwXReferenceMarks::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return 0 != GetDoc()->GetRefMarks();
}

SwXReferenceMark* SwXReferenceMarks::GetObject( SwDoc* pDoc, const SwFmtRefMark* pMark )
{
    SolarMutexGuard aGuard;

    return SwXReferenceMark::CreateXReferenceMark(*pDoc, *pMark);
}


void SwUnoCollection::Invalidate()
{
    bObjectValid = sal_False;
    pDoc = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
