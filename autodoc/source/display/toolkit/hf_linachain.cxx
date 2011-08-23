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
#include <toolkit/hf_linachain.hxx>


// NOT FULLY DEFINED SERVICES
#include <toolkit/out_position.hxx>



HF_LinkedNameChain::HF_LinkedNameChain( Xml::Element & o_rOut )
    :   HtmlMaker( o_rOut
                        >> *new Html::Paragraph
                            << new Html::ClassAttr("namechain") )
{
}

HF_LinkedNameChain::~HF_LinkedNameChain()
{
}

void
HF_LinkedNameChain::Produce_CompleteChain( const output::Position & i_curPosition,
                                           F_LinkMaker              i_linkMaker ) const
{
    produce_Level(i_curPosition.RelatedNode(), i_curPosition, i_linkMaker);
}

void
HF_LinkedNameChain::Produce_CompleteChain_forModule( const output::Position &  i_curPosition,
                                                     F_LinkMaker               i_linkMaker ) const
{
    if (i_curPosition.Depth() == 0)
        return;
    produce_Level(*i_curPosition.RelatedNode().Parent(), i_curPosition, i_linkMaker);
}



namespace
{

StreamStr aLinkBuf(200);

}

void
HF_LinkedNameChain::produce_Level( output::Node &           i_levelNode,
                                   const output::Position & i_startPosition,
                                   F_LinkMaker              i_linkMaker ) const
{
    if ( i_levelNode.Depth() > 0 )
    {
        produce_Level( *i_levelNode.Parent(),
                       i_startPosition,
                       i_linkMaker );
    }

    aLinkBuf.reset();

    String
        sFileName = (*i_linkMaker)(i_levelNode.Name());
    output::Position
        aLevelPos(i_levelNode, sFileName);

    i_startPosition.Get_LinkTo(aLinkBuf, aLevelPos);

    if ( i_levelNode.Depth() > 0 )
    {
        CurOut()
        >> *new Html::Link(aLinkBuf.c_str())
            << new Html::ClassAttr("namechain")
            << i_levelNode.Name();
        CurOut() << " :: ";
    }
    else
    {
        CurOut()
        >> *new Html::Link(aLinkBuf.c_str())
            << new Html::ClassAttr("namechain")
            << "::";
        CurOut() << " ";
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
