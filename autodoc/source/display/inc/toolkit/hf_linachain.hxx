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
