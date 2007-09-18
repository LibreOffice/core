/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cre_link.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 13:50:43 $
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
#include "cre_link.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_disply.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_define.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_macro.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_vari.hxx>
#include <ary/cpp/c_enuval.hxx>
#include <ary/cpp/cg_file.hxx>
#include <ary/cpp/crog_grp.hxx>
#include "hdimpl.hxx"
#include "opageenv.hxx"
#include "strconst.hxx"





LinkCreator::LinkCreator( char *              o_rOutput,
                          uintt               i_nOutputSize )
    :   pOut(o_rOutput),
        nOutMaxSize(i_nOutputSize),
        pEnv(0)
{
}

LinkCreator::~LinkCreator()
{
}

void
LinkCreator::Display_Namespace( const ary::cpp::Namespace & i_rData )
{
    Create_PrePath( i_rData );
    strcat( pOut, "index.html" );   // KORR   // SAFE STRCAT (#100211# - checked)
}

void
LinkCreator::Display_Class( const ary::cpp::Class & i_rData )
{
    Create_PrePath( i_rData );
    strcat( pOut, ClassFileName(i_rData.LocalName().c_str()) ); // SAFE STRCAT (#100211# - checked)
}

void
LinkCreator::Display_Enum( const ary::cpp::Enum & i_rData )
{
    Create_PrePath( i_rData );
    strcat( pOut, EnumFileName(i_rData.LocalName().c_str()) ); // SAFE STRCAT (#100211# - checked)
}

void
LinkCreator::Display_Typedef( const ary::cpp::Typedef & i_rData )
{
    Create_PrePath( i_rData );
    strcat( pOut, TypedefFileName(i_rData.LocalName().c_str()) ); // SAFE STRCAT (#100211# - checked)
}

void
LinkCreator::Display_Function( const ary::cpp::Function & i_rData )
{
    Create_PrePath( i_rData );

    if ( i_rData.Protection() != ary::cpp::PROTECT_global )
    {
        strcat( pOut, "o.html" );   // SAFE STRCAT (#100211# - checked)
    }
    else
    {
        const ary::cpp::FileGroup *
            pFile = pEnv->Gate().RoGroups().Search_FileGroup(i_rData.Location());
        if (  pFile == 0 )
        {
             *pOut = NULCH;
            return;
        }
        strcat( pOut, HtmlFileName("o-", pFile->FileName().c_str()) ); // SAFE STRCAT (#100211# - checked)
    }

    strcat( pOut, OperationLink(i_rData.LocalName(), i_rData.Signature()) ); // SAFE STRCAT (#100211# - checked)
}

void
LinkCreator::Display_Variable( const ary::cpp::Variable & i_rData )
{
    Create_PrePath( i_rData );

    if ( i_rData.Protection() != ary::cpp::PROTECT_global )
    {
        strcat( pOut, "d.html" );       // SAFE STRCAT (#100211# - checked)
    }
    else
    {
        const ary::cpp::FileGroup *
            pFile = pEnv->Gate().RoGroups().Search_FileGroup(i_rData.Location());
        if (  pFile == 0 )
        {
             *pOut = NULCH;
            return;
        }
        strcat( pOut, HtmlFileName("d-", pFile->FileName().c_str()) );  // SAFE STRCAT (#100211# - checked)
    }

    strcat( pOut, DataLink(i_rData.LocalName()) );  // SAFE STRCAT (#100211# - checked)
}

void
LinkCreator::Display_EnumValue( const ary::cpp::EnumValue & i_rData )
{
    const ary::CodeEntity *
        pEnum = pEnv->Gate().Find_Ce(i_rData.Owner());
    if (pEnum == 0)
        return;

    pEnum->StoreAt(*this);
    strcat(pOut, "#");      // SAFE STRCAT (#100211# - checked)
    strcat(pOut, i_rData.LocalName().c_str());  // SAFE STRCAT (#100211# - checked)
}

void
LinkCreator::Display_Define( const ary::cpp::Define & i_rData )
{
    // KORR
    // Only valid from Index:

    *pOut = '\0';
    strcat(pOut, "../def-all.html#");               // SAFE STRCAT (#100211# - checked)
    strcat(pOut, i_rData.DefinedName().c_str());    // SAFE STRCAT (#100211# - checked)
}

void
LinkCreator::Display_Macro( const ary::cpp::Macro & i_rData )
{
    // KORR
    // Only valid from Index:

    *pOut = '\0';
    strcat(pOut, "../def-all.html#");               // SAFE STRCAT (#100211# - checked)
    strcat(pOut, i_rData.DefinedName().c_str());    // SAFE STRCAT (#100211# - checked)
}


namespace
{

class NameScope_const_iterator
{
  public:
                        NameScope_const_iterator(
                            ary::Rid            i_nId,
                            const ary::cpp::DisplayGate &
                                                i_rGate );

                        operator bool() const   { return pCe != 0; }
    const udmstri &     operator*() const;

    void                go_up();

  private:
    const ary::CodeEntity *
                        pCe;
    const ary::cpp::DisplayGate *
                        pGate;
};


NameScope_const_iterator::NameScope_const_iterator(
                                        ary::Rid                        i_nId,
                                        const ary::cpp::DisplayGate &   i_rGate )
    :   pCe(i_rGate.Find_Ce(i_nId)),
        pGate(&i_rGate)
{
}

const udmstri &
NameScope_const_iterator::operator*() const
{
     return pCe ? pCe->LocalName()
               : udmstri::Null_();
}

void
NameScope_const_iterator::go_up()
{
     if (pCe == 0)
        return;
    pCe = pGate->Find_Ce(pCe->Owner());
}


void                Recursive_CreatePath(
                        char *              o_pOut,
                        const NameScope_const_iterator &
                                            i_it        );

void
Recursive_CreatePath( char *                            o_pOut,
                      const NameScope_const_iterator &  i_it        )
{
    if (NOT i_it)
        return;

    NameScope_const_iterator it( i_it );
    it.go_up();
    if (NOT it)
        return;     // Global Namespace
    Recursive_CreatePath( o_pOut, it );

    strcat( o_pOut, (*i_it).c_str() );          // SAFE STRCAT (#100211# - checked)
    strcat( o_pOut, "/" );                      // SAFE STRCAT (#100211# - checked)
}


}   // anonymous namespace





void
LinkCreator::Create_PrePath( const ary::CodeEntity & i_rData )
{
    *pOut = NULCH;

    if ( pEnv->CurNamespace() != 0 )
    {
        if ( pEnv->CurClass()
                ?   pEnv->CurClass()->Id() == i_rData.Owner()
                :   pEnv->CurNamespace()->Id() == i_rData.Owner() )
            return;

        strcat( pOut, PathUp(pEnv->Depth() - 1) );      // SAFE STRCAT (#100211# - checked)
    }
    else
    {   // Within Index
        strcat( pOut, "../names/" );                    // SAFE STRCAT (#100211# - checked)
    }

    NameScope_const_iterator it( i_rData.Owner(), pEnv->Gate() );
    Recursive_CreatePath( pOut, it );
}


