/*************************************************************************
 *
 *  $RCSfile: sub_peu.hxx,v $
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
                            SubPe<PE,SUB> &     i_rSubPeCreator,
                            F_INIT              i_fInit,
                            F_RETURN            i_fReturn );
                        ~SubPeUse();

    void                Push(
                            E_TokenDone         i_eDone );
    virtual void        InitParse() const;
    virtual void        GetResults() const;

    PE &                Parent() const;
    SUB &               Child() const;

  private:
    // DATA
    SubPe<PE,SUB> &     rSubPeCreator;
    F_INIT              fInit;
    F_RETURN            fReturn;
};


// IMPLEMENTATION


template <class PE, class SUB>
SubPeUse<PE,SUB>::SubPeUse( SubPe<PE,SUB> &     i_rSubPeCreator,
                        F_INIT              i_fInit,
                        F_RETURN            i_fReturn )
    :   rSubPeCreator(i_rSubPeCreator),
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

