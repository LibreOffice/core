/*************************************************************************
 *
 *  $RCSfile: sub_pe.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:33 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

