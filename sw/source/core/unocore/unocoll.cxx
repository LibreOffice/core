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
#include <iterator>
#include <unosection.hxx>
#include <unoparagraph.hxx>
#include <unobookmark.hxx>
#include <unorefmark.hxx>
#include <unometa.hxx>
#include <docsh.hxx>
#include <hints.hxx>
#include <com/sun/star/document/XCodeNameQuery.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <vbahelper/vbaaccesshelper.hxx>
#include <basic/basmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <sfx2/event.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

#if HAVE_FEATURE_SCRIPTING

class SwVbaCodeNameProvider : public ::cppu::WeakImplHelper< document::XCodeNameQuery >
{
    SwDocShell* const mpDocShell;
    OUString msThisDocumentCodeName;
public:
    explicit SwVbaCodeNameProvider( SwDocShell* pDocShell ) : mpDocShell( pDocShell ) {}
        // XCodeNameQuery

    OUString SAL_CALL getCodeNameForContainer( const uno::Reference< uno::XInterface >& /*xIf*/ ) override
    {
        // #FIXME not implemented...
        return OUString();
    }

    OUString SAL_CALL getCodeNameForObject( const uno::Reference< uno::XInterface >& xIf ) override
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

typedef std::unordered_map< OUString, OUString > StringHashMap;

class SwVbaProjectNameProvider : public ::cppu::WeakImplHelper< container::XNameContainer >
{
    StringHashMap mTemplateToProject;
public:
    SwVbaProjectNameProvider()
    {
    }
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        return ( mTemplateToProject.find( aName ) != mTemplateToProject.end() );
    }
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        return uno::makeAny( mTemplateToProject.find( aName )->second );
    }
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        return comphelper::mapKeysToSequence( mTemplateToProject );
    }

    virtual void SAL_CALL insertByName( const OUString& aName, const uno::Any& aElement ) override
    {

        OUString sProjectName;
        aElement >>= sProjectName;
        SAL_INFO("sw.uno", "Template cache inserting template name " << aName
                << " with project " << sProjectName);
        mTemplateToProject[ aName ] = sProjectName;
    }

    virtual void SAL_CALL removeByName( const OUString& Name ) override
    {
        if ( !hasByName( Name ) )
            throw container::NoSuchElementException();
        mTemplateToProject.erase( Name );
    }
    virtual void SAL_CALL replaceByName( const OUString& aName, const uno::Any& aElement ) override
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        insertByName( aName, aElement ); // insert will overwrite
    }
    // XElemenAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override
    {
        return ::cppu::UnoType<OUString>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) override
    {

        return ( !mTemplateToProject.empty() );
    }

};

class SwVbaObjectForCodeNameProvider : public ::cppu::WeakImplHelper< container::XNameAccess >
{
    SwDocShell* const mpDocShell;
public:
    explicit SwVbaObjectForCodeNameProvider( SwDocShell* pDocShell ) : mpDocShell( pDocShell )
    {
        // #FIXME #TODO is the code name for ThisDocument read anywhere?
    }

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        // #FIXME #TODO we really need to be checking against the codename for
        // ThisDocument
        if ( aName == "ThisDocument" )
            return true;
        return false;
    }

    css::uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        if ( !hasByName( aName ) )
             throw container::NoSuchElementException();
        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[0] <<= uno::Reference< uno::XInterface >();
        aArgs[1] <<= mpDocShell->GetModel();
        uno::Reference< uno::XInterface > xDocObj = ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "ooo.vba.word.Document" , aArgs );
        SAL_INFO("sw.uno",
            "Creating Object ( ooo.vba.word.Document ) 0x" << xDocObj.get());
        return  uno::makeAny( xDocObj );
    }
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        uno::Sequence< OUString > aNames;
        return aNames;
    }
    // XElemenAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override { return uno::Type(); }
    virtual sal_Bool SAL_CALL hasElements(  ) override { return true; }

};

#endif

struct  ProvNamesId_Type
{
    const char *    pName;
    SwServiceType const   nType;
};

