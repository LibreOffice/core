/*************************************************************************
 *
 *  $RCSfile: htmlexp.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:13 $
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

#ifndef SC_HTMLEXP_HXX
#define SC_HTMLEXP_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef _RTL_TEXTENC_H //autogen wg. rtl_TextEncoding
#include <rtl/textenc.h>
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#include "expbase.hxx"


class ScDocument;
class SfxItemSet;
class SdrPage;
class Graphic;
class SdrObject;
class OutputDevice;
class ScDrawLayer;
class SvStringsSortDtor;
class ScEditCell;

struct ScHTMLStyle
{   // Defaults aus StyleSheet
    Color               aBackgroundColor;
    String              aFontFamilyName;
    UINT32              nFontHeight;        // Item-Value

    const ScHTMLStyle& operator=( const ScHTMLStyle& r )
        {
            aBackgroundColor    = r.aBackgroundColor;
            aFontFamilyName     = r.aFontFamilyName;
            nFontHeight         = r.nFontHeight;
            return *this;
        }
};

struct ScHTMLGraphEntry
{
    ScRange             aRange;         // ueberlagerter Zellbereich
    Size                aSize;          // Groesse in Pixeln
    Size                aSpace;         // Spacing in Pixeln
    SdrObject*          pObject;
    BOOL                bInCell;        // ob in Zelle ausgegeben wird
    BOOL                bWritten;

    ScHTMLGraphEntry( SdrObject* pObj, const ScRange& rRange,
        const Size& rSize,  BOOL bIn, const Size& rSpace ) :
        aRange( rRange ), pObject( pObj ), aSize( rSize ), bInCell( bIn ),
        aSpace( rSpace ), bWritten( FALSE ) {}
};

DECLARE_LIST( ScHTMLGraphList, ScHTMLGraphEntry* );

#define SC_HTML_FONTSIZES 7
const short nIndentMax = 23;

class ScHTMLExport : public ScExportBase
{
    // default HtmlFontSz[1-7]
    static const USHORT nDefaultFontSize[SC_HTML_FONTSIZES];
    // HtmlFontSz[1-7] in s*3.ini [user]
    static USHORT       nFontSize[SC_HTML_FONTSIZES];
    static const USHORT nCellSpacing;
    static const sal_Char __FAR_DATA sIndentSource[];

    ScHTMLGraphList     aGraphList;
    ScHTMLStyle         aHTMLStyleGlobal;
    ScHTMLStyle         aHTMLStyle;
    String              aStreamPath;
    String              aCId;           // Content-Id fuer Mail-Export
    OutputDevice*       pAppWin;        // fuer Pixelei
    SvStringsSortDtor*  pSrcArr;        // fuer CopyLocalFileToINet
    SvStringsSortDtor*  pDestArr;
    rtl_TextEncoding    eDestEnc;
    USHORT              nUsedTables;
    short               nIndent;
    sal_Char            sIndent[nIndentMax+1];
    BOOL                bAll;           // ganzes Dokument
    BOOL                bTabHasGraphics;
    BOOL                bTabAlignedLeft;
    BOOL                bCalcAsShown;
    BOOL                bCopyLocalFileToINet;

    const SfxItemSet&   PageDefaults( USHORT nTab );

    void                WriteBody();
    void                WriteHeader();
    void                WriteOverview();
    void                WriteTables();
    void                WriteCell( USHORT nCol, USHORT nRow, USHORT nTab );
    void                WriteGraphEntry( ScHTMLGraphEntry* );
    void                WriteImage( String& rLinkName,
                                    const Graphic&, const ByteString& rImgOptions,
                                    ULONG nXOutFlags = 0 );
                            // nXOutFlags fuer XOutBitmap::WriteGraphic

                        // write to stream if and only if URL fields in edit cell
    BOOL                WriteFieldText( const ScEditCell* pCell );

                        // kopiere ggfs. eine lokale Datei ins Internet
    BOOL                CopyLocalFileToINet( String& rFileNm,
                            const String& rTargetNm, BOOL bFileToFile = FALSE );
    BOOL                HasCId() { return aCId.Len() > 0; }
    void                MakeCIdURL( String& rURL );

    void                PrepareGraphics( ScDrawLayer*, USHORT nTab,
                                        USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow );
    void                FillGraphList( const SdrPage*, USHORT nTab,
                                        USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow );

    BOOL                HasBottomBorder( USHORT nRow, USHORT nTab,
                                        USHORT nStartCol, USHORT nEndCol );
    BOOL                HasLeftBorder( USHORT nCol, USHORT nTab,
                                        USHORT nStartRow, USHORT nEndRow );
    BOOL                HasTopBorder( USHORT nRow, USHORT nTab,
                                        USHORT nStartCol, USHORT nEndCol );
    BOOL                HasRightBorder( USHORT nCol, USHORT nTab,
                                        USHORT nStartRow, USHORT nEndRow );

    USHORT              GetFontSizeNumber( USHORT nHeight );
    USHORT              ToPixel( USHORT nTwips );
    Size                MMToPixel( const Size& r100thMMSize );
    void                IncIndent( short nVal );
    const sal_Char*         GetIndentStr() { return sIndent; }

public:
                        ScHTMLExport( SvStream&, ScDocument*, const ScRange&,
                                        BOOL bAll, const String& aStreamPath );
    virtual             ~ScHTMLExport();
    ULONG               Write();
};

#endif

