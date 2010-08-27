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
#include "hfi_hierarchy.hxx"


// NOT FULLY DECLARED SERVICES
#include <udm/html/htmlitem.hxx>
#include <ary/stdconstiter.hxx>
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_type.hxx>
#include <ary/idl/ik_interface.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/idl/ip_type.hxx>
#include "hfi_interface.hxx"
#include "hfi_typetext.hxx"
#include "hi_env.hxx"



HF_IdlBaseNode::HF_IdlBaseNode( const TYPE &            i_rType,
                                const GATE &            i_rGate,
                                intt                    i_nPositionOffset,
                                HF_IdlBaseNode &        io_rDerived )
    :   nType(i_rType.TypeId()),
        aBases(),
        nCountBases(0),
        nPosition(i_nPositionOffset),
        pDerived(&io_rDerived)
{
    Ce_id nCe = i_rGate.Types().Search_CeRelatedTo(nType);
    if (nCe.IsValid())
    {
        GatherBases(i_rGate.Ces().Find_Ce(nCe), i_rGate);
    }
}

HF_IdlBaseNode::~HF_IdlBaseNode()
{
}

void
HF_IdlBaseNode::FillPositionList( std::vector< const HF_IdlBaseNode* > & o_rPositionList ) const
{
    for ( BaseList::const_iterator it = aBases.begin();
          it != aBases.end();
          ++it )
    {
        (*it)->FillPositionList(o_rPositionList);
    }  // end for

    o_rPositionList.push_back(this);
}

void
HF_IdlBaseNode::GatherBases( const CE &       i_rCe,
                             const GATE &     i_rGate )
{
    ary::Dyn_StdConstIterator<ary::idl::CommentedRelation>
        aHelp;
    ary::idl::ifc_interface::attr::Get_Bases(aHelp,i_rCe);

    for ( ary::StdConstIterator<ary::idl::CommentedRelation> & it = *aHelp;
          it.operator bool();
          ++it )
    {
        const TYPE &
            rBaseType = i_rGate.Types().Find_Type((*it).Type());

        Dyn<HF_IdlBaseNode>
            pBaseNode( new HF_IdlBaseNode( rBaseType,
                                           i_rGate,
                                           nPosition,
                                           *this )
                     );

        intt nAddedBases = pBaseNode->BaseCount() + 1;
        nCountBases += nAddedBases;
        nPosition += nAddedBases;
        aBases.push_back( pBaseNode.Release() );
    }   // end for
}


void
Write_BaseHierarchy( csi::xml::Element &            o_rOut,
                     HtmlEnvironment_Idl &          i_env,
                     const ary::idl::CodeEntity &   i_ce )
{
    csi::xml::Element &
        rPre = o_rOut
               >> *new csi::xml::AnElement("pre")
                   << new csi::html::StyleAttr("font-family:monospace;");

    std::vector<uintt>
        aSetColumns;
    rPre
        >> *new csi::html::Strong
            << i_ce.LocalName();
    rPre
        << "\n";
    Write_Bases( rPre,
                 i_env,
                 i_ce,
                         aSetColumns );
    rPre
        << "\n";

}


void
Write_Bases( csi::xml::Element &            o_out,
             HtmlEnvironment_Idl &          i_env,
             const ary::idl::CodeEntity &   i_rCe,
             std::vector<uintt> &           io_setColumns )
{
    ary::Dyn_StdConstIterator<ary::idl::CommentedRelation>
        aHelp;
    ary::idl::ifc_interface::attr::Get_Bases(aHelp,i_rCe);

    for ( ary::StdConstIterator<ary::idl::CommentedRelation> & it = *aHelp;
          it.operator bool();
          // NO INCREMENT HERE, see below
        )
    {
        ary::idl::Type_id
            nType = (*it).Type();
        ++it;
        bool
            bThereComesMore = it.operator bool();

        ary::idl::Ce_id
            nCe = i_env.Gate().Types().Search_CeRelatedTo(nType);
        if (nCe.IsValid())
        {
            // KORR_FUTURE
            //   Rather check for id(!) of com::sun::star::uno::XInterface.
            if (i_env.Gate().Ces().Find_Ce(nCe).LocalName() == "XInterface")
                continue;
        }

        for (uintt i = 0; i < io_setColumns.size(); ++i)
        {
            if (io_setColumns[i] == 1)
                o_out << new csi::xml::XmlCode("&#x2503");
            else
                o_out << "  ";
            o_out << " ";
        }

        if (bThereComesMore)
            o_out << new csi::xml::XmlCode("&#x2523");
        else
            o_out << new csi::xml::XmlCode("&#x2517");
        o_out << " ";

        HF_IdlTypeText
            aDisplay( i_env, o_out, true, i_env.CurPageCe());
        aDisplay.Produce_byData(nType);
        o_out << "\n";

        if (nCe.IsValid())
        {
            io_setColumns.push_back(bThereComesMore ? 1 : 0);

            const ary::idl::CodeEntity &
                rCe = i_env.Gate().Ces().Find_Ce(nCe);
            Write_Bases( o_out,
                         i_env,
                         rCe,
                         io_setColumns );
            io_setColumns.pop_back();
        }
    }   // end for
}
