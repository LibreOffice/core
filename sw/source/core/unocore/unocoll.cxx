/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_features.h>

#include <swtypes.hxx>
#include <cmdid.h>
#include <hintids.hxx>
#include <svx/svxids.hrc>
#include <doc.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
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
#include <textboxhelper.hxx>
#include <unofootnote.hxx>
#include <vcl/svapp.hxx>
#include <fmtcntnt.hxx>
#include <authfld.hxx>
#include <SwXTextDefaults.hxx>
#include <unochart.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <list>
#include <iterator>
#include <unosection.hxx>
#include <unoparagraph.hxx>
#include <unobookmark.hxx>
#include <unorefmark.hxx>
#include <unometa.hxx>
#include "docsh.hxx"
#include <calbck.hxx>
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
#include <cppuhelper/implbase.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

#if HAVE_FEATURE_SCRIPTING

class SwVbaCodeNameProvider : public ::cppu::WeakImplHelper< document::XCodeNameQuery >
{
    SwDocShell* mpDocShell;
    OUString msThisDocumentCodeName;
public:
    explicit SwVbaCodeNameProvider( SwDocShell* pDocShell ) : mpDocShell( pDocShell ) {}
        // XCodeNameQuery

    OUString SAL_CALL getCodeNameForContainer( const uno::Reference< uno::XInterface >& /*xIf*/ ) throw( uno::RuntimeException, std::exception ) SAL_OVERRIDE
    {
        // #FIXME not implemented...
        return OUString();
    }

    OUString SAL_CALL getCodeNameForObject( const uno::Reference< uno::XInterface >& xIf ) throw( uno::RuntimeException, std::exception ) SAL_OVERRIDE
    {
        // Initialise the code name
        if ( msThisDocumentCodeName.isEmpty() )
        {
            try
            {
                uno::Reference< beans::XPropertySet > xProps( mpDocShell->GetModel(), uno::UNO_QUERY_THROW );
                uno::Reference< container::XNameAccess > xLibContainer( xProps->getPropertyValue("BasicLibraries"), uno::UNO_QUERY_THROW );
                OUString sProjectName( "Standard");
                if ( !mpDocShell->GetBasicManager()->GetName().isEmpty() )
                {
                    sProjectName =  mpDocShell->GetBasicManager()->GetName();
                }
                uno::Reference< container::XNameAccess > xLib( xLibContainer->getByName( sProjectName ), uno::UNO_QUERY_THROW );
                uno::Sequence< OUString > sModuleNames = xLib->getElementNames();
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
        OUString sCodeName;
        if ( mpDocShell )
        {
            // need to find the page ( and index )  for this control
            uno::Reference< drawing::XDrawPageSupplier > xSupplier( mpDocShell->GetModel(), uno::UNO_QUERY_THROW );
            uno::Reference< container::XIndexAccess > xIndex( xSupplier->getDrawPage(), uno::UNO_QUERY_THROW );

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
                    bool bMatched = ( xControl == xIf );
                    if ( bMatched )
                    {
                        sCodeName = msThisDocumentCodeName;
                        break;
                    }
                }
            }
            catch( uno::Exception& )
            {
            }
        }
        // #TODO Probably should throw here ( if !bMatched )
        return sCodeName;
    }
};

typedef std::unordered_map< OUString, OUString, OUStringHash > StringHashMap;

class SwVbaProjectNameProvider : public ::cppu::WeakImplHelper< container::XNameContainer >
{
    StringHashMap mTemplateToProject;
public:
    SwVbaProjectNameProvider()
    {
    }
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
    {
        return ( mTemplateToProject.find( aName ) != mTemplateToProject.end() );
    }
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        return uno::makeAny( mTemplateToProject.find( aName )->second );
    }
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        uno::Sequence< OUString > aElements( mTemplateToProject.size() );
        StringHashMap::iterator it_end = mTemplateToProject.end();
        sal_Int32 index = 0;
        for ( StringHashMap::iterator it = mTemplateToProject.begin(); it != it_end; ++it, ++index )
            aElements[ index ] = it->first;
        return aElements;
    }

    virtual void SAL_CALL insertByName( const OUString& aName, const uno::Any& aElement ) throw ( com::sun::star::lang::IllegalArgumentException, com::sun::star::container::ElementExistException, com::sun::star::lang::WrappedTargetException, std::exception ) SAL_OVERRIDE
    {

        OUString sProjectName;
        aElement >>= sProjectName;
        SAL_INFO("sw.uno", "Template cache inserting template name " << aName
                << " with project " << sProjectName);
        mTemplateToProject[ aName ] = sProjectName;
    }

    virtual void SAL_CALL removeByName( const OUString& Name ) throw ( com::sun::star::container::NoSuchElementException, com::sun::star::lang::WrappedTargetException, std::exception ) SAL_OVERRIDE
    {
        if ( !hasByName( Name ) )
            throw container::NoSuchElementException();
        mTemplateToProject.erase( Name );
    }
    virtual void SAL_CALL replaceByName( const OUString& aName, const uno::Any& aElement ) throw ( com::sun::star::lang::IllegalArgumentException, com::sun::star::container::NoSuchElementException, com::sun::star::lang::WrappedTargetException, std::exception ) SAL_OVERRIDE
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        insertByName( aName, aElement ); // insert will overwrite
    }
    // XElemenAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        return ::cppu::UnoType<OUString>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
    {

        return ( !mTemplateToProject.empty() );
    }

};

