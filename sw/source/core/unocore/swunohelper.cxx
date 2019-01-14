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

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/extract.hxx>
#include <o3tl/any.hxx>
#include <tools/urlobj.hxx>
#include <tools/datetime.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/content.hxx>
#include <swunohelper.hxx>
#include <svx/xfillit0.hxx>
#include <editeng/memberids.h>
#include <svl/itemset.hxx>

using namespace com::sun::star;

namespace SWUnoHelper
{

sal_Int32 GetEnumAsInt32( const css::uno::Any& rVal )
{
    sal_Int32 nReturn = 0;
    if (! ::cppu::enum2int(nReturn,rVal) )
         OSL_FAIL( "can't get EnumAsInt32" );
    return nReturn;
}

// methods for UCB actions
bool UCB_DeleteFile( const OUString& rURL )
{
    bool bRemoved;
    try
    {
        ucbhelper::Content aTempContent( rURL,
                                css::uno::Reference< css::ucb::XCommandEnvironment >(),
                                comphelper::getProcessComponentContext() );
        aTempContent.executeCommand("delete", css::uno::makeAny( true ) );
        bRemoved = true;
    }
    catch( css::uno::Exception& )
    {
        bRemoved = false;
        OSL_FAIL( "Exception from executeCommand( delete )" );
    }
    return bRemoved;
}

bool UCB_MoveFile( const OUString& rURL, const OUString& rNewURL )
{
    bool bCopyCompleted = true;
    try
    {
        INetURLObject aURL( rNewURL );
        const OUString sName( aURL.GetName() );
        aURL.removeSegment();
        const OUString sMainURL( aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE) );

        ucbhelper::Content aTempContent( sMainURL,
                                css::uno::Reference< css::ucb::XCommandEnvironment >(),
                                comphelper::getProcessComponentContext() );

        css::ucb::TransferInfo aInfo;
        aInfo.NameClash = css::ucb::NameClash::ERROR;
        aInfo.NewTitle = sName;
        aInfo.SourceURL = rURL;
        aInfo.MoveData = true;
        aTempContent.executeCommand( "transfer", uno::Any(aInfo) );
    }
    catch( css::uno::Exception& )
    {
        OSL_FAIL( "Exception from executeCommand( transfer )" );
        bCopyCompleted = false;
    }
    return bCopyCompleted;
}

bool UCB_IsCaseSensitiveFileName( const OUString& rURL )
{
    bool bCaseSensitive;
    try
    {
        INetURLObject aTempObj( rURL );
        aTempObj.SetBase( aTempObj.GetBase().toAsciiLowerCase() );
        css::uno::Reference< css::ucb::XContentIdentifier > xRef1 = new
                ucbhelper::ContentIdentifier( aTempObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ));

        aTempObj.SetBase(aTempObj.GetBase().toAsciiUpperCase());
        css::uno::Reference< css::ucb::XContentIdentifier > xRef2 = new
                ucbhelper::ContentIdentifier( aTempObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ));

        css::uno::Reference< css::ucb::XUniversalContentBroker > xUcb =
              css::ucb::UniversalContentBroker::create(comphelper::getProcessComponentContext());

        sal_Int32 nCompare = xUcb->compareContentIds( xRef1, xRef2 );
        bCaseSensitive = 0 != nCompare;
    }
    catch( css::uno::Exception& )
    {
        bCaseSensitive = false;
        OSL_FAIL( "Exception from compareContentIds()" );
    }
    return bCaseSensitive;
}