// note: this thing is indexed as an array, so do not insert/remove entries!
const ProvNamesId_Type aProvNamesId[] =
{
    { "com.sun.star.text.TextTable",                          SwServiceType::TypeTextTable },
    { "com.sun.star.text.TextFrame",                          SwServiceType::TypeTextFrame },
    { "com.sun.star.text.GraphicObject",                      SwServiceType::TypeGraphic },
    { "com.sun.star.text.TextEmbeddedObject",                 SwServiceType::TypeOLE },
    { "com.sun.star.text.Bookmark",                           SwServiceType::TypeBookmark },
    { "com.sun.star.text.Footnote",                           SwServiceType::TypeFootnote },
    { "com.sun.star.text.Endnote",                            SwServiceType::TypeEndnote },
    { "com.sun.star.text.DocumentIndexMark",                  SwServiceType::TypeIndexMark },
    { "com.sun.star.text.DocumentIndex",                      SwServiceType::TypeIndex },
    { "com.sun.star.text.ReferenceMark",                      SwServiceType::ReferenceMark },
    { "com.sun.star.style.CharacterStyle",                    SwServiceType::StyleCharacter },
    { "com.sun.star.style.ParagraphStyle",                    SwServiceType::StyleParagraph },
    { "com.sun.star.style.FrameStyle",                        SwServiceType::StyleFrame },
    { "com.sun.star.style.PageStyle",                         SwServiceType::StylePage },
    { "com.sun.star.style.NumberingStyle",                    SwServiceType::StyleNumbering },
    { "com.sun.star.text.ContentIndexMark",                   SwServiceType::ContentIndexMark },
    { "com.sun.star.text.ContentIndex",                       SwServiceType::ContentIndex },
    { "com.sun.star.text.UserIndexMark",                      SwServiceType::UserIndexMark },
    { "com.sun.star.text.UserIndex",                          SwServiceType::UserIndex },
    { "com.sun.star.text.TextSection",                        SwServiceType::TextSection },
    { "com.sun.star.text.TextField.DateTime",                 SwServiceType::FieldTypeDateTime },
    { "com.sun.star.text.TextField.User",                     SwServiceType::FieldTypeUser },
    { "com.sun.star.text.TextField.SetExpression",            SwServiceType::FieldTypeSetExp },
    { "com.sun.star.text.TextField.GetExpression",            SwServiceType::FieldTypeGetExp },
    { "com.sun.star.text.TextField.FileName",                 SwServiceType::FieldTypeFileName },
    { "com.sun.star.text.TextField.PageNumber",               SwServiceType::FieldTypePageNum },
    { "com.sun.star.text.TextField.Author",                   SwServiceType::FieldTypeAuthor },
    { "com.sun.star.text.TextField.Chapter",                  SwServiceType::FieldTypeChapter },
    { "",                                                     SwServiceType::FieldTypeDummy0 },
    { "com.sun.star.text.TextField.GetReference",             SwServiceType::FieldTypeGetReference },
    { "com.sun.star.text.TextField.ConditionalText",          SwServiceType::FieldTypeConditionedText },
    { "com.sun.star.text.TextField.Annotation",               SwServiceType::FieldTypeAnnotation },
    { "com.sun.star.text.TextField.Input",                    SwServiceType::FieldTypeInput },
    { "com.sun.star.text.TextField.Macro",                    SwServiceType::FieldTypeMacro },
    { "com.sun.star.text.TextField.DDE",                      SwServiceType::FieldTypeDDE },
    { "com.sun.star.text.TextField.HiddenParagraph",          SwServiceType::FieldTypeHiddenPara },
    { "" /*com.sun.star.text.TextField.DocumentInfo"*/,       SwServiceType::FieldTypeDocInfo },
    { "com.sun.star.text.TextField.TemplateName",             SwServiceType::FieldTypeTemplateName },
    { "com.sun.star.text.TextField.ExtendedUser",             SwServiceType::FieldTypeUserExt },
    { "com.sun.star.text.TextField.ReferencePageSet",         SwServiceType::FieldTypeRefPageSet },
    { "com.sun.star.text.TextField.ReferencePageGet",         SwServiceType::FieldTypeRefPageGet },
    { "com.sun.star.text.TextField.JumpEdit",                 SwServiceType::FieldTypeJumpEdit },
    { "com.sun.star.text.TextField.Script",                   SwServiceType::FieldTypeScript },
    { "com.sun.star.text.TextField.DatabaseNextSet",          SwServiceType::FieldTypeDatabaseNextSet },
    { "com.sun.star.text.TextField.DatabaseNumberOfSet",      SwServiceType::FieldTypeDatabaseNumSet },
    { "com.sun.star.text.TextField.DatabaseSetNumber",        SwServiceType::FieldTypeDatabaseSetNum },
    { "com.sun.star.text.TextField.Database",                 SwServiceType::FieldTypeDatabase },
    { "com.sun.star.text.TextField.DatabaseName",             SwServiceType::FieldTypeDatabaseName },
    { "com.sun.star.text.TextField.TableFormula",             SwServiceType::FieldTypeTableFormula },
    { "com.sun.star.text.TextField.PageCount",                SwServiceType::FieldTypePageCount },
    { "com.sun.star.text.TextField.ParagraphCount",           SwServiceType::FieldTypeParagraphCount },
    { "com.sun.star.text.TextField.WordCount",                SwServiceType::FieldTypeWordCount },
    { "com.sun.star.text.TextField.CharacterCount",           SwServiceType::FieldTypeCharacterCount },
    { "com.sun.star.text.TextField.TableCount",               SwServiceType::FieldTypeTableCount },
    { "com.sun.star.text.TextField.GraphicObjectCount",       SwServiceType::FieldTypeGraphicObjectCount },
    { "com.sun.star.text.TextField.EmbeddedObjectCount",      SwServiceType::FieldTypeEmbeddedObjectCount },
    { "com.sun.star.text.TextField.DocInfo.ChangeAuthor",     SwServiceType::FieldTypeDocInfoChangeAuthor },
    { "com.sun.star.text.TextField.DocInfo.ChangeDateTime",   SwServiceType::FieldTypeDocInfoChangeDateTime },
    { "com.sun.star.text.TextField.DocInfo.EditTime",         SwServiceType::FieldTypeDocInfoEditTime },
    { "com.sun.star.text.TextField.DocInfo.Description",      SwServiceType::FieldTypeDocInfoDescription },
    { "com.sun.star.text.TextField.DocInfo.CreateAuthor",     SwServiceType::FieldTypeDocInfoCreateAuthor },
    { "com.sun.star.text.TextField.DocInfo.CreateDateTime",   SwServiceType::FieldTypeDocInfoCreateDateTime },
    { "",                                                     SwServiceType::FieldTypeDummy0 },
    { "",                                                     SwServiceType::FieldTypeDummy1 },
    { "",                                                     SwServiceType::FieldTypeDummy2 },
    { "",                                                     SwServiceType::FieldTypeDummy3 },
    { "com.sun.star.text.TextField.DocInfo.Custom",           SwServiceType::FieldTypeDocInfoCustom },
    { "com.sun.star.text.TextField.DocInfo.PrintAuthor",      SwServiceType::FieldTypeDocInfoPrintAuthor },
    { "com.sun.star.text.TextField.DocInfo.PrintDateTime",    SwServiceType::FieldTypeDocInfoPrintDateTime },
    { "com.sun.star.text.TextField.DocInfo.KeyWords",         SwServiceType::FieldTypeDocInfoKeywords },
    { "com.sun.star.text.TextField.DocInfo.Subject",          SwServiceType::FieldTypeDocInfoSubject },
    { "com.sun.star.text.TextField.DocInfo.Title",            SwServiceType::FieldTypeDocInfoTitle },
    { "com.sun.star.text.TextField.DocInfo.Revision",         SwServiceType::FieldTypeDocInfoRevision },
    { "com.sun.star.text.TextField.Bibliography",             SwServiceType::FieldTypeBibliography },
    { "com.sun.star.text.TextField.CombinedCharacters",       SwServiceType::FieldTypeCombinedCharacters },
    { "com.sun.star.text.TextField.DropDown",                 SwServiceType::FieldTypeDropdown },
    { "com.sun.star.text.textfield.MetadataField",            SwServiceType::FieldTypeMetafield },
    { "",                                                     SwServiceType::FieldTypeDummy4 },
    { "",                                                     SwServiceType::FieldTypeDummy5 },
    { "",                                                     SwServiceType::FieldTypeDummy6 },
    { "",                                                     SwServiceType::FieldTypeDummy7 },
    { "com.sun.star.text.FieldMaster.User",                   SwServiceType::FieldMasterUser },
    { "com.sun.star.text.FieldMaster.DDE",                    SwServiceType::FieldMasterDDE },
    { "com.sun.star.text.FieldMaster.SetExpression",          SwServiceType::FieldMasterSetExp },
    { "com.sun.star.text.FieldMaster.Database",               SwServiceType::FieldMasterDatabase },
    { "com.sun.star.text.FieldMaster.Bibliography",           SwServiceType::FieldMasterBibliography },
    { "",                                                     SwServiceType::FieldMasterDummy2 },
    { "",                                                     SwServiceType::FieldMasterDummy3 },
    { "",                                                     SwServiceType::FieldMasterDummy4 },
    { "",                                                     SwServiceType::FieldMasterDummy5 },
    { "com.sun.star.text.IllustrationsIndex",                 SwServiceType::IndexIllustrations },
    { "com.sun.star.text.ObjectIndex",                        SwServiceType::IndexObjects },
    { "com.sun.star.text.TableIndex",                         SwServiceType::IndexTables },
    { "com.sun.star.text.Bibliography",                       SwServiceType::IndexBibliography },
    { "com.sun.star.text.Paragraph",                          SwServiceType::Paragraph },
    { "com.sun.star.text.TextField.InputUser",                SwServiceType::FieldTypeInputUser },
    { "com.sun.star.text.TextField.HiddenText",               SwServiceType::FieldTypeHiddenText },
    { "com.sun.star.style.ConditionalParagraphStyle",         SwServiceType::StyleConditionalParagraph },
    { "com.sun.star.text.NumberingRules",                     SwServiceType::NumberingRules },
    { "com.sun.star.text.TextColumns",                        SwServiceType::TextColumns },
    { "com.sun.star.text.IndexHeaderSection",                 SwServiceType::IndexHeaderSection },
    { "com.sun.star.text.Defaults",                           SwServiceType::Defaults },
    { "com.sun.star.image.ImageMapRectangleObject",           SwServiceType::IMapRectangle },
    { "com.sun.star.image.ImageMapCircleObject",              SwServiceType::IMapCircle },
    { "com.sun.star.image.ImageMapPolygonObject",             SwServiceType::IMapPolygon },
    { "com.sun.star.text.TextGraphicObject",                  SwServiceType::TypeTextGraphic },
    { "com.sun.star.chart2.data.DataProvider",                SwServiceType::Chart2DataProvider },
    { "com.sun.star.text.Fieldmark",                          SwServiceType::TypeFieldMark },
    { "com.sun.star.text.FormFieldmark",                      SwServiceType::TypeFormFieldMark },
    { "com.sun.star.text.InContentMetadata",                  SwServiceType::TypeMeta },
    { "ooo.vba.VBAObjectModuleObjectProvider",                SwServiceType::VbaObjectProvider },
    { "ooo.vba.VBACodeNameProvider",                          SwServiceType::VbaCodeNameProvider },
    { "ooo.vba.VBAProjectNameProvider",                       SwServiceType::VbaProjectNameProvider },
    { "ooo.vba.VBAGlobals",                       SwServiceType::VbaGlobals },

    // case-correct versions of the service names (see #i67811)
    { CSS_TEXT_TEXTFIELD_DATE_TIME,                   SwServiceType::FieldTypeDateTime },
    { CSS_TEXT_TEXTFIELD_USER,                        SwServiceType::FieldTypeUser },
    { CSS_TEXT_TEXTFIELD_SET_EXPRESSION,              SwServiceType::FieldTypeSetExp },
    { CSS_TEXT_TEXTFIELD_GET_EXPRESSION,              SwServiceType::FieldTypeGetExp },
    { CSS_TEXT_TEXTFIELD_FILE_NAME,                   SwServiceType::FieldTypeFileName },
    { CSS_TEXT_TEXTFIELD_PAGE_NUMBER,                 SwServiceType::FieldTypePageNum },
    { CSS_TEXT_TEXTFIELD_AUTHOR,                      SwServiceType::FieldTypeAuthor },
    { CSS_TEXT_TEXTFIELD_CHAPTER,                     SwServiceType::FieldTypeChapter },
    { CSS_TEXT_TEXTFIELD_GET_REFERENCE,               SwServiceType::FieldTypeGetReference },
    { CSS_TEXT_TEXTFIELD_CONDITIONAL_TEXT,            SwServiceType::FieldTypeConditionedText },
    { CSS_TEXT_TEXTFIELD_ANNOTATION,                  SwServiceType::FieldTypeAnnotation },
    { CSS_TEXT_TEXTFIELD_INPUT,                       SwServiceType::FieldTypeInput },
    { CSS_TEXT_TEXTFIELD_MACRO,                       SwServiceType::FieldTypeMacro },
    { CSS_TEXT_TEXTFIELD_DDE,                         SwServiceType::FieldTypeDDE },
    { CSS_TEXT_TEXTFIELD_HIDDEN_PARAGRAPH,            SwServiceType::FieldTypeHiddenPara },
    { CSS_TEXT_TEXTFIELD_TEMPLATE_NAME,               SwServiceType::FieldTypeTemplateName },
    { CSS_TEXT_TEXTFIELD_EXTENDED_USER,               SwServiceType::FieldTypeUserExt },
    { CSS_TEXT_TEXTFIELD_REFERENCE_PAGE_SET,          SwServiceType::FieldTypeRefPageSet },
    { CSS_TEXT_TEXTFIELD_REFERENCE_PAGE_GET,          SwServiceType::FieldTypeRefPageGet },
    { CSS_TEXT_TEXTFIELD_JUMP_EDIT,                   SwServiceType::FieldTypeJumpEdit },
    { CSS_TEXT_TEXTFIELD_SCRIPT,                      SwServiceType::FieldTypeScript },
    { CSS_TEXT_TEXTFIELD_DATABASE_NEXT_SET,           SwServiceType::FieldTypeDatabaseNextSet },
    { CSS_TEXT_TEXTFIELD_DATABASE_NUMBER_OF_SET,      SwServiceType::FieldTypeDatabaseNumSet },
    { CSS_TEXT_TEXTFIELD_DATABASE_SET_NUMBER,         SwServiceType::FieldTypeDatabaseSetNum },
    { CSS_TEXT_TEXTFIELD_DATABASE,                    SwServiceType::FieldTypeDatabase },
    { CSS_TEXT_TEXTFIELD_DATABASE_NAME,               SwServiceType::FieldTypeDatabaseName },
    { CSS_TEXT_TEXTFIELD_TABLE_FORMULA,               SwServiceType::FieldTypeTableFormula },
    { CSS_TEXT_TEXTFIELD_PAGE_COUNT,                  SwServiceType::FieldTypePageCount },
    { CSS_TEXT_TEXTFIELD_PARAGRAPH_COUNT,             SwServiceType::FieldTypeParagraphCount },
    { CSS_TEXT_TEXTFIELD_WORD_COUNT,                  SwServiceType::FieldTypeWordCount },
    { CSS_TEXT_TEXTFIELD_CHARACTER_COUNT,             SwServiceType::FieldTypeCharacterCount },
    { CSS_TEXT_TEXTFIELD_TABLE_COUNT,                 SwServiceType::FieldTypeTableCount },
    { CSS_TEXT_TEXTFIELD_GRAPHIC_OBJECT_COUNT,        SwServiceType::FieldTypeGraphicObjectCount },
    { CSS_TEXT_TEXTFIELD_EMBEDDED_OBJECT_COUNT,       SwServiceType::FieldTypeEmbeddedObjectCount },
    { CSS_TEXT_TEXTFIELD_DOCINFO_CHANGE_AUTHOR,       SwServiceType::FieldTypeDocInfoChangeAuthor },
    { CSS_TEXT_TEXTFIELD_DOCINFO_CHANGE_DATE_TIME,    SwServiceType::FieldTypeDocInfoChangeDateTime },
    { CSS_TEXT_TEXTFIELD_DOCINFO_EDIT_TIME,           SwServiceType::FieldTypeDocInfoEditTime },
    { CSS_TEXT_TEXTFIELD_DOCINFO_DESCRIPTION,         SwServiceType::FieldTypeDocInfoDescription },
    { CSS_TEXT_TEXTFIELD_DOCINFO_CREATE_AUTHOR,       SwServiceType::FieldTypeDocInfoCreateAuthor },
    { CSS_TEXT_TEXTFIELD_DOCINFO_CREATE_DATE_TIME,    SwServiceType::FieldTypeDocInfoCreateDateTime },
    { CSS_TEXT_TEXTFIELD_DOCINFO_PRINT_AUTHOR,        SwServiceType::FieldTypeDocInfoPrintAuthor },
    { CSS_TEXT_TEXTFIELD_DOCINFO_PRINT_DATE_TIME,     SwServiceType::FieldTypeDocInfoPrintDateTime },
    { CSS_TEXT_TEXTFIELD_DOCINFO_KEY_WORDS,           SwServiceType::FieldTypeDocInfoKeywords },
    { CSS_TEXT_TEXTFIELD_DOCINFO_SUBJECT,             SwServiceType::FieldTypeDocInfoSubject },
    { CSS_TEXT_TEXTFIELD_DOCINFO_TITLE,               SwServiceType::FieldTypeDocInfoTitle },
    { CSS_TEXT_TEXTFIELD_DOCINFO_REVISION,            SwServiceType::FieldTypeDocInfoRevision },
    { CSS_TEXT_TEXTFIELD_DOCINFO_CUSTOM,              SwServiceType::FieldTypeDocInfoCustom },
    { CSS_TEXT_TEXTFIELD_BIBLIOGRAPHY,                SwServiceType::FieldTypeBibliography },
    { CSS_TEXT_TEXTFIELD_COMBINED_CHARACTERS,         SwServiceType::FieldTypeCombinedCharacters },
    { CSS_TEXT_TEXTFIELD_DROP_DOWN,                   SwServiceType::FieldTypeDropdown },
    { CSS_TEXT_TEXTFIELD_INPUT_USER,                  SwServiceType::FieldTypeInputUser },
    { CSS_TEXT_TEXTFIELD_HIDDEN_TEXT,                 SwServiceType::FieldTypeHiddenText },
    { CSS_TEXT_FIELDMASTER_USER,                      SwServiceType::FieldMasterUser },
    { CSS_TEXT_FIELDMASTER_DDE,                       SwServiceType::FieldMasterDDE },
    { CSS_TEXT_FIELDMASTER_SET_EXPRESSION,            SwServiceType::FieldMasterSetExp },
    { CSS_TEXT_FIELDMASTER_DATABASE,                  SwServiceType::FieldMasterDatabase },
    { CSS_TEXT_FIELDMASTER_BIBLIOGRAPHY,              SwServiceType::FieldMasterBibliography },
    { "com.sun.star.style.TableStyle",                SwServiceType::StyleTable },
    { "com.sun.star.style.CellStyle",                 SwServiceType::StyleCell }
};