class SwVbaObjectForCodeNameProvider : public ::cppu::WeakImplHelper< container::XNameAccess >
{
    SwDocShell* mpDocShell;
public:
    explicit SwVbaObjectForCodeNameProvider( SwDocShell* pDocShell ) : mpDocShell( pDocShell )
    {
        // #FIXME #TODO is the code name for ThisDocument read anywhere?
    }

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
    {
        // #FIXME #TODO we really need to be checking against the codename for
        // ThisDocument
        if ( aName == "ThisDocument" )
            return sal_True;
        return sal_False;
    }

    ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        if ( !hasByName( aName ) )
             throw container::NoSuchElementException();
        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[0] = uno::Any( uno::Reference< uno::XInterface >() );
        aArgs[1] = uno::Any( mpDocShell->GetModel() );
        uno::Reference< uno::XInterface > xDocObj = ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "ooo.vba.word.Document" , aArgs );
        SAL_INFO("sw.uno",
            "Creating Object ( ooo.vba.word.Document ) 0x" << xDocObj.get());
        return  uno::makeAny( xDocObj );
    }
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        uno::Sequence< OUString > aNames;
        return aNames;
    }
    // XElemenAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return uno::Type(); }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE { return sal_True; }

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

OUString    SwXServiceProvider::GetProviderName(sal_uInt16 nObjectType)
{
    SolarMutexGuard aGuard;
    OUString sRet;
    const sal_uInt16 nEntries = sizeof(aProvNamesId) / sizeof(aProvNamesId[0]);
    if(nObjectType < nEntries)
        sRet = OUString::createFromAscii(aProvNamesId[nObjectType].pName);
    return sRet;
}

