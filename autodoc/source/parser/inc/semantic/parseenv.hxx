/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parseenv.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:38:52 $
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
    void                GlobalHandlingOfSyntaxError(
                            const char *        i_sTokenText,
                            const char *        i_sFileName,
                            uintt               i_nLineCount );

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