const SvEventDescription* sw_GetSupportedMacroItems()
{
    static const SvEventDescription aMacroDescriptionsImpl[] =
    {
        { SvMacroItemId::OnMouseOver, "OnMouseOver" },
        { SvMacroItemId::OnMouseOut,  "OnMouseOut" },
        { SvMacroItemId::NONE, nullptr }
    };

    return aMacroDescriptionsImpl;
}

OUString SwXServiceProvider::GetProviderName(SwServiceType nObjectType)
{
    SolarMutexGuard aGuard;
    OUString sRet;
    const sal_uInt16 nEntries = SAL_N_ELEMENTS(aProvNamesId);
    if(static_cast<sal_uInt16>(nObjectType) < nEntries)
        sRet = OUString::createFromAscii(aProvNamesId[static_cast<sal_uInt16>(nObjectType)].pName);
    return sRet;
}

uno::Sequence<OUString>     SwXServiceProvider::GetAllServiceNames()
{
    const sal_uInt16 nEntries = SAL_N_ELEMENTS(aProvNamesId);
    uno::Sequence<OUString> aRet(nEntries);
    OUString* pArray = aRet.getArray();
    sal_uInt16 n = 0;
    for(const ProvNamesId_Type & i : aProvNamesId)
    {
        OUString sProv(OUString::createFromAscii(i.pName));
        if(!sProv.isEmpty())
        {
            pArray[n] = sProv;
            n++;
        }
    }
    aRet.realloc(n);
    return aRet;

}

