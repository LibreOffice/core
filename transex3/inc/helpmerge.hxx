/*************************************************************************
 *
 *  $RCSfile: helpmerge.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-30 17:28:37 $
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
