/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hf_linachain.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:41:40 $
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