SwServiceType  SwXServiceProvider::GetProviderType(const OUString& rServiceName)
{
    for(const ProvNamesId_Type & i : aProvNamesId)
    {
        if (rServiceName.equalsAscii(i.pName))
            return i.nType;
    }
    return SwServiceType::Invalid;
}

uno::Reference<uno::XInterface>
SwXServiceProvider::MakeInstance(SwServiceType nObjectType, SwDoc & rDoc)
{
    SolarMutexGuard aGuard;
    uno::Reference< uno::XInterface >  xRet;
    switch(nObjectType)
    {
        case  SwServiceType::TypeTextTable:
        {
            xRet = SwXTextTable::CreateXTextTable(nullptr);
        }
        break;
        case  SwServiceType::TypeTextFrame:
        {
            xRet = SwXTextFrame::CreateXTextFrame(rDoc, nullptr);
        }
        break;
        case  SwServiceType::TypeGraphic  :
        case  SwServiceType::TypeTextGraphic /* #i47503# */ :
        {
            xRet = SwXTextGraphicObject::CreateXTextGraphicObject(rDoc, nullptr);

        }
        break;
        case  SwServiceType::TypeOLE      :
        {
            xRet = SwXTextEmbeddedObject::CreateXTextEmbeddedObject(rDoc, nullptr);
        }
        break;
        case  SwServiceType::TypeBookmark :
        {
            xRet = SwXBookmark::CreateXBookmark(rDoc, nullptr);
        }
        break;
        case  SwServiceType::TypeFieldMark :
        {
            xRet = SwXFieldmark::CreateXFieldmark(rDoc, nullptr);
        }
        break;
        case  SwServiceType::TypeFormFieldMark :
        {
            xRet = SwXFieldmark::CreateXFieldmark(rDoc, nullptr, true);
        }
        break;
        case  SwServiceType::VbaObjectProvider :
#if HAVE_FEATURE_SCRIPTING
        {
            SwVbaObjectForCodeNameProvider* pObjProv =
                new SwVbaObjectForCodeNameProvider(rDoc.GetDocShell());
            xRet =  static_cast<cppu::OWeakObject*>(pObjProv);
        }
#endif
        break;
        case  SwServiceType::VbaCodeNameProvider :
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
        case  SwServiceType::VbaProjectNameProvider :
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
        case  SwServiceType::VbaGlobals :
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

        case  SwServiceType::TypeFootnote :
            xRet = SwXFootnote::CreateXFootnote(rDoc, nullptr);
        break;
        case  SwServiceType::TypeEndnote  :
            xRet = SwXFootnote::CreateXFootnote(rDoc, nullptr, true);
        break;
        case  SwServiceType::ContentIndexMark :
        case  SwServiceType::UserIndexMark    :
        case  SwServiceType::TypeIndexMark:
        {
            TOXTypes eType = TOX_INDEX;
            if(SwServiceType::ContentIndexMark== nObjectType)
                eType = TOX_CONTENT;
            else if(SwServiceType::UserIndexMark == nObjectType)
                eType = TOX_USER;
            xRet = SwXDocumentIndexMark::CreateXDocumentIndexMark(rDoc, nullptr, eType);
        }
        break;
        case  SwServiceType::ContentIndex      :
        case  SwServiceType::UserIndex         :
        case  SwServiceType::TypeIndex    :
        case SwServiceType::IndexIllustrations:
        case SwServiceType::IndexObjects      :
        case SwServiceType::IndexTables:
        case SwServiceType::IndexBibliography :
        {
            TOXTypes eType = TOX_INDEX;
            if(SwServiceType::ContentIndex == nObjectType)
                eType = TOX_CONTENT;
            else if(SwServiceType::UserIndex == nObjectType)
                eType = TOX_USER;
            else if(SwServiceType::IndexIllustrations == nObjectType)
            {
                eType = TOX_ILLUSTRATIONS;
            }
            else if(SwServiceType::IndexObjects       == nObjectType)
            {
                eType = TOX_OBJECTS;
            }
            else if(SwServiceType::IndexBibliography  == nObjectType)
            {
                eType = TOX_AUTHORITIES;
            }
            else if(SwServiceType::IndexTables == nObjectType)
            {
                eType = TOX_TABLES;
            }
            xRet = SwXDocumentIndex::CreateXDocumentIndex(rDoc, nullptr, eType);
        }
        break;
        case SwServiceType::IndexHeaderSection :
        case SwServiceType::TextSection :
            xRet = SwXTextSection::CreateXTextSection(nullptr,
                    (SwServiceType::IndexHeaderSection == nObjectType));

        break;
        case SwServiceType::ReferenceMark :
            xRet = SwXReferenceMark::CreateXReferenceMark(rDoc, nullptr);
        break;
        case SwServiceType::StyleCharacter:
        case SwServiceType::StyleParagraph:
        case SwServiceType::StyleConditionalParagraph:
        case SwServiceType::StyleFrame:
        case SwServiceType::StylePage:
        case SwServiceType::StyleNumbering:
        case SwServiceType::StyleTable:
        case SwServiceType::StyleCell:
        {
            SfxStyleFamily eFamily = SfxStyleFamily::Char;
            switch(nObjectType)
            {
                case SwServiceType::StyleParagraph:
                    eFamily = SfxStyleFamily::Para;
                break;
                case SwServiceType::StyleConditionalParagraph:
                    eFamily = SfxStyleFamily::Para;
                    xRet = SwXStyleFamilies::CreateStyleCondParagraph(rDoc);
                break;
                case SwServiceType::StyleFrame:
                    eFamily = SfxStyleFamily::Frame;
                break;
                case SwServiceType::StylePage:
                    eFamily = SfxStyleFamily::Page;
                break;
                case SwServiceType::StyleNumbering:
                    eFamily = SfxStyleFamily::Pseudo;
                break;
                case SwServiceType::StyleTable:
                    eFamily = SfxStyleFamily::Table;
                break;
                case SwServiceType::StyleCell:
                    eFamily = SfxStyleFamily::Cell;
                break;
                default: break;
            }
            if(!xRet.is())
                xRet = SwXStyleFamilies::CreateStyle(eFamily, rDoc);
        }
        break;
        case SwServiceType::FieldTypeDateTime:
        case SwServiceType::FieldTypeUser:
        case SwServiceType::FieldTypeSetExp:
        case SwServiceType::FieldTypeGetExp:
        case SwServiceType::FieldTypeFileName:
        case SwServiceType::FieldTypePageNum:
        case SwServiceType::FieldTypeAuthor:
        case SwServiceType::FieldTypeChapter:
        case SwServiceType::FieldTypeGetReference:
        case SwServiceType::FieldTypeConditionedText:
        case SwServiceType::FieldTypeInput:
        case SwServiceType::FieldTypeMacro:
        case SwServiceType::FieldTypeDDE:
        case SwServiceType::FieldTypeHiddenPara:
        case SwServiceType::FieldTypeDocInfo:
        case SwServiceType::FieldTypeTemplateName:
        case SwServiceType::FieldTypeUserExt:
        case SwServiceType::FieldTypeRefPageSet:
        case SwServiceType::FieldTypeRefPageGet:
        case SwServiceType::FieldTypeJumpEdit:
        case SwServiceType::FieldTypeScript:
        case SwServiceType::FieldTypeDatabaseNextSet:
        case SwServiceType::FieldTypeDatabaseNumSet:
        case SwServiceType::FieldTypeDatabaseSetNum:
        case SwServiceType::FieldTypeDatabase:
        case SwServiceType::FieldTypeDatabaseName:
        case SwServiceType::FieldTypePageCount:
        case SwServiceType::FieldTypeParagraphCount:
        case SwServiceType::FieldTypeWordCount:
        case SwServiceType::FieldTypeCharacterCount:
        case SwServiceType::FieldTypeTableCount:
        case SwServiceType::FieldTypeGraphicObjectCount:
        case SwServiceType::FieldTypeEmbeddedObjectCount:
        case SwServiceType::FieldTypeDocInfoChangeAuthor:
        case SwServiceType::FieldTypeDocInfoChangeDateTime:
        case SwServiceType::FieldTypeDocInfoEditTime:
        case SwServiceType::FieldTypeDocInfoDescription:
        case SwServiceType::FieldTypeDocInfoCreateAuthor:
        case SwServiceType::FieldTypeDocInfoCreateDateTime:
        case SwServiceType::FieldTypeDocInfoCustom:
        case SwServiceType::FieldTypeDocInfoPrintAuthor:
        case SwServiceType::FieldTypeDocInfoPrintDateTime:
        case SwServiceType::FieldTypeDocInfoKeywords:
        case SwServiceType::FieldTypeDocInfoSubject:
        case SwServiceType::FieldTypeDocInfoTitle:
        case SwServiceType::FieldTypeDocInfoRevision:
        case SwServiceType::FieldTypeBibliography:
        case SwServiceType::FieldTypeInputUser:
        case SwServiceType::FieldTypeHiddenText:
        case SwServiceType::FieldTypeCombinedCharacters:
        case SwServiceType::FieldTypeDropdown:
        case SwServiceType::FieldTypeTableFormula:
            // NOTE: the sw.SwXAutoTextEntry unoapi test depends on pDoc = 0
            xRet = SwXTextField::CreateXTextField(nullptr, nullptr, nObjectType);
            break;
        case SwServiceType::FieldTypeAnnotation:
            xRet = SwXTextField::CreateXTextField(&rDoc, nullptr, nObjectType);
            break;
        case SwServiceType::FieldMasterUser:
        case SwServiceType::FieldMasterDDE:
        case SwServiceType::FieldMasterSetExp :
        case SwServiceType::FieldMasterDatabase:
        {
            SwFieldIds nResId = SwFieldIds::Unknown;
            switch(nObjectType)
            {
                case SwServiceType::FieldMasterUser: nResId = SwFieldIds::User; break;
                case SwServiceType::FieldMasterDDE:  nResId = SwFieldIds::Dde; break;
                case SwServiceType::FieldMasterSetExp : nResId = SwFieldIds::SetExp; break;
                case SwServiceType::FieldMasterDatabase: nResId = SwFieldIds::Database; break;
                default: break;
            }
            xRet = SwXFieldMaster::CreateXFieldMaster(&rDoc, nullptr, nResId);
        }
        break;
        case SwServiceType::FieldMasterBibliography:
        {
            SwFieldType* pType = rDoc.getIDocumentFieldsAccess().GetFieldType(SwFieldIds::TableOfAuthorities, OUString(), true);
            if(!pType)
            {
                SwAuthorityFieldType aType(&rDoc);
                pType = rDoc.getIDocumentFieldsAccess().InsertFieldType(aType);
            }
            xRet = SwXFieldMaster::CreateXFieldMaster(&rDoc, pType);
        }
        break;
        case SwServiceType::Paragraph:
            xRet = SwXParagraph::CreateXParagraph(rDoc, nullptr);
        break;
        case SwServiceType::NumberingRules:
            xRet = static_cast<cppu::OWeakObject*>(new SwXNumberingRules(rDoc));
        break;
        case SwServiceType::TextColumns:
            xRet = static_cast<cppu::OWeakObject*>(new SwXTextColumns);
        break;
        case SwServiceType::Defaults:
            xRet = static_cast<cppu::OWeakObject*>(new SwXTextDefaults(&rDoc));
        break;
        case SwServiceType::IMapRectangle:
            xRet = SvUnoImageMapRectangleObject_createInstance( sw_GetSupportedMacroItems() );
        break;
        case SwServiceType::IMapCircle:
            xRet = SvUnoImageMapCircleObject_createInstance( sw_GetSupportedMacroItems() );
        break;
        case SwServiceType::IMapPolygon:
            xRet = SvUnoImageMapPolygonObject_createInstance( sw_GetSupportedMacroItems() );
        break;
        case SwServiceType::Chart2DataProvider:
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
        case SwServiceType::TypeMeta:
            xRet = SwXMeta::CreateXMeta(rDoc, false);
        break;
        case SwServiceType::FieldTypeMetafield:
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

sal_Int32 SwXTextTables::getCount()
{
    SolarMutexGuard aGuard;
    sal_Int32 nRet = 0;
    if(IsValid())
        nRet = static_cast<sal_Int32>(GetDoc()->GetTableFrameFormatCount(true));
    return nRet;
}

uno::Any SAL_CALL SwXTextTables::getByIndex(sal_Int32 nInputIndex)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if (!IsValid())
        throw uno::RuntimeException();

    if (nInputIndex < 0)
        throw IndexOutOfBoundsException();

    SwAutoFormatGetDocNode aGetHt( &GetDoc()->GetNodes() );
    size_t nIndex = static_cast<size_t>(nInputIndex);
    size_t nCurrentIndex = 0;

    for (SwFrameFormat* const & pFormat : *GetDoc()->GetTableFrameFormats())
    {
        if (!pFormat->GetInfo(aGetHt))
        {
            if (nCurrentIndex == nIndex)
            {
                uno::Reference<XTextTable> xTable = SwXTextTables::GetObject(*pFormat);
                aRet <<= xTable;
                return aRet;
            }
            else
                nCurrentIndex++;
        }
    }
    throw IndexOutOfBoundsException();
}

uno::Any SwXTextTables::getByName(const OUString& rItemName)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(!IsValid())
        throw uno::RuntimeException();

    const size_t nCount = GetDoc()->GetTableFrameFormatCount(true);
    uno::Reference< XTextTable >  xTable;
    for( size_t i = 0; i < nCount; ++i)
    {
        SwFrameFormat& rFormat = GetDoc()->GetTableFrameFormat(i, true);
        if (rItemName == rFormat.GetName())
        {
            xTable = SwXTextTables::GetObject(rFormat);
            aRet <<= xTable;
            break;
        }
    }
    if(!xTable.is())
        throw NoSuchElementException();

    return aRet;
}

