/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sub_pe.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:39:08 $
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

