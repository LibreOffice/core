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

#include <precomp.h>
#include <parser/unoidl.hxx>


// NOT FULLY DECLARED SERVICES
#include <stdlib.h>
#include <cosv/file.hxx>
#include <ary/ary.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <../parser/inc/x_docu.hxx>
#include <parser/parserinfo.hxx>
#include <tools/filecoll.hxx>
#include <tools/tkpchars.hxx>
#include <s2_luidl/tkp_uidl.hxx>
#include <s2_luidl/distrib.hxx>
#include <s2_luidl/pe_file2.hxx>
#include <s2_dsapi/cx_dsapi.hxx>
#include <adc_msg.hxx>
#include <x_parse2.hxx>



namespace autodoc
{


class FileParsePerformers
{
  public:
                        FileParsePerformers(
                            ary::Repository &
                                                io_rRepository,
                            ParserInfo &        io_rParserInfo );

    void                ParseFile(
                            const char *        i_sFullPath );

  private:
    CharacterSource     aFileLoader;
    Dyn<csi::uidl::TokenParser_Uidl>
                        pTokens;
    csi::uidl::TokenDistributor
                        aDistributor;
    Dyn<csi::uidl::PE_File>
                        pFileParseEnvironment;
    ary::Repository &
                        rRepository;
    ParserInfo &        rParserInfo;
};


IdlParser::IdlParser( ary::Repository & io_rRepository )
    :   pRepository(&io_rRepository)
{
}

void
IdlParser::Run( const autodoc::FileCollector_Ifc & i_rFiles )
{
    Dyn<FileParsePerformers>
        pFileParsePerformers(
            new FileParsePerformers(*pRepository,
                                    static_cast< ParserInfo& >(*this)) );

    FileCollector::const_iterator iEnd = i_rFiles.End();
    for ( FileCollector::const_iterator iter = i_rFiles.Begin();
          iter != iEnd;
          ++iter )
    {
        Cout() << (*iter) << " ..."<< Endl();

        try
        {
            pFileParsePerformers->ParseFile(*iter);
        }
        catch (X_AutodocParser &)
        {
            /// Ignore and goon
            TheMessages().Out_ParseError(CurFile(), CurLine());
            pFileParsePerformers
                = new FileParsePerformers(*pRepository,
                                          static_cast< ParserInfo& >(*this));
        }
        catch (X_Docu & xd)
        {
            // Currently thic catches only wrong since tags, while since tags are
            // transformed. In this case the program shall be terminated.
            Cerr() << xd << Endl();
            exit(1);
        }
        catch (...)
        {
            Cout() << "Unknown error." << Endl();
            exit(0);
//          pFileParsePerformers = new FileParsePerformers( *pRepository );
        }
    }
}

FileParsePerformers::FileParsePerformers( ary::Repository & io_rRepository,
                                          ParserInfo &           io_rParserInfo )
    :   pTokens(0),
        aDistributor(io_rRepository, io_rParserInfo),
        rRepository( io_rRepository ),
        rParserInfo(io_rParserInfo)
{
    DYN csi::dsapi::Context_Docu *
        dpDocuContext
            = new csi::dsapi::Context_Docu( aDistributor.DocuTokens_Receiver() );
    pTokens = new csi::uidl::TokenParser_Uidl( aDistributor.CodeTokens_Receiver(), *dpDocuContext );
    pFileParseEnvironment
            = new csi::uidl::PE_File(aDistributor,rParserInfo);

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

    rParserInfo.Set_CurFile(i_sFullPath, true); // true = count lines
    pTokens->Start(aFileLoader);
    aDistributor.Reset();

    do {
        aDistributor.TradeToken();
    } while ( NOT aFileLoader.IsFinished() );
}

}   // namespace autodoc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
