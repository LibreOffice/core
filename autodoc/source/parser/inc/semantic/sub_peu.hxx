/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sub_peu.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:39:23 $
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

