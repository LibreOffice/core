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

#include <sal/config.h>
#include <sal/log.hxx>

#include "vbafilterpropsfromformat.hxx"
#include "vbacontentcontrols.hxx"
#include "vbadocument.hxx"
#include "vbaformfields.hxx"
#include "vbarange.hxx"
#include "vbarangehelper.hxx"
#include "vbadocumentproperties.hxx"
#include "vbabookmarks.hxx"
#include "vbamailmerge.hxx"
#include "vbavariables.hxx"
#include "vbawindow.hxx"
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <cppu/unotype.hxx>

#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include <com/sun/star/util/thePathSettings.hpp>
#include <ooo/vba/XControlProvider.hpp>
#include <ooo/vba/word/WdProtectionType.hpp>
#include <ooo/vba/word/WdSaveFormat.hpp>
#include <ooo/vba/word/XDocumentOutgoing.hpp>

#include "wordvbahelper.hxx"
#include <doc.hxx>
#include <docsh.hxx>
#include "vbatemplate.hxx"
#include "vbaparagraph.hxx"
#include "vbastyles.hxx"
#include "vbatables.hxx"
#include "vbafield.hxx"
#include "vbapagesetup.hxx"
#include "vbasections.hxx"
#include "vbatablesofcontents.hxx"
#include <vbahelper/vbashapes.hxx>
#include <vbahelper/vbahelper.hxx>
#include "vbarevisions.hxx"
#include "vbaframes.hxx"
#include <basic/sberrors.hxx>
#include <osl/file.hxx>
#include <tools/urlobj.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

namespace {

class SwVbaDocumentOutgoingConnectionPoint : public cppu::WeakImplHelper<XConnectionPoint>
{
private:
    SwVbaDocument* mpDoc;

public:
    SwVbaDocumentOutgoingConnectionPoint( SwVbaDocument* pDoc );

    // XConnectionPoint
    sal_uInt32 SAL_CALL Advise(const uno::Reference< XSink >& Sink ) override;
    void SAL_CALL Unadvise( sal_uInt32 Cookie ) override;
};

}

SwVbaDocument::SwVbaDocument( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, uno::Reference< frame::XModel > const & xModel ): SwVbaDocument_BASE( xParent, xContext, xModel )
{
    Initialize();
}
SwVbaDocument::SwVbaDocument( uno::Sequence< uno::Any > const& aArgs, uno::Reference< uno::XComponentContext >const& xContext ) : SwVbaDocument_BASE( aArgs, xContext )
{
    Initialize();
}

SwVbaDocument::~SwVbaDocument()
{
}

void SwVbaDocument::Initialize()
{
    mxTextDocument.set( getModel(), uno::UNO_QUERY_THROW );
    SwDocShell& rDocSh = *word::getDocShell(mxModel);
    rDocSh.RegisterAutomationDocumentObject(this);
    rDocSh.GetDoc()->SetVbaEventProcessor();
}

sal_uInt32
SwVbaDocument::AddSink( const uno::Reference< XSink >& xSink )
{
    word::getDocShell( mxModel )->RegisterAutomationDocumentEventsCaller( uno::Reference< XSinkCaller >(this) );
    mvSinks.push_back(xSink);
    return mvSinks.size();
}

void
SwVbaDocument::RemoveSink( sal_uInt32 nNumber )
{
    if (nNumber < 1 || nNumber > mvSinks.size())
        return;

    mvSinks[nNumber-1] = uno::Reference< XSink >();
}

uno::Reference< word::XRange > SAL_CALL
SwVbaDocument::getContent()
{
    uno::Reference< text::XTextRange > xStart = mxTextDocument->getText()->getStart();
    uno::Reference< text::XTextRange > xEnd;
    return uno::Reference< word::XRange >( new SwVbaRange( this, mxContext, mxTextDocument, xStart, xEnd ) );
}

