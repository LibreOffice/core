/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prs_cpp.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:00:43 $
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

#include <precomp.h>
#include <cpp/prs_cpp.hxx>


// NOT FULLY DEFINED SERVICES
#include <cosv/file.hxx>
#include <ary/ary.hxx>
#include <ary/cpp/c_gate.hxx>
#include <autodoc/prs_docu.hxx>
#include <autodoc/filecoli.hxx>
#include <autodoc/x_parsing.hxx>
#include <tools/tkpchars.hxx>
#include <adc_cl.hxx>
#include "c_dealer.hxx"
#include "defdescr.hxx"
#include "tkp_cpp.hxx"


// Helper function
static bool         Local_LoadFile(
                        CharacterSource &       o_rTextBuffer,
                        const String  &         i_rFullFilePath );




namespace cpp
{

// This class is used for the UDK as workaround for the missing
//   feature of parsing #define s.

class Udk_MacroMap
{
  public:
    typedef std::map< String , DefineDescription* > Data;

                        Udk_MacroMap();
                        ~Udk_MacroMap();

    const Data &        GetData() const         { return aData; }

  private:
    Data                aData;
};

struct S_RunningData
{
    CharacterSource     aFileContent;
    ary::cpp::Gate &    rCppGate;
    Udk_MacroMap        aMacros;
    Distributor         aDealer;
    TokenParser_Cpp     aTkp;

                            S_RunningData(
                                ary::Repository &   o_rRepository,
                                const autodoc::DocumentationParser_Ifc &
                                                    i_rDocumentationInterpreter );
};




Cpluplus_Parser::Cpluplus_Parser()
//    : pRunningData
{
}

Cpluplus_Parser::~Cpluplus_Parser()
{
}

void
Cpluplus_Parser::Setup( ary::Repository &                        o_rRepository,
                        const autodoc::DocumentationParser_Ifc & i_rDocumentationInterpreter )
{
    pRunningData = new S_RunningData(o_rRepository, i_rDocumentationInterpreter);
}

void
Cpluplus_Parser::Run( const autodoc::FileCollector_Ifc &    i_rFiles )
{
    for ( autodoc::FileCollector_Ifc::const_iterator iter = i_rFiles.Begin();
          iter != i_rFiles.End();
          ++iter )
    {
        csv::ploc::Path
            aFilePath(*iter);

        try
        {
            if ( NOT Local_LoadFile(pRunningData->aFileContent, *iter) )
                continue;
            for ( pRunningData->aTkp.StartNewFile(aFilePath);
                  pRunningData->aTkp.HasMore();
                  pRunningData->aTkp.GetNextToken() )
                ;
        }
        catch (autodoc::X_Parser_Ifc & rX_Parse)
        {
            if ( DEBUG_ShowStoring() OR DEBUG_ShowText() )
                Cerr() << rX_Parse << Endl();
        }
        catch (...)
        {
            if ( DEBUG_ShowStoring() OR DEBUG_ShowText() )
                Cerr() << "Error:  Unknown exception." << Endl();
        }
    }   // end for (iter)
}

S_RunningData::S_RunningData( ary::Repository &                        o_rRepository,
                              const autodoc::DocumentationParser_Ifc & i_rDocumentationInterpreter )
    :   aFileContent(),
        rCppGate( o_rRepository.Gate_Cpp() ),
        aMacros(),
        aDealer(o_rRepository.Gate_Cpp()),
        aTkp( * i_rDocumentationInterpreter.Create_DocuContext() )
{
    aDealer.AssignPartners( aFileContent,
                            aMacros.GetData() );
    aTkp.AssignPartners( aFileContent, aDealer );
}


Udk_MacroMap::Udk_MacroMap()
{
    String  sSAL_CALL("SAL_CALL");
    String  sSAL_CALL_ELLIPSE("SAL_CALL_ELLIPSE");
    String  sSAL_NO_VTABLE("SAL_NO_VTABLE");
    String  sREGISTRY_CALLTYPE("REGISTRY_CALLTYPE");
    String  sSAL_THROW("SAL_THROW");
    String  sSAL_THROW_EXTERN_C("SAL_THROW_EXTERN_C");

    String  s__DEF_COMPIMPLHELPER_A("__DEF_COMPIMPLHELPER_A");
    String  s__DEF_COMPIMPLHELPER_B("__DEF_COMPIMPLHELPER_B");
    String  s__DEF_COMPIMPLHELPER("__DEF_COMPIMPLHELPER");

    String  s__DEF_IMPLHELPER_PRE("__DEF_IMPLHELPER_PRE");
    String  s__IFC_WRITEOFFSET("__IFC_WRITEOFFSET");
    String  s__DEF_IMPLHELPER_POST("__DEF_IMPLHELPER_POST");


    StringVector aEmpty;

    StringVector aParamsSAL_THROW;
    aParamsSAL_THROW.push_back( String ("exc") );
    StringVector aDefSAL_THROW;
    aDefSAL_THROW.push_back( String ("throw") );
    aDefSAL_THROW.push_back( String ("exc") );

    StringVector aCompImplHelperParams;
    aCompImplHelperParams.push_back(String ("N"));


    // filling up the list


    aData[sSAL_CALL] = new DefineDescription(sSAL_CALL, aEmpty);
    aData[sSAL_CALL_ELLIPSE] = new DefineDescription(sSAL_CALL_ELLIPSE, aEmpty);
    aData[sSAL_NO_VTABLE] = new DefineDescription(sSAL_NO_VTABLE, aEmpty);
    aData[sREGISTRY_CALLTYPE] = new DefineDescription(sREGISTRY_CALLTYPE, aEmpty);

    aData[sSAL_THROW] = new DefineDescription(sSAL_THROW, aParamsSAL_THROW, aDefSAL_THROW);
    aData[sSAL_THROW_EXTERN_C] = new DefineDescription(sSAL_THROW_EXTERN_C, aEmpty, aEmpty);

    aData[s__DEF_COMPIMPLHELPER_A]
        = new DefineDescription( s__DEF_COMPIMPLHELPER_A, aCompImplHelperParams, aEmpty);
    aData[s__DEF_COMPIMPLHELPER_B]
        = new DefineDescription(s__DEF_COMPIMPLHELPER_B, aCompImplHelperParams, aEmpty);
    aData[s__DEF_COMPIMPLHELPER]
        = new DefineDescription(s__DEF_COMPIMPLHELPER, aCompImplHelperParams, aEmpty);

    aData[s__DEF_IMPLHELPER_PRE]
        = new DefineDescription(s__DEF_IMPLHELPER_PRE, aCompImplHelperParams, aEmpty);
    aData[s__IFC_WRITEOFFSET]
        = new DefineDescription(s__IFC_WRITEOFFSET, aCompImplHelperParams, aEmpty);
    aData[s__DEF_IMPLHELPER_POST]
        = new DefineDescription(s__DEF_IMPLHELPER_POST, aCompImplHelperParams, aEmpty);

}

Udk_MacroMap::~Udk_MacroMap()
{
    for ( Data::iterator it = aData.begin(); it != aData.end(); ++it )
    {
         delete (*it).second;
    }
}



} // namespace cpp


bool
Local_LoadFile( CharacterSource &       o_rTextBuffer,
                const String  &         i_rFullFilePath )
{
    Cout() << "Parse " << i_rFullFilePath << " ..." << Endl();

    csv::File aFile( i_rFullFilePath, csv::CFM_READ );
    if (NOT aFile.open())
    {
        Cerr() << "   could not be opened.\n" << Endl();
        return false;
    }
    o_rTextBuffer.LoadText(aFile);
    aFile.close();
    return true;
}



