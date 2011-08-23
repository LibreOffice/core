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

#ifndef ADC_CPP_SUB_PEU_HXX
#define ADC_CPP_SUB_PEU_HXX



// USED SERVICES
    // BASE CLASSES
#include <semantic/parseenv.hxx>
#include <tokens/tokproct.hxx>
    // COMPONENTS
    // PARAMETERS
#include <semantic/sub_pe.hxx>



template <class PE, class SUB>
class SubPeUse  : public SubPeUseIfc,
                  private TokenProcessing_Types
{
  public:
    typedef void (PE::*F_INIT)();
    typedef void (PE::*F_RETURN)();

                        SubPeUse(
                            SubPe<PE,SUB> &		i_rSubPeCreator,
                            F_INIT				i_fInit,
                            F_RETURN 			i_fReturn );
                        ~SubPeUse();

    void				Push(
                            E_TokenDone			i_eDone	);
    virtual void		InitParse() const;
    virtual void		GetResults() const;

    PE &                Parent() const;
    SUB &               Child() const;

  private:
    // DATA
    SubPe<PE,SUB> &		rSubPeCreator;
    F_INIT              fInit;
    F_RETURN 			fReturn;
};


// IMPLEMENTATION


template <class PE, class SUB>
SubPeUse<PE,SUB>::SubPeUse( SubPe<PE,SUB> &		i_rSubPeCreator,
                        F_INIT				i_fInit,
                        F_RETURN 			i_fReturn )
    :	rSubPeCreator(i_rSubPeCreator),
        fInit(i_fInit),
        fReturn(i_fReturn)
{
}

template <class PE, class SUB>
SubPeUse<PE,SUB>::~SubPeUse()
{
}

template <class PE, class SUB>
void
SubPeUse<PE,SUB>::Push( E_TokenDone i_eDone )
{
    Parent().SetTokenResult( i_eDone, push, &rSubPeCreator.Get() );
    Parent().SetCurSPU(this);
}

template <class PE, class SUB>
void
SubPeUse<PE,SUB>::InitParse() const
{
    if (fInit != 0)
        (Parent().*fInit)();
}

template <class PE, class SUB>
void
SubPeUse<PE,SUB>::GetResults() const
{
    if (fReturn != 0)
        (Parent().*fReturn)();
}

template <class PE, class SUB>
inline PE &
SubPeUse<PE,SUB>::Parent() const
{
     return rSubPeCreator.Parent();
}

template <class PE, class SUB>
inline SUB &
SubPeUse<PE,SUB>::Child() const
{
     return rSubPeCreator.Child();
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