uno::Sequence< OUString > SwXTextTables::getElementNames()
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
{
    SolarMutexGuard aGuard;
    bool bRet= false;
    if(!IsValid())
        throw uno::RuntimeException();

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
    return bRet;
}

uno::Type SAL_CALL
    SwXTextTables::getElementType(  )
{
    return cppu::UnoType<XTextTable>::get();
}

sal_Bool SwXTextTables::hasElements()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return 0 != GetDoc()->GetTableFrameFormatCount(true);
}

OUString SwXTextTables::getImplementationName()
{
    return OUString("SwXTextTables");
}

sal_Bool SwXTextTables::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextTables::getSupportedServiceNames()
{
    uno::Sequence< OUString > aRet { "com.sun.star.text.TextTables" };
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
        static uno::Any wrapFrame(SwFrameFormat & rFrameFormat)
        {
            uno::Reference<text::XTextFrame> const xRet(
                SwXTextFrame::CreateXTextFrame(*rFrameFormat.GetDoc(), &rFrameFormat));
            return uno::makeAny(xRet);
        }
        static bool filter(const SwNode* const pNode) { return !pNode->IsNoTextNode(); };
    };

    template<>
    struct UnoFrameWrap_traits<FLYCNTTYPE_GRF>
    {
        static uno::Any wrapFrame(SwFrameFormat & rFrameFormat)
        {
            uno::Reference<text::XTextContent> const xRet(
                SwXTextGraphicObject::CreateXTextGraphicObject(*rFrameFormat.GetDoc(), &rFrameFormat));
            return uno::makeAny(xRet);
        }
        static bool filter(const SwNode* const pNode) { return pNode->IsGrfNode(); };
    };

    template<>
    struct UnoFrameWrap_traits<FLYCNTTYPE_OLE>
    {
        static uno::Any wrapFrame(SwFrameFormat & rFrameFormat)
        {
            uno::Reference<text::XTextContent> const xRet(
                SwXTextEmbeddedObject::CreateXTextEmbeddedObject(*rFrameFormat.GetDoc(), &rFrameFormat));
            return uno::makeAny(xRet);
        }
        static bool filter(const SwNode* const pNode) { return pNode->IsOLENode(); };
    };

    template<FlyCntType T>
    uno::Any lcl_UnoWrapFrame(SwFrameFormat* pFormat)
    {
        return UnoFrameWrap_traits<T>::wrapFrame(*pFormat);
    }

    // runtime adapter for lcl_UnoWrapFrame
    /// @throws uno::RuntimeException
    uno::Any lcl_UnoWrapFrame(SwFrameFormat* pFormat, FlyCntType eType)
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
            std::vector< Any > m_aFrames;
        protected:
            virtual ~SwXFrameEnumeration() override {};
        public:
            SwXFrameEnumeration(const SwDoc* const pDoc);

            //XEnumeration
            virtual sal_Bool SAL_CALL hasMoreElements() override;
            virtual Any SAL_CALL nextElement() override;

            //XServiceInfo
            virtual OUString SAL_CALL getImplementationName() override;
            virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
            virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
    // #i104937#
    SwFrameFormat* pFormat( nullptr );
    for( size_t i = 0; i < nSize; ++i )
    {
        // #i104937#
        pFormat = (*pFormats)[i];
        if(pFormat->Which() != RES_FLYFRMFMT || SwTextBoxHelper::isTextBox(pFormat, RES_FLYFRMFMT))
            continue;
        const SwNodeIndex* pIdx =  pFormat->GetContent().GetContentIdx();
        if(!pIdx || !pIdx->GetNodes().IsDocNodes())
            continue;
        const SwNode* pNd = pDoc->GetNodes()[ pIdx->GetIndex() + 1 ];
        if(UnoFrameWrap_traits<T>::filter(pNd))
            m_aFrames.push_back(lcl_UnoWrapFrame<T>(pFormat));
    }
}

