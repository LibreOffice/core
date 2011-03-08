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

#ifndef ADC_PARSEENV_HXX
#define ADC_PARSEENV_HXX



// USED SERVICES
    // BASE CLASSES
#include <tokens/tokproct.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace info
{
class CodeInfo;
}   // namespace info
}   // namespace ary)



class SubPeUseIfc;


class ParseEnvironment : protected TokenProcessing_Types
{
  public:
    virtual             ~ParseEnvironment() {}

    // Parsing
    void                Enter(
                            E_EnvStackAction    i_eWayOfEntering );
    void                Leave(
                            E_EnvStackAction    i_eWayOfLeaving );
    void                SetCurSPU(
                            const SubPeUseIfc * i_pCurSPU );

    ParseEnvironment *  Parent() const;


    // ACCESS
  protected:
                        ParseEnvironment(
                            ParseEnvironment *  i_pParent );
    const SubPeUseIfc * CurSubPeUse() const;
  private:
    virtual void        InitData() = 0;
    virtual void        TransferData() = 0;

    ParseEnvironment *  pParent;
    const SubPeUseIfc * pCurSubPe;
};

class SubPeUseIfc
{
  public:
    virtual             ~SubPeUseIfc() {}

    virtual void        InitParse() const = 0;
    virtual void        GetResults() const = 0;
};



// IMPLEMENTATION

inline void
ParseEnvironment::SetCurSPU( const SubPeUseIfc * i_pCurSPU )
    { pCurSubPe = i_pCurSPU; }

inline ParseEnvironment *
ParseEnvironment::Parent() const
    { return pParent; }

inline const SubPeUseIfc *
ParseEnvironment::CurSubPeUse() const
    { return pCurSubPe; }



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
