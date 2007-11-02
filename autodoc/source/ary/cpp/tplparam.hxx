/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tplparam.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:36:09 $
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

#ifndef ARY_CPP_TPLPARAM_HXX
#define ARY_CPP_TPLPARAM_HXX

// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>




namespace ary
{
namespace cpp
{
    class UsedType;
    class Gate;

namespace ut
{


class TemplateParameter
{
  public:
    virtual             ~TemplateParameter() {}

    virtual intt        Compare(
                            const TemplateParameter &
                                                i_rOther ) const = 0;
    virtual void        Get_Text(
                            StreamStr &         o_rOut,
                            const ary::cpp::Gate &
                                                i_rGate ) const = 0;
};


class TplParameter_Type : public TemplateParameter
{
  public:
                        TplParameter_Type(
                            Tid                 i_nType );
                        ~TplParameter_Type();

    virtual intt        Compare(
                            const TemplateParameter &
                                                i_rOther ) const;
    virtual void        Get_Text(
                            StreamStr &         o_rOut,
                            const ary::cpp::Gate &
                                                i_rGate ) const;
  private:
    Tid                 nType;
};

class TplParameter_Const : public TemplateParameter
{
  public:
                        TplParameter_Const(
                            const String  &     i_sConst );
                        ~TplParameter_Const();

    virtual intt        Compare(
                            const TemplateParameter &
                                                i_rOther ) const;
    virtual void        Get_Text(
                            StreamStr &         o_rOut,
                            const ary::cpp::Gate &
                                                i_rGate ) const;
  private:
    String              sConstant;
};




}   // namespace ut
}   // namespace cpp
}   // namespace ary
#endif
