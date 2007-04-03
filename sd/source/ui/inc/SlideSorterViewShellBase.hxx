/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideSorterViewShellBase.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:07:15 $
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

#ifndef SD_SLIDE_SORTER_VIEW_SHELL_BASE_HXX
#define SD_SLIDE_SORTER_VIEW_SHELL_BASE_HXX

#include "ImpressViewShellBase.hxx"


namespace sd {

/** This class exists to be able to register a factory that creates a
    slide sorter view shell as default.
*/
class SlideSorterViewShellBase
    : public ImpressViewShellBase
{
public:
    TYPEINFO();
    SFX_DECL_VIEWFACTORY(SlideSorterViewShellBase);

    /** This constructor is used by the view factory of the SFX
        macros.
    */
    SlideSorterViewShellBase (SfxViewFrame *pFrame, SfxViewShell* pOldShell);
    virtual ~SlideSorterViewShellBase (void);
};

} // end of namespace sd

#endif
