/*************************************************************************
 *
 *  $RCSfile: aryattrs.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-15 18:45:02 $
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
#include "aryattrs.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_disply.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/info/codeinfo.hxx>
#include "strconst.hxx"



//********************       HtmlDisplay_Impl        *********************//



const char *
Get_ClassTypeKey( const ary::cpp::Class & i_rClass )
{
    return i_rClass.ClassKey() == ary::cpp::CK_class
                            ?   C_sHFTypeTitle_Class
                            :   i_rClass.ClassKey() == ary::cpp::CK_struct
                                    ?   C_sHFTypeTitle_Struct
                                    :   C_sHFTypeTitle_Union;

}

const char *
Get_TypeKey( const ary::CodeEntity & i_rCe )
{
    if ( i_rCe.RC() == ary::cpp::Class::RC_() )
    {
        csv_assert( dynamic_cast< const ary::cpp::Class* >(&i_rCe) != 0  );
        return Get_ClassTypeKey(
                    static_cast< const ary::cpp::Class& >(i_rCe) );
    }
    if ( i_rCe.RC() == ary::cpp::Enum::RC_() )
    {
         return "enum";
    }
    return "";
}

bool
Ce_IsInternal( const ary::CodeEntity & i_rCe )
{
    return NOT i_rCe.IsVisible();
}

const char *
Namespace_DisplayName( const ary::cpp::Namespace & i_rNsp )
{
     return i_rNsp.Depth() > 0
                ?   i_rNsp.LocalName().c_str()
                :   "GlobalNamespace of C++";
}

const char *
TypeText( ary::Tid                      i_nId,
          const ary::cpp::DisplayGate & i_rAryGate )
{
     static StreamStr sResult(2000);
    sResult.seekp(0);
    i_rAryGate.Get_TypeText(sResult, i_nId);

    return sResult.c_str();
}

const char *
SyntaxText_PreName( const ary::cpp::Function &      i_rFunction,
                    const ary::cpp::DisplayGate &   i_rAryGate )
{
    static StreamStr  sResult( 150 );
    sResult.seekp(0);

    // write pre-name:
    const ary::cpp::FunctionFlags & rFlags = i_rFunction.Flags();
    if ( rFlags.IsStaticLocal() OR rFlags.IsStaticMember() )
        sResult << "static ";
    if ( rFlags.IsExplicit() )
        sResult << "explicit ";
    if ( rFlags.IsMutable() )
        sResult << "mutable ";
    if ( i_rFunction.Virtuality() != ary::cpp::VIRTUAL_none )
        sResult << "virtual ";
    i_rAryGate.Get_TypeText( sResult, i_rFunction.ReturnType() );
    sResult << " ";

    return sResult.c_str();
}

const char *
SyntaxText_PostName( const ary::cpp::Function &     i_rFunction,
                     const ary::cpp::DisplayGate &  i_rAryGate )
{
    static StreamStr  sResult( 850 );
    sResult.seekp(0);

    // parameters and con_vol
    i_rAryGate.Get_SignatureText( sResult, i_rFunction.Signature(), &i_rFunction.ParamInfos() );

    // write Exceptions:
    const std::vector< ary::Tid > *
            pThrow = i_rFunction.Exceptions();
    if ( pThrow)
    {

        std::vector< ary::Tid >::const_iterator
                it = pThrow->begin();
        std::vector< ary::Tid >::const_iterator
                it_end = pThrow->end();

        if (it != it_end)
        {
            sResult << " throw( ";
            i_rAryGate.Get_TypeText(sResult, *it);

            for ( ++it; it != it_end; ++it )
            {
                sResult << ", ";
                i_rAryGate.Get_TypeText(sResult, *it);
            }
            sResult << " )";
        }
        else
        {
            sResult << " throw( )";
        }
    }   // endif // pThrow

    // abstractness:
    if ( i_rFunction.Virtuality() == ary::cpp::VIRTUAL_abstract )
        sResult << " = 0";

    // finish:
    sResult << ";";

    return sResult.c_str();
}

bool
Get_TypeText( const char * &                o_rPreName,
              const char * &                o_rName,
              const char * &                o_rPostName,
              ary::Tid                      i_nTypeid,
              const ary::cpp::DisplayGate & i_rAryGate )
{
    static StreamStr       sResult_PreName(250);
    static StreamStr       sResult_Name(250);
    static StreamStr       sResult_PostName(250);

    sResult_PreName.seekp(0);
    sResult_Name.seekp(0);
    sResult_PostName.seekp(0);

    bool    ret = i_rAryGate.Get_TypeText(
                                sResult_PreName,
                                sResult_Name,
                                sResult_PostName,
                                i_nTypeid );
    if ( sResult_PreName.tellp() > 0 )
    {
        char cLast = *( sResult_PreName.c_str() + (sResult_PreName.tellp() - 1) );
        if (cLast != ':' AND cLast != ' ')
            sResult_PreName << " ";
    }


    if (ret)
    {
        o_rPreName  = sResult_PreName.c_str();
        o_rName     = sResult_Name.c_str();
        o_rPostName = sResult_PostName.c_str();
    }
    else
    {
        o_rPreName  =  o_rName =  o_rPostName = "";
    }
    return ret;
}




//*********************         FunctionParam_Iterator      *****************//


FunctionParam_Iterator::FunctionParam_Iterator()
    :   // itTypes
        // itTypes_end
        // itNames_andMore
        // itNames_andMore_end
        eConVol(ary::cpp::CONVOL_none)
{
    static std::vector<ary::Tid>    aTypesNull_;
    static StringVector             aNamesNull_;

    itTypes = itTypes_end = aTypesNull_.end();
    itNames_andMore = itNames_andMore_end = aNamesNull_.end();
}

FunctionParam_Iterator::~FunctionParam_Iterator()
{
}

FunctionParam_Iterator &
FunctionParam_Iterator::operator++()
{
    if ( IsValid() )
    {
        ++itTypes;
        ++itNames_andMore;
    }
    return *this;
}

void
FunctionParam_Iterator::Assign( const ary::cpp::Function &      i_rFunction,
                                const ary::cpp::DisplayGate &   i_rAryGate )
{
    const ary::cpp::OperationSignature *
        pSigna = i_rAryGate.Find_Signature( i_rFunction.Signature() );
    if (pSigna == 0 )
        return;

    const std::vector<ary::Tid> &
        rTypes = pSigna->Parameters();
    const StringVector &
        rNames = i_rFunction.ParamInfos();

    if ( rTypes.size() != rNames.size() OR rTypes.size() == 0 )
        return;

    itTypes     = rTypes.begin();
    itTypes_end = rTypes.end();
    itNames_andMore     = rNames.begin();
    itNames_andMore_end = rNames.end();

    eConVol = pSigna->ConVol();
}


