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

#ifndef ADC_DISPLAY_HFI_HIERARCHY_HXX
#define ADC_DISPLAY_HFI_HIERARCHY_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_comrela.hxx>
#include <ary/idl/i_types4idl.hxx>


namespace csi
{
namespace xml
{
    class Element;
}
}


class HF_IdlInterface;
class HtmlEnvironment_Idl;



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

void                Write_BaseHierarchy(
                        csi::xml::Element & o_rOut,
                        HtmlEnvironment_Idl &
                                            i_env,
                        const ary::idl::CodeEntity &
                                            i_rCe );

void                Write_Bases(
                        csi::xml::Element & o_rOut,
                        HtmlEnvironment_Idl &
                                            i_env,
                        const ary::idl::CodeEntity &
                                            i_rCe,
                        std::vector<uintt> &
                                            io_setColumns );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
