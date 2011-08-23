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
#include "hi_main.hxx"


// NOT FULLY DEFINED SERVICES
#include <algorithm>
#include <cosv/ploc.hxx>
#include <cosv/file.hxx>
#include <ary/idl/i_ce.hxx>
#include <ary/idl/ik_ce.hxx>
#include <ary/idl/ik_enum.hxx>
#include <ary/idl/ik_typedef.hxx>
#include <ary/idl/ik_interface.hxx>
#include <ary/idl/ik_struct.hxx>
#include <ary/idl/ik_exception.hxx>
#include <ary/idl/i_constant.hxx>
#include <ary/idl/i_constgroup.hxx>
#include <ary/idl/i_enum.hxx>
#include <ary/idl/i_singleton.hxx>
#include <ary/idl/i_sisingleton.hxx>
#include <ary/idl/i_exception.hxx>
#include <ary/idl/i_interface.hxx>
#include <ary/idl/i_service.hxx>
#include <ary/idl/i_siservice.hxx>
#include <ary/idl/i_struct.hxx>
#include <ary/idl/i_typedef.hxx>
#include <ary/idl/i_module.hxx>
#include <cfrstd.hxx>
#include <toolkit/htmlfile.hxx>
#include <toolkit/out_position.hxx>
#include <toolkit/out_tree.hxx>
#include "hfi_constgroup.hxx"
#include "hfi_enum.hxx"
#include "hfi_globalindex.hxx"
#include "hfi_interface.hxx"
#include "hfi_module.hxx"
#include "hfi_struct.hxx"
#include "hfi_service.hxx"
#include "hfi_singleton.hxx"
#include "hfi_siservice.hxx"
#include "hfi_typedef.hxx"
#include "hfi_xrefpage.hxx"
#include "hi_env.hxx"
#include "hi_linkhelper.hxx"


using       ::ary::idl::Ce_id;
using       ::ary::idl::Type_id;
using       ::ary::idl::ifc_ce::Dyn_CeIterator;



extern const String C_sCssFilename_Idl("idl.css");

/*
typedef     ::ary::Dyn_StdConstIterator< ::ary::idl::CommentedRelation>
            Dyn_ComRefIterator;
namespace   read_module     = ::ary::idl::ifc_module;
namespace   read_interface  = ::ary::idl::ifc_interface;
namespace   read_service    = ::ary::idl::ifc_service;
namespace   read_struct     = ::ary::idl::ifc_struct;
namespace   read_exception  = ::ary::idl::ifc_exception;
namespace   read_enum       = ::ary::idl::ifc_enum;
namespace   read_typedef    = ::ary::idl::ifc_typedef;
namespace   read_constgroup = ::ary::idl::ifc_constantsgroup;
*/

namespace
{

/** @resp
    Inits (constructor) and creates (destructor) the current
    html documentation file ( MainDisplay_Idl.pMyFile ).
*/
class Guard_CurFile
{
  public:
                        Guard_CurFile(          /// For CodeEntities
                            DocuFile_Html &     io_client,
                            HtmlEnvironment_Idl &
                                                io_env,
                            const ary::idl::CodeEntity &
                                                i_ce,
                            const String &      i_titlePrefix );
                        Guard_CurFile(          /// For Use pages
                            DocuFile_Html &     io_client,
                            HtmlEnvironment_Idl &
                                                io_env,
                            const String &      i_fileName,
                            const String &      i_titlePrefix );
                        Guard_CurFile(          /// For Modules
                            DocuFile_Html &     io_client,
                            HtmlEnvironment_Idl &
                                                io_env,
                            const ary::idl::CodeEntity &
                                                i_ce );
                        Guard_CurFile(          /// For Indices
                            DocuFile_Html &     io_client,
                            HtmlEnvironment_Idl &
                                                io_env,
                            char                i_letter );
                        ~Guard_CurFile();
  private:
    DocuFile_Html &     rClient;
    HtmlEnvironment_Idl &
                        rEnv;

};

/** @resp
    Sets and releases the current factory pointer
    ( MainDisplay_Idl.pCurFactory ).
*/
class Guard_CurFactoryPtr
{
  public:
                        Guard_CurFactoryPtr(
                            HtmlFactory_Idl *&  io_client,
                            HtmlFactory_Idl &   i_factory )
                            :   rpClient(io_client)
                        { rpClient = &i_factory; }