bool UCB_IsReadOnlyFileName( const OUString& rURL )
{
    bool bIsReadOnly = false;
    try
    {
        ucbhelper::Content aCnt( rURL, css::uno::Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        css::uno::Any aAny = aCnt.getPropertyValue("IsReadOnly");
        if(aAny.hasValue())
            bIsReadOnly = *o3tl::doAccess<bool>(aAny);
    }
    catch( css::uno::Exception& )
    {
        bIsReadOnly = false;
    }
    return bIsReadOnly;
}

bool UCB_IsFile( const OUString& rURL )
{
    bool bExists = false;
    try
    {
        ::ucbhelper::Content aContent( rURL, css::uno::Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        bExists = aContent.isDocument();
    }
    catch (css::uno::Exception &)
    {
    }
    return bExists;
}

bool UCB_IsDirectory( const OUString& rURL )
{
    bool bExists = false;
    try
    {
        ::ucbhelper::Content aContent( rURL, css::uno::Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        bExists = aContent.isFolder();
    }
    catch (css::uno::Exception &)
    {
    }
    return bExists;
}

    // get a list of files from the folder of the URL
    // options: pExtension = 0 -> all, else this specific extension
    //          pDateTime != 0 -> returns also the modified date/time of
    //                       the files in a std::vector<OUString> -->
    //                       !! objects must be deleted from the caller!!
bool UCB_GetFileListOfFolder( const OUString& rURL,
                                std::vector<OUString>& rList,
                                const OUString* pExtension,
                                std::vector< ::DateTime >* pDateTimeList )
{
    bool bOk = false;
    try
    {
        ucbhelper::Content aCnt( rURL, css::uno::Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        css::uno::Reference< css::sdbc::XResultSet > xResultSet;

        const sal_Int32 nSeqSize = pDateTimeList ? 2 : 1;
        css::uno::Sequence < OUString > aProps( nSeqSize );
        OUString* pProps = aProps.getArray();
        pProps[ 0 ] = "Title";
        if( pDateTimeList )
            pProps[ 1 ] = "DateModified";

        try
        {
            xResultSet = aCnt.createCursor( aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY );
        }
        catch( css::uno::Exception& )
        {
            OSL_FAIL( "create cursor failed!" );
        }

        if( xResultSet.is() )
        {
            css::uno::Reference< css::sdbc::XRow > xRow( xResultSet, css::uno::UNO_QUERY );
            const sal_Int32 nExtLen = pExtension ? pExtension->getLength() : 0;
            try
            {
                if( xResultSet->first() )
                {
                    do {
                        const OUString sTitle( xRow->getString( 1 ) );
                        if( !nExtLen ||
                            ( sTitle.getLength() > nExtLen &&
                              sTitle.endsWith( *pExtension )) )
                        {
                            rList.push_back( sTitle );

                            if( pDateTimeList )
                            {
                                css::util::DateTime aStamp = xRow->getTimestamp(2);
                                ::DateTime aDateTime(
                                        ::Date( aStamp.Day,
                                                aStamp.Month,
                                                aStamp.Year ),
                                        ::tools::Time( aStamp.Hours,
                                                aStamp.Minutes,
                                                aStamp.Seconds,
                                                aStamp.NanoSeconds ));
                                pDateTimeList->push_back( aDateTime );
                            }
                        }

                    } while( xResultSet->next() );
                }
                bOk = true;
            }
            catch( css::uno::Exception& )
            {
                OSL_FAIL( "Exception caught!" );
            }
        }
    }
    catch( css::uno::Exception& )
    {
        OSL_FAIL( "Exception caught!" );
        bOk = false;
    }
    return bOk;
}

bool needToMapFillItemsToSvxBrushItemTypes(const SfxItemSet& rSet,
        sal_uInt16 const nMID)
{
    const XFillStyleItem* pXFillStyleItem(rSet.GetItem<XFillStyleItem>(XATTR_FILLSTYLE, false));

    if(!pXFillStyleItem)
    {
        return false;
    }

    // here different FillStyles can be excluded for export; it will depend on the
    // quality these fallbacks can reach. That again is done in getSvxBrushItemFromSourceSet,
    // take a look there how the superset of DrawObject FillStyles is mapped to SvxBrushItem.
    const drawing::FillStyle eFill = pXFillStyleItem->GetValue();
    switch (eFill)
    {
        case drawing::FillStyle_NONE:
            // claim that BackColor and BackTransparent are available so that
            // fo:background="transparent" attribute is exported to override
            // the parent style in case it is != NONE
            switch (nMID)
            {
                case MID_BACK_COLOR:
                case MID_BACK_COLOR_R_G_B:
                case MID_GRAPHIC_TRANSPARENT: // this is *BackTransparent
                    return true;
                default:
                    return false;
            }
            break;
        case drawing::FillStyle_SOLID:
        case drawing::FillStyle_GRADIENT: // gradient and hatch don't exist in
        case drawing::FillStyle_HATCH: // SvxBrushItem so average color is emulated
            switch (nMID)
            {
                case MID_BACK_COLOR:
                case MID_GRAPHIC_TRANSPARENT: // this is *BackTransparent
                    // Gradient/Hatch always have emulated color
                    return (drawing::FillStyle_SOLID != eFill)
                        || SfxItemState::SET == rSet.GetItemState(XATTR_FILLCOLOR)
                        || SfxItemState::SET == rSet.GetItemState(XATTR_FILLTRANSPARENCE)
                        || SfxItemState::SET == rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE);
                case MID_BACK_COLOR_R_G_B:
                    // Gradient/Hatch always have emulated color
                    return (drawing::FillStyle_SOLID != eFill)
                        || SfxItemState::SET == rSet.GetItemState(XATTR_FILLCOLOR);
                case MID_BACK_COLOR_TRANSPARENCY:
                    return SfxItemState::SET == rSet.GetItemState(XATTR_FILLTRANSPARENCE)
                        || SfxItemState::SET == rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE);
            }
            break;
        case drawing::FillStyle_BITMAP:
            switch (nMID)
            {
                case MID_GRAPHIC:
                    return SfxItemState::SET == rSet.GetItemState(XATTR_FILLBITMAP);
                case MID_GRAPHIC_POSITION:
                    return SfxItemState::SET == rSet.GetItemState(XATTR_FILLBMP_STRETCH)
                        || SfxItemState::SET == rSet.GetItemState(XATTR_FILLBMP_TILE)
                        || SfxItemState::SET == rSet.GetItemState(XATTR_FILLBMP_POS);
                case MID_GRAPHIC_TRANSPARENT:
                case MID_GRAPHIC_TRANSPARENCY:
                    return SfxItemState::SET == rSet.GetItemState(XATTR_FILLTRANSPARENCE)
                        || SfxItemState::SET == rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE);
            }
            break;
        default:
            assert(false);
    }


    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
