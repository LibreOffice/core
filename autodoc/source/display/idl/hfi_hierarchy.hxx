/*************************************************************************
 *
 *  $RCSfile: hfi_hierarchy.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:25:53 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef ADC_DISPLAY_HFI_HIERARCHY_HXX
#define ADC_DISPLAY_HFI_HIERARCHY_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_comrela.hxx>
#include <ary/idl/i_language.hxx>


namespace csi
{
namespace xml
{
    class Element;
}
}


class HF_IdlInterface;


/** Represents a node in an pyramidic inheritance hierarchy which shall be
    displayed in text mode.
*/
class HF_IdlBaseNode
{
  public:
    typedef ary::idl::CodeEntity            CE;
    typedef ary::idl::Type                  TYPE;
    typedef ary::idl::Gate                  GATE;
    typedef ary::idl::Ce_id                 Ce_id;
    typedef ary::idl::Type_id               Type_id;

    /** Constructor for the most derived class, whose base hierarchy
        is to be built.

        The constructor recursively calls further constructors of
        HF_IdlBaseNode for the bases of ->i_rCe.
        So it builds up a complete hierarchy tree of all base classes
        of ->i_rCe.
    */
                        HF_IdlBaseNode(
                            const CE &          i_rCe,
                            const GATE &        i_rGate );

    /** @descr
        The constructor recursively calls further constructors of
        HF_IdlBaseNode for the bases of ->i_rType, if ->i_rType matches to a
        ->CE.
        So it builds up a complete hierarchy tree of all base classes
        of ->i_pEntity.
    */
                        HF_IdlBaseNode(
                            const TYPE &        i_rType,
                            const GATE &        i_rGate,
                            intt                i_nPositionOffset,
                            HF_IdlBaseNode &    io_rDerived );
                        ~HF_IdlBaseNode();

    /** Recursively fills ->o_rPositionList with the instances of base
        classes in the order in which they will be displayed.
    */
    void                FillPositionList(
                            std::vector< const HF_IdlBaseNode* > &
                                                o_rPositionList ) const;

    /** Writes the base hierarchy of this node into ->o_rOut.

        It ends at the position where the name of the most derived
        class would have to be written. Example:

        abc::MyClass
         |
         | AnotherClass
         |  |
         +--+--XYZ

         The "XYZ" would NOT be written, that is the task of the caller.

    */
    void                WriteBaseHierarchy(
                            csi::xml::Element & o_rOut,
                            const HF_IdlInterface &
                                                io_rDisplayer,
                            const String &      i_sMainNodesText );

    Type_id             Type() const            { return nType; }
    intt                BaseCount() const       { return nCountBases; }
    intt                Position() const        { return nPosition; }
    int                 Xpos() const            { return 3*Position(); }
    int                 Ypos() const            { return 2*Position(); }
    const HF_IdlBaseNode *        Derived() const         { return pDerived; }

  private:
    typedef std::vector< DYN HF_IdlBaseNode* > BaseList;

    void                GatherBases(
                            const CE &          i_rCe,
                            const GATE &        i_rGate );

    // DATA
    Type_id             nType;
    BaseList            aBases;
    intt                nCountBases;
    intt                nPosition;
    HF_IdlBaseNode *    pDerived;
};


#endif