uno::Sequence<OUString>     SwXServiceProvider::GetAllServiceNames()
{
    const sal_uInt16 nEntries = sizeof(aProvNamesId) / sizeof(aProvNamesId[0]);
    uno::Sequence<OUString> aRet(nEntries);
    OUString* pArray = aRet.getArray();
    sal_uInt16 n = 0;
    for(sal_uInt16 i = 0; i < nEntries; i++)
    {
        OUString sProv(OUString::createFromAscii(aProvNamesId[i].pName));
        if(!sProv.isEmpty())
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
    const sal_uInt16 nEntries = sizeof(aProvNamesId) / sizeof(aProvNamesId[0]);
    for(sal_uInt16 i = 0; i < nEntries; i++ )
    {
        if (rServiceName.equalsAscii(aProvNamesId[i].pName))
            return aProvNamesId[i].nType;
    }
    return SW_SERVICE_INVALID;
}

uno::Reference<uno::XInterface>
SwXServiceProvider::MakeInstance(sal_uInt16 nObjectType, SwDoc & rDoc)
{
    SolarMutexGuard aGuard;
    uno::Reference< uno::XInterface >  xRet;
    switch(nObjectType)
    {
        case  SW_SERVICE_TYPE_TEXTTABLE:
        {
            xRet = SwXTextTable::CreateXTextTable(0);
        }
        break;
        case  SW_SERVICE_TYPE_TEXTFRAME:
        {
            xRet = SwXTextFrame::CreateXTextFrame(rDoc, 0);
        }
        break;
        case  SW_SERVICE_TYPE_GRAPHIC  :
        case  SW_SERVICE_TYPE_TEXT_GRAPHIC /* #i47503# */ :
        {
            xRet = SwXTextGraphicObject::CreateXTextGraphicObject(rDoc, 0);

        }
        break;
        case  SW_SERVICE_TYPE_OLE      :
        {
            xRet = SwXTextEmbeddedObject::CreateXTextEmbeddedObject(rDoc, 0);
        }
        break;
        case  SW_SERVICE_TYPE_BOOKMARK :
        {
            xRet = SwXBookmark::CreateXBookmark(rDoc, 0);
        }
        break;
        case  SW_SERVICE_TYPE_FIELDMARK :
        {
            xRet = SwXFieldmark::CreateXFieldmark(rDoc, 0);
        }
        break;
        case  SW_SERVICE_TYPE_FORMFIELDMARK :
        {
            xRet = SwXFieldmark::CreateXFieldmark(rDoc, 0, true);
        }
        break;
        case  SW_SERVICE_VBAOBJECTPROVIDER :
#if HAVE_FEATURE_SCRIPTING
        {
            SwVbaObjectForCodeNameProvider* pObjProv =
                new SwVbaObjectForCodeNameProvider(rDoc.GetDocShell());
            xRet =  static_cast<cppu::OWeakObject*>(pObjProv);
        }
#endif
        break;
        case  SW_SERVICE_VBACODENAMEPROVIDER :
#if HAVE_FEATURE_SCRIPTING
        {
            if (rDoc.GetDocShell() && ooo::vba::isAlienWordDoc(*rDoc.GetDocShell()))
            {
                SwVbaCodeNameProvider* pObjProv = new SwVbaCodeNameProvider(rDoc.GetDocShell());
                xRet =  static_cast<cppu::OWeakObject*>(pObjProv);
            }
        }
#endif
        break;
        case  SW_SERVICE_VBAPROJECTNAMEPROVIDER :
#if HAVE_FEATURE_SCRIPTING
        {
                        uno::Reference< container::XNameContainer > xProjProv = rDoc.GetVBATemplateToProjectCache();
                        if (!xProjProv.is() && rDoc.GetDocShell()
                            && ooo::vba::isAlienWordDoc(*rDoc.GetDocShell()))
                        {
                xProjProv = new SwVbaProjectNameProvider;
                            rDoc.SetVBATemplateToProjectCache(xProjProv);
                        }
            xRet = xProjProv;
        }
#endif
        break;
        case  SW_SERVICE_VBAGLOBALS :
#if HAVE_FEATURE_SCRIPTING
        {
            uno::Any aGlobs;
            BasicManager *pBasicMan = rDoc.GetDocShell()->GetBasicManager();
            if (pBasicMan && !pBasicMan->GetGlobalUNOConstant("VBAGlobals", aGlobs))
            {
                uno::Sequence< uno::Any > aArgs(1);
                aArgs[ 0 ] <<= rDoc.GetDocShell()->GetModel();
                aGlobs <<= ::comphelper::getProcessServiceFactory()->createInstanceWithArguments( "ooo.vba.word.Globals", aArgs );
                pBasicMan->SetGlobalUNOConstant( "VBAGlobals", aGlobs );
            }
            aGlobs >>= xRet;
        }
#endif
        break;

        case  SW_SERVICE_TYPE_FOOTNOTE :
            xRet = SwXFootnote::CreateXFootnote(rDoc, 0);
        break;
        case  SW_SERVICE_TYPE_ENDNOTE  :
            xRet = SwXFootnote::CreateXFootnote(rDoc, 0, true);
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
            xRet = SwXDocumentIndexMark::CreateXDocumentIndexMark(rDoc, 0, eType);
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
            xRet = SwXDocumentIndex::CreateXDocumentIndex(rDoc, 0, eType);
        }
        break;
        case SW_SERVICE_INDEX_HEADER_SECTION :
        case SW_SERVICE_TEXT_SECTION :
            xRet = SwXTextSection::CreateXTextSection(0,
                    (SW_SERVICE_INDEX_HEADER_SECTION == nObjectType));

        break;
        case SW_SERVICE_REFERENCE_MARK :
            xRet = SwXReferenceMark::CreateXReferenceMark(rDoc, 0);
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
            SwXStyle* pNewStyle = (SFX_STYLE_FAMILY_PAGE == eFamily)
                ? new SwXPageStyle(rDoc.GetDocShell())
                : (eFamily == SFX_STYLE_FAMILY_FRAME)
                    ? new SwXFrameStyle(&rDoc)
                    : new SwXStyle(&rDoc, eFamily, nObjectType == SW_SERVICE_STYLE_CONDITIONAL_PARAGRAPH_STYLE);
            xRet = static_cast<cppu::OWeakObject*>(pNewStyle);
        }
        break;
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
            // NOTE: the sw.SwXAutoTextEntry unoapi test depends on pDoc = 0
            xRet = SwXTextField::CreateXTextField(0, 0, nObjectType);
            break;
        case SW_SERVICE_FIELDTYPE_ANNOTATION:
            xRet = SwXTextField::CreateXTextField(&rDoc, 0, nObjectType);
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
            xRet = SwXFieldMaster::CreateXFieldMaster(&rDoc, 0, nResId);
        }
        break;
        case SW_SERVICE_FIELDMASTER_BIBLIOGRAPHY:
        {
            SwFieldType* pType = rDoc.getIDocumentFieldsAccess().GetFieldType(RES_AUTHORITY, aEmptyOUStr, true);
            if(!pType)
            {
                SwAuthorityFieldType aType(&rDoc);
                pType = rDoc.getIDocumentFieldsAccess().InsertFieldType(aType);
            }
            xRet = SwXFieldMaster::CreateXFieldMaster(&rDoc, pType);
        }
        break;
        case SW_SERVICE_PARAGRAPH :
            xRet = SwXParagraph::CreateXParagraph(rDoc, 0);
        break;
        case SW_SERVICE_NUMBERING_RULES :
            xRet = static_cast<cppu::OWeakObject*>(new SwXNumberingRules(rDoc));
        break;
        case SW_SERVICE_TEXT_COLUMNS :
            xRet = static_cast<cppu::OWeakObject*>(new SwXTextColumns(0));
        break;
        case SW_SERVICE_DEFAULTS:
            xRet = static_cast<cppu::OWeakObject*>(new SwXTextDefaults(&rDoc));
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
            if (rDoc.GetDocShell()->GetCreateMode() != SfxObjectCreateMode::EMBEDDED)
                xRet = static_cast<cppu::OWeakObject*>(rDoc.getIDocumentChartDataProviderAccess().GetChartDataProvider( true /* create - if not yet available */ ));
            else
                SAL_WARN("sw.uno",
                    "not creating chart data provider for embedded object");

        break;
        case SW_SERVICE_TYPE_META:
            xRet = SwXMeta::CreateXMeta(rDoc, false);
        break;
        case SW_SERVICE_FIELDTYPE_METAFIELD:
            xRet = SwXMeta::CreateXMeta(rDoc, true);
        break;
        default:
            throw uno::RuntimeException();
    }
    return xRet;
}