                        ~Guard_CurFactoryPtr()
                        { rpClient = 0; }

  private:
    HtmlFactory_Idl *&  rpClient;

};


Guard_CurFile::Guard_CurFile( DocuFile_Html &               io_client,
                              HtmlEnvironment_Idl &         io_env,
                              const ary::idl::CodeEntity &  i_ce,
                              const String &                i_titlePrefix )
    :   rClient(io_client),
        rEnv(io_env)
{   // For Ces
    StreamLock sl(300);
    io_env.Set_CurFile( sl()    << i_ce.LocalName()
                                << ".html"
                                << c_str );
    StreamLock aCurFilePath(700);
    io_env.Get_CurFilePath(aCurFilePath());

    rClient.EmptyBody();
    csv::ploc::Path
        aLocation(aCurFilePath().c_str());
    rClient.SetLocation(aLocation);
    sl().reset();
    rClient.SetTitle( sl()  << i_titlePrefix
                            << " "
                            << i_ce.LocalName()
                            << c_str );
    sl().reset();
    rClient.SetRelativeCssPath(
                      sl()  << io_env.CurPosition().LinkToRoot()
                            << C_sCssFilename_Idl
                            << c_str );

    io_env.Set_CurPageCe(&i_ce);
}

Guard_CurFile::Guard_CurFile( DocuFile_Html &       io_client,
                              HtmlEnvironment_Idl & io_env,
                              const String &        i_fileName,
                              const String &        i_titlePrefix )
    :   rClient(io_client),
        rEnv(io_env)
{   // For Use pages
    StreamLock sl(300);
    io_env.Set_CurFile( sl()    << i_fileName
                                << ".html"
                                << c_str );
    StreamLock aCurFilePath(700);
    io_env.Get_CurFilePath(aCurFilePath());
    csv::ploc::Path
        aLocation(aCurFilePath().c_str());

    rClient.EmptyBody();
    rClient.SetLocation(aLocation);
    sl().reset();
    rClient.SetTitle( sl() << i_titlePrefix << " " << i_fileName << c_str );
    sl().reset();
    rClient.SetRelativeCssPath(
                      sl()  << io_env.CurPosition().LinkToRoot()
                            << C_sCssFilename_Idl
                            << c_str );

    io_env.Set_CurPageCe(0);
}

Guard_CurFile::Guard_CurFile( DocuFile_Html &               io_client,
                              HtmlEnvironment_Idl &         io_env,
                              const ary::idl::CodeEntity &  i_ce )
    :   rClient(io_client),
        rEnv(io_env)
{   // For Modules
    io_env.Set_CurFile( output::ModuleFileName() );
    StreamLock aCurFilePath(700);
    io_env.Get_CurFilePath(aCurFilePath());
    csv::ploc::Path
        aLocation(aCurFilePath().c_str());

    rClient.EmptyBody();
    rClient.SetLocation(aLocation);
    StreamLock sl(300);
    rClient.SetTitle( sl() << "Module " << io_env.CurPosition().Name() << c_str );
    sl().reset();
    rClient.SetRelativeCssPath(
                      sl()  << io_env.CurPosition().LinkToRoot()
                            << C_sCssFilename_Idl
                            << c_str );

    io_env.Set_CurPageCe(&i_ce);
}

Guard_CurFile::Guard_CurFile( DocuFile_Html &       io_client,
                              HtmlEnvironment_Idl & io_env,
                              char                  i_letter )
    :   rClient(io_client),
        rEnv(io_env)
{   // For Index pages
    StreamLock sl(300);
    io_env.Set_CurFile( sl()    << "index-"
                                << ( i_letter != '_'
                                        ?   int(i_letter)-'a'+1
                                        :   27 )
                                << ".html"
                                << c_str );
    StreamLock aCurFilePath(700);
    io_env.Get_CurFilePath(aCurFilePath());
    csv::ploc::Path
        aLocation(aCurFilePath().c_str());

    rClient.EmptyBody();
    rClient.SetLocation(aLocation);
    sl().reset();
    rClient.SetTitle( sl() << "Global Index "
                           << ( i_letter != '_'
                                    ?   char(i_letter-'a'+'A')
                                    :   '_' )
                           << c_str );
    sl().reset();
    rClient.SetRelativeCssPath(
                      sl()  << "../"
                            << C_sCssFilename_Idl
                            << c_str );
}

Guard_CurFile::~Guard_CurFile()
{
    rClient.CreateFile();
    rEnv.Set_CurPageCe(0);
}


}   // anonymous namespace




