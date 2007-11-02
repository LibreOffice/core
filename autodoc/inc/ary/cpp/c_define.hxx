/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_define.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:47:11 $
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

#ifndef ARY_CPP_C_DEFINE_HXX
#define ARY_CPP_C_DEFINE_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_de.hxx>
    // OTHER




namespace ary
{
namespace cpp
{


/** A C/C++ #define ("#define DEF") statement, but no macro.

    @see Macro
*/
class Define : public DefineEntity
{
  public:
    enum E_ClassId { class_id = 1601 };

                        Define(      /// Used for: #define DEFINE xyz
                            const String  &     i_name,
                            const StringVector &
                                                i_definition,
                            loc::Le_id          i_declaringFile );
    virtual             ~Define();

    // INQUIRY
    void                GetText(
                            csv::StreamStr &    o_rText ) const;
  private:
    // Interface csv::ConstProcessorClient
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;

    // Interface ary::Object:
    virtual ClassId     get_AryClass() const;

    // Interface DefineEntity:
    virtual const StringVector &
                        inq_DefinitionText() const;
    // DATA
    StringVector        aDefinition;
};





}   // namespace cpp
}   // namespace ary
#endif
