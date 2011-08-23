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

#ifndef ADC_DISPLAY_HTML_EASYWRI_HXX
#define ADC_DISPLAY_HTML_EASYWRI_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <estack.hxx>
    // PARAMETERS
#include <udm/html/htmlitem.hxx>


class EasyWriter
{
  public:
    // LIFECYCLE
                        EasyWriter();
                        ~EasyWriter();

    // OPERATIONS
    /// Pushes csi::xml::Element on stack.
    void                Open_OutputNode(
                            csi::xml::Element & io_rDestination );
    /// Pops front csi::xml::Element from stack.
    void                Finish_OutputNode();

    void                Enter(
                            csi::xml::Element & io_rDestination )
                                                { Open_OutputNode(io_rDestination); }
    void                Leave()                 { Finish_OutputNode(); }

    // ACCESS
    csi::xml::Element & Out();                  // CurOutputNode

  private:
    EStack< csi::xml::Element * >
                        aCurDestination;        // The front element is the currently used.
                                                //   The later ones are the parents.
};

/*
inline csi::xml::Element &
EasyWriter::Out()
    { csv_assert( aCurDestination.size() > 0 );
      return *aCurDestination.top(); }
*/

// IMPLEMENTATION


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