MainDisplay_Idl::MainDisplay_Idl( HtmlEnvironment_Idl & io_rEnv )
    :   pEnv(&io_rEnv),
        pMyFile(new DocuFile_Html),
        pCurFactory(0)
{
//  pMyFile->SetStyle( Env().Layout().CssStyle() );
    pMyFile->SetCopyright( Env().Layout().CopyrightText() );
}

MainDisplay_Idl::~MainDisplay_Idl()
{
}


void
MainDisplay_Idl::WriteGlobalIndices()
{
    for ( const char * pLetter = "abcdefghijklmnopqrstuvwxyz_X"; *pLetter != 'X'; ++pLetter )
    {
        Guard_CurFile       gFile( *pMyFile, Env(), *pLetter );

        HF_IdlGlobalIndex   aFactory( *pEnv, pMyFile->Body() );
        Guard_CurFactoryPtr gFactory(pCurFactory,aFactory);

        aFactory.Produce_Page( ary::idl::alphabetical_index::E_Letter(*pLetter) );
    }	// end for
}


void
MainDisplay_Idl::do_Process( const ary::idl::Module & i_ce )
{
    Guard_CurFile    gFile( *pMyFile,
                            Env(),
                            i_ce );
    HF_IdlModule     aFactory( *pEnv, pMyFile->Body() );
    Guard_CurFactoryPtr gFactory(pCurFactory,aFactory);

    aFactory.Produce_byData(i_ce);
}

void
MainDisplay_Idl::do_Process( const ary::idl::Interface & i_ce )
{
    do_InterfaceDescr(i_ce);
    do_Interface2s(i_ce);
}

void
MainDisplay_Idl::do_Process( const ary::idl::Service & i_ce )
{
    do_ServiceDescr(i_ce);
    do_Service2s(i_ce);
}

void
MainDisplay_Idl::do_Process( const ary::idl::SglIfcService & i_ce )
{
    do_SglIfcServiceDescr(i_ce);
}

void
MainDisplay_Idl::do_Process( const ary::idl::Struct & i_ce )
{
    do_StructDescr(i_ce);
    do_Struct2s(i_ce);
}

void
MainDisplay_Idl::do_Process( const ary::idl::Exception & i_ce )
{
    do_ExceptionDescr(i_ce);
    do_Exception2s(i_ce);
}

void
MainDisplay_Idl::do_Process( const ary::idl::Enum & i_ce )
{
    do_EnumDescr(i_ce);
    do_Enum2s(i_ce);
}

void
MainDisplay_Idl::do_Process( const ary::idl::Typedef & i_ce )
{
    do_TypedefDescr(i_ce);
    do_Typedef2s(i_ce);
}

void
MainDisplay_Idl::do_Process( const ary::idl::ConstantsGroup & i_ce )
{
    Guard_CurFile       gFile( *pMyFile,
                               Env(),
                               i_ce,
                               "Constants' Group" );
    HF_IdlConstGroup    aFactory( *pEnv, pMyFile->Body() );
    Guard_CurFactoryPtr gFactory(pCurFactory,aFactory);

    aFactory.Produce_byData(i_ce);
}

void
MainDisplay_Idl::do_Process( const ary::idl::Singleton & i_ce )
{
    Guard_CurFile       gFile( *pMyFile,
                               Env(),
                               i_ce,
                               "Singleton" );
    HF_IdlSingleton     aFactory( *pEnv, pMyFile->Body() );
    Guard_CurFactoryPtr gFactory(pCurFactory,aFactory);

    aFactory.Produce_byData_ServiceBased(i_ce);
}