template<FlyCntType T>
sal_Bool SwXFrameEnumeration<T>::hasMoreElements()
{
    SolarMutexGuard aGuard;
    return !m_aFrames.empty();
}

template<FlyCntType T>
Any SwXFrameEnumeration<T>::nextElement()
{
    SolarMutexGuard aGuard;
    if(m_aFrames.empty())
        throw NoSuchElementException();

    Any aResult = m_aFrames.back();
    m_aFrames.pop_back();
    return aResult;
}

template<FlyCntType T>
OUString SwXFrameEnumeration<T>::getImplementationName()
{
    return OUString("SwXFrameEnumeration");
}

template<FlyCntType T>
sal_Bool SwXFrameEnumeration<T>::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

template<FlyCntType T>
Sequence< OUString > SwXFrameEnumeration<T>::getSupportedServiceNames()
{
    return { OUString("com.sun.star.container.XEnumeration") };
}

OUString SwXFrames::getImplementationName()
{
    return OUString("SwXFrames");
}

sal_Bool SwXFrames::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SwXFrames::getSupportedServiceNames()
{
    return { OUString("com.sun.star.text.TextFrames") };
}

SwXFrames::SwXFrames(SwDoc* _pDoc, FlyCntType eSet) :
    SwUnoCollection(_pDoc),
    m_eType(eSet)
{}

