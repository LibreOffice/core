/*************************************************************************
 *
 *  $RCSfile: unoidl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:35 $
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


#include <precomp.h>
#include <parser/unoidl.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/file.hxx>
#include <ary/ary.hxx>
#include <ary_i/codeinf2.hxx>
#include <ary_i/uidl/gate.hxx>
#include <tools/filecoll.hxx>
#include <tools/tkpchars.hxx>
#include <s2_luidl/tkp_uidl.hxx>
#include <s2_luidl/distrib.hxx>
#include <s2_luidl/pe_file2.hxx>
#include <s2_dsapi/cx_dsapi.hxx>



namespace csi
{
namespace uidl
{


class FileParsePerformers
{
  public:
                        FileParsePerformers( ary::Repository & io_rRepository );

    void                ParseFile(
                            const char *        i_sFullPath );

    void                ConnectLinks();

  private:
    CharacterSource     aFileLoader;
    Dyn<TokenParser_Uidl>
                        pTokens;
    TokenDistributor    aDistributor;
    Dyn<PE_File>        pFileParseEnvironment;
    ary::Repository &   rRepository;
};


Uidl_Parser::Uidl_Parser( ary::Repository & io_rRepository )
    :   pRepository(&io_rRepository)
{
}

void
Uidl_Parser::Run( const autodoc::FileCollector_Ifc & i_rFiles )
{
    FileParsePerformers aFileParsePerformers( *pRepository );

    FileCollector::const_iterator iEnd = i_rFiles.End();
    for ( FileCollector::const_iterator iter = i_rFiles.Begin();
          iter != iEnd;
          ++iter )
    {
        Cout() << (*iter) << " ..."<< Endl();
        aFileParsePerformers.ParseFile(*iter);
    }

    aFileParsePerformers.ConnectLinks();
}

FileParsePerformers::FileParsePerformers( ary::Repository & io_rRepository )
    :   aDistributor(io_rRepository),
        rRepository( io_rRepository )
{
    DYN csi::dsapi::Context_Docu * dpDocuContext
            = new csi::dsapi::Context_Docu( aDistributor.DocuTokens_Receiver() );
    pTokens = new TokenParser_Uidl( aDistributor.CodeTokens_Receiver(), *dpDocuContext );
    pFileParseEnvironment
            = new PE_File(aDistributor);

    aDistributor.SetTokenProvider(*pTokens);
    aDistributor.SetTopParseEnvironment(*pFileParseEnvironment);
}

void
FileParsePerformers::ParseFile( const char * i_sFullPath )
{
    csv::File aFile(i_sFullPath);

    aFile.open( csv::CFM_READ );
    csv_assert( aFile.is_open() );
    aFileLoader.LoadText(aFile);
    aFile.close();

    pTokens->Start(aFileLoader);
    aDistributor.Reset();

    do {
        aDistributor.TradeToken();
    } while ( NOT aFileLoader.IsFinished() );
}

void
FileParsePerformers::ConnectLinks()
{
    rRepository.RwGate_Idl().ConnectAdditionalLinks();
}

}   // namespace uidl
}   // namespace csi


