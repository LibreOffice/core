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

#ifndef ARY_DOC_D_PARAMETER_HXX
#define ARY_DOC_D_PARAMETER_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/doc/d_node.hxx>

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
    explicit    		Parametrized(
                            nodetype::id        i_id,
                            T                   i_Parameter );
    virtual				~Parametrized();

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
