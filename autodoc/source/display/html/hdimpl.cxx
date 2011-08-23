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
#include "hdimpl.hxx"


// NOT FULLY DEFINED SERVICES
#include <stdlib.h>
#include <stdio.h>
#include <ary/ceslot.hxx>
#include <ary/qualiname.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_de.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/cp_ce.hxx>
#include <udm/html/htmlitem.hxx>
#include "cre_link.hxx"
#include "hd_docu.hxx"
#include "html_kit.hxx"
#include "opageenv.hxx"
#include "pagemake.hxx"
#include "strconst.hxx"


using namespace csi;


//********************       HtmlDisplay_Impl        *********************//

HtmlDisplay_Impl::~HtmlDisplay_Impl()
{
}

HtmlDisplay_Impl::HtmlDisplay_Impl( OuputPage_Environment & io_rEnv )
    :   pEnv(&io_rEnv)
        // aWriteHelper
{
}


//********************       Free Functions        *********************//



namespace dshelp
{

void
DisplaySlot( ary::Display &          o_rDisplay,
             const ary::AryGroup &   i_rGroup,
             ary::SlotAccessId       i_nSlot )
{
     ary::Slot_AutoPtr pSlot( i_rGroup.Create_Slot(i_nSlot) );
    pSlot->StoreAt( o_rDisplay );
}


const char *
PathUp( uintt i_nLevels )
{
    static char sResult[300];

    sResult[0] = NULCH;
    for ( uintt lev = 0; lev < i_nLevels; ++lev )
    {
        strcat( sResult, "../");        // SAFE STRCAT (#100211# - checked)
    }
    return sResult;
}

const char *
PathPerLevelsUp( uintt                   i_nLevels,
                 const char *            i_nPathBelowDestinationLevel )
{
    static char sResult[500];
    strcpy( sResult, PathUp(i_nLevels) );               // SAFE STRCPY (#100211# - checked)
    // KORR_FUTURE: Make it still safer here:
    strcat( sResult, i_nPathBelowDestinationLevel );    // SAFE STRCAT (#100211# - checked)
    return sResult;
}


const char *
PathPerRoot( const OuputPage_Environment & i_rEnv,
             const char *                  i_sPathFromRootDir )
{
    return PathPerLevelsUp( i_rEnv.Depth(), i_sPathFromRootDir );
}

const char *
PathPerNamespace( const OuputPage_Environment & i_rEnv,
                  const char *                  i_sPathFromNamespaceDir )
{
    const ary::cpp::Namespace * pNsp = i_rEnv.CurNamespace();
    if ( pNsp == 0 )
        return "";

    uintt nCount = i_rEnv.Depth() - (pNsp->Depth() + 1) ;
    csv_assert( nCount < 100 );
    return PathPerLevelsUp( nCount, i_sPathFromNamespaceDir );
}

const char *
HtmlFileName( const char *            i_sPrefix,
              const char *            i_sEntityName )
{
    // KORR_FUTURE: Make it still safer here:
    static char sResult[300];
    strcpy( sResult, i_sPrefix );       // SAFE STRCPY (#100211# - checked)
    strcat( sResult, i_sEntityName );   // SAFE STRCAT (#100211# - checked)
    strcat( sResult, ".html" );         // SAFE STRCAT (#100211# - checked)
    return sResult;
}

const char *
Path2Class( uintt                   i_nLevelsUp,
            const char *            i_sClassLocalName )
{
    return PathPerLevelsUp( i_nLevelsUp, ClassFileName(i_sClassLocalName) );
}

const char *
Path2Child( const char *            i_sFileName,
            const char *            i_sSubDir )
{
    static char sResult[400];
    if ( i_sSubDir != 0 )
    {
        // KORR_FUTURE: Make it still safer here:
        strcpy( sResult, i_sSubDir );       // SAFE STRCPY (#100211# - checked)
        strcat( sResult, "/" );             // SAFE STRCAT (#100211# - checked)
    }
    else
    {
        sResult[0] = NULCH;
    }

    strcat( sResult, i_sFileName );         // SAFE STRCAT (#100211# - checked)
    return sResult;
}

const char *
Path2ChildNamespace( const char * i_sLocalName )
{
    return Path2Child( C_sHFN_Namespace, i_sLocalName );
}

String
OperationLink( const ary::cpp::Gate &               ,
               const String  &                      i_sOpName,
               ary::cpp::Ce_id                      i_nOpId,
               const char *                         i_sPrePath )
{
    StreamLock
        slResult(3000);
    StreamStr &
        sResult = slResult();

    sResult
        << i_sPrePath
        << "#"
        << i_sOpName
        << "-"
        << i_nOpId.Value();



    return sResult.c_str();
}

const char *
DataLink( const String  &         i_sLocalName,
          const char *            i_sPrePath )
{
    StreamLock
        slResult(3000);
    StreamStr &
        sResult = slResult();

    sResult
        << i_sPrePath
        << "#"
        << i_sLocalName;

    return sResult.c_str();
}

void
Get_LinkedTypeText( csi::xml::Element &             o_rOut,
                    const OuputPage_Environment &   i_rEnv,
                    ary::cpp::Type_id               i_nId,
                    bool                            i_bWithAbsolutifier )
{
    if (NOT i_nId.IsValid())
        return;

     const char * sPreName = "";
     const char * sName = "";
     const char * sPostName = "";

    bool bTypeExists = Get_TypeText( sPreName,
                                     sName,
                                     sPostName,
                                     i_nId,
                                     i_rEnv.Gate() );
    if ( NOT bTypeExists )
        return;

    if ( NOT i_bWithAbsolutifier AND strncmp(sPreName,"::",2) == 0 )
        sPreName+=2;

    const ary::cpp::CodeEntity *
        pCe = i_rEnv.Gate().Search_RelatedCe(i_nId);

    String  sLink;
    if ( pCe != 0 )
    {
        sLink = Link2Ce(i_rEnv,*pCe);
    }
    else
    {
        if ( strstr(sPreName,"com::sun::star") != 0 )
        {
            static StreamStr aLink(400);
            aLink.seekp(0);
            aLink << PathPerRoot(i_rEnv, "../../common/ref");
            if ( *sPreName != ':' )
                aLink << '/';
            for ( const char * s = sPreName;
                  *s != 0;
                  ++s )
            {
                if ( *s == ':' )
                {
                     aLink << '/';
                    ++s;
                }
                else
                {
                     aLink << *s;
                }
            }   // end for
            aLink << sName
                  << ".html";
            sLink = aLink.c_str();
        }
    }   // endif( pCe != 0 )

    o_rOut
        << sPreName;
    csi::xml::Element &
        o_Goon = sLink.length() > 0
                     ?   o_rOut >> * new html::Link( sLink.c_str() )
                     :   o_rOut;
    o_Goon
        <<  sName;
    o_rOut
        << sPostName;
}

void
Create_ChildListLabel( csi::xml::Element &     o_rParentElement,
                       const char *            i_sLabel )
{
    if ( NOT csv::no_str(i_sLabel) )
    {
        o_rParentElement
            >> *new html::Label(i_sLabel)
                    << " ";
    }
}

DYN csi::html::Table &
Create_ChildListTable( const char * i_sTitle )
{
    html::Table *
            dpTable = new html::Table;
    *dpTable
        << new html::ClassAttr( "childlist")
        << new xml::AnAttribute( "border", "1" )
        << new xml::AnAttribute( "cellpadding", "5" )
        << new xml::AnAttribute( "cellspacing", "0" )
        << new html::WidthAttr( "100%" );

    html::TableRow &
            rRow = dpTable->AddRow();
    rRow
        << new html::ClassAttr("subtitle")
        >> *new html::TableCell
                << new xml::AnAttribute( "colspan","2" )
                >> *new html::Headline(4)
                        << i_sTitle;
    return *dpTable;
}

const char *
Link2Ce( const OuputPage_Environment & i_rEnv,
         const ary::cpp::CodeEntity &  i_rCe )
{
    const uintt         nMaxSize
                            = 3000;
    static char         sLink[nMaxSize];
    static LinkCreator  aLinkCreator( &sLink[0], nMaxSize );
    sLink[0] = NULCH;

    aLinkCreator.SetEnv(i_rEnv);
    i_rCe.Accept(aLinkCreator);

    return sLink;
}

const char *
Link2CppDefinition( const OuputPage_Environment & i_rEnv,
                    const ary::cpp::DefineEntity &    i_rDef )
{
    const uintt         nMaxSize
                            = 1000;
    static char         sLink[nMaxSize];
    static LinkCreator  aLinkCreator( &sLink[0], nMaxSize );
    sLink[0] = NULCH;

    aLinkCreator.SetEnv(i_rEnv);
    i_rDef.Accept(aLinkCreator);

    return sLink;
}

const ary::cpp::CodeEntity *
FindUnambiguousCe( const OuputPage_Environment & i_rEnv,
                   const ary::QualifiedName &    i_rQuName,
                   const ary::cpp::Class *       i_pJustDocumentedClass )
{
     if ( i_rEnv.CurNamespace() == 0 )
        return 0;

    const ary::cpp::CodeEntity * ret = 0;

    if ( NOT i_rQuName.IsQualified() )
    {
        if ( i_pJustDocumentedClass != 0 )
            ret = i_rEnv.Gate().Ces().Search_CeLocal( i_rQuName.LocalName(),
                                                      i_rQuName.IsFunction(),
                                                      *i_rEnv.CurNamespace(),
                                                      i_pJustDocumentedClass );
        if (ret != 0)
            return ret;

        ret = i_rEnv.Gate().Ces().Search_CeLocal( i_rQuName.LocalName(),
                                                  i_rQuName.IsFunction(),
                                                  *i_rEnv.CurNamespace(),
                                                  i_rEnv.CurClass() );
    }
    if (ret != 0)
        return ret;

    return i_rEnv.Gate().Ces().Search_CeAbsolute( *i_rEnv.CurNamespace(),
                                                  i_rQuName );
}

void
ShowDocu_On( csi::xml::Element &            o_rOut,
             Docu_Display &                 io_rDisplay,
             const ary::cpp::CppEntity &    i_rRE )
{
    if (i_rRE.Docu().Data() != 0)
    {
        io_rDisplay.Assign_Out( o_rOut );
        io_rDisplay.Process(i_rRE.Docu());
        io_rDisplay.Unassign_Out();
    }
}

void
WriteOut_TokenList( csi::xml::Element &     o_rOut,
                    const StringVector &    i_rTokens,
                    const char *            i_sSeparator )
{
    if ( i_rTokens.size() > 0 )
    {
        StringVector::const_iterator
            it      = i_rTokens.begin();
        StringVector::const_iterator
            itEnd   = i_rTokens.end();

        o_rOut << *it;
        for ( ++it; it != itEnd; ++it )
        {
            o_rOut << i_sSeparator << *it;
        }
    };

}

void
EraseLeadingSpace( String  & io_rStr )
{
    if ( *io_rStr.c_str() < 33 AND io_rStr.length() > 0 )
    {
        const unsigned char * pNew;
        for ( pNew = (const unsigned char * ) io_rStr.c_str();
              *pNew < 33 AND *pNew != 0;
              ++pNew ) {}
        String  sNew( (const char*)pNew );
        io_rStr = sNew;
    }
}

void
WriteOut_LinkedFunctionText( csi::xml::Element &            o_rTitleOut,
                             adcdisp::ParameterTable &      o_rParameters,
                             const ary::cpp::Function &     i_rFunction,
                             const OuputPage_Environment &  i_rEnv,
                             bool *                         o_bIsConst,
                             bool *                         o_bIsVirtual )
{
    // write pre-name:
    const ary::cpp::FunctionFlags & rFlags = i_rFunction.Flags();
    if ( rFlags.IsStaticLocal() OR rFlags.IsStaticMember() )
        o_rTitleOut << "static ";
    if ( rFlags.IsExplicit() )
        o_rTitleOut << "explicit ";
    if ( rFlags.IsMutable() )
        o_rTitleOut << "mutable ";
    if ( i_rFunction.Virtuality() != ary::cpp::VIRTUAL_none )
        o_rTitleOut << "virtual ";
//    o_rTitleOut << new html::LineBreak;

    Get_LinkedTypeText( o_rTitleOut, i_rEnv, i_rFunction.ReturnType() );

    // write name:
    o_rTitleOut
        << " "
        >> *new html::Strong
            << i_rFunction.LocalName();
    o_rTitleOut
            << "(";


    csi::xml::Element * pOutLast = &o_rTitleOut;

    // write post-name:
    FunctionParam_Iterator fit;
    fit.Assign(i_rFunction);

    if (fit)
    {
        o_rParameters.AddEntry();
        Get_LinkedTypeText( o_rParameters.Type(), i_rEnv, fit.CurType() );
        o_rParameters.Type() << " ";
        o_rParameters.Name() << " " << fit.CurName();

        for ( ++fit; fit; ++fit )
        {
            o_rParameters.Name() << ",";
            o_rParameters.AddEntry();
            Get_LinkedTypeText( o_rParameters.Type(), i_rEnv, fit.CurType() );
            o_rParameters.Name() << fit.CurName();
        }

        pOutLast = &o_rParameters.Name();
        o_rParameters.Name() << " ";
    }

    *pOutLast  << ")";
    if ( fit.IsFunctionConst() )
    {
        *pOutLast << " const";
        if ( o_bIsConst != 0 )
            *o_bIsConst = true;
    }
    if ( fit.IsFunctionVolatile() )
    {
        *pOutLast << " volatile";
        if ( o_bIsVirtual != 0 )
            *o_bIsVirtual = true;
    }

    // write Exceptions:
    const std::vector< ary::cpp::Type_id > *
            pThrow = i_rFunction.Exceptions();
    if ( pThrow)
    {
        std::vector< ary::cpp::Type_id >::const_iterator
                it = pThrow->begin();
        std::vector< ary::cpp::Type_id >::const_iterator
                it_end = pThrow->end();

        if (it != it_end)
        {
            o_rParameters.AddEntry();
            pOutLast = &o_rParameters.Name();

            o_rParameters.Name() << " throw( ";
            Get_LinkedTypeText(o_rParameters.Name(), i_rEnv, *it);

            for ( ++it; it != it_end; ++it )
            {
                o_rParameters.Name() << ", ";
                Get_LinkedTypeText(o_rParameters.Name(), i_rEnv, *it);
            }
            o_rParameters.Name() << " )";
        }
        else
        {
            *pOutLast << " throw()";
        }
    }   // endif // pThrow

    // abstractness:
    if ( i_rFunction.Virtuality() == ary::cpp::VIRTUAL_abstract )
        *pOutLast << " = 0";

    // finish:
    *pOutLast << ";";
}



}   // namespace dshelp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