//SMART_UNO_IMPLEMENTATION( SwXTextTables, UsrObject );
SwXTextTables::SwXTextTables(SwDoc* pDc) :
        SwUnoCollection(pDc)
{

}

SwXTextTables::~SwXTextTables()
{

}

sal_Int32 SwXTextTables::getCount() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    sal_Int32 nRet = 0;
    if(IsValid())
        nRet = static_cast<sal_Int32>(GetDoc()->GetTableFrameFormatCount(true));
    return nRet;
}

uno::Any SAL_CALL SwXTextTables::getByIndex(sal_Int32 nIndex)
        throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(IsValid())
    {
        if(0 <= nIndex && GetDoc()->GetTableFrameFormatCount(true) > static_cast<size_t>(nIndex))
        {
            SwFrameFormat& rFormat = GetDoc()->GetTableFrameFormat(nIndex, true);
            uno::Reference< XTextTable >  xTable = SwXTextTables::GetObject(rFormat);
            aRet.setValue( &xTable,
                cppu::UnoType<XTextTable>::get());
        }
        else
            throw IndexOutOfBoundsException();
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Any SwXTextTables::getByName(const OUString& rItemName)
    throw( NoSuchElementException, WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(IsValid())
    {
        const size_t nCount = GetDoc()->GetTableFrameFormatCount(true);
        uno::Reference< XTextTable >  xTable;
        for( size_t i = 0; i < nCount; ++i)
        {
            SwFrameFormat& rFormat = GetDoc()->GetTableFrameFormat(i, true);
            if (rItemName == rFormat.GetName())
            {
                xTable = SwXTextTables::GetObject(rFormat);
                aRet.setValue(&xTable,
                    cppu::UnoType<XTextTable>::get());
                break;
            }
        }
        if(!xTable.is())
            throw NoSuchElementException();
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Sequence< OUString > SwXTextTables::getElementNames()
        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    const size_t nCount = GetDoc()->GetTableFrameFormatCount(true);
    uno::Sequence<OUString> aSeq(static_cast<sal_Int32>(nCount));
    if(nCount)
    {
        OUString* pArray = aSeq.getArray();
        for( size_t i = 0; i < nCount; ++i)
        {
            SwFrameFormat& rFormat = GetDoc()->GetTableFrameFormat(i, true);

            pArray[i] = rFormat.GetName();
        }
    }
    return aSeq;
}

sal_Bool SwXTextTables::hasByName(const OUString& rName)
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet= false;
    if(IsValid())
    {
        const size_t nCount = GetDoc()->GetTableFrameFormatCount(true);
        for( size_t i = 0; i < nCount; ++i)
        {
            SwFrameFormat& rFormat = GetDoc()->GetTableFrameFormat(i, true);
            if (rName == rFormat.GetName())
            {
                bRet = true;
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
        throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<XTextTable>::get();
}

sal_Bool SwXTextTables::hasElements() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return 0 != GetDoc()->GetTableFrameFormatCount(true);
}

OUString SwXTextTables::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXTextTables");
}

sal_Bool SwXTextTables::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextTables::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArr = aRet.getArray();
    pArr[0] = "com.sun.star.text.TextTables";
    return aRet;
}

uno::Reference<text::XTextTable> SwXTextTables::GetObject(SwFrameFormat& rFormat)
{
    return SwXTextTable::CreateXTextTable(& rFormat);
}

namespace
{
    template<FlyCntType T> struct UnoFrameWrap_traits {};

    template<>
    struct UnoFrameWrap_traits<FLYCNTTYPE_FRM>
    {
        typedef SwXTextFrame core_frame_t;
        typedef XTextFrame uno_frame_t;
        static inline uno::Any wrapFrame(SwFrameFormat & rFrameFormat)
        {
            uno::Reference<text::XTextFrame> const xRet(
                SwXTextFrame::CreateXTextFrame(*rFrameFormat.GetDoc(), &rFrameFormat));
            return uno::makeAny(xRet);
        }
        static inline bool filter(const SwNode* const pNode) { return !pNode->IsNoTextNode(); };
    };

    template<>
    struct UnoFrameWrap_traits<FLYCNTTYPE_GRF>
    {
        typedef SwXTextGraphicObject core_frame_t;
        typedef XTextContent uno_frame_t;
        static inline uno::Any wrapFrame(SwFrameFormat & rFrameFormat)
        {
            uno::Reference<text::XTextContent> const xRet(
                SwXTextGraphicObject::CreateXTextGraphicObject(*rFrameFormat.GetDoc(), &rFrameFormat));
            return uno::makeAny(xRet);
        }
        static inline bool filter(const SwNode* const pNode) { return pNode->IsGrfNode(); };
    };

    template<>
    struct UnoFrameWrap_traits<FLYCNTTYPE_OLE>
    {
        typedef SwXTextEmbeddedObject core_frame_t;
        typedef XEmbeddedObjectSupplier uno_frame_t;
        static inline uno::Any wrapFrame(SwFrameFormat & rFrameFormat)
        {
            uno::Reference<text::XTextContent> const xRet(
                SwXTextEmbeddedObject::CreateXTextEmbeddedObject(*rFrameFormat.GetDoc(), &rFrameFormat));
            return uno::makeAny(xRet);
        }
        static inline bool filter(const SwNode* const pNode) { return pNode->IsOLENode(); };
    };

    template<FlyCntType T>
    static uno::Any lcl_UnoWrapFrame(SwFrameFormat* pFormat)
    {
        return UnoFrameWrap_traits<T>::wrapFrame(*pFormat);
    }

    // runtime adapter for lcl_UnoWrapFrame
    static uno::Any lcl_UnoWrapFrame(SwFrameFormat* pFormat, FlyCntType eType) throw(uno::RuntimeException)
    {
        switch(eType)
        {
            case FLYCNTTYPE_FRM:
                return lcl_UnoWrapFrame<FLYCNTTYPE_FRM>(pFormat);
            case FLYCNTTYPE_GRF:
                return lcl_UnoWrapFrame<FLYCNTTYPE_GRF>(pFormat);
            case FLYCNTTYPE_OLE:
                return lcl_UnoWrapFrame<FLYCNTTYPE_OLE>(pFormat);
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
            virtual sal_Bool SAL_CALL hasMoreElements() throw( RuntimeException, std::exception ) SAL_OVERRIDE;
            virtual Any SAL_CALL nextElement() throw( NoSuchElementException, WrappedTargetException, RuntimeException, std::exception ) SAL_OVERRIDE;

            //XServiceInfo
            virtual OUString SAL_CALL getImplementationName() throw( RuntimeException, std::exception ) SAL_OVERRIDE;
            virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( RuntimeException, std::exception ) SAL_OVERRIDE;
            virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( RuntimeException, std::exception ) SAL_OVERRIDE;
    };
}

template<FlyCntType T>
SwXFrameEnumeration<T>::SwXFrameEnumeration(const SwDoc* const pDoc)
    : m_aFrames()
{
    SolarMutexGuard aGuard;
    const SwFrameFormats* const pFormats = pDoc->GetSpzFrameFormats();
    if(pFormats->empty())
        return;
    // #i104937#
    const size_t nSize = pFormats->size();
    ::std::insert_iterator<frmcontainer_t> pInserter = ::std::insert_iterator<frmcontainer_t>(m_aFrames, m_aFrames.begin());
    // #i104937#
    SwFrameFormat* pFormat( 0 );

    std::set<const SwFrameFormat*> aTextBoxes = SwTextBoxHelper::findTextBoxes(pDoc);

    for( size_t i = 0; i < nSize; ++i )
    {
        // #i104937#
        pFormat = (*pFormats)[i];
        if(pFormat->Which() != RES_FLYFRMFMT || aTextBoxes.find(pFormat) != aTextBoxes.end())
            continue;
        const SwNodeIndex* pIdx =  pFormat->GetContent().GetContentIdx();
        if(!pIdx || !pIdx->GetNodes().IsDocNodes())
            continue;
        const SwNode* pNd = pDoc->GetNodes()[ pIdx->GetIndex() + 1 ];
        if(UnoFrameWrap_traits<T>::filter(pNd))
            *pInserter++ = lcl_UnoWrapFrame<T>(pFormat);
    }
}

template<FlyCntType T>
sal_Bool SwXFrameEnumeration<T>::hasMoreElements() throw( RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    return !m_aFrames.empty();
}

template<FlyCntType T>
Any SwXFrameEnumeration<T>::nextElement() throw( NoSuchElementException, WrappedTargetException, RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(m_aFrames.empty())
        throw NoSuchElementException();
    Any aResult = *m_aFrames.begin();
    m_aFrames.pop_front();
    return aResult;
}

template<FlyCntType T>
OUString SwXFrameEnumeration<T>::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("SwXFrameEnumeration");
}

template<FlyCntType T>
sal_Bool SwXFrameEnumeration<T>::supportsService(const OUString& ServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

template<FlyCntType T>
Sequence< OUString > SwXFrameEnumeration<T>::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    return { OUString("com.sun.star.container.XEnumeration") };
}

OUString SwXFrames::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("SwXFrames");
}