uno::Reference< word::XRange > SAL_CALL
SwVbaDocument::Range( const uno::Any& rStart, const uno::Any& rEnd )
{
    if( !rStart.hasValue() && !rEnd.hasValue() )
        return getContent();

    sal_Int32 nStart = 0;
    sal_Int32 nEnd = 0;
    rStart >>= nStart;
    rEnd >>= nEnd;

    uno::Reference< text::XTextRange > xStart;
    uno::Reference< text::XTextRange > xEnd;

    if( nStart > nEnd)
       throw uno::RuntimeException();

    if( nEnd != 0)
    {
        if( nEnd == nStart )
        {
            xStart = mxTextDocument->getText()->getEnd();
            xEnd = mxTextDocument->getText()->getEnd();
        }
        else
        {
            xEnd = SwVbaRangeHelper::getRangeByPosition( mxTextDocument->getText(), nEnd );

            if( nStart != 0 )
                xStart = SwVbaRangeHelper::getRangeByPosition( mxTextDocument->getText(), nStart );
            else
                xStart = mxTextDocument->getText()->getStart();
        }
    }
    else
    {
        xStart = mxTextDocument->getText()->getEnd();
        xEnd = mxTextDocument->getText()->getEnd();
    }

    if( !xStart.is() && !xEnd.is() )
    {
        try
        {
            // FIXME
            xStart = mxTextDocument->getText()->getStart();
            xEnd = mxTextDocument->getText()->getEnd();
        }
        catch(const uno::Exception&)
        {
            DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
        }
    }
    return uno::Reference< word::XRange >( new SwVbaRange( this, mxContext, mxTextDocument, xStart, xEnd ) );
}

