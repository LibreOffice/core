/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dsp_txt_flist.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:26:24 $
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

#ifndef AUTODOC_DSP_TXT_FLIST_HXX
#define AUTODOC_DSP_TXT_FLIST_HXX

#include <iostream>


namespace ary
{
    namespace cpp
    {
        class DisplayGate;
    }
}


namespace autodoc
{

class TextDisplay_FunctionList_Ifc
{
  public:
    virtual             ~TextDisplay_FunctionList_Ifc() {}

    /** Displays the names of all C++ functions and methods within the
        given namespace (or the global namespace as default). All
        subnamespaces are included.
    */
    virtual void        Run(
                            ostream &           o_rStream,
                            const ary::cpp::DisplayGate &
                                                i_rAryGate ) = 0; /// If i_pNamespace == 0, the global namespace is displayed.
};


}   // namespace autodoc

#endif