sal_Bool SwXFrames::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SwXFrames::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    return { OUString("com.sun.star.text.TextFrames") };
}

SwXFrames::SwXFrames(SwDoc* _pDoc, FlyCntType eSet) :
    SwUnoCollection(_pDoc),
    eType(eSet)
{}

SwXFrames::~SwXFrames()
{}

uno::Reference<container::XEnumeration> SwXFrames::createEnumeration() throw(uno::RuntimeException, std::exception)
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

sal_Int32 SwXFrames::getCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    // Ignore TextBoxes for TextFrames.
    return static_cast<sal_Int32>(GetDoc()->GetFlyCount(eType, /*bIgnoreTextBoxes=*/eType == FLYCNTTYPE_FRM));
}

uno::Any SwXFrames::getByIndex(sal_Int32 nIndex)
    throw(IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    if(nIndex < 0)
        throw IndexOutOfBoundsException();
    // Ignore TextBoxes for TextFrames.
    SwFrameFormat* pFormat = GetDoc()->GetFlyNum(static_cast<size_t>(nIndex), eType, /*bIgnoreTextBoxes=*/eType == FLYCNTTYPE_FRM);
    if(!pFormat)
        throw IndexOutOfBoundsException();
    return lcl_UnoWrapFrame(pFormat, eType);
}