uno::Any SAL_CALL
SwVbaDocument::BuiltInDocumentProperties( const uno::Any& index )
{
    uno::Reference< XCollection > xCol( new SwVbaBuiltinDocumentProperties( mxParent, mxContext, getModel() ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaDocument::CustomDocumentProperties( const uno::Any& index )
{
    uno::Reference< XCollection > xCol( new SwVbaCustomDocumentProperties( mxParent, mxContext, getModel() ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaDocument::Bookmarks( const uno::Any& rIndex )
{
    uno::Reference< text::XBookmarksSupplier > xBookmarksSupplier( getModel(),uno::UNO_QUERY_THROW );
    uno::Reference<container::XIndexAccess > xBookmarks( xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xBookmarksVba( new SwVbaBookmarks( this, mxContext, xBookmarks, getModel() ) );
    if (  rIndex.getValueTypeClass() == uno::TypeClass_VOID )
        return uno::Any( xBookmarksVba );

    return xBookmarksVba->Item( rIndex, uno::Any() );
}

uno::Any SwVbaDocument::ContentControls(const uno::Any& index)
{
    uno::Reference<XCollection> xContentControls(
        new SwVbaContentControls(this, mxContext, mxTextDocument, u""_ustr, u""_ustr));
    if (index.hasValue())
    {
        try
        {
            return xContentControls->Item(index, uno::Any());
        }
        catch (lang::IndexOutOfBoundsException&)
        {
            // Hack: Instead of an index, it might be a float that was mistakenly treated as a long,
            // which can happen with any valid positive integer when specified as a double like
            // ActiveDocument.ContentControls(1841581653#).
            if (index.getValueTypeClass() == css::uno::TypeClass_LONG)
            {
                sal_Int32 nLong(0);
                index >>= nLong;
                return xContentControls->Item(uno::Any(static_cast<double>(nLong)), uno::Any());
            }
        }
    }

    return uno::Any(xContentControls);
}

uno::Any SwVbaDocument::SelectContentControlsByTag(const uno::Any& index)
{
    OUString sTag;
    index >>= sTag;
    return uno::Any(uno::Reference<XCollection>(
                        new SwVbaContentControls(this, mxContext, mxTextDocument, sTag, u""_ustr)));
}

uno::Any SwVbaDocument::SelectContentControlsByTitle(const uno::Any& index)
{
    OUString sTitle;
    index >>= sTitle;
    return uno::Any(uno::Reference<XCollection>(
                        new SwVbaContentControls(this, mxContext, mxTextDocument, u""_ustr, sTitle)));
}

uno::Reference<word::XWindow> SwVbaDocument::getActiveWindow()
{
    // copied from vbaapplication which has a #FIXME so far can't determine Parent
    return new SwVbaWindow(uno::Reference< XHelperInterface >(), mxContext, mxModel,
                           mxModel->getCurrentController());
}

uno::Any SAL_CALL
SwVbaDocument::Variables( const uno::Any& rIndex )
{
    uno::Reference< css::document::XDocumentPropertiesSupplier > xDocumentPropertiesSupplier( getModel(),uno::UNO_QUERY_THROW );
    uno::Reference< css::document::XDocumentProperties > xDocumentProperties =  xDocumentPropertiesSupplier->getDocumentProperties();
    uno::Reference< beans::XPropertyAccess > xUserDefined( xDocumentProperties->getUserDefinedProperties(), uno::UNO_QUERY_THROW );

    uno::Reference< XCollection > xVariables( new SwVbaVariables( this, mxContext, xUserDefined ) );
    if (  rIndex.getValueTypeClass() == uno::TypeClass_VOID )
        return uno::Any( xVariables );

    return xVariables->Item( rIndex, uno::Any() );
}

uno::Any SAL_CALL
SwVbaDocument::Paragraphs( const uno::Any& index )
{
    uno::Reference< XCollection > xCol( new SwVbaParagraphs( mxParent, mxContext, mxTextDocument ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaDocument::Styles( const uno::Any& index )
{
    uno::Reference< XCollection > xCol( new SwVbaStyles( mxParent, mxContext, getModel() ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaDocument::Fields( const uno::Any& index )
{
    uno::Reference< XCollection > xCol( new SwVbaFields( mxParent, mxContext, getModel() ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaDocument::Shapes( const uno::Any& index )
{
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( getModel(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndexAccess( xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW );
    uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xCol( new ScVbaShapes( this, mxContext, xIndexAccess, xModel ) );

    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

void SAL_CALL
SwVbaDocument::Select()
{
    auto xRange = getContent();
    if ( xRange )
        xRange->Select();
}

uno::Any SAL_CALL
SwVbaDocument::Sections( const uno::Any& index )
{
    uno::Reference< XCollection > xCol( new SwVbaSections( mxParent, mxContext, getModel() ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaDocument::TablesOfContents( const uno::Any& index )
{
    uno::Reference< XCollection > xCol( new SwVbaTablesOfContents( this, mxContext, mxTextDocument ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL SwVbaDocument::FormFields(const uno::Any& index)
{
    uno::Reference<XCollection> xCol(new SwVbaFormFields(this, mxContext, mxTextDocument));
    if (index.hasValue())
        return xCol->Item(index, uno::Any());
    return uno::Any(xCol);
}

uno::Any SAL_CALL
SwVbaDocument::PageSetup( )
{
    uno::Reference< beans::XPropertySet > xPageProps( word::getCurrentPageStyle( mxModel ), uno::UNO_QUERY_THROW );
    return uno::Any( uno::Reference< word::XPageSetup >( new SwVbaPageSetup( this, mxContext, mxModel, xPageProps ) ) );
}

OUString
SwVbaDocument::getServiceImplName()
{
    return u"SwVbaDocument"_ustr;
}

uno::Any SAL_CALL
SwVbaDocument::getAttachedTemplate()
{
    uno::Reference< word::XTemplate > xTemplate;
    uno::Reference<css::document::XDocumentPropertiesSupplier> const xDocPropSupp(
            getModel(), uno::UNO_QUERY_THROW);
    uno::Reference< css::document::XDocumentProperties > xDocProps( xDocPropSupp->getDocumentProperties(), uno::UNO_SET_THROW );

    xTemplate = new SwVbaTemplate( this, mxContext, xDocProps->getTemplateURL() );
    return uno::Any( xTemplate );
}

void SAL_CALL
SwVbaDocument::setAttachedTemplate( const css::uno::Any& _attachedtemplate )
{
    OUString sTemplate;
    if( !( _attachedtemplate >>= sTemplate ) )
    {
        throw uno::RuntimeException();
    }
    OUString aURL;
    INetURLObject aObj;
    aObj.SetURL( sTemplate );
    bool bIsURL = aObj.GetProtocol() != INetProtocol::NotValid;
    if ( bIsURL )
        aURL = sTemplate;
    else
        osl::FileBase::getFileURLFromSystemPath( sTemplate, aURL );

    uno::Reference<css::document::XDocumentPropertiesSupplier> const xDocPropSupp(
            getModel(), uno::UNO_QUERY_THROW );
    uno::Reference< css::document::XDocumentProperties > xDocProps( xDocPropSupp->getDocumentProperties(), uno::UNO_SET_THROW );
    xDocProps->setTemplateURL( aURL );
}

uno::Any SAL_CALL
SwVbaDocument::Tables( const css::uno::Any& aIndex )
{
    uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xColl( new SwVbaTables( mxParent, mxContext, xModel ) );

    if ( aIndex.hasValue() )
        return xColl->Item( aIndex, uno::Any() );
    return uno::Any( xColl );
}

void SAL_CALL SwVbaDocument::Activate()
{
    VbaDocumentBase::Activate();
}

::sal_Int32 SAL_CALL SwVbaDocument::getProtectionType()
{
    //TODO
    return word::WdProtectionType::wdNoProtection;
}

void SAL_CALL SwVbaDocument::setProtectionType( ::sal_Int32 /*_protectiontype*/ )
{
    //TODO
}

sal_Bool SAL_CALL SwVbaDocument::getUpdateStylesOnOpen()
{
    //TODO
    return false;
}

void SAL_CALL SwVbaDocument::setUpdateStylesOnOpen( sal_Bool /*_updatestylesonopen*/ )
{
    //TODO
}

sal_Bool SAL_CALL SwVbaDocument::getAutoHyphenation()
{
    // check this property only in default paragraph style
    bool IsAutoHyphenation = false;
    uno::Reference< beans::XPropertySet > xParaProps( word::getDefaultParagraphStyle( getModel() ), uno::UNO_QUERY_THROW );
    xParaProps->getPropertyValue(u"ParaIsHyphenation"_ustr) >>= IsAutoHyphenation;
    return IsAutoHyphenation;
}

void SAL_CALL SwVbaDocument::setAutoHyphenation( sal_Bool _autohyphenation )
{
    //TODO
    uno::Reference< beans::XPropertySet > xParaProps( word::getDefaultParagraphStyle( getModel() ), uno::UNO_QUERY_THROW );
    xParaProps->setPropertyValue(u"ParaIsHyphenation"_ustr, uno::Any( _autohyphenation ) );
}

::sal_Int32 SAL_CALL SwVbaDocument::getHyphenationZone()
{
    //TODO
    return 0;
}

void SAL_CALL SwVbaDocument::setHyphenationZone( ::sal_Int32 /*_hyphenationzone*/ )
{
    //TODO
}

::sal_Int32 SAL_CALL SwVbaDocument::getConsecutiveHyphensLimit()
{
    //TODO
    sal_Int16 nHyphensLimit = 0;
    uno::Reference< beans::XPropertySet > xParaProps( word::getDefaultParagraphStyle( getModel() ), uno::UNO_QUERY_THROW );
    xParaProps->getPropertyValue(u"ParaHyphenationMaxHyphens"_ustr) >>= nHyphensLimit;
    return nHyphensLimit;
}

void SAL_CALL SwVbaDocument::setConsecutiveHyphensLimit( ::sal_Int32 _consecutivehyphenslimit )
{
    sal_Int16 nHyphensLimit = static_cast< sal_Int16 >( _consecutivehyphenslimit );
    uno::Reference< beans::XPropertySet > xParaProps( word::getDefaultParagraphStyle( getModel() ), uno::UNO_QUERY_THROW );
    xParaProps->setPropertyValue(u"ParaHyphenationMaxHyphens"_ustr, uno::Any( nHyphensLimit ) );
}

uno::Reference< ooo::vba::word::XMailMerge > SAL_CALL SwVbaDocument::getMailMerge()
{
    return SwVbaMailMerge::get(mxParent, mxContext);
}

void SAL_CALL SwVbaDocument::Protect( ::sal_Int32 /*Type*/, const uno::Any& /*NOReset*/, const uno::Any& /*Password*/, const uno::Any& /*UseIRM*/, const uno::Any& /*EnforceStyleLock*/ )
{
    // Seems not support in Writer
    // VbaDocumentBase::Protect( Password );
}

void SAL_CALL SwVbaDocument::PrintOut( const uno::Any& /*Background*/, const uno::Any& /*Append*/, const uno::Any& /*Range*/, const uno::Any& /*OutputFileName*/, const uno::Any& /*From*/, const uno::Any& /*To*/, const uno::Any& /*Item*/, const uno::Any& /*Copies*/, const uno::Any& /*Pages*/, const uno::Any& /*PageType*/, const uno::Any& /*PrintToFile*/, const uno::Any& /*Collate*/, const uno::Any& /*FileName*/, const uno::Any& /*ActivePrinterMacGX*/, const uno::Any& /*ManualDuplexPrint*/, const uno::Any& /*PrintZoomColumn*/, const uno::Any& /*PrintZoomRow*/, const uno::Any& /*PrintZoomPaperWidth*/, const uno::Any& /*PrintZoomPaperHeight*/ )
{
    //TODO
}

void SAL_CALL SwVbaDocument::PrintPreview(  )
{
    dispatchRequests( mxModel,u".uno:PrintPreview"_ustr );
}

void SAL_CALL SwVbaDocument::ClosePrintPreview(  )
{
    dispatchRequests( mxModel,u".uno:ClosePreview"_ustr );
}

uno::Any SAL_CALL
SwVbaDocument::Revisions( const uno::Any& index )
{
    uno::Reference< css::document::XRedlinesSupplier > xRedlinesSupp( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xRedlines( xRedlinesSupp->getRedlines(), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xCol( new SwVbaRevisions( this, mxContext, getModel(), xRedlines ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaDocument::Frames( const uno::Any& index )
{
    uno::Reference< text::XTextFramesSupplier > xTextFramesSupp( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xFrames( xTextFramesSupp->getTextFrames(), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xCol( new SwVbaFrames( this, mxContext, xFrames, getModel() ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

void SAL_CALL
SwVbaDocument::SaveAs2000( const uno::Any& FileName, const uno::Any& FileFormat, const uno::Any& /*LockComments*/, const uno::Any& /*Password*/, const uno::Any& /*AddToRecentFiles*/, const uno::Any& /*WritePassword*/, const uno::Any& /*ReadOnlyRecommended*/, const uno::Any& /*EmbedTrueTypeFonts*/, const uno::Any& /*SaveNativePictureFormat*/, const uno::Any& /*SaveFormsData*/, const uno::Any& /*SaveAsAOCELetter*/ )
{
    SAL_INFO("sw.vba", "Document.SaveAs2000(FileName:=" << FileName << ",FileFormat:=" << FileFormat << ")");

    // Based on ScVbaWorkbook::SaveAs.
    OUString sFileName;
    FileName >>= sFileName;
    OUString sURL;
    osl::FileBase::getFileURLFromSystemPath( sFileName, sURL );

    // Detect if there is no path then we need to use the current folder.
    INetURLObject aURL( sURL );
    sURL = aURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
    if( sURL.isEmpty() )
    {
        // Need to add cur dir ( of this document ) or else the 'Work' dir
        sURL = getModel()->getURL();

        if ( sURL.isEmpty() )
        {
            // Not path available from 'this' document. Need to add the 'document'/work directory then.
            // Based on SwVbaOptions::getValueEvent()
            uno::Reference< util::XPathSettings > xPathSettings = util::thePathSettings::get( comphelper::getProcessComponentContext() );
            OUString sPathUrl;
            xPathSettings->getPropertyValue( u"Work"_ustr ) >>= sPathUrl;
            // Path could be a multipath, Microsoft doesn't support this feature in Word currently.
            // Only the last path is from interest.
            sal_Int32 nIndex = sPathUrl.lastIndexOf( ';' );
            if( nIndex != -1 )
            {
                sPathUrl = sPathUrl.copy( nIndex + 1 );
            }

            aURL.SetURL( sPathUrl );
        }
        else
        {
            aURL.SetURL( sURL );
            aURL.Append( sFileName );
        }
        sURL = aURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );

    }

    sal_Int32 nFileFormat = word::WdSaveFormat::wdFormatDocument;
    FileFormat >>= nFileFormat;

    uno::Sequence storeProps{ comphelper::makePropertyValue(u"FilterName"_ustr, uno::Any()) };

    setFilterPropsFromFormat( nFileFormat, storeProps );

    uno::Reference< frame::XStorable > xStor( getModel(), uno::UNO_QUERY_THROW );
    xStor->storeAsURL( sURL, storeProps );
}

void SAL_CALL
SwVbaDocument::SaveAs( const uno::Any& FileName, const uno::Any& FileFormat, const uno::Any& LockComments, const uno::Any& Password, const uno::Any& AddToRecentFiles, const uno::Any& WritePassword, const uno::Any& ReadOnlyRecommended, const uno::Any& EmbedTrueTypeFonts, const uno::Any& SaveNativePictureFormat, const uno::Any& SaveFormsData, const uno::Any& SaveAsAOCELetter, const uno::Any& /*Encoding*/, const uno::Any& /*InsertLineBreaks*/, const uno::Any& /*AllowSubstitutions*/, const uno::Any& /*LineEnding*/, const uno::Any& /*AddBiDiMarks*/ )
{
    return SaveAs2000( FileName, FileFormat, LockComments, Password, AddToRecentFiles, WritePassword, ReadOnlyRecommended, EmbedTrueTypeFonts, SaveNativePictureFormat, SaveFormsData, SaveAsAOCELetter );
}

void SAL_CALL
SwVbaDocument::Close( const uno::Any& SaveChanges, const uno::Any& /*OriginalFormat*/, const uno::Any& /*RouteDocument*/ )
{
    VbaDocumentBase::Close( SaveChanges, uno::Any(), uno::Any() );
}

void SAL_CALL
SwVbaDocument::SavePreviewPngAs( const uno::Any& FileName )
{
    OUString sFileName;
    FileName >>= sFileName;
    OUString sURL;
    osl::FileBase::getFileURLFromSystemPath( sFileName, sURL );

    uno::Sequence storeProps{ comphelper::makePropertyValue(u"FilterName"_ustr,
                                                            u"writer_png_Export"_ustr) };

    uno::Reference< frame::XStorable > xStor( getModel(), uno::UNO_QUERY_THROW );
    xStor->storeToURL( sURL, storeProps );
}

uno::Any
SwVbaDocument::getControlShape( std::u16string_view sName )
{
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndexAccess( xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW );

    sal_Int32 nCount = xIndexAccess->getCount();
    for( int index = 0; index < nCount; index++ )
    {
        uno::Any aUnoObj =  xIndexAccess->getByIndex( index );
        // It seems there are some drawing objects that can not query into Control shapes?
        uno::Reference< drawing::XControlShape > xControlShape( aUnoObj, uno::UNO_QUERY );
        if( xControlShape.is() )
        {
            uno::Reference< container::XNamed > xNamed( xControlShape->getControl(), uno::UNO_QUERY_THROW );
            if( sName == xNamed->getName() )
            {
                return aUnoObj;
            }
        }
    }
    return uno::Any();
}

uno::Reference< beans::XIntrospectionAccess > SAL_CALL
SwVbaDocument::getIntrospection(  )
{
    return uno::Reference< beans::XIntrospectionAccess >();
}

uno::Any SAL_CALL
SwVbaDocument::invoke( const OUString& aFunctionName, const uno::Sequence< uno::Any >& /*aParams*/, uno::Sequence< ::sal_Int16 >& /*aOutParamIndex*/, uno::Sequence< uno::Any >& /*aOutParam*/ )
{
    SAL_INFO("sw.vba", "** will barf " << aFunctionName );
    throw uno::RuntimeException(); // unsupported operation
}

void SAL_CALL
SwVbaDocument::setValue( const OUString& /*aPropertyName*/, const uno::Any& /*aValue*/ )
{
    throw uno::RuntimeException(); // unsupported operation
}
uno::Any SAL_CALL
SwVbaDocument::getValue( const OUString& aPropertyName )
{
    uno::Reference< drawing::XControlShape > xControlShape( getControlShape( aPropertyName ), uno::UNO_QUERY_THROW );

    uno::Reference<lang::XMultiComponentFactory > xServiceManager( mxContext->getServiceManager(), uno::UNO_SET_THROW );
    uno::Reference< XControlProvider > xControlProvider( xServiceManager->createInstanceWithContext(u"ooo.vba.ControlProvider"_ustr, mxContext ), uno::UNO_QUERY_THROW );
    uno::Reference< msforms::XControl > xControl( xControlProvider->createControl(  xControlShape, getModel() ) );
    return uno::Any( xControl );
}

sal_Bool SAL_CALL
SwVbaDocument::hasMethod( const OUString& /*aName*/ )
{
    return false;
}

sal_Bool SAL_CALL
SwVbaDocument::hasProperty( const OUString& aName )
{
    uno::Reference< container::XNameAccess > xFormControls( getFormControls() );
    if ( xFormControls.is() )
        return xFormControls->hasByName( aName );
    return false;
}

uno::Reference< container::XNameAccess >
SwVbaDocument::getFormControls() const
{
    uno::Reference< container::XNameAccess > xFormControls;
    try
    {
        uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( mxTextDocument, uno::UNO_QUERY_THROW );
        uno::Reference< form::XFormsSupplier >  xFormSupplier( xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xIndexAccess( xFormSupplier->getForms(), uno::UNO_QUERY_THROW );
        // get the www-standard container ( maybe we should access the
        // 'www-standard' by name rather than index, this seems an
        // implementation detail
        xFormControls.set( xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW );
    }
    catch(const uno::Exception&)
    {
    }
    return xFormControls;
}

// XInterfaceWithIID

OUString SAL_CALL
SwVbaDocument::getIID()
{
    return u"{82154424-0FBF-11d4-8313-005004526AB4}"_ustr;
}

// XConnectable

OUString SAL_CALL
SwVbaDocument::GetIIDForClassItselfNotCoclass()
{
    return u"{82154428-0FBF-11D4-8313-005004526AB4}"_ustr;
}

TypeAndIID SAL_CALL
SwVbaDocument::GetConnectionPoint()
{
    TypeAndIID aResult =
        { cppu::UnoType<word::XDocumentOutgoing>::get(),
          u"{82154429-0FBF-11D4-8313-005004526AB4}"_ustr
        };

    return aResult;
}

// XSinkCaller

void SAL_CALL
SwVbaDocument::CallSinks( const OUString& Method, uno::Sequence< uno::Any >& Arguments )
{
    for (auto& i : mvSinks)
    {
        if (i.is())
            i->Call(Method, Arguments);
    }
}

uno::Reference<XConnectionPoint> SAL_CALL
SwVbaDocument::FindConnectionPoint()
{
    uno::Reference<XConnectionPoint> xCP(new SwVbaDocumentOutgoingConnectionPoint(this));
    return xCP;
}

// SwVbaApplicationOutgoingConnectionPoint

SwVbaDocumentOutgoingConnectionPoint::SwVbaDocumentOutgoingConnectionPoint( SwVbaDocument* pDoc ) :
    mpDoc(pDoc)
{
}

// XConnectionPoint

sal_uInt32 SAL_CALL
SwVbaDocumentOutgoingConnectionPoint::Advise( const uno::Reference< XSink >& Sink )
{
    return mpDoc->AddSink(Sink);
}

void SAL_CALL
SwVbaDocumentOutgoingConnectionPoint::Unadvise( sal_uInt32 Cookie )
{
    mpDoc->RemoveSink( Cookie );
}

uno::Sequence< OUString >
SwVbaDocument::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Document"_ustr
    };
    return aServiceNames;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Writer_SwVbaDocument_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& args)
{
    return cppu::acquire(new SwVbaDocument(args, context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
