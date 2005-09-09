/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helpmerge.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:55:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
// local includes
#include "export.hxx"
#include "xmlparse.hxx"
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <hash_map>
#include <memory> /* auto_ptr */
#include "tools/isofallback.hxx"
#include <vector>



/// This Class is responsible for extracting/merging OpenOffice XML Helpfiles
class HelpParser
{
private:
    bool       bUTF8;
    ByteString sHelpFile;

/// Copy fallback language String (ENUS,DE) into position of the numeric language iso code
/// @PRECOND 0 < langIdx_in < MAX_IDX
    void FillInFallbacks( LangHashMap& rElem_out, ByteString sLangIdx_in );

/// Debugmethod, prints the content of the map to stdout
    static  void Dump(  LangHashMap* rElem_in , const ByteString sKey_in );

/// Debugmethod, prints the content of the map to stdout
    static  void Dump(  XMLHashMap* rElem_in ) ;

public:
    HelpParser( const ByteString &rHelpFile, bool bUTF8 );
    ~HelpParser(){};

/// Method creates/append a SDF file with the content of a parsed XML file
/// @PRECOND rHelpFile is valid
    bool CreateSDF( const ByteString &rSDFFile_in, const ByteString &rPrj_in, const ByteString &rRoot_in );

/// Method merges the String from the SDFfile into XMLfile. Both Strings must
/// point to existing files.
    bool Merge( const ByteString &rSDFFile_in, const ByteString &rDestinationFile_in );

    bool Merge( const ByteString &rSDFFile, const ByteString &rPathX , const ByteString &rPathY , bool bISO );

private:
    ByteString HelpParser::GetOutpath( const ByteString& rPathX , const ByteString& sCur , const ByteString& rPathY );
    void Process( LangHashMap* aLangHM , ByteString& sCur , ResData *pResData , MergeDataFile& aMergeDataFile );
    void ProcessHelp( LangHashMap* aLangHM , ByteString& sCur , ResData *pResData , MergeDataFile& aMergeDataFile );
    void HelpParser::MakeDir( const ByteString& sPath );
};