void
MainDisplay_Idl::do_Process( const ary::idl::SglIfcSingleton & i_ce )
{
    Guard_CurFile       gFile( *pMyFile,
                               Env(),
                               i_ce,
                               "Singleton" );
    HF_IdlSingleton     aFactory( *pEnv, pMyFile->Body() );
    Guard_CurFactoryPtr gFactory(pCurFactory,aFactory);

    aFactory.Produce_byData_InterfaceBased(i_ce);
}

void
MainDisplay_Idl::do_InterfaceDescr( const ary::idl::CodeEntity & i_ce )
{
    Guard_CurFile       gFile( *pMyFile,
                               Env(),
                               i_ce,
                               "Interface" );
    HF_IdlInterface     aInterface( *pEnv, pMyFile->Body() );
    Guard_CurFactoryPtr gFactory(pCurFactory,aInterface);

    aInterface.Produce_byData(i_ce);
}

void
MainDisplay_Idl::do_ServiceDescr( const ary::idl::CodeEntity & i_ce )
{
    Guard_CurFile       gFile( *pMyFile,
                               Env(),
                               i_ce,
                               "Service" );
    HF_IdlService       aFactory( *pEnv, pMyFile->Body() );
    Guard_CurFactoryPtr gFactory(pCurFactory,aFactory);

    aFactory.Produce_byData(i_ce);
}

void
MainDisplay_Idl::do_SglIfcServiceDescr( const ary::idl::CodeEntity & i_ce )
{
    Guard_CurFile       gFile( *pMyFile,
                               Env(),
                               i_ce,
                               "Service" );
    HF_IdlSglIfcService aFactory( *pEnv, pMyFile->Body() );
    Guard_CurFactoryPtr gFactory(pCurFactory,aFactory);

    aFactory.Produce_byData(i_ce);
}

void
MainDisplay_Idl::do_StructDescr( const ary::idl::CodeEntity & i_ce )
{
    Guard_CurFile       gFile( *pMyFile,
                               Env(),
                               i_ce,
                               "Struct" );
    HF_IdlStruct        aFactory( *pEnv, pMyFile->Body(), false );
    Guard_CurFactoryPtr gFactory(pCurFactory,aFactory);

    aFactory.Produce_byData(i_ce);
}

void
MainDisplay_Idl::do_ExceptionDescr( const ary::idl::CodeEntity & i_ce )
{
    Guard_CurFile       gFile( *pMyFile,
                               Env(),
                               i_ce,
                               "Exception" );
    HF_IdlStruct        aFactory( *pEnv, pMyFile->Body(), true );
    Guard_CurFactoryPtr gFactory(pCurFactory,aFactory);

    aFactory.Produce_byData(i_ce);
}

void
MainDisplay_Idl::do_EnumDescr( const ary::idl::CodeEntity & i_ce )
{
    Guard_CurFile       gFile( *pMyFile,
                               Env(),
                               i_ce,
                               "Enum" );
    HF_IdlEnum        aFactory( *pEnv, pMyFile->Body() );
    Guard_CurFactoryPtr gFactory(pCurFactory,aFactory);

    aFactory.Produce_byData(i_ce);
}

void
MainDisplay_Idl::do_TypedefDescr( const ary::idl::CodeEntity & i_ce )
{
    Guard_CurFile       gFile( *pMyFile,
                               Env(),
                               i_ce,
                               "Typedef" );
    HF_IdlTypedef       aFactory( *pEnv, pMyFile->Body() );
    Guard_CurFactoryPtr gFactory(pCurFactory,aFactory);

    aFactory.Produce_byData(i_ce);
}

