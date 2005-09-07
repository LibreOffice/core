/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outputstack.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:58:39 $
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

#ifndef ADC_DISPLAY_OUTPUTSTACK_HXX
#define ADC_DISPLAY_OUTPUTSTACK_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <estack.hxx>
    // PARAMETERS
#include <udm/xml/xmlitem.hxx>


class OutputStack
{
  public:
    // LIFECYCLE
                        OutputStack();
                        ~OutputStack();

    // OPERATIONS
    void                Enter(
                            csi::xml::Element & io_rDestination );
    void                Leave();

    // ACCESS
    csi::xml::Element & Out() const;                  // CurOutputNode

  private:
    EStack< csi::xml::Element * >
                        aCurDestination;        // The front element is the currently used.
                                                //   The later ones are the parents.
};

inline csi::xml::Element &
OutputStack::Out() const
{
    csv_assert( aCurDestination.size() > 0 );
    return *aCurDestination.top();
}

// IMPLEMENTATION


#endif


