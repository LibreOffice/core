/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <vcl/bitmapex.hxx>
#include <vcl/imagerepository.hxx>
#include <vcl/svapp.hxx>
#include <vcl/image.hxx>
#include <vcl/pngread.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>

#include "impimagetree.hxx"

namespace vcl
{
    bool ImageRepository::loadImage( const ::rtl::OUString& _rName,
                                     BitmapEx& _out_rImage,
                                     bool _bSearchLanguageDependent )
    {
        ::rtl::OUString sCurrentSymbolsStyle =
            Application::GetSettings().GetStyleSettings().GetCurrentSymbolsStyleName();

        ImplImageTreeSingletonRef aImplImageTree;

        return aImplImageTree->loadImage( _rName,
                                          sCurrentSymbolsStyle,
                                          _out_rImage,
                                          _bSearchLanguageDependent );
    }


    static bool lcl_loadPNG( const rtl::OUString &rPath,
                             const rtl::OUString &rImageFileName,
                             Image &rImage )
    {
        INetURLObject aObj( rPath );
        aObj.insertName( rImageFileName );
        SvFileStream aStrm( aObj.PathToFileName(), STREAM_STD_READ );
        if ( !aStrm.GetError() )
        {
            PNGReader aReader( aStrm );
            BitmapEx aBmp = aReader.Read();
            if (!aBmp.IsEmpty())
            {
                rImage = Image( aBmp );
                return true;
            }
        }

        return false;
    }

    /* TODO support bSearchLanguageDependent */
    bool ImageRepository::loadBrandingImage( const rtl::OUString &rName,
                                             Image &rImage,
                                             bool /* bSearchLanguageDependent */ )
    {
        rtl::OUString sImages;
        rtl::OUStringBuffer aBuff( rName );
        rtl::OUString aBasePath( RTL_CONSTASCII_USTRINGPARAM( "$BRAND_BASE_DIR/program" ) );
        rtl::Bootstrap::expandMacros( aBasePath );

        bool bLoaded = false;
        sal_Int32 nIndex = 0;

        if ( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        {
            aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( "_hc.png," ) );
            aBuff.append( rName );
        }

        aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( ".png" ) );
        sImages = aBuff.makeStringAndClear();

        do
        {
            bLoaded = lcl_loadPNG( aBasePath,
                                   sImages.getToken( 0, ',', nIndex ),
                                   rImage );
        }
        while ( !bLoaded && ( nIndex >= 0 ) );

        return bLoaded;
    }
}
