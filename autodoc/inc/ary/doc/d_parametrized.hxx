/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_parametrized.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:00:16 $
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

#ifndef ARY_DOC_D_PARAMETER_HXX
#define ARY_DOC_D_PARAMETER_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/doc/d_node.hxx>
    // OTHER
#include <ary/doc/d_hypertext.hxx>




namespace ary
{
namespace doc
{


/** Documentation unit with Parameter.
*/
template <class T>
class Parametrized : public Node
{
  public:
    // LIFECYCLE
    explicit            Parametrized(
                            nodetype::id        i_id,
                            T                   i_Parameter );
    virtual             ~Parametrized();

    // INQUIRY
    const HyperText &   Doc() const;
    const T &           Parameter() const;

    // ACESS
    HyperText &         Doc();
    void                Set_Parameter(
                            const T &           i_param );
  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // DATA
    HyperText           aDoc;
    T                   aParameter;
};




// IMPLEMENTATION
template <class T>
Parametrized<T>::Parametrized( nodetype::id     i_id,
                               T                i_Parameter )
    :   Node(i_id),
        aDoc(),
        aParameter(i_Parameter)
{
}

template <class T>
Parametrized<T>::~Parametrized()
{
}

template <class T>
const HyperText &
Parametrized<T>::Doc() const
{
    return aDoc;
}

template <class T>
const T &
Parametrized<T>::Parameter() const
{
    return aParameter;
}

template <class T>
HyperText &
Parametrized<T>::Doc()
{
    return aDoc;
}

template <class T>
inline void
Parametrized<T>::Set_Parameter(const T & i_param)
{
    aParameter = i_param;
}




}   // namespace doc
}   // namespace ary
#endif