SwXFrames::~SwXFrames()
{}

uno::Reference<container::XEnumeration> SwXFrames::createEnumeration()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    switch(m_eType)
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

sal_Int32 SwXFrames::getCount()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    // Ignore TextBoxes for TextFrames.
    return static_cast<sal_Int32>(GetDoc()->GetFlyCount(m_eType, /*bIgnoreTextBoxes=*/m_eType == FLYCNTTYPE_FRM));
}

uno::Any SwXFrames::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    if(nIndex < 0)
        throw IndexOutOfBoundsException();
    // Ignore TextBoxes for TextFrames.
    SwFrameFormat* pFormat = GetDoc()->GetFlyNum(static_cast<size_t>(nIndex), m_eType, /*bIgnoreTextBoxes=*/m_eType == FLYCNTTYPE_FRM);
    if(!pFormat)
        throw IndexOutOfBoundsException();
    return lcl_UnoWrapFrame(pFormat, m_eType);
}

uno::Any SwXFrames::getByName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    const SwFrameFormat* pFormat;
    switch(m_eType)
    {
        case FLYCNTTYPE_GRF:
            pFormat = GetDoc()->FindFlyByName(rName, SwNodeType::Grf);
            break;
        case FLYCNTTYPE_OLE:
            pFormat = GetDoc()->FindFlyByName(rName, SwNodeType::Ole);
            break;
        default:
            pFormat = GetDoc()->FindFlyByName(rName, SwNodeType::Text);
            break;
    }
    if(!pFormat)
        throw NoSuchElementException();
    return lcl_UnoWrapFrame(const_cast<SwFrameFormat*>(pFormat), m_eType);
}

uno::Sequence<OUString> SwXFrames::getElementNames()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    const Reference<XEnumeration> xEnum = createEnumeration();
    std::vector<OUString> vNames;
    while(xEnum->hasMoreElements())
    {
        Reference<container::XNamed> xNamed;
        xEnum->nextElement() >>= xNamed;
        if(xNamed.is())
            vNames.push_back(xNamed->getName());
    }
    return ::comphelper::containerToSequence(vNames);
}

sal_Bool SwXFrames::hasByName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    switch(m_eType)
    {
        case FLYCNTTYPE_GRF:
            return GetDoc()->FindFlyByName(rName, SwNodeType::Grf) != nullptr;
        case FLYCNTTYPE_OLE:
            return GetDoc()->FindFlyByName(rName, SwNodeType::Ole) != nullptr;
        default:
            return GetDoc()->FindFlyByName(rName, SwNodeType::Text) != nullptr;
    }
}

uno::Type SAL_CALL SwXFrames::getElementType()
{
    SolarMutexGuard aGuard;
    switch(m_eType)
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

sal_Bool SwXFrames::hasElements()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return GetDoc()->GetFlyCount(m_eType) > 0;
}


OUString SwXTextFrames::getImplementationName()
{
    return OUString("SwXTextFrames");
}

sal_Bool SwXTextFrames::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXTextFrames::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.text.TextFrames" };
    return aRet;
}

SwXTextFrames::SwXTextFrames(SwDoc* _pDoc) :
    SwXFrames(_pDoc, FLYCNTTYPE_FRM)
{
}

SwXTextFrames::~SwXTextFrames()
{
}

OUString SwXTextGraphicObjects::getImplementationName()
{
    return OUString("SwXTextGraphicObjects");
}

sal_Bool SwXTextGraphicObjects::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXTextGraphicObjects::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.text.TextGraphicObjects" };
    return aRet;
}

SwXTextGraphicObjects::SwXTextGraphicObjects(SwDoc* _pDoc) :
    SwXFrames(_pDoc, FLYCNTTYPE_GRF)
{
}

SwXTextGraphicObjects::~SwXTextGraphicObjects()
{
}

OUString SwXTextEmbeddedObjects::getImplementationName()
{
    return OUString("SwXTextEmbeddedObjects");
}

sal_Bool SwXTextEmbeddedObjects::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXTextEmbeddedObjects::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.text.TextEmbeddedObjects" };
    return aRet;
}

SwXTextEmbeddedObjects::SwXTextEmbeddedObjects(SwDoc* _pDoc) :
        SwXFrames(_pDoc, FLYCNTTYPE_OLE)
{
}

SwXTextEmbeddedObjects::~SwXTextEmbeddedObjects()
{
}

OUString SwXTextSections::getImplementationName()
{
    return OUString("SwXTextSections");
}

