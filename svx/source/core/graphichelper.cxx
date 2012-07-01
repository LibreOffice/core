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

#include <tools/string.hxx>
#include <svx/graphichelper.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/filter.hxx>
#include <sfx2/docfile.hxx>
#include <svx/xoutbmp.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::sfx2;

void GraphicHelper::GetPreferedExtension( String &rExtension, const Graphic &rGraphic )
{
    // propose the "best" filter using the native-info, if applicable
    const sal_Char* pExtension = "png";
    switch( const_cast<Graphic&>( rGraphic ).GetLink().GetType() )
    {
        case GFX_LINK_TYPE_NATIVE_GIF:
            pExtension = "gif";
            break;
        case GFX_LINK_TYPE_NATIVE_TIF:
            pExtension = "tif";
            break;
        case GFX_LINK_TYPE_NATIVE_WMF:
            pExtension = "wmf";
            break;
        case GFX_LINK_TYPE_NATIVE_MET:
            pExtension = "met";
            break;
        case GFX_LINK_TYPE_NATIVE_PCT:
            pExtension = "pct";
            break;
        case GFX_LINK_TYPE_NATIVE_JPG:
            pExtension = "jpg";
            break;
        default:
            break;
    }
    rExtension.AssignAscii( pExtension );
}

String GraphicHelper::ExportGraphic( const Graphic &rGraphic, const String &rGrfName )
{
    SvtPathOptions aPathOpt;
    String sGrfPath( aPathOpt.GetGraphicPath() );

    FileDialogHelper aDlgHelper( TemplateDescription::FILESAVE_AUTOEXTENSION, 0 );
    Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

    INetURLObject aPath;
    aPath.SetSmartURL( sGrfPath );

    // fish out the graphic's name
    String aName = rGrfName;

    aDlgHelper.SetTitle( SVX_RESSTR(RID_SVXSTR_EXPORT_GRAPHIC_TITLE));
    aDlgHelper.SetDisplayDirectory( aPath.GetMainURL(INetURLObject::DECODE_TO_IURI) );
    INetURLObject aURL;
    aURL.SetSmartURL( aName );
    aDlgHelper.SetFileName( aURL.GetName() );

    GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();
    const sal_uInt16 nCount = rGF.GetExportFormatCount();

    String aExt( aURL.GetExtension() );
    if( !aExt.Len() )
        GetPreferedExtension( aExt, rGraphic );

    aExt.ToLowerAscii();
    sal_uInt16 nDfltFilter = USHRT_MAX;

    Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);

    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        xFltMgr->appendFilter( rGF.GetExportFormatName( i ), rGF.GetExportWildcard( i ) );
        if ( COMPARE_EQUAL == aExt.CompareIgnoreCaseToAscii(rGF.GetExportFormatShortName( i ).ToLowerAscii() ))
            nDfltFilter = i;
    }
    if ( USHRT_MAX == nDfltFilter )
    {
        // "wrong" extension?
        GetPreferedExtension( aExt, rGraphic );
        for ( sal_uInt16 i = 0; i < nCount; ++i )
            if ( aExt == rGF.GetExportFormatShortName( i ).ToLowerAscii() )
            {
                nDfltFilter =  i;
                break;
            }
    }

    if( USHRT_MAX != nDfltFilter )
    {
        xFltMgr->setCurrentFilter( rGF.GetExportFormatName( nDfltFilter ) ) ;

        if( aDlgHelper.Execute() == ERRCODE_NONE )
        {
            String sPath( xFP->getFiles().getConstArray()[0] );
            // remember used path - please don't optimize away!
            aPath.SetSmartURL( sPath);
            sGrfPath = aPath.GetPath();

            if( rGrfName.Len() &&
                 nDfltFilter == rGF.GetExportFormatNumber( xFltMgr->getCurrentFilter()))
            {
                // try to save the original graphic
                SfxMedium aIn( rGrfName, STREAM_READ | STREAM_NOCREATE );
                if( aIn.GetInStream() && !aIn.GetInStream()->GetError() )
                {
                    SfxMedium aOut( sPath, STREAM_WRITE | STREAM_SHARE_DENYNONE);
                    if( aOut.GetOutStream() && !aOut.GetOutStream()->GetError())
                    {
                        *aOut.GetOutStream() << *aIn.GetInStream();
                        if ( 0 == aIn.GetError() )
                        {
                            aOut.Close();
                            aOut.Commit();
                            if ( 0 == aOut.GetError() )
                                return sPath;
                        }
                    }
                }
            }

            sal_uInt16 nFilter;
            if ( !xFltMgr->getCurrentFilter().isEmpty() && rGF.GetExportFormatCount() )
                nFilter = rGF.GetExportFormatNumber( xFltMgr->getCurrentFilter() );
            else
                nFilter = GRFILTER_FORMAT_DONTKNOW;
            String aFilter( rGF.GetExportFormatShortName( nFilter ) );
            XOutBitmap::WriteGraphic( rGraphic, sPath, aFilter,
                                        XOUTBMP_DONT_EXPAND_FILENAME|
                                        XOUTBMP_DONT_ADD_EXTENSION|
                                        XOUTBMP_USE_NATIVE_IF_POSSIBLE );
            return sPath;
        }
    }
    return String();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