void
MainDisplay_Idl::do_Interface2s( const ary::idl::CodeEntity & i_ce )
{
    StreamLock sl(100);
    String sUsesFileName(
                sl()
                    << i_ce.LocalName()
                    << Env().Linker().XrefsSuffix()
                    << c_str );
    Guard_CurFile       gFile( *pMyFile,
                               Env(),
                               sUsesFileName,
                               "Uses of Interface" );
    HF_IdlXrefs         aUses( *pEnv,
                               pMyFile->Body(),
                               C_sCePrefix_Interface,
                               i_ce );


    aUses.Produce_Tree(
        "Derived Interfaces",
        "#Deriveds",
        i_ce,
        &ary::idl::ifc_interface::xref::Get_Derivations );

    Dyn_CeIterator  pXrefList;

    ary::idl::ifc_interface::xref::Get_SynonymTypedefs(pXrefList,i_ce);
    aUses.Produce_List(
        "Synonym Typedefs",
        "#Synonyms",
        *pXrefList );
    ary::idl::ifc_interface::xref::Get_ExportingServices(pXrefList,i_ce);
    aUses.Produce_List(
        "Services which Support this Interface",
        "#SupportingServices",
        *pXrefList );
    ary::idl::ifc_interface::xref::Get_ExportingSingletons(pXrefList,i_ce);
    aUses.Produce_List(
        "Singletons which Support this Interface",
        "#SupportingSingletons",
        *pXrefList );
    ary::idl::ifc_interface::xref::Get_AsReturns(pXrefList,i_ce);
    aUses.Produce_List(
        "Uses as Return Type",
        "#Returns",
        *pXrefList );
    ary::idl::ifc_interface::xref::Get_AsParameters(pXrefList,i_ce);
    aUses.Produce_List(
        "Uses as Parameter",
        "#Parameters",
        *pXrefList );
    ary::idl::ifc_interface::xref::Get_AsDataTypes(pXrefList,i_ce);
    aUses.Produce_List(
        "Uses as Data Type",
        "#DataTypes",
        *pXrefList );
    aUses.Write_ManualLinks(i_ce);
}

void
MainDisplay_Idl::do_Service2s( const ary::idl::CodeEntity & i_ce )
{
    StreamLock sl(100);
    String sUsesFileName(
                sl()
                    << i_ce.LocalName()
                    << Env().Linker().XrefsSuffix()
                    << c_str );
    Guard_CurFile   gFile(  *pMyFile,
                            Env(),
                            sUsesFileName,
                            "Uses of Service" );
    HF_IdlXrefs     aUses(  *pEnv,
                            pMyFile->Body(),
                            C_sCePrefix_Service,
                            i_ce );
    Dyn_CeIterator  pXrefList;
    ary::idl::ifc_service::xref::Get_IncludingServices(pXrefList,i_ce);
    aUses.Produce_List(
        "Services which Include this Service",
        "#IncludingServices",
        *pXrefList );

    ary::idl::ifc_service::xref::Get_InstantiatingSingletons(pXrefList,i_ce);
    aUses.Produce_List(
        "Singletons which Instantiate this Service",
        "#Singletons",
        *pXrefList );
    aUses.Write_ManualLinks(i_ce);
}

void
MainDisplay_Idl::do_Struct2s( const ary::idl::CodeEntity & i_ce )
{
    StreamLock sl(100);
    String sUsesFileName(
                sl()
                    << i_ce.LocalName()
                    << Env().Linker().XrefsSuffix()
                    << c_str );
    Guard_CurFile   gFile(  *pMyFile,
                            Env(),
                            sUsesFileName,
                            "Uses of Struct" );
    HF_IdlXrefs     aUses(  *pEnv,
                            pMyFile->Body(),
                            C_sCePrefix_Struct,
                            i_ce );

    aUses.Produce_Tree(
        "Derived Structs",
        "#Deriveds",
        i_ce,
        &ary::idl::ifc_struct::xref::Get_Derivations );

    Dyn_CeIterator  pXrefList;

    ary::idl::ifc_struct::xref::Get_SynonymTypedefs(pXrefList,i_ce);
    aUses.Produce_List(
        "Synonym Typedefs",
        "#Synonyms",
        *pXrefList );
    ary::idl::ifc_struct::xref::Get_AsReturns(pXrefList,i_ce);
    aUses.Produce_List(
        "Uses as Return Type",
        "#Returns",
        *pXrefList );
    ary::idl::ifc_struct::xref::Get_AsParameters(pXrefList,i_ce);
    aUses.Produce_List(
        "Uses as Parameter",
        "#Parameters",
        *pXrefList );
    ary::idl::ifc_struct::xref::Get_AsDataTypes(pXrefList,i_ce);
    aUses.Produce_List(
        "Uses as Data Type",
        "#DataTypes",
        *pXrefList );
    aUses.Write_ManualLinks(i_ce);
}

