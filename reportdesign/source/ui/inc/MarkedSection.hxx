#ifndef RPTUI_MARKEDSECTION_HXX
#define RPTUI_MARKEDSECTION_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MarkedSection.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:30 $
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


#include <boost/shared_ptr.hpp>

namespace rptui
{
    class OReportSection; // forward declaration

    enum NearSectionAccess
    {
        CURRENT = 0,
        PREVIOUS = -1,
        POST = 1
    };

    class IMarkedSection
    {
    public:
        /** returns the section which is currently marked.
        */
        virtual ::boost::shared_ptr<OReportSection> getMarkedSection(NearSectionAccess nsa) const =0;

        /** mark the section on the given position .
        *
        * \param _nPos the position is zero based.
        */
        virtual void markSection(const sal_uInt16 _nPos) = 0;
    };

} // rptui

#endif /* RPTUI_MARKEDSECTION_HXX */
