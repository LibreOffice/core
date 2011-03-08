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

#ifndef ADC_CPP_SUB_PE_HXX
#define ADC_CPP_SUB_PE_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


class ParseEnvironment;

template <class PE, class SUB>
class SubPe
{
  public:
    typedef SubPe< PE, SUB >    self;

                        SubPe(
                            PE &                i_rParent );
    PE &                Parent() const;
    SUB &               Child() const;

    ParseEnvironment &  Get() const;

  private:
    SUB &               CreateChild() const;

    PE &                rParent;
    Dyn<SUB>            pChild;
};



// IMPLEMENTATION


//  SubPe

template <class PE, class SUB>
SubPe<PE,SUB>::SubPe( PE & i_rParent )
    :   rParent(i_rParent)
{
}

template <class PE, class SUB>
PE &
SubPe<PE,SUB>::Parent() const
{
     return rParent;
}

template <class PE, class SUB>
inline SUB &
SubPe<PE,SUB>::Child() const
{
    return pChild ? *pChild.MutablePtr() : CreateChild();
}

template <class PE, class SUB>
ParseEnvironment &
SubPe<PE,SUB>::Get() const
{
    return Child();
}

template <class PE, class SUB>
SUB &
SubPe<PE,SUB>::CreateChild() const
{
    self * pThis = const_cast< self* >(this);

    SUB * pNewChild = new SUB( &rParent);

    pThis->pChild = pNewChild;

    return *pChild.MutablePtr();
}




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