sal_Bool SwXTextSections::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXTextSections::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.text.TextSections" };
    return aRet;
}

SwXTextSections::SwXTextSections(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{
}

SwXTextSections::~SwXTextSections()
{
}

sal_Int32 SwXTextSections::getCount()
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
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextSection >  xRet;
    if(!IsValid())
        throw uno::RuntimeException();

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
    if(!(nIndex >= 0 && static_cast<size_t>(nIndex) < rFormats.size()))
        throw IndexOutOfBoundsException();

    SwSectionFormat* pFormat = rFormats[nIndex];
    xRet = GetObject(*pFormat);

    return makeAny(xRet);
}

uno::Any SwXTextSections::getByName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(!IsValid())
        throw uno::RuntimeException();

    SwSectionFormats& rFormats = GetDoc()->GetSections();
    uno::Reference< XTextSection >  xSect;
    for(size_t i = 0; i < rFormats.size(); ++i)
    {
        SwSectionFormat* pFormat = rFormats[i];
        if (pFormat->IsInNodesArr()
            && (rName == pFormat->GetSection()->GetSectionName()))
        {
            xSect = GetObject(*pFormat);
            aRet <<= xSect;
            break;
        }
    }
    if(!xSect.is())
        throw NoSuchElementException();

    return aRet;
}

uno::Sequence< OUString > SwXTextSections::getElementNames()
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

uno::Type SAL_CALL SwXTextSections::getElementType()
{
    return cppu::UnoType<XTextSection>::get();
}

sal_Bool SwXTextSections::hasElements()
{
    SolarMutexGuard aGuard;
    size_t nCount = 0;
    if(!IsValid())
        throw uno::RuntimeException();

    SwSectionFormats& rFormats = GetDoc()->GetSections();
    nCount = rFormats.size();

    return nCount > 0;
}

uno::Reference< XTextSection >  SwXTextSections::GetObject( SwSectionFormat& rFormat )
{
    return SwXTextSection::CreateXTextSection(&rFormat);
}

OUString SwXBookmarks::getImplementationName()
{
    return OUString("SwXBookmarks");
}

sal_Bool SwXBookmarks::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXBookmarks::getSupportedServiceNames()
{
    Sequence< OUString > aRet { "com.sun.star.text.Bookmarks" };
    return aRet;
}

SwXBookmarks::SwXBookmarks(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{ }

SwXBookmarks::~SwXBookmarks()
{ }

sal_Int32 SwXBookmarks::getCount()
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
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();

    std::vector< OUString > ret;
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
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();

    IDocumentMarkAccess* const pMarkAccess = GetDoc()->getIDocumentMarkAccess();
    return pMarkAccess->findBookmark(rName) != pMarkAccess->getBookmarksEnd();
}

uno::Type SAL_CALL SwXBookmarks::getElementType()
{
    return cppu::UnoType<XTextContent>::get();
}

sal_Bool SwXBookmarks::hasElements()
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

sal_Int32 SwXNumberingRulesCollection::getCount()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return GetDoc()->GetNumRuleTable().size();
}

uno::Any SwXNumberingRulesCollection::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(!IsValid())
        throw uno::RuntimeException();

    uno::Reference< XIndexReplace >  xRef;
    if ( static_cast<size_t>(nIndex) < GetDoc()->GetNumRuleTable().size() )
    {
        xRef = new SwXNumberingRules( *GetDoc()->GetNumRuleTable()[ nIndex ], GetDoc());
        aRet <<= xRef;
    }

    if(!xRef.is())
        throw IndexOutOfBoundsException();

    return aRet;
}

uno::Type SAL_CALL SwXNumberingRulesCollection::getElementType()
{
    return cppu::UnoType<XIndexReplace>::get();
}

sal_Bool SwXNumberingRulesCollection::hasElements()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return !GetDoc()->GetNumRuleTable().empty();
}

OUString SwXFootnotes::getImplementationName()
{
    return OUString("SwXFootnotes");
}

sal_Bool SwXFootnotes::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXFootnotes::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.text.Footnotes" };
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

sal_Int32 SwXFootnotes::getCount()
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
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    sal_Int32 nCount = 0;
    if(!IsValid())
        throw uno::RuntimeException();

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

    return aRet;
}

uno::Type SAL_CALL SwXFootnotes::getElementType()
{
    return cppu::UnoType<XFootnote>::get();
}

sal_Bool SwXFootnotes::hasElements()
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

OUString SwXReferenceMarks::getImplementationName()
{
    return OUString("SwXReferenceMarks");
}

sal_Bool SwXReferenceMarks::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXReferenceMarks::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.text.ReferenceMarks" };
    return aRet;
}

SwXReferenceMarks::SwXReferenceMarks(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{
}

SwXReferenceMarks::~SwXReferenceMarks()
{
}

sal_Int32 SwXReferenceMarks::getCount()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return GetDoc()->GetRefMarks();
}

uno::Any SwXReferenceMarks::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(!IsValid())
        throw uno::RuntimeException();
    uno::Reference< XTextContent >  xRef;
    if(0 <= nIndex && nIndex < SAL_MAX_UINT16)
    {
        SwFormatRefMark *const pMark = const_cast<SwFormatRefMark*>(
                GetDoc()->GetRefMark(static_cast<sal_uInt16>(nIndex)));
        if(pMark)
        {
            xRef = SwXReferenceMark::CreateXReferenceMark(*GetDoc(), pMark);
            aRet <<= xRef;
        }
    }
    if(!xRef.is())
        throw IndexOutOfBoundsException();
    return aRet;
}

uno::Any SwXReferenceMarks::getByName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(!IsValid())
        throw uno::RuntimeException();

    SwFormatRefMark *const pMark =
        const_cast<SwFormatRefMark*>(GetDoc()->GetRefMark(rName));
    if(!pMark)
        throw NoSuchElementException();

    uno::Reference<XTextContent> const xRef =
        SwXReferenceMark::CreateXReferenceMark(*GetDoc(), pMark);
    aRet <<= xRef;

    return aRet;
}

uno::Sequence< OUString > SwXReferenceMarks::getElementNames()
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aRet;
    if(!IsValid())
        throw uno::RuntimeException();

    std::vector<OUString> aStrings;
    const sal_uInt16 nCount = GetDoc()->GetRefMarks( &aStrings );
    aRet.realloc(nCount);
    OUString* pNames = aRet.getArray();
    for(sal_uInt16 i = 0; i < nCount; i++)
        pNames[i] = aStrings[i];

    return aRet;
}

sal_Bool SwXReferenceMarks::hasByName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return nullptr != GetDoc()->GetRefMark( rName);
}

uno::Type SAL_CALL SwXReferenceMarks::getElementType()
{
    return cppu::UnoType<XTextContent>::get();
}

sal_Bool SwXReferenceMarks::hasElements()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return 0 != GetDoc()->GetRefMarks();
}

void SwUnoCollection::Invalidate()
{
    m_bObjectValid = false;
    m_pDoc = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