uno::Any SwXFrames::getByName(const OUString& rName)
    throw(NoSuchElementException, WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    const SwFrameFormat* pFormat;
    switch(eType)
    {
        case FLYCNTTYPE_GRF:
            pFormat = GetDoc()->FindFlyByName(rName, ND_GRFNODE);
            break;
        case FLYCNTTYPE_OLE:
            pFormat = GetDoc()->FindFlyByName(rName, ND_OLENODE);
            break;
        default:
            pFormat = GetDoc()->FindFlyByName(rName, ND_TEXTNODE);
            break;
    }
    if(!pFormat)
        throw NoSuchElementException();
    return lcl_UnoWrapFrame(const_cast<SwFrameFormat*>(pFormat), eType);
}

uno::Sequence<OUString> SwXFrames::getElementNames() throw( uno::RuntimeException, std::exception )
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

sal_Bool SwXFrames::hasByName(const OUString& rName) throw( uno::RuntimeException, std::exception )
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

uno::Type SAL_CALL SwXFrames::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    switch(eType)
    {
        case FLYCNTTYPE_FRM:
            return cppu::UnoType<XTextFrame>::get();
        case FLYCNTTYPE_GRF:
            return cppu::UnoType<XTextContent>::get();
        case FLYCNTTYPE_OLE:
            return cppu::UnoType<XEmbeddedObjectSupplier>::get();
        default:
            return uno::Type();
    }
}

sal_Bool SwXFrames::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return GetDoc()->GetFlyCount(eType) > 0;
}


OUString SwXTextFrames::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("SwXTextFrames");
}

sal_Bool SwXTextFrames::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXTextFrames::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextFrames";
    return aRet;
}

SwXTextFrames::SwXTextFrames(SwDoc* _pDoc) :
    SwXFrames(_pDoc, FLYCNTTYPE_FRM)
{
}

SwXTextFrames::~SwXTextFrames()
{
}

OUString SwXTextGraphicObjects::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("SwXTextGraphicObjects");
}

sal_Bool SwXTextGraphicObjects::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXTextGraphicObjects::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextGraphicObjects";
    return aRet;
}

SwXTextGraphicObjects::SwXTextGraphicObjects(SwDoc* _pDoc) :
    SwXFrames(_pDoc, FLYCNTTYPE_GRF)
{
}

SwXTextGraphicObjects::~SwXTextGraphicObjects()
{
}

OUString SwXTextEmbeddedObjects::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("SwXTextEmbeddedObjects");
}

sal_Bool SwXTextEmbeddedObjects::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXTextEmbeddedObjects::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextEmbeddedObjects";
    return aRet;
}

SwXTextEmbeddedObjects::SwXTextEmbeddedObjects(SwDoc* _pDoc) :
        SwXFrames(_pDoc, FLYCNTTYPE_OLE)
{
}

SwXTextEmbeddedObjects::~SwXTextEmbeddedObjects()
{
}

OUString SwXTextSections::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("SwXTextSections");
}

sal_Bool SwXTextSections::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXTextSections::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextSections";
    return aRet;
}

SwXTextSections::SwXTextSections(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{
}

SwXTextSections::~SwXTextSections()
{
}

sal_Int32 SwXTextSections::getCount() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    const SwSectionFormats& rSectFormats = GetDoc()->GetSections();
    size_t nCount = rSectFormats.size();
    for(size_t i = nCount; i; --i)
    {
        if( !rSectFormats[i - 1]->IsInNodesArr())
            nCount--;
    }
    return nCount;
}

uno::Any SwXTextSections::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextSection >  xRet;
    if(IsValid())
    {
        SwSectionFormats& rFormats = GetDoc()->GetSections();

        const SwSectionFormats& rSectFormats = GetDoc()->GetSections();
        const size_t nCount = rSectFormats.size();
        for(size_t i = 0; i < nCount; ++i)
        {
            if( !rSectFormats[i]->IsInNodesArr())
                nIndex ++;
            else if(static_cast<size_t>(nIndex) == i)
                break;
            if(static_cast<size_t>(nIndex) == i)
                break;
        }
        if(nIndex >= 0 && static_cast<size_t>(nIndex) < rFormats.size())
        {
            SwSectionFormat* pFormat = rFormats[nIndex];
            xRet = GetObject(*pFormat);
        }
        else
            throw IndexOutOfBoundsException();
    }
    else
        throw uno::RuntimeException();
    return makeAny(xRet);
}