void
MainDisplay_Idl::do_Exception2s( const ary::idl::CodeEntity & i_ce )
{
    StreamLock sl(100);
    String sUsesFileName(
                sl()
                    << i_ce.LocalName()
                    << Env().Linker().XrefsSuffix()
                    << c_str );
    Guard_CurFile   gFile(  *pMyFile,
                            Env(),
                            sUsesFileName,
                            "Uses of Exception" );
    HF_IdlXrefs     aUses(  *pEnv,
                            pMyFile->Body(),
                            C_sCePrefix_Exception,
                            i_ce );

    aUses.Produce_Tree(
        "Derived Exceptions",
        "#Deriveds",
        i_ce,
        &ary::idl::ifc_exception::xref::Get_Derivations );

    Dyn_CeIterator  pXrefList;

    ary::idl::ifc_exception::xref::Get_RaisingFunctions(pXrefList,i_ce);
    aUses.Produce_List(
        "Raising Functions",
        "#Raisers",
        *pXrefList );
    aUses.Write_ManualLinks(i_ce);
}

void
MainDisplay_Idl::do_Enum2s( const ary::idl::CodeEntity & i_ce )
{
    StreamLock sl(100);
    String sUsesFileName(
                sl()
                    << i_ce.LocalName()
                    << Env().Linker().XrefsSuffix()
                    << c_str );
    Guard_CurFile   gFile(  *pMyFile,
                            Env(),
                            sUsesFileName,
                            "Uses of Enum" );
    HF_IdlXrefs     aUses(  *pEnv,
                            pMyFile->Body(),
                            C_sCePrefix_Enum,
                            i_ce );
    Dyn_CeIterator  pXrefList;
    ary::idl::ifc_enum::xref::Get_SynonymTypedefs(pXrefList,i_ce);
    aUses.Produce_List(
        "Synonym Typedefs",
        "#Synonyms",
        *pXrefList );
    ary::idl::ifc_enum::xref::Get_AsReturns(pXrefList,i_ce);
    aUses.Produce_List(
        "Uses as Return Type",
        "#Returns",
        *pXrefList );
    ary::idl::ifc_enum::xref::Get_AsParameters(pXrefList,i_ce);
    aUses.Produce_List(
        "Uses as Parameter",
        "#Parameters",
        *pXrefList );
    ary::idl::ifc_enum::xref::Get_AsDataTypes(pXrefList,i_ce);
    aUses.Produce_List(
        "Uses as Data Type",
        "#DataTypes",
        *pXrefList );
    aUses.Write_ManualLinks(i_ce);
}

void
MainDisplay_Idl::do_Typedef2s( const ary::idl::CodeEntity & i_ce )
{
    StreamLock sl(100);
    String sUsesFileName(
               sl() << i_ce.LocalName()
                    << Env().Linker().XrefsSuffix()
                    << c_str );
    Guard_CurFile   gFile(  *pMyFile,
                            Env(),
                            sUsesFileName,
                            "Uses of Typedef" );
    HF_IdlXrefs     aUses(  *pEnv,
                            pMyFile->Body(),
                            C_sCePrefix_Typedef,
                            i_ce );
    Dyn_CeIterator  pXrefList;
    ary::idl::ifc_typedef::xref::Get_SynonymTypedefs(pXrefList,i_ce);
    aUses.Produce_List(
        "Synonym Typedefs",
        "#Synonyms",
        *pXrefList );
    ary::idl::ifc_typedef::xref::Get_AsReturns(pXrefList,i_ce);
    aUses.Produce_List(
        "Uses as Return Type",
        "#Returns",
        *pXrefList );
    ary::idl::ifc_typedef::xref::Get_AsParameters(pXrefList,i_ce);
    aUses.Produce_List(
        "Uses as Parameter",
        "#Parameters",
        *pXrefList );
    ary::idl::ifc_typedef::xref::Get_AsDataTypes(pXrefList,i_ce);
    aUses.Produce_List(
        "Uses as Data Type",
        "#DataTypes",
        *pXrefList );
    aUses.Write_ManualLinks(i_ce);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
