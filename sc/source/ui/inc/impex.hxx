/*************************************************************************
 *
 *  $RCSfile: impex.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-08 20:50:39 $
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

#ifndef SC_IMPEX_HXX
#define SC_IMPEX_HXX

#ifndef _SOT_EXCHANGE_HXX //autogen
#include <sot/exchange.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

class SvDataObject;
class SvDataTypeList;
class ScDocShell;
class ScDocument;
class SvStream;
class SfxMedium;
class ScAsciiOptions;
class SvData;

class ScImportExport
{
    ScDocShell* pDocSh;
    ScDocument* pDoc;
    ScDocument* pUndoDoc;
    ScRange     aRange;
    String      aStreamPath;
    ULONG       nSizeLimit;
    sal_Unicode cSep;                   // Separator
    sal_Unicode cStr;                   // String Delimiter
    BOOL        bFormulas;              // Formeln im Text?
    BOOL        bAll;                   // keine Selektion
    BOOL        bSingle;                // Einfachselektion
    BOOL        bUndo;                  // Mit Undo?
    BOOL        bOverflow;              // zuviele Zeilen/Spalten

    ScAsciiOptions* pExtOptions;        // erweiterte Optionen

    BOOL StartPaste();                  // Protect-Check, Undo einrichten
    void EndPaste();                    // Undo/Redo-Aktionen, Repaint
    BOOL Doc2Text( SvStream& );
    BOOL Text2Doc( SvStream& );
    BOOL Doc2Sylk( SvStream& );
    BOOL Sylk2Doc( SvStream& );
    BOOL Doc2HTML( SvStream& );
    BOOL Doc2RTF( SvStream& );
    BOOL Doc2Dif( SvStream& );
    BOOL Dif2Doc( SvStream& );
    BOOL ExtText2Doc( SvStream& );      // mit pExtOptions
    BOOL RTF2Doc( SvStream& );
    BOOL HTML2Doc( SvStream& );

    //! only if stream is only used in own (!) memory
    static  inline  void    SetNoEndianSwap( SvStream& rStrm );

public:
    ScImportExport( ScDocument* );                  // Gesamtdokument
    ScImportExport( ScDocument*, const String& );   // Bereichs/Zellangabe
    ScImportExport( ScDocument*, const ScAddress& );
    ScImportExport( ScDocument*, const ScRange& );
   ~ScImportExport();

    void SetExtOptions( const ScAsciiOptions& rOpt );

    BOOL IsDoubleRef() const { return BOOL( !( bAll || bSingle ) ); }
    BOOL IsSingleRef() const { return bSingle; }
    BOOL IsNoRef() const     { return bAll;    }
    BOOL IsRef() const       { return BOOL( !bAll ); }

    const ScRange& GetRange() const { return aRange; }

    BOOL IsUndo() const      { return bUndo; }
    void SetUndo( BOOL b )   { bUndo = b;    }

    static void  AddFormats( SvDataTypeList* );
    static BOOL  IsFormatSupported( SvDataObject* );
    static BOOL  IsFormatSupported( ULONG nFormat );
    static const sal_Unicode* ScanNextFieldFromString( const sal_Unicode* p,
            String& rField, sal_Unicode cStr, const sal_Unicode* pSeps, BOOL bMergeSeps );
    static  void    WriteUnicodeOrByteString( SvStream& rStrm, const String& rString, BOOL bZero = FALSE );
    static  void    WriteUnicodeOrByteEndl( SvStream& rStrm );
    static  inline  BOOL    IsEndianSwap( const SvStream& rStrm );

    sal_Unicode GetSeparator() const { return cSep; }
    void SetSeparator( sal_Unicode c ) { cSep = c; }
    sal_Unicode GetDelimiter() const { return cStr; }
    void SetDelimiter( sal_Unicode c ) { cStr = c; }
    BOOL IsFormulas() const { return bFormulas; }
    void SetFormulas( BOOL b ) { bFormulas = b; }

    void SetSizeLimit( ULONG nNew ) { nSizeLimit = nNew; }  // momentan nur fuer Ascii

    void            SetStreamPath( const String& rPath ) { aStreamPath = rPath; }
    const String&   GetStreamPath() const { return aStreamPath; }

    BOOL ImportString( const String&, ULONG=FORMAT_STRING );
    BOOL ExportString( String&, ULONG=FORMAT_STRING );
    BOOL ExportByteString( ByteString&, rtl_TextEncoding, ULONG=FORMAT_STRING );

    BOOL ImportStream( SvStream&, ULONG=FORMAT_STRING );
    BOOL ExportStream( SvStream&, ULONG=FORMAT_STRING );

    BOOL ImportData( const String& rMimeType,
                     const ::com::sun::star::uno::Any & rValue );
    BOOL ExportData( const String& rMimeType,
                     ::com::sun::star::uno::Any & rValue  );

    BOOL ImportData( SvData& rData );
    BOOL ExportData( SvData& rData );

    BOOL IsOverflow() const { return bOverflow; }       // nach dem Importieren
};


// static
inline BOOL ScImportExport::IsEndianSwap( const SvStream& rStrm )
{
#ifdef __BIGENDIAN
    return rStrm.GetNumberFormatInt() != NUMBERFORMAT_INT_BIGENDIAN;
#else
    return rStrm.GetNumberFormatInt() != NUMBERFORMAT_INT_LITTLEENDIAN;
#endif
}


#endif