uno::Any SwXTextSections::getByName(const OUString& rName)
    throw( NoSuchElementException, WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(IsValid())
    {
        SwSectionFormats& rFormats = GetDoc()->GetSections();
        uno::Reference< XTextSection >  xSect;
        for(size_t i = 0; i < rFormats.size(); ++i)
        {
            SwSectionFormat* pFormat = rFormats[i];
            if (pFormat->IsInNodesArr()
                && (rName == pFormat->GetSection()->GetSectionName()))
            {
                xSect = GetObject(*pFormat);
                aRet.setValue(&xSect, cppu::UnoType<XTextSection>::get());
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

uno::Sequence< OUString > SwXTextSections::getElementNames()
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    size_t nCount = GetDoc()->GetSections().size();
    SwSectionFormats& rSectFormats = GetDoc()->GetSections();
    for(size_t i = nCount; i; --i)
    {
        if( !rSectFormats[i - 1]->IsInNodesArr())
            nCount--;
    }

    uno::Sequence<OUString> aSeq(nCount);
    if(nCount)
    {
        SwSectionFormats& rFormats = GetDoc()->GetSections();
        OUString* pArray = aSeq.getArray();
        size_t nIndex = 0;
        for( size_t i = 0; i < nCount; ++i, ++nIndex)
        {
            const SwSectionFormat* pFormat = rFormats[nIndex];
            while(!pFormat->IsInNodesArr())
            {
                pFormat = rFormats[++nIndex];
            }
            pArray[i] = pFormat->GetSection()->GetSectionName();
        }
    }
    return aSeq;
}

sal_Bool SwXTextSections::hasByName(const OUString& rName)
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(IsValid())
    {
        SwSectionFormats& rFormats = GetDoc()->GetSections();
        for(size_t i = 0; i < rFormats.size(); ++i)
        {
            const SwSectionFormat* pFormat = rFormats[i];
            if (rName == pFormat->GetSection()->GetSectionName())
            {
                bRet = true;
                break;
            }
        }
    }
    else
    {
        // special handling for dbg_ methods
        if( !rName.startsWith("dbg_"))
            throw uno::RuntimeException();
    }
    return bRet;
}

uno::Type SAL_CALL SwXTextSections::getElementType() throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<XTextSection>::get();
}

sal_Bool SwXTextSections::hasElements() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    size_t nCount = 0;
    if(IsValid())
    {
        SwSectionFormats& rFormats = GetDoc()->GetSections();
        nCount = rFormats.size();
    }
    else
        throw uno::RuntimeException();
    return nCount > 0;
}

uno::Reference< XTextSection >  SwXTextSections::GetObject( SwSectionFormat& rFormat )
{
    return SwXTextSection::CreateXTextSection(&rFormat);
}

OUString SwXBookmarks::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("SwXBookmarks");
}

sal_Bool SwXBookmarks::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXBookmarks::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(1);
    aRet[0] = "com.sun.star.text.Bookmarks";
    return aRet;
}

SwXBookmarks::SwXBookmarks(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{ }

SwXBookmarks::~SwXBookmarks()
{ }

sal_Int32 SwXBookmarks::getCount()
    throw( uno::RuntimeException, std::exception )
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
        if (IDocumentMarkAccess::MarkType::BOOKMARK ==
                IDocumentMarkAccess::GetType(**ppMark))
        {
            ++count; // only count real bookmarks
        }
    }
    return count;
}

uno::Any SwXBookmarks::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException, std::exception )
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
        if (IDocumentMarkAccess::MarkType::BOOKMARK ==
                IDocumentMarkAccess::GetType(**ppMark))
        {
            if (count == nIndex)
            {
                uno::Any aRet;
                const uno::Reference< text::XTextContent > xRef =
                    SwXBookmark::CreateXBookmark(*GetDoc(), ppMark->get());
                aRet <<= xRef;
                return aRet;
            }
            ++count; // only count real bookmarks
        }
    }
    throw IndexOutOfBoundsException();
}

uno::Any SwXBookmarks::getByName(const OUString& rName)
    throw( NoSuchElementException, WrappedTargetException, uno::RuntimeException, std::exception )
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
        SwXBookmark::CreateXBookmark(*GetDoc(), ppBkmk->get());
    aRet <<= xRef;
    return aRet;
}

uno::Sequence< OUString > SwXBookmarks::getElementNames()
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();

    ::std::vector< OUString > ret;
    IDocumentMarkAccess* const pMarkAccess = GetDoc()->getIDocumentMarkAccess();
    for (IDocumentMarkAccess::const_iterator_t ppMark =
            pMarkAccess->getBookmarksBegin();
         ppMark != pMarkAccess->getBookmarksEnd(); ++ppMark)
    {
        if (IDocumentMarkAccess::MarkType::BOOKMARK ==
                IDocumentMarkAccess::GetType(**ppMark))
        {
            ret.push_back((*ppMark)->GetName()); // only add real bookmarks
        }
    }
    return comphelper::containerToSequence(ret);
}

sal_Bool SwXBookmarks::hasByName(const OUString& rName)
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();

    IDocumentMarkAccess* const pMarkAccess = GetDoc()->getIDocumentMarkAccess();
    return pMarkAccess->findBookmark(rName) != pMarkAccess->getBookmarksEnd();
}

uno::Type SAL_CALL SwXBookmarks::getElementType()
    throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<XTextContent>::get();
}

sal_Bool SwXBookmarks::hasElements()
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();

    IDocumentMarkAccess* const pMarkAccess = GetDoc()->getIDocumentMarkAccess();
    for (IDocumentMarkAccess::const_iterator_t ppMark =
            pMarkAccess->getBookmarksBegin();
         ppMark != pMarkAccess->getBookmarksEnd(); ++ppMark)
    {
        if (IDocumentMarkAccess::MarkType::BOOKMARK ==
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

sal_Int32 SwXNumberingRulesCollection::getCount() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return GetDoc()->GetNumRuleTable().size();
}

uno::Any SwXNumberingRulesCollection::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(IsValid())
    {
        uno::Reference< XIndexReplace >  xRef;
        if ( static_cast<size_t>(nIndex) < GetDoc()->GetNumRuleTable().size() )
        {
            xRef = new SwXNumberingRules( *GetDoc()->GetNumRuleTable()[ nIndex ], GetDoc());
            aRet.setValue(&xRef, cppu::UnoType<XIndexReplace>::get());
        }

        if(!xRef.is())
            throw IndexOutOfBoundsException();
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Type SAL_CALL SwXNumberingRulesCollection::getElementType() throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<XIndexReplace>::get();
}

sal_Bool SwXNumberingRulesCollection::hasElements() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return !GetDoc()->GetNumRuleTable().empty();
}

