/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hf_linachain.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:40:51 $
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

#ifndef ADC_DISPLAY_HF_LINACHAIN_HXX
#define ADC_DISPLAY_HF_LINACHAIN_HXX

// BASE CLASSES
#include "htmlfactory.hxx"
#include "out_position.hxx"




class HF_LinkedNameChain : public HtmlMaker
{
  public:
    /** F_LinkMaker makes a link out of the name of the
        parent position.

        Returns true, if there is a link, false if not.
    */
    typedef String (*F_LinkMaker)(const char *);


                        HF_LinkedNameChain(
                            Xml::Element &      o_rOut );
    virtual             ~HF_LinkedNameChain();

    void                Produce_CompleteChain(
                            const output::Position &
                                                i_curPosition,
                            F_LinkMaker         i_linkMaker ) const;
    void                Produce_CompleteChain_forModule(
                            const output::Position &
                                                i_curPosition,  /// current Module's node
                            F_LinkMaker         i_linkMaker ) const;
  private:
    void                produce_Level(
                            output::Node &      i_levelNode,
                            const output::Position &
                                                i_startPosition,
                            F_LinkMaker         i_linkMaker ) const;
};




#endif