OUString SwXFootnotes::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("SwXFootnotes");
}

sal_Bool SwXFootnotes::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXFootnotes::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.Footnotes";
    return aRet;
}

SwXFootnotes::SwXFootnotes(bool bEnd, SwDoc* _pDoc)
    : SwUnoCollection(_pDoc)
    , m_bEndnote(bEnd)
{
}

SwXFootnotes::~SwXFootnotes()
{
}

sal_Int32 SwXFootnotes::getCount() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    sal_Int32 nCount = 0;
    const size_t nFootnoteCnt = GetDoc()->GetFootnoteIdxs().size();
    SwTextFootnote* pTextFootnote;
    for( size_t n = 0; n < nFootnoteCnt; ++n )
    {
        pTextFootnote = GetDoc()->GetFootnoteIdxs()[ n ];
        const SwFormatFootnote& rFootnote = pTextFootnote->GetFootnote();
        if ( rFootnote.IsEndNote() != m_bEndnote )
            continue;
        nCount++;
    }
    return nCount;
}

uno::Any SwXFootnotes::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    sal_Int32 nCount = 0;
    if(IsValid())
    {
        const size_t nFootnoteCnt = GetDoc()->GetFootnoteIdxs().size();
        SwTextFootnote* pTextFootnote;
        uno::Reference< XFootnote >  xRef;
        for( size_t n = 0; n < nFootnoteCnt; ++n )
        {
            pTextFootnote = GetDoc()->GetFootnoteIdxs()[ n ];
            const SwFormatFootnote& rFootnote = pTextFootnote->GetFootnote();
            if ( rFootnote.IsEndNote() != m_bEndnote )
                continue;

            if(nCount == nIndex)
            {
                xRef = SwXFootnote::CreateXFootnote(*GetDoc(),
                        &const_cast<SwFormatFootnote&>(rFootnote));
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

uno::Type SAL_CALL SwXFootnotes::getElementType() throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<XFootnote>::get();
}

sal_Bool SwXFootnotes::hasElements() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return !GetDoc()->GetFootnoteIdxs().empty();
}

Reference<XFootnote>    SwXFootnotes::GetObject( SwDoc& rDoc, const SwFormatFootnote& rFormat )
{
    return SwXFootnote::CreateXFootnote(rDoc, &const_cast<SwFormatFootnote&>(rFormat));
}

OUString SwXReferenceMarks::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("SwXReferenceMarks");
}

sal_Bool SwXReferenceMarks::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXReferenceMarks::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.ReferenceMarks";
    return aRet;
}

SwXReferenceMarks::SwXReferenceMarks(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{
}

SwXReferenceMarks::~SwXReferenceMarks()
{
}

sal_Int32 SwXReferenceMarks::getCount() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return GetDoc()->GetRefMarks();
}

uno::Any SwXReferenceMarks::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(!IsValid())
        throw uno::RuntimeException();
    uno::Reference< XTextContent >  xRef;
    if(0 <= nIndex && nIndex < USHRT_MAX)
    {
        SwFormatRefMark *const pMark = const_cast<SwFormatRefMark*>(
                GetDoc()->GetRefMark(static_cast<sal_uInt16>(nIndex)));
        if(pMark)
        {
            xRef = SwXReferenceMark::CreateXReferenceMark(*GetDoc(), pMark);
            aRet.setValue(&xRef, cppu::UnoType<XTextContent>::get());
        }
    }
    if(!xRef.is())
        throw IndexOutOfBoundsException();
    return aRet;
}

uno::Any SwXReferenceMarks::getByName(const OUString& rName)
    throw( NoSuchElementException, WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(IsValid())
    {
        SwFormatRefMark *const pMark =
            const_cast<SwFormatRefMark*>(GetDoc()->GetRefMark(rName));
        if(pMark)
        {
            uno::Reference<XTextContent> const xRef =
                SwXReferenceMark::CreateXReferenceMark(*GetDoc(), pMark);
            aRet.setValue(&xRef, cppu::UnoType<XTextContent>::get());
        }
        else
            throw NoSuchElementException();
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Sequence< OUString > SwXReferenceMarks::getElementNames() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aRet;
    if(IsValid())
    {
        std::vector<OUString> aStrings;
        const sal_uInt16 nCount = GetDoc()->GetRefMarks( &aStrings );
        aRet.realloc(nCount);
        OUString* pNames = aRet.getArray();
        for(sal_uInt16 i = 0; i < nCount; i++)
            pNames[i] = aStrings[i];
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

sal_Bool SwXReferenceMarks::hasByName(const OUString& rName) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return 0 != GetDoc()->GetRefMark( rName);
}

uno::Type SAL_CALL SwXReferenceMarks::getElementType() throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<XTextContent>::get();
}

sal_Bool SwXReferenceMarks::hasElements() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return 0 != GetDoc()->GetRefMarks();
}

void SwUnoCollection::Invalidate()
{
    bObjectValid = false;
    pDoc = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
